#pragma once

#include <Arduino.h>
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/adc/adc_sensor.h"
#include "esphome/components/api/custom_api_device.h"
#include "esphome/core/preferences.h"
#include "esphome/core/component.h"
#include "esphome/core/application.h"

#define PH_8_VOLTAGE 1122.0
#define PH_6_VOLTAGE 1478.0
#define PH_5_VOLTAGE 1654.0
#define PH_4_VOLTAGE 2032.4
#define PH_7_LAB_VOLTAGE 1500

struct pHCalibrationPoint
{
    float pH;
    float mV;
};

struct pHCalibrationData
{
    pHCalibrationPoint base;
    pHCalibrationPoint neutral;
    pHCalibrationPoint acid;
};

class GravityPhSensor : public esphome::PollingComponent,
                        public esphome::sensor::Sensor
{
private:
    pHCalibrationData calibrationData = {
        {0.0, 0.0},
        {7.0, PH_7_LAB_VOLTAGE},
        {4.0, PH_4_VOLTAGE}};
    esphome::ESPPreferenceObject pref_;

    esphome::sensor::Sensor *ph_sensor = new esphome::sensor::Sensor();
    esphome::adc::ADCSensor *voltage_sensor;

    void on_calibration_acid(float mV, float ph = 4.0);
    void on_calibration_neutral(float mV, float ph = 7.0);
    void on_calibration_base(float mV, float ph = 10.0);

public:
    GravityPhSensor(esphome::adc::ADCSensor *voltageSensor, uint32_t updateInterval = 15000);

    std::vector<esphome::sensor::Sensor *> sensors();

    float get_setup_priority() const override;

    void setup() override;

    void update() override;
};