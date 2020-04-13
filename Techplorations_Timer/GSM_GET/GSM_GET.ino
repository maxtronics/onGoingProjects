#include <SoftwareSerial.h>
/* Create object named SIM900 of the class SoftwareSerial */
SoftwareSerial SIM900(2, 3);

unsigned long previousMillis = 0;
const long interval = 1000;     

void setup() {
  SIM900.begin(9600);  /* Define baud rate for software serial communication */
  Serial.begin(9600); /* Define baud rate for serial communication */

  Serial.println("HTTP get method :");
  Serial.print("AT\\r\\n");
  SIM900.println("AT"); /* Check Communication */
  delay(5000);
  ShowSerialData(); /* Print response on the serial monitor */
  delay(5000);
  /* Configure bearer profile 1 */
  Serial.print("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\\r\\n");    
  SIM900.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");  /* Connection type GPRS */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+SAPBR=3,1,\"APN\",\"TATA.DOCOMO.INTERNET\"\\r\\n");  
  SIM900.println("AT+SAPBR=3,1,\"APN\",\"TATA.DOCOMO.INTERNET\"");  /* APN of the provider */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+SAPBR=1,1\\r\\n");
  SIM900.println("AT+SAPBR=1,1"); /* Open GPRS context */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+SAPBR=2,1\\r\\n");
  SIM900.println("AT+SAPBR=2,1"); /* Query the GPRS context */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+HTTPINIT\\r\\n");
  SIM900.println("AT+HTTPINIT"); /* Initialize HTTP service */
  delay(5000); 
  ShowSerialData();
  delay(5000);
  Serial.print("AT+HTTPPARA=\"CID\",1\\r\\n");
  SIM900.println("AT+HTTPPARA=\"CID\",1");  /* Set parameters for HTTP session */
  delay(5000);
  ShowSerialData();
  delay(5000);
}
bool getData = false;

void sendDataToWeb(String data){

  String url = "AT+HTTPPARA=\"URL\",\"http://tinkerpost.com/techplo/getData.php?str=" + data + "\"";
  Serial.print(url);
  SIM900.println(url);  /* Set parameters for HTTP session */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+HTTPACTION=0\\r\\n");
  SIM900.println("AT+HTTPACTION=0");  /* Start GET session */
  delay(10000);
  ShowSerialData();
  delay(10000);
  Serial.print("AT+HTTPREAD\\r\\n");
  getData = true;
  SIM900.println("AT+HTTPREAD");  /* Read data from HTTP server */
  delay(8000);
  ShowSerialData();
  delay(8000);
  
  
}

int waitCounter = 0;
int waitTime = 60;
void loop() {
  
    if(Serial.available() > 0){
      String data = Serial.readStringUntil('\n');
      int startIndex = data.indexOf('!');
      int stopIndex  = data.indexOf('<');

     

      if(data.substring(startIndex, startIndex + 1) == "!"){

         data = data.substring(startIndex + 1, stopIndex);

         sendDataToWeb(data);
        
      }

      
    }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    waitCounter = waitCounter + 1;
    Serial.print("TIMER: "); Serial.println(waitCounter);

    if(waitCounter >= waitTime){
      softwareReset(1000);
    }

  }

    
  
}

void softwareReset( unsigned long delayMillis) {
  uint32_t resetTime = millis() + delayMillis;
  Serial.print("RESETING");
  while ( resetTime > millis()) { 
    Serial.print(" .");
    delay(10);
  }
  // jump to the start of the program
  asm volatile ( "jmp 10");  
}

void ShowSerialData()
{
  String dataCaptured = "";
  while(SIM900.available()!=0){  
   
      char c = SIM900.read();
      Serial.write(c); 

    if(getData){
      dataCaptured = dataCaptured + c;
      if(c == '%'){
        if(dataCaptured.indexOf("#^#") != -1){
          waitCounter = 0;
        }
        dataCaptured = dataCaptured.substring(dataCaptured.indexOf("#^#") + 3, dataCaptured.indexOf("%"));
        Serial.print("DATA FROM WEB:=="); Serial.print(dataCaptured); Serial.println("==");  
        dataCaptured = "";   
        getData = false;
        
      }
    }
  
  }
}
