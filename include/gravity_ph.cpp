#include "gravity_ph.h"
#include "Matrix.h"

static const char *const TAG = "gravity_ph";

GravityPhSensor::GravityPhSensor(esphome::adc::ADCSensor *voltageSensor, uint32_t updateInterval) : PollingComponent(updateInterval)
{
  this->voltage_sensor = voltageSensor;
}

std::vector<esphome::sensor::Sensor *> GravityPhSensor::sensors()
{
  esphome::App.register_component(this);

  return {this->ph_sensor, this->calibrationData.acid.sensor, this->calibrationData.neutral.sensor, this->calibrationData.base.sensor};
}

void GravityPhSensor::onCalibrationChange()
{
  const float x1 = this->calibrationData.acid.mV;
  const float x2 = this->calibrationData.neutral.mV;
  const float x3 = this->calibrationData.base.mV;
  const float y1 = this->calibrationData.acid.pH;
  const float y2 = this->calibrationData.neutral.pH;
  const float y3 = this->calibrationData.base.pH;

  this->calibrationData.acid.sensor->publish_state(x1);
  this->calibrationData.neutral.sensor->publish_state(x2);
  this->calibrationData.base.sensor->publish_state(x3);

  float array1[3][3] = {
      {1, x1, x1 * x1},
      {1, x2, x2 * x2},
      {1, x3, x3 * x3}};
  float array2[3][1] =
      {{y1},
       {y2},
       {y3}};

  Matrix A(3, 3, (float *)array1);
  Matrix B(3, 1, (float *)array2);
  Matrix C = solveFor(A, B);

  const float c1 = C._entity[0][0];
  const float c2 = C._entity[1][0];
  const float c3 = C._entity[2][0];
  esphome::ESP_LOGI("gravity_ph", "%.2f + %.2f x + %.2f x^2", c1, c2, c3);
  this->calibration = new esphome::sensor::CalibratePolynomialFilter({c1, c2, c3});
}

void GravityPhSensor::setup()
{
  this->updateInterval = this->get_update_interval();
  esphome::ESP_LOGI(TAG, "setting up...");
  this->pref_ = esphome::global_preferences->make_preference<pHCalibrationData>(this->get_object_id_hash());
  this->pref_.load(&this->calibrationData);
  if (this->calibrationData.acid.mV != PH_4_VOLTAGE || this->calibrationData.base.mV != PH_8_VOLTAGE || this->calibrationData.neutral.mV != PH_7_LAB_VOLTAGE)
  {
    this->onCalibrationChange();
  }

  register_service(&GravityPhSensor::begin_calibration, "begin_calibration");
  register_service(&GravityPhSensor::end_calibration, "end_calibration");
  register_service(&GravityPhSensor::on_calibration_acid, "calibration_point_acid", {"buffer_ph"});
  register_service(&GravityPhSensor::on_calibration_neutral, "calibration_point_neutral", {"buffer_ph"});
  register_service(&GravityPhSensor::on_calibration_base, "calibration_point_base", {"buffer_ph"});
}

float GravityPhSensor::get_setup_priority() const
{
  return esphome::setup_priority::DATA;
}

void GravityPhSensor::update()
{
  esphome::ESP_LOGI(TAG, "updating");

  float V = this->voltage_sensor->state;
  // float pH = this->calibration->new_value(V).value_or(0);

  float neutral = (this->calibrationData.neutral.mV - PH_7_LAB_VOLTAGE) / 3.0;
  float acid = (this->calibrationData.acid.mV - PH_7_LAB_VOLTAGE) / 3.0;
  float slope = (this->calibrationData.neutral.pH - this->calibrationData.acid.pH) / (neutral - acid);
  float intercept = this->calibrationData.neutral.pH - slope * neutral;
  float ph = slope * (V - PH_7_LAB_VOLTAGE) / 3.0 + intercept;

  esphome::ESP_LOGD("gravity_ph", "%.2f V | %.2f pH | %.2f cal", V, ph, this->calibration->new_value(V));

  this->ph_sensor->publish_state(ph);
}

void GravityPhSensor::begin_calibration()
{
  this->voltageUpdateInterval = this->voltage_sensor->get_update_interval();
  this->updateInterval = this->get_update_interval();
  this->voltage_sensor->set_update_interval(3000);
  this->set_update_interval(5000);
}

void GravityPhSensor::end_calibration()
{
  this->set_update_interval(this->updateInterval);
  this->voltage_sensor->set_update_interval(this->voltageUpdateInterval);
}

void GravityPhSensor::on_calibration_acid(float buffer_pH)
{
  this->calibrationData.acid.pH = buffer_pH;
  this->calibrationData.acid.mV = this->voltage_sensor->state * 1000.0;
  this->pref_.save(&this->calibrationData);
  this->onCalibrationChange();
}

void GravityPhSensor::on_calibration_neutral(float buffer_pH)
{
  this->calibrationData.neutral.pH = buffer_pH;
  this->calibrationData.neutral.mV = this->voltage_sensor->state * 1000.0;
  this->pref_.save(&this->calibrationData);
  this->onCalibrationChange();
}

void GravityPhSensor::on_calibration_base(float buffer_pH)
{
  this->calibrationData.base.pH = buffer_pH;
  this->calibrationData.base.mV = this->voltage_sensor->state * 1000.0;
  this->pref_.save(&this->calibrationData);
  this->onCalibrationChange();
}
