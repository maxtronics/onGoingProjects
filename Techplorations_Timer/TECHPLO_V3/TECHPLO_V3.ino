#include <Wire.h>
#include <TimeLib.h>;                                             //Library for Epoch Timer
#include <SPI.h>                                                  //SD Card Module
#include <SD.h>   
#include <SoftwareSerial.h>

SoftwareSerial espSerial(5, 6);
SoftwareSerial nanoSerial(8, 9);


#define EEPROM_I2C_ADDRESS 0x57
#define DS3231_I2C_ADDRESS 0x68

//PIN DEFINITIONS
#define engineON_trig  2                                        //Interrupt trigger for engine ON
#define engineOFF_trig 3                                        //Interrupt trigger for engine OFF
#define trigPin        2
#define stateIndicator 7
#define sdPin      10   
byte x = 0;
byte STATE = 0;                                                   // 0 - From Power Down
                                                                  // 1 - Powered up and Engine is Off
                                                                  // 2 - Powered up and Engine is On
                                                                  // 3 - Engine Off from Engine On
                                                                  // 4 - Engine On from Engine Off
bool isIntActive = true;   
bool saveInitOnTime = true;
bool saveRuntime = false;
bool isInitMode =false;
unsigned long int engineOnTime, currentRuntime;
float totalRuntime, efficiency; 
byte webDelay = 0;
byte nanoDelay = 0;



   

//timer vals
byte g_hour, g_min, g_sec, g_day, g_month, g_year;
unsigned long previousMillis = 0;  
const long interval = 1000;  
const int unitDivisor = 1.0;  
/*  Store unit
 *   1 = Seconds
 *   60 = minutes
 *   3600 = hours
 */ 

void setup()
{
  Wire.begin();                                      //initialize i2c bus
  Serial.begin(9600);
  Wire.setClock(100000); 
  espSerial.begin(115200);
  nanoSerial.begin(9600);
  

  pinMode(engineON_trig, INPUT_PULLUP);
  pinMode(engineOFF_trig, INPUT_PULLUP);
  pinMode(stateIndicator, OUTPUT);
 // attachInterrupt(digitalPinToInterrupt(engineON_trig), engineOn, FALLING);
 // attachInterrupt(digitalPinToInterrupt(engineOFF_trig), engineOff,  RISING);


  if(isInitMode){
    saveToEEPROM("0");
    delay(1000);
    saveStartTimeToEEPROM(String(getEpochTime()));
    delay(1000);
  }

  initSD();
  
  totalRuntime = readFromEEPROM().toFloat();
  Serial.print("TOTAL RECEIVED: "); Serial.println(totalRuntime);
 

  // set the initial time here:
  // DS3231 seconds, minutes, hours, day, date, month, year
  //setDS3231time(30,42,23,5,11,04,20);

  
  
  switch(digitalRead(trigPin)){
    case 0:
      STATE = 2;
      digitalWrite(stateIndicator, HIGH);
      if(saveInitOnTime == true){
        saveInitOnTime = false;
        engineOnTime = getEpochTime();
        Serial.println("ON INIT");
        datalogEvent("ON");
      }
      break;
    case 1:
      digitalWrite(stateIndicator, LOW);
      Serial.println("OFF INIT");
      STATE = 1;
      break;
  }

}

void loop()
{
  switch(STATE){
    case 4:          // 3 - Engine Off from Engine On                                                

      digitalWrite(stateIndicator, HIGH);
      if(saveInitOnTime == true){
        saveInitOnTime = false;
        engineOnTime = getEpochTime();
        datalogEvent("ON");
      }
      delay(1000);
      isIntActive = true; 
      break;
      
    case 3:          // 4 - Engine On from Engine Off 

      digitalWrite(stateIndicator, LOW);
      if(saveRuntime == true){
        saveRuntime = false;
        currentRuntime = getEpochTime() - engineOnTime;
        totalRuntime = totalRuntime + ( currentRuntime / unitDivisor );
        saveToEEPROM(String(totalRuntime, 2));

        efficiency = totalRuntime / (getEpochTime() - atol(readStartTimeEEPROM().c_str()));
        datalogEvent("OFF");
      }     
      delay(1000);
      isIntActive = true;
      break;
  }

  
   unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    //displayTime();
     
    if(STATE == 2 || STATE == 4){
      //ENGINE ON
      if(saveInitOnTime == false){
        currentRuntime = getEpochTime() - engineOnTime;
      }
      else{
        currentRuntime = 0;
      }
       efficiency = (totalRuntime + currentRuntime) / (getEpochTime() - atol(readStartTimeEEPROM().c_str()));
      // Serial.print("ENGINE TOTAL RUNTIME: "); Serial.print(totalRuntime + currentRuntime);  Serial.print("\t");
      // Serial.print("CURRENT RUNTIME: "); Serial.print(currentRuntime); Serial.print("\t");
      // Serial.print("EFFICIENCY: "); Serial.print(efficiency * 100); Serial.println("%");

      
     }
     else if(STATE == 1 || STATE == 3){
      //ENGINE ON
      currentRuntime = 0;
      efficiency = (totalRuntime + currentRuntime) / (getEpochTime() - atol(readStartTimeEEPROM().c_str()));
     if(saveRuntime == false){
     // Serial.print("ENGINE TOTAL RUNTIME: "); Serial.print(totalRuntime + currentRuntime);  Serial.print("\t");
     // Serial.print("CURRENT RUNTIME: "); Serial.print(currentRuntime); Serial.print("\t");
     // Serial.print("EFFICIENCY: "); Serial.print(efficiency * 100); Serial.println("%");
     
  
     }
     
     }

     if(nanoDelay >= 2){
        nanoDelay = 0;
       String data = String(totalRuntime + currentRuntime);
       nanoSerial.listen();
       nanoSerial.println(data);
       Serial.println(data);
       delay(100);
     }
     else{
      nanoDelay++;
     }
     

    

       

     if(webDelay >= 180){
        webDelay = 0;
       String data = "!" + String(totalRuntime + currentRuntime) + ":" + String(currentRuntime) + ":" + String(efficiency * 100.0) + "<";
       espSerial.listen();
       espSerial.println(data);
       Serial.println(data);
       delay(100);
     }
     else{
      webDelay++;
     }
  
  }

  





  

  if((digitalRead(trigPin) == 0)){
    
    if(STATE == 4){
      //do nothing
    }
    else if(STATE == 1){
      STATE = 4;
    }
    else if(STATE == 3){
      //correct the state 
      // this error can be cause by noise interruptions          
      saveInitOnTime = true;
      STATE = 4;
      //Serial.println("ON2");
      digitalWrite(stateIndicator, HIGH);
    }
  }
  else if((digitalRead(trigPin) == 1)){
    
    if(STATE == 3){
      //do nothing
    }
    else if(STATE == 2){
      STATE = 3;

      saveRuntime = true;
     // Serial.println("OFF2");
      digitalWrite(stateIndicator, LOW);
    }
    else if(STATE == 4){
      //correct the state 
      // this error can be cause by noise interruptions
      saveRuntime = true;
      
      STATE = 3;
      //Serial.println("OFF2");
      digitalWrite(stateIndicator, LOW);
    }
  }
  
}
