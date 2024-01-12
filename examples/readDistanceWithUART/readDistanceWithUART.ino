/*****************************************************************
File:         readDistanceWithUART.ino
Description:  1. Set the measurement mode to long distance mode and ask the measurement distance every 200ms.
              2. Turn on the serial monitor. The measurement distance is displayed on the serial monitor.
******************************************************************/
#include "BM42S5321-1.h"
BM42S5321_1     BM42(3,4,5); //rxPin=5,txPin=4,Please comment out this line of code if you don't use SW Serial
//BM42S5321_1     BM42(22,&Serial1); //Please uncomment out this line of code if you use HW Serial on BMduino
//BM42S5321_1     BM42(25,&Serial2); //Please uncomment out this line of code if you use HW Serial2 on BMduino
//BM42S5321_1     BM42(3,&Serial3); //Please uncomment out this line of code if you use HW Serial3 on BMduino
//BM42S5321_1     BM42(3,&Serial4); //Please uncomment out this line of code if you use HW Serial4 on BMduino
uint16_t dataState;
uint16_t Distance;
void setup() 
{
  Serial.begin(9600);
  BM42.begin(BUARD_9600);  //Module initialization
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
