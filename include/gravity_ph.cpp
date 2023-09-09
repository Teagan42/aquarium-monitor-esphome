#include "gravity_ph.h"

static const char *const TAG = "gravity_ph";

GravityPhSensor::GravityPhSensor(esphome::adc::ADCSensor *voltageSensor, uint32_t updateInterval) : PollingComponent(updateInterval)
{
  this->voltage_sensor = voltageSensor;
}

std::vector<esphome::sensor::Sensor *> GravityPhSensor::sensors()
{
  esphome::App.register_component(this);

  return {this->ph_sensor};
}

void GravityPhSensor::setup()
{
  esphome::ESP_LOGI(TAG, "setting up...");
  this->pref_ = esphome::global_preferences->make_preference<pHCalibrationData>(this->get_object_id_hash());
  this->pref_.load(&this->calibrationData);
}

float GravityPhSensor::get_setup_priority() const
{
  return esphome::setup_priority::DATA;
}

void GravityPhSensor::update()
{
  esphome::ESP_LOGI(TAG, "updating");

  float mV = this->voltage_sensor->state * 1000.0;
  float neutral = (this->calibrationData.neutral.mV - PH_7_LAB_VOLTAGE) / 3.0;
  float acid = (this->calibrationData.acid.mV - PH_7_LAB_VOLTAGE) / 3.0;
  float slope = (this->calibrationData.neutral.pH - this->calibrationData.acid.pH) / (neutral - acid);
  float intercept = this->calibrationData.neutral.pH - slope * neutral;
  float ph = slope * (mV - PH_7_LAB_VOLTAGE) / 3.0 + intercept;

  esphome::ESP_LOGD("gravity_ph", "%.2f mV | %.2f pH", mV, ph);
  this->ph_sensor->publish_state(ph);
}