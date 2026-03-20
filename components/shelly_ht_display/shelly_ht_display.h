#pragma once
// Shelly H&T Gen3 Display Layer
// SPDX-License-Identifier: MIT
//
// Contains: segment mapping, 7-segment encoding, sensor state machine.
// Uses UC8119 driver for low-level display access.

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/time/real_time_clock.h"
#include "../uc8119/uc8119.h"
#include "../uc8119/siekoo.h"

// Forward declaration
namespace esphome { namespace deep_sleep { class DeepSleepComponent; } }

namespace esphome {
namespace uc8119 {

enum SegmentFont : uint8_t {
  FONT_SIEKOO = 0,   // Siekoo by Alexander Fakoó — confusion-free (default)
  FONT_CLASSIC = 1,  // Traditional 7-segment (O=0, I=1, S=5 ambiguity)
};

// ── Segment mapping (Shelly H&T Gen3 LCD panel) ────────────────

struct DigitMap { uint8_t a, b, c, d, e, f, g; };

static const DigitMap DIG_T1 = { 12,  26,  32,  28,  19,  17,  29};
static const DigitMap DIG_T2 = { 13,  23,  35,  37,  36,  24,  34};
static const DigitMap DIG_T3 = { 14,  18,  44, 131,  40,  22,  39};
static const DigitMap DIG_T4 = {  8,   7,   9,  11,  30,  15,  10};
static const DigitMap DIG_D1 = { 41, 129, 128,  43,  21,  20,  42};
static const DigitMap DIG_D2 = { 95,  91,  92,  94,  96, 127,  93};
static const DigitMap DIG_D3 = { 89,  85,  51,  87,  88,  90,  86};
static const DigitMap DIG_H1 = { 55,  58,  61,  62,  60,  57,  59};
static const DigitMap DIG_H2 = { 74,  76,  75,  72,  71,  56,  73};
static const DigitMap DIG_UNIT = { 53, 79, 84, 81, 77, 54, 78};

static const uint8_t SEG_BATT[] = {4, 3, 2, 1, 0};
static const uint8_t SEG_SIG[]  = {6, 16, 25, 33};
static const uint8_t SEG_COL_TOP = 38, SEG_COL_MID = 130;
static const uint8_t SEG_BT = 45, SEG_GLOBE = 46, SEG_HEIZ = 47;
static const uint8_t SEG_VENT = 48, SEG_KALEN = 49, SEG_DP = 50;
static const uint8_t SEG_GRAD = 52, SEG_FROST = 27, SEG_PFEIL = 5;
static const uint8_t SEG_PROZENT = 83;

static const uint8_t S7_0=0x7E,S7_1=0x30,S7_2=0x6D,S7_3=0x79,S7_4=0x33;
static const uint8_t S7_5=0x5B,S7_6=0x5F,S7_7=0x70,S7_8=0x7F,S7_9=0x7B;
static const uint8_t S7_BLANK=0x00, S7_MINUS=0x01;
static const uint8_t S7_NUM[] = {S7_0,S7_1,S7_2,S7_3,S7_4,S7_5,S7_6,S7_7,S7_8,S7_9};

class ShellyHTDisplay : public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  // Config
  void set_display(UC8119 *d) { this->display_ = d; }
  void set_deep_sleep_mode(bool v) { this->deep_sleep_mode_ = v; }
  void set_font(SegmentFont f) { this->font_ = f; }
  void set_wifi_update_every(uint32_t n) { this->wifi_update_every_ = n; }
  void set_sleep_duration(uint32_t ms) { this->sleep_duration_ms_ = ms; }
  void set_usb_detect_pin(GPIOPin *pin) { this->usb_detect_pin_ = pin; }

  // Analog sensor references
  void set_temperature_sensor(sensor::Sensor *s) { this->temp_sensor_ = s; }
  void set_humidity_sensor(sensor::Sensor *s) { this->humi_sensor_ = s; }
  void set_battery_sensor(sensor::Sensor *s) { this->batt_sensor_ = s; }
  void set_wifi_signal_sensor(sensor::Sensor *s) { this->wifi_sensor_ = s; }
  void set_time(time::RealTimeClock *t) { this->time_ = t; }
  void set_deep_sleep_component(deep_sleep::DeepSleepComponent *ds) { this->deep_sleep_ = ds; }

