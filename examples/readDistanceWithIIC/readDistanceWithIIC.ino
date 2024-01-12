/*****************************************************************
File:         readDistanceWithIIC.ino
Description:  1. Set the measurement mode to long distance mode and ask the measurement distance every 200ms.
              2. Turn on the serial monitor. The measurement distance is displayed on the serial monitor.
******************************************************************/
#include "BM42S5321-1.h"
BM42S5321_1     BM42(2,&Wire); //Please uncomment out this line of code if you use HW Serial4 on BMduino
//BM42S5321_1     BM42(22,&Wire1); //Please uncomment out this line of code if you use HW Serial4 on BMduino
//BM42S5321_1     BM42(25,&Wire2); //Please uncomment out this line of code if you use HW Serial4 on BMduino
uint16_t dataState;
uint16_t Distance;
void setup() 
{
  Serial.begin(9600);
  BM42.begin();  //Module initialization 
  BM42.setDistanceModeLong(); //Select a long distance measurement mode
  BM42.setMeasureMode(ContinuousRanging);//Select the ranging mode as continuous measurement
  BM42.startMeasure(); //Start ranging
}

void loop()
{
  Distance = BM42.readDistance(dataState); //Acquired detection range
  if(dataState == 0) //Determine whether the distance data is reliable
  {
   Serial.print("Distance(mm):");
   Serial.println(Distance); //Displays the detected distance
  }
  else
  {
   Serial.println("Distance data is unreliable"); //Display distance data is not reliable
  }
  delay(200);
}
