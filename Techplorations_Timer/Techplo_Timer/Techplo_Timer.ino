#include <TimeLib.h>;                                             //Library for Epoch Timer
#include "Wire.h"                                                 //RTC
#include <EEPROM.h>
#include <SPI.h>                                                  //SD Card Module
#include <SD.h>    
#include <Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial webSerial(5, 6);
#define DS3231_I2C_ADDRESS 0x68                   
#define TOTAL_ACCUM_TIME_IN_HOUR_ADDR 10

//Pin definitions
#define trigPin_NC 2                                        //Interrupt trigger for engine ON
#define trigPin_NO 3                                        //Interrupt trigger for engine OFF
#define enginePin  4                                        //Pin that detects current state off Engine
#define sdPin      10                                       //CS pin for SD

unsigned long previousMillis = 0;
const long interval = 1000;   
unsigned long currentMillis;


bool isEngineOn = false;
bool displayTimer = false;
int g_hour, g_min, g_sec, g_day, g_month, g_year, g_dayOfWeek;
unsigned long int engineOnTime, engineOffTime;                    // handles epoc time when engine os
unsigned long int currentTimeRendered, totalTimeRenderedSinceLastCycle;
double totalTimeRenderedSinceLastCycle_hours;



int STATE = 0;                                                    // 0 - From Power Down
                                                                  // 1 - Powered up and Engine is Off
                                                                  // 2 - Powered up and Engine is On
                                                                  // 3 - Engine Off from Engine On
                                                                  // 4 - Engine On from Engine Off
                                                                  // 5 - Engine is On from power up

int webTimer = 0;


void setup(){
  Wire.begin();
  Serial.begin(9600);
  webSerial.begin(115200);

  pinMode(trigPin_NC, INPUT_PULLUP);
  pinMode(trigPin_NO, INPUT_PULLUP);
  pinMode(enginePin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(trigPin_NO), engineOff, RISING);
  attachInterrupt(digitalPinToInterrupt(trigPin_NC), engineOn,  FALLING);

  initSD();
  

  // set the initial time here:
  // DS3231 seconds, minutes, hours, day, date, month, year
  //setDS3231time(30, 35, 0, 7, 5, 4, 20);

  String totalTimeRenderedSinceLastCycle_retrieved = read_String(TOTAL_ACCUM_TIME_IN_HOUR_ADDR);
  float totalTimeRenderedSinceLastCycle_float = totalTimeRenderedSinceLastCycle_retrieved.toFloat();
  totalTimeRenderedSinceLastCycle = totalTimeRenderedSinceLastCycle_float * 3600;
 // Serial.print("TOTAL TIME SINCE LAST CYCLE: "); Serial.println(totalTimeRenderedSinceLastCycle);

  int engineState = digitalRead(enginePin);
  if(engineState == 1){
    STATE = 1;
    Serial.println("STATE = 1");
  }
  else if(engineState == 0){
    STATE = 2;
    Serial.println("STATE = 2");
  }

  // initOLED();
   
}

bool isExecuted = false;

void loop(){

  switch(STATE){
    case 0:
      break;
      
    case 2:
      if(!isExecuted){
        STATE = 5;
        isExecuted = true;
        engineOnTime = getEpochTime();
        datalogEvent("ON");
      }
      break;

    case 3:
      if(!isExecuted){
        isExecuted = true;
        engineOffTime = getEpochTime();
        totalTimeRenderedSinceLastCycle = totalTimeRenderedSinceLastCycle + currentTimeRendered;         //update last known total time from last engine off
        totalTimeRenderedSinceLastCycle_hours = totalTimeRenderedSinceLastCycle / 3600.00;               //convert it to hours
        Serial.print(totalTimeRenderedSinceLastCycle); Serial.println(" / 3600");
        Serial.print("SAVING.... "); Serial.println(totalTimeRenderedSinceLastCycle_hours, 6);
        writeString(TOTAL_ACCUM_TIME_IN_HOUR_ADDR, String(totalTimeRenderedSinceLastCycle_hours, 6));    //save to EEPROM 6 decimal places for accuracy in seconds
        datalogEvent("OFF");
        
      }
      break;
      
    case 4:
      if(!isExecuted){
        isExecuted = true;
        engineOnTime = getEpochTime();
        Serial.println(getDateAndTime());

        datalogEvent("ON");
        
      }
      break;
      
    case 5:
      if(!isExecuted){
        isExecuted = true;
        engineOnTime = getEpochTime();
        Serial.println(getDateAndTime());

        datalogEvent("ON");
        
      }
      break;
      
  }

 
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {                                                        //updates everything every second
    previousMillis = currentMillis;

    
     
    
    
    if(STATE == 4 || STATE == 5){
      currentTimeRendered = getEpochTime() - engineOnTime;                                                 //get running time since latest engine On
      unsigned long int runningTimeTotal = totalTimeRenderedSinceLastCycle + currentTimeRendered;          //compute full - life running time of machine
      
      Serial.print("TOTAL TIME: ");   Serial.print(runningTimeTotal); Serial.print("\t");
      Serial.print("RUNNING TIME: "); Serial.println(currentTimeRendered);
   //   printToOLED(String((runningTimeTotal / 3600.0), 3));

      webTimer = webTimer + 1;
      if(webTimer >= 30){
        webTimer = 0;

        String data = "!" + String((runningTimeTotal / 3600.0), 3) + ":<";
        webSerial.println(data);
        
      }
     
    
    }
    else if(STATE == 3){
      Serial.print("TOTAL TIME: ");   Serial.print(totalTimeRenderedSinceLastCycle); Serial.print("\t");  //if Engine is Of, Running time is 0, total time is static
      Serial.println("RUNNING TIME: 0");
//      printToOLED(String((totalTimeRenderedSinceLastCycle / 3600.0), 3));
       webTimer = webTimer + 1;
        if(webTimer >= 30){
          webTimer = 0;
          String data = "!" + String((totalTimeRenderedSinceLastCycle / 3600.0), 3) + ":<";
          webSerial.println(data);
        }
    }

    
  }

  delay(10);
}
