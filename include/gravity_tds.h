#pragma once

#include <Arduino.h>
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/adc/adc_sensor.h"
#include "esphome/core/preferences.h"
#include "esphome/core/application.h"
#include "esphome/core/component.h"

#define TdsFactor 0.5 // tds = ec / 2

class GravityTdsSensor : public esphome::PollingComponent, public esphome::sensor::Sensor
{

private:
    uint8_t pin;
    float kValue = 1.0;
    float adcVoltage = 3.3; // default 5.0V on Arduino UNO
    float adcRange = 1024;  // 1024 for 10bit ADC;4096 for 12bit ADC
    esphome::sensor::Sensor *temperature_sensor;
    esphome::sensor::Sensor *tds_sensor = new esphome::sensor::Sensor();
    esphome::sensor::Sensor *voltage_sensor;
    esphome::ESPPreferenceObject pref_;

    float ftoc(float temp)
    {
        return (temp - 32.0) / 1.8;
    }

public:
    GravityTdsSensor(esphome::adc::ADCSensor *voltageSensor, esphome::sensor::Sensor *tempSensor, uint32_t updateInterval = 15000);

    void calibrate(float buffer_ppm = 707.0);

    std::vector<esphome::sensor::Sensor *> sensors();

    float get_setup_priority() const override;

    void setup() override;

    void update() override;
};
