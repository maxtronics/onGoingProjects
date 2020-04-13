/**
   BasicHTTPClient.ino
    Created on: 24.05.2015
*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

#define USE_SERIAL Serial


#include <SoftwareSerial.h>

SoftwareSerial swSer(14, 12, false, 256);     //rxPin, txPin, nverse_logic, buffer size



void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  swSer.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("HUAWEI-g54N", "y7qvA78k");

}


void sendDataToWeb(String current, String total, String eff){
      // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;

        "http://tinkerpost.com/techplo/finalTemp/HTML/addQuery.php?ID=ABC123&current=" + current + "&total=" + total + "&n=" + eff;

        String url = "http://tinkerpost.com/techplo/finalTemp/HTML/addQuery.php?ID=ABC123&current=" + current + "&total=" + total + "&n=" + eff;
        String data = "SENDING DATA: " + url;
        Serial.println(data);

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
        http.begin(url); //HTTP

        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
                swSer.println(payload);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }

  
}




void loop() {
  if(swSer.available() > 0){
  String data = swSer.readStringUntil('<');
  Serial.println(data);

  int startIndex = data.indexOf("!");
  int index_1 = data.indexOf(":"); 
  int index_2 = data.indexOf(":", index_1 + 1);
  int index_3 = data.indexOf("<");                         

  String data_1 = data.substring(startIndex + 1, index_1);
  String data_2 = data.substring(index_1 + 1, index_2);
  String data_3 = data.substring(index_2 + 1, index_3);
 
  
 
 


  String head = data.substring(startIndex, startIndex + 1);
  if(head.equals("!")){
      


      sendDataToWeb(data_2, data_1, data_3);

      
  }
  

}

}
