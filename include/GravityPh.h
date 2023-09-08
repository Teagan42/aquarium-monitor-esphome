/*
 * file DFRobot_ESP_PH.h * @ https://github.com/GreenPonik/DFRobot_ESP_PH_BY_GREENPONIK
 *
 * Arduino library for Gravity: Analog pH Sensor / Meter Kit V2, SKU: SEN0161-V2
 *
 * Based on the @ https://github.com/DFRobot/DFRobot_PH
 * Copyright   [DFRobot](http://www.dfrobot.com), 2018
 * Copyright   GNU Lesser General Public License
 *
 * ##################################################
 * ##################################################
 * ########## Fork on github by GreenPonik ##########
 * ############# ONLY ESP COMPATIBLE ################
 * ##################################################
 * ##################################################
 *
 * version  V1.1
 * date  2019-06
 */
#pragma once

#ifndef _DFROBOT_ESP_PH_H_
#define _DFROBOT_ESP_PH_H_

#include "Arduino.h"
#include "EEPROM.h"

#define PHVALUEADDR 0x00 // the start address of the pH calibration parameters stored in the EEPROM

#define PH_8_VOLTAGE 1122
#define PH_6_VOLTAGE 1478
#define PH_5_VOLTAGE 1654
#define PH_3_VOLTAGE 2010

#define ReceivedBufferLength 10 // length of the Serial CMD buffer

class GravityPh
{
public:
    GravityPh();
    ~GravityPh();
    void calibration(float voltage, float temperature, char *cmd); // calibration by Serial CMD
    void calibration(float voltage, float temperature);
    float readPH(float voltage, float temperature); // voltage to pH value, with temperature compensation
    void begin();                                   // initialization

private:
    float _phValue;
    float _acidVoltage;
    float _neutralVoltage;
    float _voltage;
    float _temperature;

    char _cmdReceivedBuffer[ReceivedBufferLength]; // store the Serial CMD
    byte _cmdReceivedBufferIndex;

private:
    boolean cmdSerialDataAvailable();
    void phCalibration(byte mode); // calibration process, wirte key parameters to EEPROM
    byte cmdParse(const char *cmd);
    byte cmdParse();
};

#define PH_3_VOLTAGE 2010

GravityPh::GravityPh()
{
    this->_temperature = 25.0;
    this->_phValue = 7.0;
    this->_acidVoltage = 2032.44;   // buffer solution 4.0 at 25C
    this->_neutralVoltage = 1500.0; // buffer solution 7.0 at 25C
    this->_voltage = 1500.0;
}

GravityPh::~GravityPh()
{
}

void GravityPh::begin()
{
    // check if calibration values (neutral and acid) are stored in eeprom
    this->_neutralVoltage = EEPROM.readFloat(PHVALUEADDR); // load the neutral (pH = 7.0)voltage of the pH board from the EEPROM
    if (this->_neutralVoltage == float() || isnan(this->_neutralVoltage))
    {
        this->_neutralVoltage = 1500.0; // new EEPROM, write typical voltage
        EEPROM.writeFloat(PHVALUEADDR, this->_neutralVoltage);
        EEPROM.commit();
    }

    this->_acidVoltage = EEPROM.readFloat(PHVALUEADDR + sizeof(float)); // load the acid (pH = 4.0) voltage of the pH board from the EEPROM
    if (this->_acidVoltage == float() || isnan(this->_acidVoltage))
    {
        this->_acidVoltage = 2032.44; // new EEPROM, write typical voltage
        EEPROM.writeFloat(PHVALUEADDR + sizeof(float), this->_acidVoltage);
        EEPROM.commit();
    }
}

float GravityPh::readPH(float voltage, float temperature)
{
    // Serial.print("_neutraVoltage:");
    // Serial.print(this->_neutralVoltage);
    // Serial.print(", _acidVoltage:");
    // Serial.print(this->_acidVoltage);
    float slope = (7.0 - 4.0) / ((this->_neutralVoltage - 1500.0) / 3.0 - (this->_acidVoltage - 1500.0) / 3.0); // two point: (_neutralVoltage,7.0),(_acidVoltage,4.0)
    float intercept = 7.0 - slope * (this->_neutralVoltage - 1500.0) / 3.0;
    // Serial.print(", slope:");
    // Serial.print(slope);
    // Serial.print(", intercept:");
    // Serial.println(intercept);
    this->_phValue = slope * (voltage - 1500.0) / 3.0 + intercept; // y = k*x + b
    Serial.print("[readPH]... phValue ");
    Serial.println(this->_phValue);
    return this->_phValue;
}

void GravityPh::calibration(float voltage, float temperature, char *cmd)
{
    this->_voltage = voltage;
    this->_temperature = temperature;
    strupr(cmd);
    phCalibration(cmdParse(cmd)); // if received Serial CMD from the serial monitor, enter into the calibration mode
}

