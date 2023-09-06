#include "dfrobot_gravity_ph.h"

class CustomDFRobotPhSensor : public PollingComponent, public Sensor, public CustomAPIDevice {
 public:
  // constructor
  CustomDFRobotPhSensor(uint32_t update_interval) : PollingComponent(update_interval) {}

  float get_setup_priority() const override { return esphome::setup_priority::IO; }

  void setup() override {
    // This will be called by App.setup()

    // services
    register_service(&CustomDFRobotPhSensor::enter_ph_calibration_mode, "enter_ph");
    register_service(&CustomDFRobotPhSensor::exit_ph_calibration_mode, "exit_ph");
    register_service(&CustomDFRobotPhSensor::calibrate_ph, "calibrate_ph");

  }
  void update() override {
		voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    temperature = readTemperature();
    phValue = ph.readPH(voltage, temperature); // convert voltage to pH with temperature compensation
    // This will be called every "update_interval" milliseconds.
    publish_state(phValue);
  }
  void enter_ph_calibration_mode() {
    voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    temperature = readTemperature();
    char ENTERPH[8] = "ENTERPH";
    ph.calibration(voltage, temperature, ENTERPH);
  }
  void exit_ph_calibration_mode() {
    voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    temperature = readTemperature();
    char EXITPH[7] = "EXITPH";
    ph.calibration(voltage, temperature, EXITPH);
  }
  void calibrate_ph() {
    voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    temperature = readTemperature();
    char CALPH[6] = "CALPH";
    ph.calibration(voltage, temperature, CALPH);
  }

};