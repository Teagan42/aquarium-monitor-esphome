#include "gravity_tds.h"

static const char *const TAG = "gravity_tds";

GravityTdsSensor::GravityTdsSensor(esphome::adc::ADCSensor *voltageSensor, Sensor *tempSensor, uint32_t updateInterval) : PollingComponent(updateInterval)
{
  this->temperature_sensor = tempSensor;
  this->voltage_sensor = voltageSensor;
};

float GravityTdsSensor::get_setup_priority() const
{
  return esphome::setup_priority::DATA;
}

void GravityTdsSensor::setup()
{
  esphome::ESP_LOGI(TAG, "setting up...");
  this->pref_ = esphome::global_preferences->make_preference<float>(this->get_object_id_hash());
  this->pref_.load(&this->kValue);
}

std::vector<esphome::sensor::Sensor *> GravityTdsSensor::sensors()
{
  esphome::App.register_component(this);

  return {this->tds_sensor};
}

void GravityTdsSensor::update()
{
  esphome::ESP_LOGI(TAG, "updating...");
  float temperature = ftoc(this->temperature_sensor->state);

  float v = this->voltage_sensor->state;
  float ec = (133.42 * v * v * v - 255.86 * v * v + 857.39 * v) * this->kValue;
  esphome::ESP_LOGD("gravity_tds", "%.2f V? | %.2f EC", v, ec);
  float ec25 = ec / (1.0 + 0.02 * (temperature - 25.0)); // temperature compensation
  float tds = ec25 * TdsFactor;

  esphome::ESP_LOGD("gravity_tds", "%.2f ppm | %.2f C", tds, temperature);
  tds_sensor->publish_state(tds);
}

void GravityTdsSensor::calibrate(float buffer_ppm)
{
  float temperature = ftoc(this->temperature_sensor->state);
  float v = this->voltage_sensor->state;

  float bufferedEC = buffer_ppm * (1.0 + 0.02 * (temperature - 25.0));
  float ec = (133.42 * v * v * v - 255.86 * v * v + 857.39 * v);

  float kValue = bufferedEC / ec;

  this->pref_.save(&kValue);
}

// void GravityTDS::ecCalibration(uint8_t mode)
// {
//     char *cmdReceivedBufferPtr;
//     static boolean ecCalibrationFinish = 0;
//     static boolean enterCalibrationFlag = 0;
//     float KValueTemp, rawECsolution;
//     switch (mode)
//     {
//     case 0:
//         if (enterCalibrationFlag)
//             Serial.println(F("Command Error"));
//         break;

//     case 1:
//         enterCalibrationFlag = 1;
//         ecCalibrationFinish = 0;
//         Serial.println();
//         Serial.println(F(">>>Enter Calibration Mode<<<"));
//         Serial.println(F(">>>Please put the probe into the standard buffer solution<<<"));
//         Serial.println();
//         break;

//     case 2:
//         cmdReceivedBufferPtr = strstr(cmdReceivedBuffer, "CAL:");
//         cmdReceivedBufferPtr += strlen("CAL:");
//         rawECsolution = strtod(cmdReceivedBufferPtr, NULL) / (float)(TdsFactor);
//         rawECsolution = rawECsolution * (1.0 + 0.02 * (temperature - 25.0));
//         if (enterCalibrationFlag)
//         {
//             // Serial.print("rawECsolution:");
//             // Serial.print(rawECsolution);
//             // Serial.print("  ecvalue:");
//             // Serial.println(ecValue);
//             KValueTemp = rawECsolution / (133.42 * voltage * voltage * voltage - 255.86 * voltage * voltage + 857.39 * voltage); // calibrate in the  buffer solution, such as 707ppm(1413us/cm)@25^c
//             if ((rawECsolution > 0) && (rawECsolution < 2000) && (KValueTemp > 0.25) && (KValueTemp < 4.0))
//             {
//                 Serial.println();
//                 Serial.print(F(">>>Confrim Successful,K:"));
//                 Serial.print(KValueTemp);
//                 Serial.println(F(", Send EXIT to Save and Exit<<<"));
//                 kValue = KValueTemp;
//                 ecCalibrationFinish = 1;
//             }
//             else
//             {
//                 Serial.println();
//                 Serial.println(F(">>>Confirm Failed,Try Again<<<"));
//                 Serial.println();
//                 ecCalibrationFinish = 0;
//             }
//         }
//         break;

//     case 3:
//         if (enterCalibrationFlag)
//         {
//             Serial.println();
//             if (ecCalibrationFinish)
//             {
//                 EEPROM_write(kValueAddress, kValue);
//                 Serial.print(F(">>>Calibration Successful,K Value Saved"));
//             }
//             else
//                 Serial.print(F(">>>Calibration Failed"));
//             Serial.println(F(",Exit Calibration Mode<<<"));
//             Serial.println();
//             ecCalibrationFinish = 0;
//             enterCalibrationFlag = 0;
//         }
//         break;
//     }
// }