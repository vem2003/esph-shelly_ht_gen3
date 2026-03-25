import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor, time as time_comp
from esphome.const import (
    CONF_ID,
    CONF_TIME_ID,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_BATTERY,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
    UNIT_VOLT,
)
from esphome.core import CORE
from esphome import automation

DEPENDENCIES = ["uc8119"]
CODEOWNERS = ["@alex"]

uc8119_ns = cg.esphome_ns.namespace("uc8119")
UC8119 = uc8119_ns.class_("UC8119")

shelly_htg3_ns = cg.esphome_ns.namespace("shelly_htg3")
ShellyHTDisplay = shelly_htg3_ns.class_("ShellyHTDisplay", cg.PollingComponent)

SegmentFont = shelly_htg3_ns.enum("SegmentFont")
FONT_OPTIONS = {
    "siekoo": SegmentFont.FONT_SIEKOO,
    "classic": SegmentFont.FONT_CLASSIC,
}

# Config keys
CONF_DISPLAY_ID = "display_id"
CONF_FONT = "font"
CONF_WIFI_UPDATE_EVERY = "wifi_update_every"
CONF_ON_UPDATE = "on_update"
CONF_ON_READY = "on_ready"

# Input sensors
CONF_TEMPERATURE_SENSOR = "temperature_sensor"
CONF_HUMIDITY_SENSOR = "humidity_sensor"
CONF_WIFI_SIGNAL_SENSOR = "wifi_signal_sensor"

# Icon binary_sensor overrides
CONF_FROST_SENSOR = "frost_sensor"
CONF_HEATING_SENSOR = "heating_sensor"
CONF_VENTILATOR_SENSOR = "ventilator_sensor"
CONF_BLUETOOTH_SENSOR = "bluetooth_sensor"
CONF_GLOBE_SENSOR = "globe_sensor"
CONF_CALENDAR_SENSOR = "calendar_sensor"
CONF_ARROW_SENSOR = "arrow_sensor"

# Battery hardware config
CONF_BATTERY_ADC_PIN = "battery_adc_pin"
CONF_BATTERY_PRESENCE_PIN = "battery_presence_pin"
CONF_BATTERY_POWER_ENABLE_PIN = "battery_power_enable_pin"
CONF_BATTERY_DIVIDER = "battery_divider"
CONF_BATTERY_FULL_VOLTAGE = "battery_full_voltage"
CONF_BATTERY_EMPTY_VOLTAGE = "battery_empty_voltage"
CONF_BATTERY_SAMPLES = "battery_samples"

# Battery output sensors
CONF_BATTERY_VOLTAGE = "battery_voltage"
CONF_BATTERY_PERCENT = "battery_percent"
CONF_BATTERY_PRESENT = "battery_present"
CONF_EXTERNAL_POWER = "external_power"

