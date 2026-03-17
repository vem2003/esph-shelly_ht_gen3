#include "shelly_ht_display.h"
#include "esphome/core/log.h"
#include "esphome/components/wifi/wifi_component.h"
#ifdef USE_DEEP_SLEEP
#include "esphome/components/deep_sleep/deep_sleep_component.h"
#endif
#include <cmath>

namespace esphome {
namespace uc8119 {

static const char *const TAG = "shelly_ht";

void ShellyHTDisplay::setup() {
  ESP_LOGI(TAG, "Shelly H&T display layer ready");
  this->last_check_ms_ = millis();
}

void ShellyHTDisplay::loop() {
  if (!this->display_ || !this->display_->is_ready()) return;
  uint32_t now = millis();

  if (now - this->last_check_ms_ < this->check_interval_ms_) return;

  this->last_check_ms_ = now;
  this->check_and_update_();

}

void ShellyHTDisplay::dump_config() {
  ESP_LOGCONFIG(TAG, "Shelly H&T Gen3 Display:");  
  ESP_LOGCONFIG(TAG, "  Font: %s", this->font_ == FONT_SIEKOO ? "siekoo" : "classic");
  ESP_LOGCONFIG(TAG, "  Check interval: %ums", this->check_interval_ms_);
  ESP_LOGCONFIG(TAG, "  Sensors: temp=%s humi=%s batt=%s wifi=%s time=%s",
    this->temp_sensor_ ? "yes" : "no", this->humi_sensor_ ? "yes" : "no",
    this->batt_sensor_ ? "yes" : "no", this->wifi_sensor_ ? "yes" : "no",
    this->time_ ? "yes" : "no");
}

// ── 7-segment helpers ───────────────────────────────────────────

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

