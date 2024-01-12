/***********************************************************
File:           BM42S5321-1.h
Author:         BESTMODULES
Description:    Define classes and required variables
Version:        V1.0.1   -- 2024-01-02
***********************************************************/
#ifndef _BM42S5321_1_H__
#define _BM42S5321_1_H__

#include "Arduino.h"
#include <Wire.h> 
#include <SoftwareSerial.h>

#define   SUCCESS           0
#define   FAIL              1

#define   CHECK_OK          0
#define   CHECK_ERROR       1
#define   TIMEOUT_ERROR     2

#define   IIC_MODE          0x01
#define   UART_MODE         0x02

#define Smaptim_15  15
#define Smaptim_20  20
#define Smaptim_33  33
#define Smaptim_50  50
#define Smaptim_100 100
#define Smaptim_200 200
#define Smaptim_500 500

#define RoiRegion_16x16 0
#define RoiRegion_8x8   1
#define RoiRegion_4x4   2

#define BUARD_9600 0
#define BUARD_115200 1

#define ContinuousRanging  0
#define SingleRanging      1
#define PowerSaveRanging   2

class BM42S5321_1
{     
 public:
    BM42S5321_1(uint8_t intPin,HardwareSerial *theSerial = &Serial);  
    BM42S5321_1(uint8_t intPin,uint8_t rxPin,uint8_t txPin);
    BM42S5321_1(uint8_t intPin,TwoWire *theWire = &Wire,uint8_t i2c_addr = 0x07);
    void begin(uint8_t buard);
    void begin();
    uint8_t getINT();
    uint8_t startMeasure(); 
    uint8_t stopMeasure(); 
    uint16_t readDistance(uint16_t &dataState);
    uint8_t calibrateOffset(); 
    bool IsCalibration();
    uint8_t sleep();
    uint8_t wakeUp();  
    uint8_t resetMCU(); 
    uint8_t restoreDefault(); 
    uint8_t saveSettings(); 

    uint8_t getDistanceMode();
    uint8_t getMeasureMode(); 
    uint16_t getTimingBudgetInMs(); 
    uint16_t getIntermeasurementPeriod(); 
    uint8_t getRoiRegion(); 
    void getThshold(uint16_t &upperLimit, uint16_t &lowerLimit); 
    uint16_t getLigthKcps(); 
    uint16_t getFWver(); 
    uint16_t getFactoryVal(); 
    
    uint8_t setDistanceModeLong(); 
    uint8_t setDistanceModeShort(); 
    uint8_t setMeasureMode(uint8_t measureMode); 
    uint8_t setTimingBudgetInMs(uint16_t timingBudget); 
    uint8_t setIntermeasurementPeriod(uint16_t intermeasurement); 
    uint8_t setRoiRegion(uint8_t region); 
    uint8_t setThshold(uint16_t upperLimit, uint16_t lowerLimit);
    uint8_t setUartBuard(uint8_t buard);
    uint8_t setI2CAddress(uint8_t i2c_addr); 
                                       
  private:  
    uint8_t checksum(uint8_t len,uint8_t data[]);          
    void writeBytes(uint8_t wbuf[], uint8_t wlen);
    uint8_t readBytes(uint8_t rbuf[], uint8_t rlen, uint16_t timeOut = 100);
    uint8_t _intPin;
    uint8_t _rxPin;
    uint8_t _txPin;
    uint8_t _ModeSlect;
    uint8_t _i2c_addr;
    uint8_t _ID = 0x07;
    HardwareSerial *_hardSerial = NULL;
    SoftwareSerial *_softSerial = NULL ;
    TwoWire        *_wire = NULL;
};

#endif
