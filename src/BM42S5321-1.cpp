/***********************************************************
File:           BM42S5321-1.cpp
Author:         BESTMODULES
Description:    Define classes and required variables
Version:        V1.0.1   -- 2024-01-02
***********************************************************/
#include "BM42S5321-1.h"
/**********************************************************
Description: Constructor
Parameters:  
             intPin:INT PIN
             *theSerial: Wire object if your board has more than one UART interface      
                         parameter range:&Serial、&Serial1、&Serial2、&Serial3、&Serial4
Return:          
Others:     
**********************************************************/
BM42S5321_1::BM42S5321_1(uint8_t intPin,HardwareSerial *theSerial)
{
    _softSerial = NULL ;
    _wire = NULL;
    _hardSerial = theSerial;
    _intPin = intPin;
    _ModeSlect = UART_MODE;
}

/**********************************************************
Description: Constructor
Parameters:  
             intPin:INT PIN
             rxPin : Receiver pin of the UART
             txPin : Send signal pin of UART         
Return:          
Others:   
**********************************************************/
BM42S5321_1::BM42S5321_1(uint8_t intPin,uint8_t rxPin,uint8_t txPin)
{
    _hardSerial = NULL;
    _wire = NULL;
    _intPin = intPin;
    _rxPin = rxPin;
    _txPin = txPin;
    _softSerial = new SoftwareSerial(_rxPin,_txPin);
    _ModeSlect = UART_MODE;
}

/**********************************************************
Description: Constructor
Parameters:  
             intPin: INT PIN
             *theWire: Wire object if your board has more than one IIC interface      
                         parameter range:&wire、&wire1、&wire2
             i2c_addr: IIC communication address            
Return:          
Others:     
**********************************************************/
BM42S5321_1::BM42S5321_1(uint8_t intPin,TwoWire *theWire,uint8_t i2c_addr)
{
    _hardSerial = NULL;
    _softSerial = NULL ;
    _wire = theWire;
    _intPin = intPin;
    _i2c_addr = i2c_addr;
    _ModeSlect = IIC_MODE;
}

/**********************************************************
Description: Initialize mode
Parameters:  buard: Select the module communication rate
                   BUARD_9600: 9600bps
                   BUARD_115200:115200bps          
Return:      void        
Others:      Used when selecting UART communication
**********************************************************/
void BM42S5321_1::begin(uint8_t buard)
{ 
  pinMode(_intPin,INPUT_PULLUP);
  bool x=0;
  uint32_t BR_Parameter[2] = {9600,115200};
  uint8_t temp=0;
  delay(400);
  if(_ModeSlect == UART_MODE)
  {
   if(_softSerial != NULL)
   {
    do
    {
      _softSerial->begin(BR_Parameter[temp]);
      _softSerial->setTimeout(107);
     if(!setUartBuard(buard))
     {
      x = 1;
      _softSerial->begin(BR_Parameter[buard]); 
     }
    else 
     temp++;
    }
    while(!x);
    }
      else
     {
      do
      {
        _hardSerial->begin(BR_Parameter[temp]);
         _hardSerial->setTimeout(107); 
        if(!setUartBuard(buard))
        {
          x = 1;
          _hardSerial->begin(BR_Parameter[buard]); 
        }
        else temp++;
      }while(!x); 
    }
    } 
}

/**********************************************************
Description: Initialize mode
Parameters:          
Return:      void        
Others:      Used when selecting IIC communication
**********************************************************/
void BM42S5321_1::begin()
{ 
  pinMode(_intPin,INPUT_PULLUP);
  delay(400);
  if(_ModeSlect == IIC_MODE)
    {
      if(_wire != NULL)
      {
        _wire->begin();
        _wire->setClock(100000);
      }
    }
}

/**********************************************************
Description: Get INT Status
Parameters:         
Return:     Returns the INT Status  0:INT output low level 
                                    1:INT output high level  
Others:     
**********************************************************/
uint8_t BM42S5321_1::getINT()
{
     return (digitalRead(_intPin));
}

/**********************************************************
Description:  Start the distance measurement
Parameters:          
Return:      
              0:SUCCESS  
              1:FAIL
Others:      
**********************************************************/
uint8_t BM42S5321_1::startMeasure()
{
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x0A,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,8) == CHECK_OK)
  {
   delay(2);
   return buff[6];
  }
   delay(2);
   return 1; 
}