  // Icon binary_sensor overrides
  void set_frost_sensor(binary_sensor::BinarySensor *s) { this->frost_sensor_ = s; }
  void set_heating_sensor(binary_sensor::BinarySensor *s) { this->heating_sensor_ = s; }
  void set_ventilator_sensor(binary_sensor::BinarySensor *s) { this->ventilator_sensor_ = s; }
  void set_bluetooth_sensor(binary_sensor::BinarySensor *s) { this->bluetooth_sensor_ = s; }
  void set_globe_sensor(binary_sensor::BinarySensor *s) { this->globe_sensor_ = s; }
  void set_calendar_sensor(binary_sensor::BinarySensor *s) { this->calendar_sensor_ = s; }
  void set_arrow_sensor(binary_sensor::BinarySensor *s) { this->arrow_sensor_ = s; }

  // on_update trigger
  Trigger<> *get_on_update_trigger() { return &this->on_update_trigger_; }

  // ── High-level display API (usable from lambdas) ─────────────

  void show_temperature(float temp_c, bool fahrenheit = false);
  void show_humidity(int humidity);
  void show_time(int hours, int minutes);
  void show_text_big(const char *text);
  void show_text_clock(const char *text);
  void show_battery(uint8_t level);
  void show_signal(uint8_t bars);

  // Icons
  void show_bluetooth(bool on);
  void show_globe(bool on);
  void show_frost(bool on);
  void show_heating(bool on);
  void show_ventilator(bool on);
  void show_calendar(bool on);
  void show_arrow(bool on);
  void show_colon(bool on);
  void show_dp(bool on);
  void show_degree(bool on);
  void show_percent(bool on);
  void show_unit(char c);

  // OTA
  void show_ota_begin();
  void show_ota_progress(float progress);
  void show_ota_end();
  void show_ota_error();

  void force_refresh() { this->disp_temp_ = -999; }
  bool is_deep_sleep_mode() const { return this->deep_sleep_mode_; }
  bool is_usb_powered() const { return this->usb_powered_; }
  bool is_wifi_skipped() const { return this->wifi_skipped_; }

 protected:
  UC8119 *display_{nullptr};
  bool deep_sleep_mode_{false};
  bool usb_powered_{false};
  SegmentFont font_{FONT_SIEKOO};
  bool ota_active_{false};

  // USB detection
  GPIOPin *usb_detect_pin_{nullptr};  // HIGH=USB, LOW=battery (GPIO8 on Shelly)

  // Deep sleep WiFi optimization
  uint32_t wifi_update_every_{5};
  uint32_t sleep_duration_ms_{60000};
  bool wifi_skipped_{false};
  int rtc_hour_{-1};
  int rtc_min_{-1};

  // Analog sensors
  sensor::Sensor *temp_sensor_{nullptr};
  sensor::Sensor *humi_sensor_{nullptr};
  sensor::Sensor *batt_sensor_{nullptr};
  sensor::Sensor *wifi_sensor_{nullptr};
  time::RealTimeClock *time_{nullptr};
  deep_sleep::DeepSleepComponent *deep_sleep_{nullptr};

  // Icon binary_sensor overrides
  binary_sensor::BinarySensor *frost_sensor_{nullptr};
  binary_sensor::BinarySensor *heating_sensor_{nullptr};
  binary_sensor::BinarySensor *ventilator_sensor_{nullptr};
  binary_sensor::BinarySensor *bluetooth_sensor_{nullptr};
  binary_sensor::BinarySensor *globe_sensor_{nullptr};
  binary_sensor::BinarySensor *calendar_sensor_{nullptr};
  binary_sensor::BinarySensor *arrow_sensor_{nullptr};

  // on_update trigger
  Trigger<> on_update_trigger_;

  // Display state
  int disp_temp_{-999};
  int disp_humi_{-1};
  int disp_hour_{-1};
  int disp_min_{-1};
  int disp_bars_{-1};
  bool disp_wifi_{false};
  bool disp_frost_{false};
  bool disp_heating_{false};
  bool disp_vent_{false};
  bool disp_bt_{false};
  bool disp_calendar_{false};
  bool disp_arrow_{false};

  // Internal
  void check_and_update_();
  void write_digit_(const DigitMap &d, uint8_t code);
  void write_number_(const DigitMap &d, int8_t n);
  uint8_t char_to_seg7_(char c);
  bool get_icon_state_(binary_sensor::BinarySensor *ext, bool default_val);

  // RTC time tracking
  void save_time_to_rtc_();
  bool load_time_from_rtc_();
  void enter_deep_sleep_();
};

}  // namespace uc8119
}  // namespace esphome
