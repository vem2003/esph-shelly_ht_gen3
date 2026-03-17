#pragma once
// Shelly H&T Gen3 Display Layer
// SPDX-License-Identifier: MIT
//
// Contains: segment mapping, 7-segment encoding, sensor state machine.
// Uses UC8119 driver for low-level display access.

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/time/real_time_clock.h"
#include "../uc8119/uc8119.h"
#include "../uc8119/siekoo.h"

// Forward declaration — only included in .cpp when available
namespace esphome { namespace deep_sleep { class DeepSleepComponent; } }

namespace esphome {
namespace uc8119 {

enum SegmentFont : uint8_t {
  FONT_SIEKOO = 0,   // Siekoo by Alexander Fakoó — confusion-free (default)
  FONT_CLASSIC = 1,  // Traditional 7-segment (O=0, I=1, S=5 ambiguity)
};

// ── Segment mapping (Shelly H&T Gen3 LCD panel) ────────────────

struct DigitMap { uint8_t a, b, c, d, e, f, g; };

// Clock digits (small, top row)
static const DigitMap DIG_T1 = { 12,  26,  32,  28,  19,  17,  29};
static const DigitMap DIG_T2 = { 13,  23,  35,  37,  36,  24,  34};
static const DigitMap DIG_T3 = { 14,  18,  44, 131,  40,  22,  39};
static const DigitMap DIG_T4 = {  8,   7,   9,  11,  30,  15,  10};

// Temperature digits (large, center)
static const DigitMap DIG_D1 = { 41, 129, 128,  43,  21,  20,  42};  // tens
static const DigitMap DIG_D2 = { 95,  91,  92,  94,  96, 127,  93};  // ones
static const DigitMap DIG_D3 = { 89,  85,  51,  87,  88,  90,  86};  // decimal

// Humidity digits (bottom right)
static const DigitMap DIG_H1 = { 55,  58,  61,  62,  60,  57,  59};  // tens
static const DigitMap DIG_H2 = { 74,  76,  75,  72,  71,  56,  73};  // ones

// Unit digit (small, right of temp)
static const DigitMap DIG_UNIT = { 53, 79, 84, 81, 77, 54, 78};      // C or F

// Icons & special segments
static const uint8_t SEG_BATT[] = {4, 3, 2, 1, 0};  // bars 1-4 + frame(5)
static const uint8_t SEG_SIG[]  = {6, 16, 25, 33};   // bars 1-4
static const uint8_t SEG_COL_TOP = 38, SEG_COL_MID = 130;  // TOP=colon (both dots), MID=middle dot (unused)
static const uint8_t SEG_BT = 45, SEG_GLOBE = 46, SEG_HEIZ = 47;
static const uint8_t SEG_VENT = 48, SEG_KALEN = 49, SEG_DP = 50;
static const uint8_t SEG_GRAD = 52, SEG_FROST = 27, SEG_PFEIL = 5;
static const uint8_t SEG_PROZENT = 83;

// 7-segment char codes (bit6=A..bit0=G)
static const uint8_t S7_0=0x7E,S7_1=0x30,S7_2=0x6D,S7_3=0x79,S7_4=0x33;
static const uint8_t S7_5=0x5B,S7_6=0x5F,S7_7=0x70,S7_8=0x7F,S7_9=0x7B;
static const uint8_t S7_BLANK=0x00, S7_MINUS=0x01;
static const uint8_t S7_NUM[] = {S7_0,S7_1,S7_2,S7_3,S7_4,S7_5,S7_6,S7_7,S7_8,S7_9};

class ShellyHTDisplay : public PollingComponent {
 public:
  void setup() override;
  void loop() override;
  void update() override {}
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  // Config
  void set_display(UC8119 *d) { this->display_ = d; }
  void set_check_interval(uint32_t ms) { this->check_interval_ms_ = ms; }
  void set_font(SegmentFont f) { this->font_ = f; }
  void set_usb_powered(bool p) { this->usb_powered_ = p;}

  // Sensor references
  void set_temperature_sensor(sensor::Sensor *s) { this->temp_sensor_ = s; }
  void set_humidity_sensor(sensor::Sensor *s) { this->humi_sensor_ = s; }
  void set_battery_sensor(sensor::Sensor *s) { this->batt_sensor_ = s; }
  void set_wifi_signal_sensor(sensor::Sensor *s) { this->wifi_sensor_ = s; }
  void set_time(time::RealTimeClock *t) { this->time_ = t; }

  // ── High-level display API (usable from lambdas) ─────────────

  void show_temperature(float temp_c, bool fahrenheit = false);
  void show_humidity(int humidity);
  void show_time(int hours, int minutes);
  void show_text_big(const char *text);    // D1-D3 (3 chars)
  void show_text_clock(const char *text);  // T1-T4 (4 chars)
  void show_battery(uint8_t level);        // 0-5
  void show_signal(uint8_t bars);          // 0-4

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

  // ── OTA progress display ───────────────────────────────────────
  /// Call from ota on_begin
  void show_ota_begin();
  /// Call from ota on_progress (0.0 - 100.0)
  void show_ota_progress(float progress);
  /// Call from ota on_end
  void show_ota_end();
  /// Call from ota on_error
  void show_ota_error();

  /// Force display refresh
  void force_refresh() { this->disp_temp_ = -999; }


 protected:
  UC8119 *display_{nullptr};
  uint32_t check_interval_ms_{1000};
  bool usb_powered_{false};
  SegmentFont font_{FONT_SIEKOO};
  bool ota_active_{false};       // Runtime: USB detected, deep sleep prevented

  // Sensor references
  sensor::Sensor *temp_sensor_{nullptr};
  sensor::Sensor *humi_sensor_{nullptr};
  sensor::Sensor *batt_sensor_{nullptr};
  sensor::Sensor *wifi_sensor_{nullptr};
  time::RealTimeClock *time_{nullptr};

  // Display state (what's currently shown)
  int disp_temp_{-999};
  int disp_humi_{-1};
  int disp_hour_{-1};
  int disp_min_{-1};
  int disp_bars_{-1};
  bool disp_wifi_{false};
  bool disp_frost_{false};

  uint32_t last_check_ms_{0};

  // Internal
  void check_and_update_();
  void write_digit_(const DigitMap &d, uint8_t code);
  void write_number_(const DigitMap &d, int8_t n);
  uint8_t char_to_seg7_(char c);
};

}  // namespace uc8119
}  // namespace esphome
