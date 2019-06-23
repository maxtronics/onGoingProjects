#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX
//SoftwareSerial PICSerial(10, 11);
#include <TimeLib.h>;
#include <SPI.h>
#include <SD.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
String statusData;
#define alarmLED 3
#define statusPin 12
#define ledPin 13
#define btnAlarm  2
bool saveState = false;
String _status;
File myFile;
bool isAlarmOn = false;
bool printStatus = false;
unsigned long previousMillis = 0; 

unsigned long currentMillis;
bool readData  = false;
bool dataCapture = true;


enum _parseState {
  PS_DETECT_MSG_TYPE,

  PS_IGNORING_COMMAND_ECHO,

  PS_HTTPACTION_TYPE,
  PS_HTTPACTION_RESULT,
  PS_HTTPACTION_LENGTH,

  PS_HTTPREAD_LENGTH,
  PS_HTTPREAD_CONTENT
};

String dataRX = "";

byte parseState = PS_DETECT_MSG_TYPE;
char buffer[80];
byte pos = 0;
String gDate, gTime;
int contentLength = 0;

int g_hour, g_min, g_sec, g_day, g_month, g_year;

#include <stdio.h>
#include <DS1302.h>
namespace {

// Set the appropriate digital I/O pin connections. These are the pin
// assignments for the Arduino as well for as the DS1302 chip. See the DS1302
// datasheet:
//
//   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
const int kCePin   = 5;  // Chip Enable
const int kIoPin   = 6;  // Input/Output
const int kSclkPin = 7;  // Serial Clock

// Create a DS1302 object.
DS1302 rtc(kCePin, kIoPin, kSclkPin);

String dayAsString(const Time::Day day) {
  switch (day) {
    case Time::kSunday: return "Sunday";
    case Time::kMonday: return "Monday";
    case Time::kTuesday: return "Tuesday";
    case Time::kWednesday: return "Wednesday";
    case Time::kThursday: return "Thursday";
    case Time::kFriday: return "Friday";
    case Time::kSaturday: return "Saturday";
  }
  return "(unknown day)";
}

void printTime() {
  // Get the current time and date from the chip.
  Time t = rtc.time();

  g_hour = t.hr;
  g_min = t.min;
  g_sec = t.sec;
  g_day = t.date;
  g_month = t.mon;
  g_year = t.yr;

  // Name the day of the week.
  const String day = dayAsString(t.day);

  // Format the time and date and insert into the temporary buffer.
  char buf[50];
  snprintf(buf, sizeof(buf), "%s %04d-%02d-%02d %02d:%02d:%02d",
           String(day),
           t.yr, t.mon, t.date,
           t.hr, t.min, t.sec);

  // Print the formatted string to serial so we can see the time.
 // Serial.println(buf);

  String dataFromRTC = String(buf);
  Serial.println(dataFromRTC);
  int startIndex = dataFromRTC.indexOf(" ");
  int midIndex = dataFromRTC.indexOf(" ", startIndex + 1);
  int stopIndex = dataFromRTC.length();

      gDate = dataFromRTC.substring(startIndex + 1, midIndex);
      gTime = dataFromRTC.substring(midIndex + 1, midIndex + 9);
  
/*
  Serial.print("DATE: ");
  Serial.println(gDate);
  Serial.print("TIME: ");
  Serial.println(gTime);
*/
 
}

}  // namespace

void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

