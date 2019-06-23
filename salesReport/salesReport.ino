#include <ESP8266WiFi.h>

const int interruptPin = D3; //GPIO 0 (Flash Button) 
const int LED=2; //On board blue LED 
int counter = 0;

const char* ssid = "HUAWEIKO"; // your wireless network name (SSID)
const char* password = "maxim123456"; // your Wi-Fi network password

WiFiClient client;

const int channelID = 787164;
String writeAPIKey = "C5KFPO2OX40T2V6P"; // write API key for your ThingSpeak Channel
const char* server = "api.thingspeak.com";

bool dataSend = false;


void setup() { 

  
   
  Serial.begin(115200); 
  
   WiFi.begin(ssid, password);
  pinMode(LED,OUTPUT); 
  pinMode(interruptPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING); 

 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    
  }

  

} 

void loop() 
{ 
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("CONNECTED");
  }
  else if(WiFi.status() != WL_CONNECTED){
    Serial.println("NOT CONNECTED");
  }
  
    digitalWrite(LED,HIGH); //LED off 
    delay(500); 
    digitalWrite(LED,LOW); //LED on 
    delay(500); 

    
      sendDataToWeb();
      delay(20000);
     
    
} 

//This program get executed when interrupt is occures i.e.change of input state
void handleInterrupt() { 
     
    counter = counter + 1;

    Serial.println(counter);
    dataSend = true;

    
}

void sendDataToWeb(){
 Serial.println("START");
if (client.connect(server, 80)) {

 
 

    // Construct API request body
    String body = "field1=";
           body += String(counter);

  
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(body.length());
    client.print("\n\n");
    client.print(body);
    client.print("\n\n");

}
client.stop();

// wait 20 seconds and post again
Serial.println("DATA SENT");

}