  // Classic 7-segment fallback
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

// ── High-level display API ──────────────────────────────────────

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
  this->show_dp(true);     // Decimal point between D2 and D3
  this->show_degree(true);
  // Siekoo's 'C' = DEG (looks like small c), '(' = ADEF (looks like proper C)
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

void ShellyHTDisplay::show_battery(uint8_t l) {
  // Hide battery icon only when USB-powered (no battery present)
  // Show in all other cases: deep-sleep with battery, always-on with battery
  if (this->usb_powered_) return;
  if (l > 5) l = 5;
  this->display_->set_segment(SEG_BATT[4], true);  // Frame always on
  for (int i = 0; i < 4; i++) this->display_->set_segment(SEG_BATT[i], l >= (i + 1));
}

void ShellyHTDisplay::show_signal(uint8_t b) {
  if (b > 4) b = 4;
  for (int i = 0; i < 4; i++) this->display_->set_segment(SEG_SIG[i], b >= (i + 1));
}

void ShellyHTDisplay::show_bluetooth(bool on) { this->display_->set_segment(SEG_BT, on); }
void ShellyHTDisplay::show_globe(bool on)     { this->display_->set_segment(SEG_GLOBE, on); }
void ShellyHTDisplay::show_frost(bool on)     { this->display_->set_segment(SEG_FROST, on); }
void ShellyHTDisplay::show_heating(bool on)   { this->display_->set_segment(SEG_HEIZ, on); }
void ShellyHTDisplay::show_ventilator(bool on){ this->display_->set_segment(SEG_VENT, on); }
void ShellyHTDisplay::show_calendar(bool on)  { this->display_->set_segment(SEG_KALEN, on); }
void ShellyHTDisplay::show_arrow(bool on)     { this->display_->set_segment(SEG_PFEIL, on); }
void ShellyHTDisplay::show_dp(bool on)        { this->display_->set_segment(SEG_DP, on); }
void ShellyHTDisplay::show_degree(bool on)    { this->display_->set_segment(SEG_GRAD, on); }
void ShellyHTDisplay::show_percent(bool on)   { this->display_->set_segment(SEG_PROZENT, on); }
void ShellyHTDisplay::show_unit(char c)       { this->write_digit_(DIG_UNIT, this->char_to_seg7_(c)); }

void ShellyHTDisplay::show_colon(bool on) {
  // Bit 38 = upper + lower dot (real colon)
  // Bit 130 = middle dot (DO NOT use — creates unwanted 3rd dot)
  this->display_->set_segment(SEG_COL_TOP, on);
}

// ── Internal state machine ──────────────────────────────────────

void ShellyHTDisplay::check_and_update_() {
  if (this->ota_active_) return;  // Display locked during OTA

  bool has_t = this->temp_sensor_ && this->temp_sensor_->has_state();
  bool has_h = this->humi_sensor_ && this->humi_sensor_->has_state();
  if (!has_t || !has_h) return;

  // Read current values (RAM only, no I2C)
  int new_temp = (int)roundf(this->temp_sensor_->state * 10.0f);
  int new_humi = (int)this->humi_sensor_->state;
  int new_hour = -1, new_min = -1;
  if (this->time_) {
    auto now = this->time_->now();
    if (now.is_valid()) { new_hour = now.hour; new_min = now.minute; }
  }
  int new_bars = 0;
  if (this->wifi_sensor_ && this->wifi_sensor_->has_state()) {
    float rssi = this->wifi_sensor_->state;
    if (rssi > -50) new_bars = 4; else if (rssi > -65) new_bars = 3;
    else if (rssi > -75) new_bars = 2; else if (rssi > -85) new_bars = 1;
  }
  uint8_t level = 0;
  if (this->batt_sensor_ && this->batt_sensor_->has_state()) {
    float v = this->batt_sensor_->state;    
    if (v > 5.8f) level = 5; else if (v > 5.4f) level = 4;
    else if (v > 5.0f) level = 3; else if (v > 4.6f) level = 2;
    else if (v > 4.2f) level = 1;    
  }
  bool new_wifi = wifi::global_wifi_component->is_connected();
  bool new_frost = this->temp_sensor_->state < 3.0f;

  // Changed?
  bool changed = (new_temp != this->disp_temp_) || (new_humi != this->disp_humi_) ||
                 (new_hour != this->disp_hour_) || (new_min  != this->disp_min_)  ||
                 (new_bars != this->disp_bars_) || (new_wifi != this->disp_wifi_) ||
                 (new_frost!= this->disp_frost_);

  if (!changed) return;

  ESP_LOGD(TAG, "Update: %.1f°C %d%% %02d:%02d sig:%d wifi:%d frost:%d battery:%d",
           new_temp / 10.0f, new_humi, new_hour, new_min, new_bars, new_wifi, new_frost, level);

  this->disp_temp_ = new_temp; 
  this->disp_humi_ = new_humi;
  this->disp_hour_ = new_hour; 
  this->disp_min_  = new_min;
  this->disp_bars_ = new_bars; 
  this->disp_wifi_ = new_wifi;
  this->disp_frost_ = new_frost;

  // Build framebuffer
  this->display_->clear();
  this->show_temperature(this->temp_sensor_->state, false);
  this->show_humidity(new_humi);
  if (new_hour >= 0) this->show_time(new_hour, new_min);
  this->show_signal(new_bars);
  this->show_globe(new_wifi);
  this->show_frost(new_frost);
  this->show_battery(level);

  this->display_->commit();
}


// ── OTA progress display ────────────────────────────────────────

void ShellyHTDisplay::show_ota_begin() {
  this->ota_active_ = true;
  this->display_->clear();
  // Siekoo: S=ACDF (unfamiliar), 5=ACDFG (looks like classic S)
  this->show_text_clock(this->font_ == FONT_SIEKOO ? " E5P" : " ESP");
  this->show_text_big("otA");
  this->write_digit_(DIG_H1, S7_BLANK);
  this->write_digit_(DIG_H2, SK_DOT);
  this->display_->commit();
  ESP_LOGD(TAG, "OTA started — display locked");
}

void ShellyHTDisplay::show_ota_progress(float progress) {
  if (!this->ota_active_) return;
  uint8_t h1 = S7_BLANK, h2 = S7_BLANK;
  if (progress >= 100.0f)     { h2 = SK_EXCLAIM; }
  else if (progress >= 66.0f) { h2 = SK_K; }
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
  ESP_LOGD(TAG, "OTA complete");
}

void ShellyHTDisplay::show_ota_error() {
  this->display_->clear();
  this->show_text_clock("Err ");
  this->show_text_big("otA");
  this->display_->commit();
  ESP_LOGE(TAG, "OTA error");
  this->ota_active_ = false;
  this->force_refresh();
}


}  // namespace uc8119
}  // namespace esphome
