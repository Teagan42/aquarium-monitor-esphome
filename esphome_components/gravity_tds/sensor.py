import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import sensor, voltage_sampler
from esphome.const import (
    CONF_ID,
    CONF_PIN,
    UNIT_PARTS_PER_MILLION,
    DEVICE_CLASS_WATER,
    STATE_CLASS_MEASUREMENT
)


MULTI_CONF = True

DEPENDENCIES = ['sensor', 'adc']
gravity_tds_ns = cg.esphome_ns.namespace("gravity_tds")

GravityTDSSensor = gravity_tds_ns.class_(
    "GravityTDSSensor",
    sensor.Sensor,
    cg.PollingComponent,
    voltage_sampler.VoltageSampler
)


CONFIG_SCHEMA = cv.All(
    sensor.sensor.sensor_schema(
        GravityTDSSensor,
        unit_of_measurements=UNIT_PARTS_PER_MILLION,
        accuracy_decimals=2,
        devie_class=DEVICE_CLASS_WATER,
        state_class=STATE_CLASS_MEASUREMENT
    )
    .extend(cv.polling_component_schema("60s"))
)