/**********************************************************
Description:  Stop measuring distance
Parameters:          
Return:       
              0:SUCCESS  
              1:FAIL 
Others: 
**********************************************************/
uint8_t BM42S5321_1::stopMeasure()
{
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x0B,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,8) == CHECK_OK)
  {
   delay(2);
   return buff[6];
  }
   delay(2);
   return 1; 
}

/**********************************************************
Description:  Read the detected distance
Parameters:   &dataState: Store distance data status
                         (dataState&0x0100) =0 :The current data is the latest 
                         (dataState&0x0100) =1 :Data not updated
                         (dataState&0x0001) =0 :Reliable data
                         (dataState&0x0001) !=0 :Unreliable data  
Return:       distance:detected distance(Unit: mm)    
Others:  
**********************************************************/
uint16_t BM42S5321_1::readDistance(uint16_t &dataState)
{
  uint16_t Distance = 0;
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x10,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[11] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,11) == CHECK_OK)
  {
   Distance = (buff[6]<<8)+buff[7];
   dataState = (buff[8]<<8)+buff[9];
  }
   delay(2);
   return Distance; 
}

/**********************************************************
Description: Offset calibration 
Parameters:              
Return:       
              0: Start calibration
              1: Setup failure  
Others:      Place a barrier that is at least 10cmx10cm larger at 14cm ,the time is about 12 second
**********************************************************/
uint8_t BM42S5321_1::calibrateOffset()
{
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x0C,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,8) == CHECK_OK)
  {
   delay(2);
   return buff[6];
  }
   delay(2);
   return 1; 
}

/**********************************************************
Description: Gets whether the calibration is complete
Parameters:              
Return:      
             true: Calibration complete
             false: Calibration incomplete       
Others:      Do not set other functions until calibration is complete
**********************************************************/
bool BM42S5321_1::IsCalibration()
{
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x19,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,8) == CHECK_OK)
  {
   if(buff[6] == 0)
    {
     delay(2);
     return true; 
    }
  }
   delay(2);
   return false;
}

/**********************************************************
Description: Sleep module
Parameters:              
Return:  
             0:SUCCESS  
             1:FAIL         
Others:  
**********************************************************/
uint8_t BM42S5321_1::sleep()
{
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x22,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,8) == CHECK_OK)
  {
   delay(2);
   return buff[6];
  }
   delay(2);
   return 1; 
}

/**********************************************************
Description: wakeUp module
Parameters:              
Return: 
             0:SUCCESS  
             1:FAIL           
Others:  
**********************************************************/
uint8_t BM42S5321_1::wakeUp()
{
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x23,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,8) == CHECK_OK)
  {
   delay(400);
   return buff[6];
  }
   delay(400);
   return 1; 
}

/**********************************************************
Description: Reset MCU
Parameters:              
Return: 
             0:SUCCESS  
             1:FAIL           
Others:      The sensor has no reset function, so only the MCU is reset
**********************************************************/
uint8_t BM42S5321_1::resetMCU()
{
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x24,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,8) == CHECK_OK)
  {
   delay(450);
   return buff[6];
  }
   delay(450);
   return 1; 
}

/**********************************************************
Description: factory data reset
Parameters:              
Return: 
             0:SUCCESS  
             1:FAIL           
Others:      need to power it on again
**********************************************************/
uint8_t BM42S5321_1::restoreDefault()
{
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x25,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,8) == CHECK_OK)
  {
   delay(110);
   return buff[6];
  }
   delay(110);
   return 1; 
}

/**********************************************************
Description: Save Settings 
Parameters:              
Return:
             0:SUCCESS  
             1:FAIL            
Others:  
**********************************************************/
uint8_t BM42S5321_1::saveSettings()
{
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x21,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(200);
  if(readBytes(buff,8) == CHECK_OK)
  {
   delay(6);
   return buff[6];
  }
   delay(6);
   return 1; 
}