void GravityPh::calibration(float voltage, float temperature)
{
    this->_voltage = voltage;
    this->_temperature = temperature;
    if (cmdSerialDataAvailable() > 0)
    {
        phCalibration(cmdParse()); // if received Serial CMD from the serial monitor, enter into the calibration mode
    }
}

boolean GravityPh::cmdSerialDataAvailable()
{
    char cmdReceivedChar;
    static unsigned long cmdReceivedTimeOut = millis();
    while (Serial.available() > 0)
    {
        if (millis() - cmdReceivedTimeOut > 500U)
        {
            this->_cmdReceivedBufferIndex = 0;
            memset(this->_cmdReceivedBuffer, 0, (ReceivedBufferLength));
        }
        cmdReceivedTimeOut = millis();
        cmdReceivedChar = Serial.read();
        if (cmdReceivedChar == '\n' || this->_cmdReceivedBufferIndex == ReceivedBufferLength - 1)
        {
            this->_cmdReceivedBufferIndex = 0;
            strupr(this->_cmdReceivedBuffer);
            return true;
        }
        else
        {
            this->_cmdReceivedBuffer[this->_cmdReceivedBufferIndex] = cmdReceivedChar;
            this->_cmdReceivedBufferIndex++;
        }
    }
    return false;
}

byte GravityPh::cmdParse(const char *cmd)
{
    byte modeIndex = 0;
    if (strstr(cmd, "ENTERPH") != NULL)
    {
        modeIndex = 1;
    }
    else if (strstr(cmd, "EXITPH") != NULL)
    {
        modeIndex = 3;
    }
    else if (strstr(cmd, "CALPH") != NULL)
    {
        modeIndex = 2;
    }
    return modeIndex;
}

byte GravityPh::cmdParse()
{
    byte modeIndex = 0;
    if (strstr(this->_cmdReceivedBuffer, "ENTERPH") != NULL)
    {
        modeIndex = 1;
    }
    else if (strstr(this->_cmdReceivedBuffer, "EXITPH") != NULL)
    {
        modeIndex = 3;
    }
    else if (strstr(this->_cmdReceivedBuffer, "CALPH") != NULL)
    {
        modeIndex = 2;
    }
    return modeIndex;
}

void GravityPh::phCalibration(byte mode)
{
    char *receivedBufferPtr;
    static boolean phCalibrationFinish = 0;
    static boolean enterCalibrationFlag = 0;
    switch (mode)
    {
    case 0:
        if (enterCalibrationFlag)
        {
            Serial.println(F(">>>Command Error<<<"));
        }
        break;

    case 1:
        enterCalibrationFlag = 1;
        phCalibrationFinish = 0;
        Serial.println();
        Serial.println(F(">>>Enter PH Calibration Mode<<<"));
        Serial.println(F(">>>Please put the probe into the 4.0 or 7.0 standard buffer solution<<<"));
        Serial.println();
        break;

    case 2:
        if (enterCalibrationFlag)
        {
            if ((this->_voltage > PH_8_VOLTAGE) && (this->_voltage < PH_6_VOLTAGE))
            { // buffer solution:7.0
                Serial.println();
                Serial.print(F(">>>Buffer Solution:7.0"));
                this->_neutralVoltage = this->_voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<"));
                Serial.println();
                phCalibrationFinish = 1;
            }
            else if ((this->_voltage > PH_5_VOLTAGE) && (this->_voltage < PH_3_VOLTAGE))
            { // buffer solution:4.0
                Serial.println();
                Serial.print(F(">>>Buffer Solution:4.0"));
                this->_acidVoltage = this->_voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<"));
                Serial.println();
                phCalibrationFinish = 1;
            }
            else
            {
                Serial.println();
                Serial.print(F(">>>Buffer Solution Error Try Again<<<"));
                Serial.println(); // not buffer solution or faulty operation
                phCalibrationFinish = 0;
            }
        }
        break;

    case 3: // store calibration value in eeprom
        if (enterCalibrationFlag)
        {
            Serial.println();
            if (phCalibrationFinish)
            {
                if ((this->_voltage > PH_8_VOLTAGE) && (this->_voltage < PH_5_VOLTAGE))
                {
                    EEPROM.writeFloat(PHVALUEADDR, this->_neutralVoltage);
                    EEPROM.commit();
                }
                else if ((this->_voltage > PH_5_VOLTAGE) && (this->_voltage < PH_3_VOLTAGE))
                {
                    EEPROM.writeFloat(PHVALUEADDR + sizeof(float), this->_acidVoltage);
                    EEPROM.commit();
                }
                Serial.print(F(">>>Calibration Successful"));
            }
            else
            {
                Serial.print(F(">>>Calibration Failed"));
            }
            Serial.println(F(",Exit PH Calibration Mode<<<"));
            Serial.println();
            phCalibrationFinish = 0;
            enterCalibrationFlag = 0;
        }
        break;
    }
}

#endif