void parseATText(byte b) {

 //GSM.begin(9600);

  buffer[pos++] = b;

  if ( pos >= sizeof(buffer) )
    resetBuffer(); // just to be safe

  /*
   // Detailed debugging
   Serial.println();
   Serial.print("state = ");
   Serial.println(state);
   Serial.print("b = ");
   Serial.println(b);
   Serial.print("pos = ");
   Serial.println(pos);
   Serial.print("buffer = ");
   Serial.println(buffer);*/

  switch (parseState) {
  case PS_DETECT_MSG_TYPE: 
    {
      
      if ( b == '\n' )
        resetBuffer();
      else {        
        if ( pos == 3 && strcmp(buffer, "AT+") == 0 ) {
          parseState = PS_IGNORING_COMMAND_ECHO;
        }
        else if ( b == ':' ) {
          //Serial.print("Checking message type: ");
          //Serial.println(buffer);

          if ( strcmp(buffer, "+HTTPACTION:") == 0 ) {
            Serial.println("Received HTTPACTION");
            parseState = PS_HTTPACTION_TYPE;
          }
          else if ( strcmp(buffer, "+HTTPREAD:") == 0 ) {
            Serial.println("Received HTTPREAD");            
            parseState = PS_HTTPREAD_LENGTH;
          }
          resetBuffer();
        }
      }
    }
    break;

  case PS_IGNORING_COMMAND_ECHO:
    {
      if ( b == '\n' ) {
        Serial.print("Ignoring echo: ");
        Serial.println(buffer);
        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_TYPE:
    {
      if ( b == ',' ) {
        Serial.print("HTTPACTION type is ");
        Serial.println(buffer);
        parseState = PS_HTTPACTION_RESULT;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_RESULT:
    {
      if ( b == ',' ) {
        Serial.print("HTTPACTION result is ");
        Serial.println(buffer);
        parseState = PS_HTTPACTION_LENGTH;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_LENGTH:
    {
      if ( b == '\n' ) {
        Serial.print("HTTPACTION length is ");
        Serial.println(buffer);
        
        // now request content
        mySerial.print("AT+HTTPREAD=0,");
        mySerial.println(buffer);
        
        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPREAD_LENGTH:
    {
      if ( b == '\n' ) {
        contentLength = atoi(buffer);
        Serial.print("HTTPREAD length is ");
        Serial.println(contentLength);
        
        Serial.print("HTTPREAD content: ");
        
        parseState = PS_HTTPREAD_CONTENT;
        
        resetBuffer();

       
      }

      
    }
    break;

  case PS_HTTPREAD_CONTENT:
    {
      // for this demo I'm just showing the content bytes in the serial monitor
      Serial.write(b);
      
      dataRX =  String(buffer);
      contentLength--;
      
      if ( contentLength <= 0 ) {
       dataCapture = false;
       printStatus = true;
        // all content bytes have now been read

        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;
  }
}


void sendGSM(String msg, int waitMs = 500) {
  mySerial.println(msg);
  delay(waitMs);
  
  while(mySerial.available()) {
    parseATText(mySerial.read());
  }
}


void writeToSD(String _data, String _fileName){
   myFile = SD.open(_fileName, FILE_WRITE);
      String txtToSave = _data;
      
      if (myFile) {
        Serial.print("Writing to");
        Serial.print(_fileName);
        Serial.println(".txt");
        myFile.println(txtToSave);
        // close the file:
        myFile.close();
        Serial.println("done.");
        } 
      else {
    // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
        }
}

void setup()
{

  pinMode(ledPin, OUTPUT);
  pinMode(statusPin, INPUT_PULLUP);
  pinMode(alarmLED, OUTPUT);
  pinMode(btnAlarm, INPUT_PULLUP);

  digitalWrite(alarmLED, LOW);
  
  //Serial1.begin(9600);
  Serial.begin(9600);

   Serial.print("Initializing SD card...");

  if (!SD.begin(8)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(500);
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(500);
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(5000);
  


/*
  sendGSM("AT+CMGF=1", 1000); 
  sendGSM("AT+CMGS=\"+639959119334\"\r"); 
  sendGSM("ERNIE MAKULIT BAKO?"); 
  sendGSM((char)26); 
  //delay(10000);*/


   mySerial.begin(9600);
   
  sendGSM("AT+SAPBR=3,1,\"APN\",\"vodafone\"");  
  sendGSM("AT+SAPBR=1,1",3000);
  sendGSM("AT+HTTPINIT");  
  sendGSM("AT+HTTPPARA=\"CID\",1");

  delay(5000);

  rtc.writeProtect(false);
  rtc.halt(false);

  
 // Serial1.begin(9600);
 // Serial.println("START");

 // delay(3000);
 
}



String readFromSD(String _fileName){
      myFile = SD.open(_fileName);
      if (myFile) {
        
        char timeData;
        String data;
        // read from the file until there's nothing else in it:
        while (myFile.available()) {
          timeData = myFile.read();
          data = data + String(timeData);
        }

          //Serial.println(data);
        
  
         myFile.close();
         return  data;
      }
       else {
   
          Serial.println("error opening test.txt");
          return  "ERROR";
     
        }
}



void sendDataToWeb(String _status){
  
 
  String url = "AT+HTTPPARA=\"URL\",\"http://tractor.vsystemsapp.com/tracktor/api/send?data=test11:1,1,24:00,24:00,40.714224,-73.961452,7.8,5.4,11.2," + _status + "\"";
  Serial.print("SENDING DATA TO: ");
  Serial.println(url);
  sendGSM(url);
  sendGSM("AT+HTTPACTION=0");
}
bool isStarted = false;
bool isLogData = false;
bool isCAL = false;
void loop()
{ 
  
//Serial.begin(9600);
//GSM.begin(9600);

int x = digitalRead(statusPin);
if(isStarted == false){
  if(x == 1){
    _status = "1";
    isStarted = true;
    isLogData = true;
  }
}

if(isStarted == true){
  if(x == 0){
    _status = "0";
    isStarted = false;
    isLogData = true;
  }
}

int y = digitalRead(btnAlarm);
if(isCAL == false){
  if(y == 0){
   
    isCAL = true;
    
  }
}

if(isCAL == true){
  if(y == 1){
    
    isCAL = false;

  String url = "AT+HTTPPARA=\"URL\",\"http://tractor.vsystemsapp.com/tracktor/api/apply/preventive-maintenance?data=test11\"";
  Serial.print("SENDING CAL COMMAND...");
  Serial.println(url);
  sendGSM(url);
  sendGSM("AT+HTTPACTION=0");

   readData = true;
    
  }
}

//if((Serial1.available() > 0)){
//  _status = Serial1.readStringUntil('\n');
//  _status = Serial.readStringUntil('\n');
  if(isLogData == true)
  {
    isLogData = false;
  Serial.print("STATUS: ");
  Serial.println(_status);

  if(_status.equals("1") || _status.equals("0")){

    if(_status.equals("1")){
      digitalWrite(ledPin, HIGH);
      
      saveState = true;
      printTime();
      String dataToSave = ">" + gDate + "," + gTime + "," + "1";
      writeToSD(dataToSave, "DATALOGS.txt");
      delay(100);

      setTime(g_hour,g_min,g_sec,g_day,g_month,g_year);
      
      long unsigned int epoch_1 = now();
      Serial.print("EPOCH_1: ");
      Serial.println(epoch_1);
      Serial1.print("EPOCH_1: ");
      Serial1.println(epoch_1);
      writeToSD(String(epoch_1), "EPOCH.txt");
      
      //String 
    }
    
    if(_status.equals("0")){
       // writeToSD("0", "METER.txt");
       Serial1.println("HEHEHEHE");
      digitalWrite(ledPin, LOW);
      String dataToSave = ">" + gDate + "," + gTime + "," + "0";
      writeToSD(dataToSave, "DATALOGS.txt");
      printTime();
      setTime(g_hour,g_min,g_sec,g_day,g_month,g_year);

      unsigned long int epoch_2 = now();
      Serial.print("EPOCH 2: ");
      Serial.println(epoch_2);
      Serial1.print("EPOCH 2: ");
      Serial1.println(epoch_2);
      
      String data = readFromSD("EPOCH.txt");

      String parsedData = data.substring(0, data.indexOf('\n'));
      Serial.print("EPOCH SAVED: ");
      
      unsigned long int epoch_1 = parsedData.toInt();
      Serial.println(epoch_1);

      float timeDiff_hr = float((epoch_2 - epoch_1));

      Serial.print("TIME DIFF: ");
      Serial.print(timeDiff_hr);
      Serial.println("s");
      Serial1.print("TIME DIFF: ");
      Serial1.print(timeDiff_hr);
      Serial1.println("s");

      String timePrev = readFromSD("METER.txt");
      
      timePrev = timePrev.substring(0, timePrev.indexOf('\n'));
      Serial.print("TIME PREVIOUS: ");
      Serial.println(timePrev);
      float f_timePrev = timePrev.toFloat();

      float totalTime = f_timePrev + timeDiff_hr;
      
      

      SD.remove("EPOCH.txt");
      delay(100);
      SD.remove("METER.txt");
      delay(100);
      writeToSD(String(totalTime), "METER.txt");
      
      delay(100);
      Serial.print("TOTAL: ");
      Serial.println(String(totalTime));
      Serial1.print("TOTAL: ");
      Serial1.println(String(totalTime));
      sendDataToWeb(String(totalTime)); 
     // delay(10000);
      //SD.remove("METER.txt"); 
      //writeToSD("0", "METER.txt");
      
    }
  //  sendDataToWeb(_status);  

   // Serial.println("READ DATA IS TRUE");
    readData = true;
  }

}
//}
 
  if(readData == true){
   
    if(!dataRX.equals("")){

      //////
      //Serial.println("MINTA YA KENI");

      if(printStatus == true){
      
      Serial.print("DATA IS: ");
      Serial.println();

    
      int startIndex = dataRX.indexOf('>');
      int midIndex = dataRX.indexOf('/');
      int stopIndex = dataRX.indexOf('!');
      int endIndex = dataRX.indexOf('<');

      String timeData = dataRX.substring(startIndex + 1, midIndex);
      String dateData = dataRX.substring(stopIndex + 1, endIndex);
      statusData = dataRX.substring(midIndex + 1, stopIndex);

        Serial.print("STATUS                    A: ");
        
        Serial.println(statusData);
  

     /* if(statusData.equals("SUCCESS")){

         writeToSD("0", "METER.txt");
        
      }*/

      int iHour = timeData.indexOf(':');
      int iMin  = timeData.indexOf(':', iHour + 1);
      int iSec  = timeData.indexOf('/');

      int iYear   = dateData.indexOf('-');
      int iMonth  = dateData.indexOf('-', iYear + 1);
      int iDay    = dateData.indexOf('<');

      String hour   = timeData.substring(startIndex, iHour);
      String minute = timeData.substring(iHour + 1, iMin);
      String second = timeData.substring(iMin + 1, iSec);

      String year   = dateData.substring(startIndex, iYear);
      String month = dateData.substring(iYear + 1, iMonth);
      String day = dateData.substring(iMonth + 1, iDay );
    /* 
      Serial.print("TIME: ");
      Serial.println(timeData);
      Serial.print("Hour: ");
      Serial.println(hour);
      Serial.print("minute: ");
      Serial.println(minute);
      Serial.print("second: ");
      Serial.println(second);
      Serial.println("");

      Serial.print("Year: ");
      Serial.println(year);
      Serial.print("Month: ");
      Serial.println(month);
      Serial.print("Day: ");
      Serial.println(day);

      */
/*
      if(printStatus == true){
        Serial.print("STATUS: ");
        
        Serial.println(statusData);
  
        Serial1.print("STATUS: ");
        
        Serial1.println(statusData);
        printStatus = false;
      }
      */

      int i_year = year.toInt();
      int i_month = month.toInt();
      int i_day = day.toInt();

      int i_hour = hour.toInt();
      int i_minute = minute.toInt();
      int i_second = second.toInt();
      
     // Time t(i_year, i_month, i_day, i_hour, i_minute, i_second, Time::kThursday);
       setTime(i_hour,i_minute,i_second,i_day,i_month,i_year);

      delay(100);
  // Set the time and date on the chip.
     //  rtc.time(t);
       delay(100);
      printTime();
      readData = false;

    }
    }
    
  }

currentMillis = millis();

  if (currentMillis - previousMillis >= 1000) {
    
    previousMillis = currentMillis;

    if(isAlarmOn == true){
       digitalWrite(alarmLED, HIGH);
       delay(500);
       digitalWrite(alarmLED, LOW);
       delay(500);
       
    }
   if(printStatus == true){
        Serial.print("STATUS____: ");
        
        Serial.println(statusData);
  

        printStatus = false;
        

        if(statusData.equals("Success")){

          SD.remove("METER.txt");
          delay(100);
      
          writeToSD("0", "METER.txt");
          isAlarmOn = false;
        }
        else if(statusData.equals("Alarm")){
          isAlarmOn = true;
        }
        else if(statusData.equals("CAL")){
          isAlarmOn = false;
        }
        statusData = "";
   }
      
    //printTime();
  }

 // GSM.begin(9600);
 // delay(100);

  
  while(mySerial.available()) {
    parseATText(mySerial.read());
  }

  

  
  
}

