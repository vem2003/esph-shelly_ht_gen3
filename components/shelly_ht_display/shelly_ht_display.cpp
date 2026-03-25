#include "shelly_ht_display.h"
#include "esphome/core/log.h"
#include "esphome/components/wifi/wifi_component.h"
#include <cmath>

// RTC memory for time tracking across deep sleep cycles
#ifdef USE_ESP32
static RTC_DATA_ATTR uint32_t rtc_time_magic;
static RTC_DATA_ATTR int16_t  rtc_saved_hour;
static RTC_DATA_ATTR int16_t  rtc_saved_min;
static RTC_DATA_ATTR uint32_t rtc_wake_count;
static RTC_DATA_ATTR int16_t  rtc_saved_temp;   // temp * 10 (e.g. 235 = 23.5C)
static RTC_DATA_ATTR int16_t  rtc_saved_humi;   // humidity (e.g. 67)
#else
static uint32_t rtc_time_magic;
static int16_t  rtc_saved_hour;
static int16_t  rtc_saved_min;
static uint32_t rtc_wake_count;
static int16_t  rtc_saved_temp;
static int16_t  rtc_saved_humi;
#endif
static const uint32_t RTC_TIME_MAGIC = 0x54494D45;  // "TIME"

namespace esphome {
namespace shelly_htg3 {

static const char *const TAG = "shelly_ht";

// ── Battery read cycle ─────────────────────────────────────────

void ShellyHTDisplay::read_battery_() {
  // Check presence via external binary_sensor
  bool present = false;
  if (this->batt_presence_ && this->batt_presence_->has_state()) {
    present = this->batt_presence_->state;
  }
  if (this->ext_power_sensor_) this->ext_power_sensor_->publish_state(!present);

  if (!present || !this->batt_adc_ || !this->batt_power_en_) {
    if (!present) ESP_LOGD(TAG, "Battery: USB mode, skipping ADC");
    if (this->batt_voltage_sensor_) this->batt_voltage_sensor_->publish_state(NAN);
    if (this->batt_percent_sensor_) this->batt_percent_sensor_->publish_state(NAN);
    return;
  }

  // 1) Power enable ON via external output
  this->batt_power_en_->turn_on();
  delay(10);  // settle time

  // 2) Sample voltage via VoltageSampler
  float v_adc = this->batt_adc_->sample();

  // 3) Power enable OFF
  this->batt_power_en_->turn_off();

  if (std::isnan(v_adc)) {
    ESP_LOGW(TAG, "Battery: ADC read failed");
    return;
  }

  // 4) Apply divider and calculate percentage
  float v_bat = v_adc * this->batt_divider_;
  float pct = (v_bat - this->batt_v_empty_) / (this->batt_v_full_ - this->batt_v_empty_) * 100.0f;
  if (pct > 100.0f) pct = 100.0f;
  if (pct < 0.0f) pct = 0.0f;

  ESP_LOGD(TAG, "Battery: %.2fV %.0f%% (adc=%.3fV)", v_bat, pct, v_adc);

  if (this->batt_voltage_sensor_) this->batt_voltage_sensor_->publish_state(v_bat);
  if (this->batt_percent_sensor_) this->batt_percent_sensor_->publish_state(pct);
}

// ── Battery state queries ──────────────────────────────────────

bool ShellyHTDisplay::is_battery_present() const {
  if (this->batt_presence_ && this->batt_presence_->has_state())
    return this->batt_presence_->state;
  return false;
}

bool ShellyHTDisplay::is_usb_powered() const { return !this->is_battery_present(); }

int ShellyHTDisplay::get_battery_segments() const {
  if (!this->batt_percent_sensor_ || !this->batt_percent_sensor_->has_state())
    return 0;
  float p = this->batt_percent_sensor_->state;
  if (std::isnan(p)) return 0;
  if (p >= 80) return 5;
  if (p >= 60) return 4;
  if (p >= 40) return 3;
  if (p >= 20) return 2;
  if (p > 0)   return 1;
  return 0;
}

// ── RTC time tracking ──────────────────────────────────────────

void ShellyHTDisplay::save_time_to_rtc_() {
  if (this->disp_hour_ < 0) return;
  rtc_saved_hour = this->disp_hour_;
  rtc_saved_min = this->disp_min_;
  rtc_saved_temp = this->disp_temp_;
  rtc_saved_humi = this->disp_humi_;
  rtc_time_magic = RTC_TIME_MAGIC;
  ESP_LOGD(TAG, "RTC save %02d:%02d %.1fC %d%%",
           rtc_saved_hour, rtc_saved_min, rtc_saved_temp / 10.0f, rtc_saved_humi);
}

bool ShellyHTDisplay::load_time_from_rtc_() {
  if (rtc_time_magic != RTC_TIME_MAGIC) return false;
  ESP_LOGD(TAG, "RTC load %02d:%02d cycle %u", rtc_saved_hour, rtc_saved_min, rtc_wake_count);
  return true;
}

// ── 7-segment helpers ──────────────────────────────────────────

void ShellyHTDisplay::write_digit_(const DigitMap &d, uint8_t c) {
  this->display_->set_segment(d.a, (c>>6)&1);
  this->display_->set_segment(d.b, (c>>5)&1);
  this->display_->set_segment(d.c, (c>>4)&1);
  this->display_->set_segment(d.d, (c>>3)&1);
  this->display_->set_segment(d.e, (c>>2)&1);
  this->display_->set_segment(d.f, (c>>1)&1);
  this->display_->set_segment(d.g, (c>>0)&1);
}

void ShellyHTDisplay::write_number_(const DigitMap &d, int8_t n) {
  this->write_digit_(d, (n >= 0 && n <= 9) ? S7_NUM[n] : S7_BLANK);
}

uint8_t ShellyHTDisplay::char_to_seg7_(char c) {
  if (this->font_ == FONT_SIEKOO)
    return siekoo_encode(c);

  if (c >= '0' && c <= '9') return S7_NUM[c - '0'];
  switch (c) {
    case 'A': case 'a': return 0x77; case 'B': case 'b': return 0x1F;
    case 'C': return 0x4E; case 'c': return 0x0D;
    case 'D': case 'd': return 0x3D; case 'E': case 'e': return 0x4F;
    case 'F': case 'f': return 0x47; case 'G': case 'g': return 0x5E;
    case 'H': return 0x37; case 'h': return 0x17;
    case 'I': case 'i': return 0x06; case 'J': case 'j': return 0x38;
    case 'L': case 'l': return 0x0E; case 'N': case 'n': return 0x15;
    case 'O': return S7_0; case 'o': return 0x1D;
    case 'P': case 'p': return 0x67; case 'Q': case 'q': return 0x73;
    case 'R': case 'r': return 0x05; case 'S': case 's': return S7_5;
    case 'T': case 't': return 0x0F; case 'U': return 0x3E;
    case 'u': return 0x1C; case 'Y': case 'y': return 0x3B;
    case '-': return S7_MINUS; case '_': return 0x08;
    default: return S7_BLANK;
  }
}

// ── Icon state helper ──────────────────────────────────────────

bool ShellyHTDisplay::get_icon_state_(binary_sensor::BinarySensor *ext, bool default_val) {
  if (ext != nullptr && ext->has_state())
    return ext->state;
  return default_val;
}

// ── High-level display API ─────────────────────────────────────

void ShellyHTDisplay::show_temperature(float t, bool f) {
  float v = f ? t * 9.0f / 5.0f + 32.0f : t;
  bool neg = v < 0; float a = std::abs(v); if (a > 99.9f) a = 99.9f;
  int val = (int)roundf(a * 10.0f);
  int tens = val / 100, ones = (val / 10) % 10, dec = val % 10;

  if (neg) this->write_digit_(DIG_D1, S7_MINUS);
  else if (tens > 0) this->write_number_(DIG_D1, tens);
  else this->write_digit_(DIG_D1, S7_BLANK);
  this->write_number_(DIG_D2, ones);
  this->write_number_(DIG_D3, dec);
  this->show_dp(true);
  this->show_degree(true);
  char unit_c = (this->font_ == FONT_SIEKOO) ? '(' : 'C';
  this->show_unit(f ? 'F' : unit_c);
}

void ShellyHTDisplay::show_humidity(int h) {
  if (h < 0) h = 0; if (h > 99) h = 99;
  this->write_digit_(DIG_H1, h / 10 > 0 ? S7_NUM[h / 10] : S7_BLANK);
  this->write_number_(DIG_H2, h % 10);
  this->show_percent(true);
}

void ShellyHTDisplay::show_time(int h, int m) {
  this->write_number_(DIG_T1, h / 10);
  this->write_number_(DIG_T2, h % 10);
  this->write_number_(DIG_T3, m / 10);
  this->write_number_(DIG_T4, m % 10);
  this->show_colon(true);
}

void ShellyHTDisplay::show_text_big(const char *t) {
  const DigitMap *d[] = {&DIG_D1, &DIG_D2, &DIG_D3};
  for (int i = 0; i < 3; i++)
    this->write_digit_(*d[i], this->char_to_seg7_((t && t[i]) ? t[i] : ' '));
}

void ShellyHTDisplay::show_text_clock(const char *t) {
  const DigitMap *d[] = {&DIG_T1, &DIG_T2, &DIG_T3, &DIG_T4};
  for (int i = 0; i < 4; i++)
    this->write_digit_(*d[i], this->char_to_seg7_((t && t[i]) ? t[i] : ' '));
}

// ── Icons ──────────────────────────────────────────────────────

void ShellyHTDisplay::show_battery(uint8_t l) {
  if (l > 5) l = 5;
  this->display_->set_segment(SEG_BATT[4], true);  // frame always on
  for (int i = 0; i < 4; i++) this->display_->set_segment(SEG_BATT[i], l >= (i + 1));
}

void ShellyHTDisplay::show_signal(uint8_t b) {
  if (b > 4) b = 4;
  for (int i = 0; i < 4; i++) this->display_->set_segment(SEG_SIG[i], b >= (i + 1));
}

void ShellyHTDisplay::show_bluetooth(bool on) { this->display_->set_segment(SEG_BT, on); }
void ShellyHTDisplay::show_globe(bool on)     { this->display_->set_segment(SEG_GLOBE, on); }
void ShellyHTDisplay::show_frost(bool on)     { this->display_->set_segment(SEG_FROST, on); }
void ShellyHTDisplay::show_heating(bool on)   { this->display_->set_segment(SEG_HEATING, on); }
void ShellyHTDisplay::show_ventilator(bool on){ this->display_->set_segment(SEG_VENT, on); }
void ShellyHTDisplay::show_calendar(bool on)  { this->display_->set_segment(SEG_CALENDAR, on); }
void ShellyHTDisplay::show_arrow(bool on)     { this->display_->set_segment(SEG_ARROW, on); }
void ShellyHTDisplay::show_dp(bool on)        { this->display_->set_segment(SEG_DP, on); }
void ShellyHTDisplay::show_degree(bool on)    { this->display_->set_segment(SEG_DEGREE, on); }
void ShellyHTDisplay::show_percent(bool on)   { this->display_->set_segment(SEG_PERCENT, on); }
void ShellyHTDisplay::show_unit(char u)       { this->write_digit_(DIG_UNIT, this->char_to_seg7_(u)); }

void ShellyHTDisplay::show_colon(bool on) {
  this->display_->set_segment(SEG_COL_TOP, on);
}

// ── OTA progress display ───────────────────────────────────────

void ShellyHTDisplay::show_ota_begin() {
  this->ota_active_ = true;
  this->display_->clear();
  this->show_text_clock(this->font_ == FONT_SIEKOO ? " E5P" : " ESP");
  this->show_text_big("otA");
  this->write_digit_(DIG_H1, S7_BLANK);
  this->write_digit_(DIG_H2, SK_DOT);
  this->display_->commit();
  ESP_LOGI(TAG, "OTA begin");
}

void ShellyHTDisplay::show_ota_progress(float progress) {
  if (!this->ota_active_) return;
  uint8_t h1 = S7_BLANK, h2 = S7_BLANK;
  if (progress >= 100.0f)     { h2 = SK_EXCLAIM; }
  else if (progress >= 66.0f) { h2 = SK_W; }
  else if (progress >= 33.0f) { h2 = SK_M; }
  else                        { h2 = SK_N; }
  this->write_digit_(DIG_H1, h1);
  this->write_digit_(DIG_H2, h2);
  this->display_->commit();
}

void ShellyHTDisplay::show_ota_end() {
  this->display_->clear();
  this->show_text_clock("donE");
  this->write_digit_(DIG_H1, S7_BLANK);
  this->write_digit_(DIG_H2, SK_EXCLAIM);
  this->display_->commit();
  ESP_LOGI(TAG, "OTA complete");
}

void ShellyHTDisplay::show_ota_error() {
  this->display_->clear();
  this->show_text_clock("Err ");
  this->show_text_big("otA");
  this->display_->commit();
  ESP_LOGW(TAG, "OTA error");
  this->ota_active_ = false;
  this->force_refresh();
}

// ── Internal state machine ─────────────────────────────────────

void ShellyHTDisplay::check_and_update_() {
  if (this->ota_active_) return;
  if (!this->display_ || !this->display_->is_ready()) return;

  bool has_t = this->temp_sensor_ && this->temp_sensor_->has_state();
  bool has_h = this->humi_sensor_ && this->humi_sensor_->has_state();

  int new_temp, new_humi;
  float raw_temp;

  if (has_t && has_h) {
    new_temp = (int)roundf(this->temp_sensor_->state * 10.0f);
    new_humi = (int)this->humi_sensor_->state;
    raw_temp = this->temp_sensor_->state;
  } else if (this->wifi_skipped_ && rtc_time_magic == RTC_TIME_MAGIC) {
    new_temp = rtc_saved_temp;
    new_humi = rtc_saved_humi;
    raw_temp = rtc_saved_temp / 10.0f;
    ESP_LOGD(TAG, "Sensor not ready, using RTC: %.1fC %d%%", raw_temp, new_humi);
  } else {
    return;
  }

  // Time: use RTC time if WiFi was skipped, otherwise HA time
  int new_hour = -1, new_min = -1;
  if (this->wifi_skipped_) {
    new_hour = this->rtc_hour_;
    new_min = this->rtc_min_;
  } else if (this->time_) {
    auto now = this->time_->now();
    if (now.is_valid()) {
      new_hour = now.hour;
      new_min = now.minute;
    }
  }

  // Signal bars: 0 if WiFi skipped, otherwise from RSSI
  int new_bars = 0;
  bool new_wifi = false;
  if (!this->wifi_skipped_) {
    if (this->wifi_sensor_ && this->wifi_sensor_->has_state()) {
      float rssi = this->wifi_sensor_->state;
      if (rssi > -50) new_bars = 4; else if (rssi > -65) new_bars = 3;
      else if (rssi > -75) new_bars = 2; else if (rssi > -85) new_bars = 1;
    }
    new_wifi = wifi::global_wifi_component->is_connected();
  }

  // Icon states
  bool def_frost = raw_temp < 3.0f;

  bool new_frost    = this->get_icon_state_(this->frost_sensor_, def_frost);
  bool new_heating  = this->get_icon_state_(this->heating_sensor_, false);
  bool new_vent     = this->get_icon_state_(this->ventilator_sensor_, false);
  bool new_bt       = this->get_icon_state_(this->bluetooth_sensor_, false);
  bool new_globe    = this->get_icon_state_(this->globe_sensor_, new_wifi);
  bool new_calendar = this->get_icon_state_(this->calendar_sensor_, false);
  bool new_arrow    = this->get_icon_state_(this->arrow_sensor_, false);

  // Changed?
  bool changed = (new_temp     != this->disp_temp_)     ||
                 (new_humi     != this->disp_humi_)     ||
                 (new_hour     != this->disp_hour_)     ||
                 (new_min      != this->disp_min_)      ||
                 (new_bars     != this->disp_bars_)     ||
                 (new_wifi     != this->disp_wifi_)     ||
                 (new_frost    != this->disp_frost_)    ||
                 (new_heating  != this->disp_heating_)  ||
                 (new_vent     != this->disp_vent_)     ||
                 (new_bt       != this->disp_bt_)       ||
                 (new_calendar != this->disp_calendar_) ||
                 (new_arrow    != this->disp_arrow_);

  if (!changed) return;

  ESP_LOGD(TAG, "Update: %.1fC %d%% %02d:%02d sig:%d wifi:%d frost:%d%s",
           new_temp / 10.0f, new_humi, new_hour, new_min, new_bars,
           new_wifi, new_frost, this->wifi_skipped_ ? " [no-wifi]" : "");

  this->disp_temp_ = new_temp;       this->disp_humi_ = new_humi;
  this->disp_hour_ = new_hour;       this->disp_min_  = new_min;
  this->disp_bars_ = new_bars;       this->disp_wifi_ = new_wifi;
  this->disp_frost_ = new_frost;     this->disp_heating_ = new_heating;
  this->disp_vent_ = new_vent;       this->disp_bt_ = new_bt;
  this->disp_calendar_ = new_calendar; this->disp_arrow_ = new_arrow;

  // Save HA-synced time to RTC (only on WiFi cycles when time changed)
  if (!this->wifi_skipped_ && new_hour >= 0) {
    this->save_time_to_rtc_();
  }

  // Build framebuffer
  this->display_->clear();
  this->show_temperature(raw_temp, false);
  this->show_humidity(new_humi);
  if (new_hour >= 0) this->show_time(new_hour, new_min);
  this->show_signal(new_bars);

  // Icons
  this->show_globe(new_globe);
  this->show_frost(new_frost);
  this->show_heating(new_heating);
  this->show_ventilator(new_vent);
  this->show_bluetooth(new_bt);
  this->show_calendar(new_calendar);
  this->show_arrow(new_arrow);

  // Battery: auto-show from percent sensor if available
  if (this->batt_percent_sensor_ && this->batt_percent_sensor_->has_state() &&
      !std::isnan(this->batt_percent_sensor_->state)) {
    this->show_battery(this->get_battery_segments());
  }

  // Fire on_update trigger (icon overrides, etc.)
  this->on_update_trigger_.trigger();

  // Commit
  this->display_->commit();

  // Non-WiFi cycle: display is done, fire on_ready for power_off + sleep
  if (this->wifi_skipped_) {
    this->save_time_to_rtc_();
    this->on_ready_trigger_.trigger();
  }
}

// ── Lifecycle ──────────────────────────────────────────────────

void ShellyHTDisplay::setup() {
  // Deep sleep WiFi optimization
  if (this->deep_sleep_mode_ && this->wifi_update_every_ > 0) {
    if (this->load_time_from_rtc_()) {
      rtc_wake_count++;

      bool wifi_cycle = (rtc_wake_count % this->wifi_update_every_) == 0;

      if (!wifi_cycle) {
        wifi::global_wifi_component->disable();
        this->wifi_skipped_ = true;

        int total_min = rtc_saved_hour * 60 + rtc_saved_min +
                        (int)(this->sleep_duration_ms_ / 60000);
        this->rtc_hour_ = (total_min / 60) % 24;
        this->rtc_min_ = total_min % 60;

        ESP_LOGI(TAG, "No-WiFi wake %u/%u, time %02d:%02d",
                 rtc_wake_count, this->wifi_update_every_,
                 this->rtc_hour_, this->rtc_min_);
      } else {
        ESP_LOGI(TAG, "WiFi wake %u/%u",
                 rtc_wake_count, this->wifi_update_every_);
      }
    } else {
      rtc_wake_count = 0;
      ESP_LOGI(TAG, "First boot, WiFi sync needed");
    }
  }

  ESP_LOGI(TAG, "Ready (%s, battery=%s)",
           this->deep_sleep_mode_ ? "deep-sleep" : "always-on",
           this->batt_adc_ ? "yes" : "no");
}

void ShellyHTDisplay::update() {
  this->read_battery_();
  this->check_and_update_();
}

void ShellyHTDisplay::dump_config() {
  ESP_LOGCONFIG(TAG,
                "Shelly H&T Gen3:\n"
                "  Mode: %s\n"
                "  Font: %s\n"
                "  Update interval: %ums\n"
                "  Sensors: temp=%s humi=%s wifi=%s time=%s",
                this->deep_sleep_mode_ ? "deep-sleep" : "always-on",
                this->font_ == FONT_SIEKOO ? "siekoo" : "classic",
                this->get_update_interval(),
                this->temp_sensor_ ? "yes" : "no",
                this->humi_sensor_ ? "yes" : "no",
                this->wifi_sensor_ ? "yes" : "no",
                this->time_ ? "yes" : "no");
  if (this->deep_sleep_mode_) {
    ESP_LOGCONFIG(TAG,
                  "  WiFi every: %u cycles\n"
                  "  Sleep duration: %us",
                  this->wifi_update_every_,
                  this->sleep_duration_ms_ / 1000);
  }
  ESP_LOGCONFIG(TAG,
                "  Battery: adc=%s presence=%s power_en=%s\n"
                "           divider=%.3f range=%.1f-%.1fV",
                this->batt_adc_ ? "yes" : "no",
                this->batt_presence_ ? "yes" : "no",
                this->batt_power_en_ ? "yes" : "no",
                this->batt_divider_, this->batt_v_empty_, this->batt_v_full_);
  ESP_LOGCONFIG(TAG,
                "  Battery outputs: voltage=%s percent=%s ext_power=%s",
                this->batt_voltage_sensor_ ? "yes" : "no",
                this->batt_percent_sensor_ ? "yes" : "no",
                this->ext_power_sensor_ ? "yes" : "no");
  ESP_LOGCONFIG(TAG,
                "  Icons: frost=%s heat=%s vent=%s bt=%s globe=%s cal=%s arrow=%s",
                this->frost_sensor_ ? "ext" : "def",
                this->heating_sensor_ ? "ext" : "def",
                this->ventilator_sensor_ ? "ext" : "def",
                this->bluetooth_sensor_ ? "ext" : "def",
                this->globe_sensor_ ? "ext" : "def",
                this->calendar_sensor_ ? "ext" : "def",
                this->arrow_sensor_ ? "ext" : "def");
}

}  // namespace shelly_htg3
}  // namespace esphome
