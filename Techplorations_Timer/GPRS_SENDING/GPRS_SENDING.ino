

int counterMultiplier = 0;

#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(2, 3); // RX, TX

bool printStatus = false;
bool readData = true;
bool dataCapture = true;
String dataRX;

char buffer[80];
byte pos = 0;
int contentLength = 0;
int updateInterval = 10000;

String msg1, msg2, msg3, msg4;
int msgCount = 1;
int passCount = 0;
int sendCountTh = 3;
unsigned long currentMillis2;
unsigned long previousMillis2;
String timeBuffer = "0";

enum _parseState {
  PS_DETECT_MSG_TYPE,

  PS_IGNORING_COMMAND_ECHO,

  PS_HTTPACTION_TYPE,
  PS_HTTPACTION_RESULT,
  PS_HTTPACTION_LENGTH,

  PS_HTTPREAD_LENGTH,
  PS_HTTPREAD_CONTENT
};

byte parseState = PS_DETECT_MSG_TYPE;


void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

void parseATText(byte b) {

 //GSM.begin(9600);

  buffer[pos++] = b;

  if ( pos >= sizeof(buffer) )
    resetBuffer(); // just to be safe

  
  /* // Detailed debugging
   Serial.println();
   Serial.print("state = ");
   Serial.println(state);
   Serial.print("b = ");
   Serial.println(b);
   Serial.print("pos = ");
   Serial.println(pos);
   Serial.print("buffer = ");
   Serial.println(buffer); */

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
        if(msgCount == 1){
          msg1 = String(buffer);
          //Serial.print("MSG1: ");
          //Serial.println(msg1);
          String parsedMsg_1 = msg1.substring(0, 12);
          //Serial.print("PARSED MSG1: ");
          //Serial.println(parsedMsg_1);
          if(parsedMsg_1.equals("AT+SAPBR=3,1")){
            Serial.println("MESSAGE 1 PASSED");
            passCount = passCount + 1;
          }
          else {
            Serial.println("MESSAGE 1 NOT PASSED");
          }
        }
        else if(msgCount == 2){
          msg2 = String(buffer);
          //Serial.print("MSG2: ");
          //Serial.println(msg2);
          String parsedMsg_2 = msg2.substring(0, 12);
          //Serial.print("PARSED MSG2: ");
          //Serial.println(parsedMsg_2);
          if(parsedMsg_2.equals("AT+SAPBR=1,1")){
            Serial.println("MESSAGE 2 PASSED");
            passCount = passCount + 1;
          }
          else {
            Serial.println("MESSAGE 2 NOT PASSED");
          }
         
        }
        else if(msgCount == 3){
          msg3 = String(buffer);
         // Serial.print("MSG3: ");
          //Serial.println(msg3);
          String parsedMsg_3 = msg3.substring(0, 11);
          //Serial.print("PARSED MSG3: ");
          //Serial.println(parsedMsg_3);
          if(parsedMsg_3.equals("AT+HTTPINIT")){
            Serial.println("MESSAGE 3 PASSED");
            passCount = passCount + 1;
          }
          else {
            Serial.println("MESSAGE 3 NOT PASSED");
          }
        }
        else if(msgCount == 4){
          msg4 = String(buffer);
          //Serial.print("MSG4: ");
          //Serial.println(msg4);

          String parsedMsg_4 = msg4.substring(0, 11);
          //Serial.print("PARSED MSG4: ");
          //Serial.println(parsedMsg_4);
          
          if(parsedMsg_4.equals("AT+HTTPPARA")){
            Serial.println("MESSAGE 4 PASSED");
            passCount = passCount + 1;
          }
          else {
            Serial.println("MESSAGE 4 NOT PASSED");
          }
          
        }
        
        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
        msgCount++;
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
        gsmSerial.print("AT+HTTPREAD=0,");
        gsmSerial.println(buffer);
        
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
      Serial.println(dataRX);

  int len = dataRX.length();

  if(len >= 25){
      printStatus = true;
      readData = true;
  }
      
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
  gsmSerial.println(msg);
  delay(waitMs);
  
  while(gsmSerial.available()) {
    parseATText(gsmSerial.read());
  }
}
void softwareReset( unsigned long delayMillis) {
  uint32_t resetTime = millis() + delayMillis;
  while ( resetTime > millis()) { 
    /* wait and do nothing until the required delay expires... */
  }
  // jump to the start of the program
  asm volatile ( "jmp 0");  
}


void setup()
{
 
     Serial.begin(9600);
      gsmSerial.begin(9600);
       
      sendGSM("AT+SAPBR=3,1,\"APN\",\"vodafone\"");  
      sendGSM("AT+SAPBR=1,1",3000);
      sendGSM("AT+HTTPINIT");  
      sendGSM("AT+HTTPPARA=\"CID\",1");

      String url = "http://publicx.x10host.com/bikeTracker/fetchgps.php?ID=1&lat=20&long=120";
       
      Serial.print("SENDING DATA TO: ");
      Serial.println(url);
      sendGSM(url);
      sendGSM("AT+HTTPACTION=0");
    
      delay(2000);

     if(passCount < 4){          //included pass parameter to GSM init commands
                                 //Total number of pass parameter is 4
       softwareReset(100);       //if NOT ALL 4 PARAMS Passed, need to software reset.    
       Serial.println("RETRY");
     }

      Serial.println("SUCCESS!");
       
}








void sendDataToWeb(float latVal, float longVal){

      
      // String url = "AT+HTTPPARA=\"URL\",\"http://test.fpmms.com.ph/d.php?d=" + tractorID  +  _status + _activity + _seconds + s_antiCache_current + "\"";
       String url = "http://publicx.x10host.com/bikeTracker/fetchgps.php?ID=1&lat=20&long=120";
       
       Serial.print("SENDING DATA TO: ");
       Serial.println(url);
       sendGSM(url);
       sendGSM("AT+HTTPACTION=0");
    
}

int sendCount = 0;

void loop()
{ 

  currentMillis2 = millis();
  if (currentMillis2 - previousMillis2 >= updateInterval) {   //updateStatus every x = sendingInterval ms
      previousMillis2 = currentMillis2;
  
      sendDataToWeb(10.0, 120.0);
 
     if(sendCount >= sendCountTh){
        //resetCode
        softwareReset(100);
        
     }
  
   
  }
  gsmSerial.listen();
  while(gsmSerial.available()) {
    parseATText(gsmSerial.read());
  }

}