/**********************************************************
Description:  Get ranging mode
Parameters:   
Return:       distanceMode
                          distanceMode = 0: long range mode 
                          distanceMode = 1: short range mode            
Others:  
**********************************************************/
uint8_t BM42S5321_1::getDistanceMode()
{
  uint8_t DistanceMode =0;
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x11,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,8) == CHECK_OK)
  {
   DistanceMode = buff[6];
  }
   delay(2);
   return DistanceMode; 
}

/**********************************************************
Description: get ranging mode
Parameters:              
Return:      MeasureMode
                        MeasureMode = 0: Continuous ranging
                        MeasureMode = 1: Single ranging
                        MeasureMode = 2: Low power ranging   
Others:  
**********************************************************/
uint8_t BM42S5321_1::getMeasureMode()
{
  uint8_t MeasureMode = 0;
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x12,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,8) == CHECK_OK)
  {
   MeasureMode = buff[6];
  }
   delay(2);
   return MeasureMode; 
}


/**********************************************************
Description:  Get the timing budget for a measurement
Parameters:   
Return:       timingBudget:20、33、50、100、200、500(Unit: millisecond)
Others:  
**********************************************************/
uint16_t BM42S5321_1::getTimingBudgetInMs()
{
  uint16_t TimingBudgetInMs = 0;
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x13,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[9] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,9) == CHECK_OK)
  {
   TimingBudgetInMs = (buff[6]<<8)+buff[7];
  }
  delay(2);
  return TimingBudgetInMs; 
}

/**********************************************************
Description:  Get the measurement interval
Parameters:   
Return:       intermeasurement:20~1000(Unit: millisecond)    
Others:  
**********************************************************/
uint16_t BM42S5321_1::getIntermeasurementPeriod()
{
  uint16_t IntermeasurementPeriod = 0;
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x14,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[9] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,9) == CHECK_OK)
  {
   IntermeasurementPeriod = (buff[6]<<8)+buff[7];
  }
  delay(2);
  return IntermeasurementPeriod; 
}

/**********************************************************
Description: Get sensor ROI
Parameters:              
Return:      RoiRegion:
                       RoiRegion = 0: 16x16 
                       RoiRegion = 1: 8x8
                       RoiRegion = 2: 4x4                       
Others:  
**********************************************************/
uint8_t BM42S5321_1::getRoiRegion(void)
{
  uint8_t RoiRegion = 0;
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x15,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,8) == CHECK_OK)
  {
   RoiRegion = buff[6];
  }
  delay(2);
  return RoiRegion; 
}

/**********************************************************
Description: get alarm upper and lower limits
Parameters:  
             &upperLimit: Storage alarm upper limit
             &lowerLimit: Storage alarm lower limit            
Return:      void        
Others: 
**********************************************************/
void BM42S5321_1::getThshold(uint16_t &upperLimit, uint16_t &lowerLimit)
{
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x16,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[11] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,11) == CHECK_OK)
  {
   lowerLimit = (buff[6]<<8)+buff[7];
   upperLimit = (buff[8]<<8)+buff[9];
  }
  delay(2);
}

/**********************************************************
Description: Gain light intensity
Parameters:              
Return:      LigthKcps:Light intensity     
Others: 
**********************************************************/
uint16_t BM42S5321_1::getLigthKcps()
{
  uint16_t LigthKcps = 0;
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x17,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[9] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,9) == CHECK_OK)
  {
   LigthKcps = (buff[6]<<8)+buff[7];
  }
  delay(2);
  return LigthKcps; 
}

/**********************************************************
Description: Get version number
Parameters:              
Return:      FWver: version number    
Others:      
**********************************************************/
uint16_t BM42S5321_1::getFWver()
{
  uint16_t FWver = 0;
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x1A,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[9] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,9) == CHECK_OK)
  {
   FWver = (buff[6]<<8)+buff[7];
  }
  delay(2);
  return FWver; 
}