GPIO_PIN_SCHEMA = cv.int_range(min=0, max=21)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ShellyHTDisplay),
            cv.Required(CONF_DISPLAY_ID): cv.use_id(UC8119),
            # Font
            cv.Optional(CONF_FONT, default="siekoo"): cv.enum(
                FONT_OPTIONS, lower=True
            ),
            # Deep sleep: connect WiFi only every Nth wake cycle (0=always)
            cv.Optional(CONF_WIFI_UPDATE_EVERY, default=5): cv.uint32_t,
            # ── Input sensors ────────────────────────────────────────
            cv.Optional(CONF_TEMPERATURE_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_HUMIDITY_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_WIFI_SIGNAL_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_TIME_ID): cv.use_id(time_comp.RealTimeClock),
            # ── Icon binary_sensor overrides ─────────────────────────
            cv.Optional(CONF_FROST_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_HEATING_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_VENTILATOR_SENSOR): cv.use_id(
                binary_sensor.BinarySensor
            ),
            cv.Optional(CONF_BLUETOOTH_SENSOR): cv.use_id(
                binary_sensor.BinarySensor
            ),
            cv.Optional(CONF_GLOBE_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_CALENDAR_SENSOR): cv.use_id(
                binary_sensor.BinarySensor
            ),
            cv.Optional(CONF_ARROW_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            # ── Battery hardware config ──────────────────────────────
            cv.Optional(CONF_BATTERY_ADC_PIN, default=4): GPIO_PIN_SCHEMA,
            cv.Optional(CONF_BATTERY_PRESENCE_PIN, default=5): GPIO_PIN_SCHEMA,
            cv.Optional(CONF_BATTERY_POWER_ENABLE_PIN, default=18): GPIO_PIN_SCHEMA,
            cv.Optional(CONF_BATTERY_DIVIDER, default=2.263): cv.float_,
            cv.Optional(CONF_BATTERY_FULL_VOLTAGE, default=6.0): cv.float_,
            cv.Optional(CONF_BATTERY_EMPTY_VOLTAGE, default=4.0): cv.float_,
            cv.Optional(CONF_BATTERY_SAMPLES, default=15): cv.int_range(
                min=1, max=64
            ),
            # ── Battery output sensors ───────────────────────────────
            cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_PERCENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_PRESENT): binary_sensor.binary_sensor_schema(
                device_class="battery",
            ),
            cv.Optional(CONF_EXTERNAL_POWER): binary_sensor.binary_sensor_schema(
                device_class="plug",
            ),
            # ── Triggers ─────────────────────────────────────────────
            cv.Optional(CONF_ON_UPDATE): automation.validate_automation(single=True),
            cv.Optional(CONF_ON_READY): automation.validate_automation(single=True),
        }
    )
    .extend(cv.polling_component_schema("1s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    display = await cg.get_variable(config[CONF_DISPLAY_ID])
    cg.add(var.set_display(display))
    cg.add(var.set_font(config[CONF_FONT]))
    cg.add(var.set_wifi_update_every(config[CONF_WIFI_UPDATE_EVERY]))

    # Auto-detect deep_sleep and extract sleep_duration
    is_deep_sleep = "deep_sleep" in CORE.config
    cg.add(var.set_deep_sleep_mode(is_deep_sleep))
    if is_deep_sleep and "sleep_duration" in CORE.config["deep_sleep"]:
        cg.add(var.set_sleep_duration(CORE.config["deep_sleep"]["sleep_duration"]))

    # ── Input sensors ────────────────────────────────────────────
    for conf_key, setter in [
        (CONF_TEMPERATURE_SENSOR, "set_temperature_sensor"),
        (CONF_HUMIDITY_SENSOR, "set_humidity_sensor"),
        (CONF_WIFI_SIGNAL_SENSOR, "set_wifi_signal_sensor"),
    ]:
        if conf_key in config:
            s = await cg.get_variable(config[conf_key])
            cg.add(getattr(var, setter)(s))

    if CONF_TIME_ID in config:
        t = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time(t))

    # ── Icon binary_sensor overrides ─────────────────────────────
    for conf_key, setter in [
        (CONF_FROST_SENSOR, "set_frost_sensor"),
        (CONF_HEATING_SENSOR, "set_heating_sensor"),
        (CONF_VENTILATOR_SENSOR, "set_ventilator_sensor"),
        (CONF_BLUETOOTH_SENSOR, "set_bluetooth_sensor"),
        (CONF_GLOBE_SENSOR, "set_globe_sensor"),
        (CONF_CALENDAR_SENSOR, "set_calendar_sensor"),
        (CONF_ARROW_SENSOR, "set_arrow_sensor"),
    ]:
        if conf_key in config:
            s = await cg.get_variable(config[conf_key])
            cg.add(getattr(var, setter)(s))

    # ── Battery hardware config ──────────────────────────────────
    cg.add(var.set_battery_adc_pin(config[CONF_BATTERY_ADC_PIN]))
    cg.add(var.set_battery_presence_pin(config[CONF_BATTERY_PRESENCE_PIN]))
    cg.add(var.set_battery_power_enable_pin(config[CONF_BATTERY_POWER_ENABLE_PIN]))
    cg.add(var.set_battery_divider(config[CONF_BATTERY_DIVIDER]))
    cg.add(var.set_battery_full_voltage(config[CONF_BATTERY_FULL_VOLTAGE]))
    cg.add(var.set_battery_empty_voltage(config[CONF_BATTERY_EMPTY_VOLTAGE]))
    cg.add(var.set_battery_samples(config[CONF_BATTERY_SAMPLES]))

    # ── Battery output sensors ───────────────────────────────────
    if CONF_BATTERY_VOLTAGE in config:
        s = await sensor.new_sensor(config[CONF_BATTERY_VOLTAGE])
        cg.add(var.set_battery_voltage_sensor(s))

    if CONF_BATTERY_PERCENT in config:
        s = await sensor.new_sensor(config[CONF_BATTERY_PERCENT])
        cg.add(var.set_battery_percent_sensor(s))

    if CONF_BATTERY_PRESENT in config:
        s = await binary_sensor.new_binary_sensor(config[CONF_BATTERY_PRESENT])
        cg.add(var.set_battery_present_sensor(s))

    if CONF_EXTERNAL_POWER in config:
        s = await binary_sensor.new_binary_sensor(config[CONF_EXTERNAL_POWER])
        cg.add(var.set_external_power_sensor(s))

    # ── Triggers ─────────────────────────────────────────────────
    if CONF_ON_UPDATE in config:
        await automation.build_automation(
            var.get_on_update_trigger(), [], config[CONF_ON_UPDATE]
        )
    if CONF_ON_READY in config:
        await automation.build_automation(
            var.get_on_ready_trigger(), [], config[CONF_ON_READY]
        )
