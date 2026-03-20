import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor, time as time_comp
from esphome.const import CONF_ID, CONF_TIME_ID
from esphome.core import CORE
from esphome import automation, pins

DEPENDENCIES = ["uc8119"]
CODEOWNERS = ["@alex"]

uc8119_ns = cg.esphome_ns.namespace("uc8119")
UC8119 = uc8119_ns.class_("UC8119")
ShellyHTDisplay = uc8119_ns.class_("ShellyHTDisplay", cg.PollingComponent)

deep_sleep_ns = cg.esphome_ns.namespace("deep_sleep")
DeepSleepComponent = deep_sleep_ns.class_("DeepSleepComponent")

CONF_DISPLAY_ID = "display_id"
CONF_TEMPERATURE_SENSOR = "temperature_sensor"
CONF_HUMIDITY_SENSOR = "humidity_sensor"
CONF_BATTERY_SENSOR = "battery_sensor"
CONF_WIFI_SIGNAL_SENSOR = "wifi_signal_sensor"
CONF_FONT = "font"
CONF_ON_UPDATE = "on_update"
CONF_WIFI_UPDATE_EVERY = "wifi_update_every"
CONF_USB_DETECT_PIN = "usb_detect_pin"

# Icon binary_sensor overrides
CONF_FROST_SENSOR = "frost_sensor"
CONF_HEATING_SENSOR = "heating_sensor"
CONF_VENTILATOR_SENSOR = "ventilator_sensor"
CONF_BLUETOOTH_SENSOR = "bluetooth_sensor"
CONF_GLOBE_SENSOR = "globe_sensor"
CONF_CALENDAR_SENSOR = "calendar_sensor"
CONF_ARROW_SENSOR = "arrow_sensor"

SegmentFont = uc8119_ns.enum("SegmentFont")
FONT_OPTIONS = {
    "siekoo": SegmentFont.FONT_SIEKOO,
    "classic": SegmentFont.FONT_CLASSIC,
}

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ShellyHTDisplay),
            cv.Required(CONF_DISPLAY_ID): cv.use_id(UC8119),
            # Sensor references
            cv.Optional(CONF_TEMPERATURE_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_HUMIDITY_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_BATTERY_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_WIFI_SIGNAL_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_TIME_ID): cv.use_id(time_comp.RealTimeClock),
            # Icon binary_sensor overrides
            cv.Optional(CONF_FROST_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_HEATING_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_VENTILATOR_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_BLUETOOTH_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_GLOBE_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_CALENDAR_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_ARROW_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            # Config
            cv.Optional(CONF_FONT, default="siekoo"): cv.enum(FONT_OPTIONS, lower=True),
            # Deep sleep optimization
            cv.Optional(CONF_WIFI_UPDATE_EVERY, default=5): cv.uint32_t,
            # USB detection pin (HIGH=USB, LOW=battery). On Shelly H&T Gen3: GPIO8
            cv.Optional(CONF_USB_DETECT_PIN): pins.gpio_input_pin_schema,
            # Lambda hook
            cv.Optional(CONF_ON_UPDATE): automation.validate_automation(single=True),
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
    if is_deep_sleep:
        ds_conf = CORE.config["deep_sleep"]
        if CONF_ID in ds_conf:
            ds = await cg.get_variable(ds_conf[CONF_ID])
            cg.add(var.set_deep_sleep_component(ds))
        if "sleep_duration" in ds_conf:
            cg.add(var.set_sleep_duration(ds_conf["sleep_duration"]))

    # USB detect pin
    if CONF_USB_DETECT_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_USB_DETECT_PIN])
        cg.add(var.set_usb_detect_pin(pin))

    # Analog sensors
    for conf_key, setter in [
        (CONF_TEMPERATURE_SENSOR, "set_temperature_sensor"),
        (CONF_HUMIDITY_SENSOR, "set_humidity_sensor"),
        (CONF_BATTERY_SENSOR, "set_battery_sensor"),
        (CONF_WIFI_SIGNAL_SENSOR, "set_wifi_signal_sensor"),
    ]:
        if conf_key in config:
            s = await cg.get_variable(config[conf_key])
            cg.add(getattr(var, setter)(s))

    if CONF_TIME_ID in config:
        t = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time(t))

    # Icon binary_sensor overrides
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

    # on_update lambda
    if CONF_ON_UPDATE in config:
        await automation.build_automation(
            var.get_on_update_trigger(), [], config[CONF_ON_UPDATE]
        )