/**********************************************************
Description: Get factory calibration values
Parameters:              
Return:      FactoryVal: factory calibration values      
Others: 
**********************************************************/
uint16_t BM42S5321_1::getFactoryVal()
{
  uint16_t FactoryVal = 0;
  uint8_t sendBuf[7] = {0x42,0x4D,0x67,0x00,0x1C,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = checksum(7,sendBuf);
  uint8_t buff[9] = {0};
  writeBytes(sendBuf,7);
  delay(10);
  if(readBytes(buff,9) == CHECK_OK)
  {
   FactoryVal = (buff[6]<<8)+buff[7];
  }
  delay(2);
  return FactoryVal; 
}

/**********************************************************
Description:  Set ranging mode to long range mode
Parameters:     
Return:       
              0:SUCCESS  
              1:FAIL    
Others:       Measuring range: 40~4000(Unit: mm)     
**********************************************************/
uint8_t BM42S5321_1::setDistanceModeLong()
{
    uint8_t sendBuf[8] = {0x42,0x4D,0x67,0x00,0x01,0x01,0x00,0x00};
    sendBuf[3] = _ID;
    sendBuf[7] = checksum(7,sendBuf);
    uint8_t buff[8] = {0};
    writeBytes(sendBuf,8);
    delay(10);
    if(readBytes(buff,8) == CHECK_OK)
     {
      delay(35);
      return buff[6];
     }
      delay(2);
     return 1; 
}

/**********************************************************
Description:  Set ranging mode to short range mode
Parameters:        
Return:       
              0:SUCCESS  
              1:FAIL   
Others:       Measuring range: 40~1300(Unit: mm)
              Environmental conditions: Ambient light darkness       
**********************************************************/
uint8_t BM42S5321_1::setDistanceModeShort()
{
    uint8_t sendBuf[8] = {0x42,0x4D,0x67,0x00,0x01,0x01,0x01,0x00};
    sendBuf[3] = _ID;
    sendBuf[7] = checksum(7,sendBuf);
    uint8_t buff[8] = {0};
    writeBytes(sendBuf,8);
    delay(10);
    if(readBytes(buff,8) == CHECK_OK)
     {
       delay(35);
      return buff[6];
     }
       delay(2);
      return 1; 
}

/**********************************************************
Description: Set range mode
Parameters:  measureMode: range mode
                         0x00: Continuous Ranging 
                         0x01: Single Ranging 
                         0x02: PowerSave Ranging   
Return:       
             0:SUCCESS  
             1:FAIL           
Others:      
**********************************************************/
uint8_t BM42S5321_1::setMeasureMode(uint8_t measureMode)
{
    uint8_t sendBuf[8] = {0x42,0x4D,0x67,0x00,0x02,0x01,0x00,0x00};
    sendBuf[3] = _ID;
    sendBuf[6] = measureMode;
    sendBuf[7] = checksum(7,sendBuf);
    uint8_t buff[8] = {0};
    writeBytes(sendBuf,8);
    delay(10);
    if(readBytes(buff,8) == CHECK_OK)
     {
      delay(35);
      return buff[6];
     }
      delay(2);
      return 1; 
}

/**********************************************************
Description:  Set a single measurement duration
Parameters:   timingBudget: interval time(20、33、50、100、200、500(Unit: millisecond))      
Return:       
              0:SUCCESS  
              1:FAIL          
Others:       The interval time should be greater than or equal to the time budget      
**********************************************************/
uint8_t BM42S5321_1::setTimingBudgetInMs(uint16_t timingBudget)
{
    uint8_t sendBuf[9] = {0x42,0x4D,0x67,0x00,0x03,0x02,0x00,0x00,0x00};
    sendBuf[3] = _ID;
    sendBuf[6] = uint8_t(timingBudget >> 8);
    sendBuf[7] = uint8_t(timingBudget);
    sendBuf[8] = checksum(8,sendBuf);
    uint8_t buff[8] = {0};
    writeBytes(sendBuf,9);
    delay(10);
    if(readBytes(buff,8) == CHECK_OK)
     {
      delay(35);
      return buff[6];
     }
      delay(2);
      return 1; 
}

/**********************************************************
Description:  Set the measurement interval
Parameters:   intermeasurement:time budget(20~5000(Unit: millisecond))      
Return:       
              0:SUCCESS  
              1:FAIL         
Others:       The time budget should be less than or equal to the interval time       
**********************************************************/
uint8_t BM42S5321_1::setIntermeasurementPeriod(uint16_t intermeasurement)
{
   uint8_t sendBuf[9] = {0x42,0x4D,0x67,0x00,0x04,0x02,0x00,0x00,0x00};
   sendBuf[3] = _ID;
   sendBuf[6] = uint8_t(intermeasurement >> 8);
   sendBuf[7] = uint8_t(intermeasurement);
   sendBuf[8] = checksum(8,sendBuf);
   uint8_t buff[8] = {0};
   writeBytes(sendBuf,9);
   delay(10);
   if(readBytes(buff,8) == CHECK_OK)
    {
     delay(35);
     return buff[6];
    }
     delay(2);
     return 1; 
}

/**********************************************************
Description: Set sensor ROI
Parameters:  region: sensor ROI
                   0x02: RoiRegion_4x4
                   0x01: RoiRegion_8x8
                   0x00: RoiRegion_16x16        
Return:       
              0:SUCCESS  
              1:FAIL            
Others:      
**********************************************************/
uint8_t BM42S5321_1::setRoiRegion(uint8_t region)
{
    uint8_t sendBuf[8] = {0x42,0x4D,0x67,0x00,0x05,0x01,0x00,0x00};
    sendBuf[3] = _ID;
    sendBuf[6] = region;
    sendBuf[7] = checksum(7,sendBuf);
    uint8_t buff[8] = {0};
    writeBytes(sendBuf,8);
    delay(10);
    if(readBytes(buff,8) == CHECK_OK)
     {
      delay(35);
      return buff[6];
     }
      delay(2);
      return 1; 
}

/**********************************************************
Description: Set alarm upper and lower limits
Parameters:  
             upperLimit: Upper alarm limit(40~4000)
             lowerLimit: Upper alarm limit(40~4000)          
Return:       
             0:SUCCESS  
             1:FAIL            
Others:      upperLimit > lowerLimit 
**********************************************************/
uint8_t BM42S5321_1::setThshold(uint16_t upperLimit, uint16_t lowerLimit)
{
   uint8_t sendBuf[11] = {0x42,0x4D,0x67,0x00,0x06,0x04,0x00,0x00,0x00,0x00,0x00};
   sendBuf[3] = _ID;
   sendBuf[6] = uint8_t(lowerLimit >> 8);
   sendBuf[7] = uint8_t(lowerLimit);
   sendBuf[8] = uint8_t(upperLimit >> 8);
   sendBuf[9] = uint8_t(upperLimit);
   sendBuf[10] = checksum(10,sendBuf);
   uint8_t buff[8] = {0};
   writeBytes(sendBuf,11);
   delay(10);
   if(readBytes(buff,8) == CHECK_OK)
    {
     delay(2);
     return buff[6];
    }
     delay(2);
     return 1; 
}

/**********************************************************
Description: Set baud rate
Parameters:  buard: baud rate
                  0x00: 9600bps  
                  0x01: 115200bps           
Return:       
             0:SUCCESS  
             1:FAIL            
Others:      
**********************************************************/
uint8_t BM42S5321_1::setUartBuard(uint8_t buard)
{
   uint8_t sendBuf[11] = {0x42,0x4D,0x67,0x00,0x07,0x04,0x00,0x00,0x00,0x00,0x00};
   sendBuf[3] = _ID;
   if(buard == 0)
   {
    sendBuf[6] = 0x00;
    sendBuf[7] = 0x00;
    sendBuf[8] = 0x25;
    sendBuf[9] = 0x80;
   }
   else
   {
    sendBuf[6] = 0x00;
    sendBuf[7] = 0x01;
    sendBuf[8] = 0xC2;
    sendBuf[9] = 0x00;
   }
   sendBuf[10] = checksum(10,sendBuf);
   uint8_t buff[8] = {0};
   writeBytes(sendBuf,11);
   delay(10);
   if(readBytes(buff,8) == CHECK_OK)
    {
     delay(2);
     return buff[6];
    }
     delay(2);
     return 1; 
}

/**********************************************************
Description: Set module address
Parameters:  i2c_addr:module address(0~127)            
Return:       
             0:SUCCESS  
             1:FAIL         
Others:  
**********************************************************/
uint8_t BM42S5321_1::setI2CAddress(uint8_t i2c_addr)
{
  if(i2c_addr>127)
  {
   return 1;  
  }
  uint8_t sendBuf[8] = {0x42,0x4D,0x67,0x00,0x08,0x01,0x00,0x00};
  sendBuf[3] = _ID;
  sendBuf[6] = i2c_addr;
  sendBuf[7] = checksum(7,sendBuf);
  uint8_t buff[8] = {0};
  writeBytes(sendBuf,8);
  delay(10);
  _i2c_addr = i2c_addr;
  _ID = i2c_addr;
  if(readBytes(buff,8) == CHECK_OK)
  {
   delay(2);
   return buff[6];
  }
   delay(2);
   return 1; 
}
/**********************************************************
Description: checksum
Parameters:  
             len: Data length
             data[]: Data 
Return:      result: checksum
Others:           
**********************************************************/
uint8_t BM42S5321_1::checksum(uint8_t len,uint8_t data[])
{
  uint8_t a=0;
  uint8_t result=0;
  for(a=0;a<len;a++)
  result  += data[a];
  result = ~result+1;
  return result;
}

/**********************************************************
Description: writeBytes
Parameters:  wbuf[]:Variables for storing Data to be sent
             wlen:Length of data sent  
Return:      void
Others:
**********************************************************/
void BM42S5321_1::writeBytes(uint8_t wbuf[], uint8_t wlen)
{
 if(_ModeSlect==IIC_MODE)
 {
  if (_wire != NULL)
  {
    while (_wire->available() > 0)
    {
      _wire->read();
    }
    _wire->beginTransmission(_i2c_addr);
    _wire->write(wbuf, wlen);
    _wire->endTransmission();
  }
 }
 else if(_ModeSlect==UART_MODE)
 {
  /* Select SoftwareSerial Interface */
  if (_softSerial != NULL)
  {
    while (_softSerial->available() > 0)
    {
      _softSerial->read();
    }
    _softSerial->write(wbuf, wlen);
  }
  /* Select HardwareSerial Interface */
  else
  {
    while (_hardSerial->available() > 0)
    {
      _hardSerial->read();
    }
    _hardSerial->write(wbuf, wlen);
  }
 }
}

/**********************************************************
Description: readBytes
Parameters:  rbuf[]: Variables for storing Data to be obtained
             rlen: Length of data to be obtained
             timeOut: Timeout period
Return:      condition:
                      TIMEOUT_ERROR: Overtime
                      CHECK_OK: Check success
                      CHECK_ERROR: Check failure
Others:
**********************************************************/
uint8_t BM42S5321_1::readBytes(uint8_t rbuf[], uint8_t rlen, uint16_t timeOut)
{
  uint8_t i = 0, delayCnt = 0,checkSum = 0;
  /****** IIC MODE ******/
  if(_ModeSlect==IIC_MODE)
  {
    _wire->requestFrom(_i2c_addr, rlen);
    if (_wire->available() == rlen)
   {
     for (i = 0; i < rlen; i++)
     {
      rbuf[i] = _wire->read();
     }
   }
   else
   {
    return TIMEOUT_ERROR;
   }
 }
 
/****** UART MODE ******/
 else if(_ModeSlect==UART_MODE)
{
  if (_softSerial != NULL)
  {
    for (i = 0; i < rlen; i++)
    {
      delayCnt = 0;
      while (_softSerial->available() == 0)
      {
        if (delayCnt > timeOut)
        {
          return TIMEOUT_ERROR; // Timeout error
        }
        delay(1);
        delayCnt++;
      }
      rbuf[i] = _softSerial->read();
    }
  }

  else
  {
    for (i = 0; i < rlen; i++)
    {
      delayCnt = 0;
      while (_hardSerial->available() == 0)
      {
        if (delayCnt > timeOut)
        {
          return TIMEOUT_ERROR; // Timeout error
        }
        delay(1);
        delayCnt++;
      }
      rbuf[i] = _hardSerial->read();
    }
  }
 }
 
/* check Sum */
  for (i = 0; i < (rlen - 1); i++)
  {
    checkSum += rbuf[i];
  }
  checkSum = ((~checkSum)+1);
  if (checkSum == rbuf[rlen - 1])
  {
    return CHECK_OK; // Check correct
  }
  else
  {
    return CHECK_ERROR; // Check error
  }
}
