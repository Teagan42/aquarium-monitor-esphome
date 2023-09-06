#pragma once

#include <EEPROM.h>
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

#define ReceivedBufferLength 15
#define TdsFactor 0.5  // tds = ec / 2

#define EEPROM_write(address, p) {int i = 0; uint8_t *pp = (uint8_t*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; uint8_t *pp = (uint8_t*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

namespace esphome {
  namespace gravitytds {

    class GravityTDSComponent : public PollingComponent {
      public:
      void setup() override;
      void dump_config() override;
      void update() override;
      float get_setup_priority() const override;

      protected:
    }

  } // mnamespace gravityTds
} // namespace esphome


class GravityTDS
{
public:
    GravityTDS();
    ~GravityTDS();

    void begin();  //initialization
    void update(); //read and calculate
    void setPin(int pin); 
    void setTemperature(float temp);  //set the temperature and execute temperature compensation
    void setAref(float value);  //reference voltage on ADC, default 5.0V on Arduino UNO
    void setAdcRange(float range);  //1024 for 10bit ADC;4096 for 12bit ADC
    void setKvalueAddress(int address); //set the EEPROM address to store the k value,default address:0x08
    float getKvalue(); 
    float getTdsValue();
    float getEcValue();

private:
    int pin;
    float aref;  // default 5.0V on Arduino UNO
    float adcRange;
    float temperature;
    int kValueAddress;     //the address of the K value stored in the EEPROM
    char cmdReceivedBuffer[ReceivedBufferLength+1];   // store the serial cmd from the serial monitor
    uint8_t cmdReceivedBufferIndex;
 
    float kValue;      // k value of the probe,you can calibrate in buffer solution ,such as 706.5ppm(1413us/cm)@25^C 
    float analogValue;
    float voltage;
    float ecValue; //before temperature compensation
    float ecValue25; //after temperature compensation
    float tdsValue;

    void readKValues();
    bool cmdSerialDataAvailable();
    uint8_t cmdParse();
    void ecCalibration(uint8_t mode);
};  
