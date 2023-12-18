#pragma once

#include <Arduino.h>
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/sensor/filter.h"
#include "esphome/components/adc/adc_sensor.h"
#include "esphome/components/api/custom_api_device.h"
#include "esphome/core/preferences.h"
#include "esphome/core/component.h"
#include "esphome/core/application.h"

#define PH_8_VOLTAGE 1.1220
#define PH_6_VOLTAGE 1.4780
#define PH_5_VOLTAGE 1.6540
#define PH_4_VOLTAGE 2.0324
#define PH_7_LAB_VOLTAGE 1.500

struct pHCalibrationPoint
{
    float pH;
    float mV;
    esphome::sensor::Sensor *sensor;
};

struct pHCalibrationData
{
    pHCalibrationPoint base;
    pHCalibrationPoint neutral;
    pHCalibrationPoint acid;
};

class GravityPhSensor : public esphome::PollingComponent,
                        public esphome::sensor::Sensor,
                        public esphome::api::CustomAPIDevice
{
private:
    pHCalibrationData calibrationData = {
        {10.0, 1.038646, new esphome::sensor::Sensor()},
        {7.0, 1.442143, new esphome::sensor::Sensor()},
        {4.0, 1.910964, new esphome::sensor::Sensor()}};
    esphome::sensor::Filter *calibration = new esphome::sensor::ClampFilter(0.0, 14.0, true);
    uint32_t voltageUpdateInterval;
    uint32_t updateInterval;
    esphome::ESPPreferenceObject pref_;

    esphome::sensor::Sensor *ph_sensor = new esphome::sensor::Sensor();
    esphome::adc::ADCSensor *voltage_sensor;

    void onCalibrationChange();

public:
    GravityPhSensor(esphome::adc::ADCSensor *voltageSensor, uint32_t updateInterval = 15000);

    std::vector<esphome::sensor::Sensor *> sensors();

    float get_setup_priority() const override;

    void setup() override;

    void update() override;

    void begin_calibration();
    void end_calibration();
    void on_calibration_acid(float buffer_ph = 4.0);
    void on_calibration_neutral(float buffer_ph = 7.0);
    void on_calibration_base(float buffer_ph = 10.0);
};
