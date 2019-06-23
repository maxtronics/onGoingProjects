#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>



#include <ESP8266HTTPClient.h>

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;



String payload = "";

void setup() {
  USE_SERIAL.begin(115200);
   // USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFiMulti.addAP("HUAWEIKO", "maxim123456");
}

void sendDataToWeb(String fid){

 
 if((WiFiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        String url = "http://maxtronics.x10host.com/templates/fid.php?varA=" + fid;
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
                payload = http.getString();
                int startIndex = payload.indexOf("001100");
                int stopIndex  = payload.indexOf("110011");
                payload = payload.substring(startIndex + 6, stopIndex);
                String dataToSend = payload;
                dataToSend = "110011|" + dataToSend +  "|11:20PM| May 29, 2019!" + "001100";
                USE_SERIAL.print("LOGGED: ");
                USE_SERIAL.println(dataToSend);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
}

void loop() {
  // put your main code here, to run repeatedly:
  if(USE_SERIAL.available() > 0){
    String data = Serial.readStringUntil('\n');
    Serial.println(data);

    sendDataToWeb(data);
  }
}


