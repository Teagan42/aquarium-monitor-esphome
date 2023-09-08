#include "esphome.h"
#include "GravityPh.h"

using namespace esphome;

Gravi gravityPH;

class GravityPhSensor : public PollingComponent, public sensor::Sensor, public api::CustomAPIDevice
{
public:
  // constructor
  GravityPhSensor(uint32_t update_interval) : PollingComponent(update_interval) {}

  float get_setup_priority() const override { return esphome::setup_priority::IO; }

  void setup() override
  {
    // This will be called by App.setup()

    // services
    register_service(&GravityPhSensor::enter_ph_calibration_mode, "enter_ph");
    register_service(&GravityPhSensor::exit_ph_calibration_mode, "exit_ph");
    register_service(&GravityPhSensor::calibrate_ph, "calibrate_ph");
  }
  void update() override
  {
    float voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    float temperature = readTemperature();
    float phValue = ph.readPH(voltage, temperature); // convert voltage to pH with temperature compensation
    // This will be called every "update_interval" milliseconds.
    publish_state(phValue);
  }
  void enter_ph_calibration_mode()
  {
    float voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    float temperature = readTemperature();
    char ENTERPH[8] = "ENTERPH";
    ph.calibration(voltage, temperature, ENTERPH);
  }
  void exit_ph_calibration_mode()
  {
    float voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    float temperature = readTemperature();
    char EXITPH[7] = "EXITPH";
    ph.calibration(voltage, temperature, EXITPH);
  }
  void calibrate_ph()
  {
    float voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    float temperature = readTemperature();
    char CALPH[6] = "CALPH";
    ph.calibration(voltage, temperature, CALPH);
  }
};