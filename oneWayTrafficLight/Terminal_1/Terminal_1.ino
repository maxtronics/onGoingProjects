/*
* Arduino Wireless Communication Tutorial
*       Example 1 - Receiver Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

unsigned long previousMillis = 0;  

int timer = 0;

int counter;

int btnFlag;

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);

#define btn 3
#define redLed 4
#define greenLed 5

bool overrideState = true;

const long interval = 30000;
unsigned long currentMillis;

RF24 radio(7, 8); // CE, CSN
const byte address_uno2Mega[6] = "00001";
const byte address_Mega2Uno[6] = "00002";

void setup() {

   lcd.init();
  
  lcd.backlight();
  
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address_uno2Mega);
  radio.openWritingPipe(address_Mega2Uno);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();

  pinMode(btn, INPUT_PULLUP);
    pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);

 // digitalWrite(vccPin, HIGH);
}

void printTimer(String color, int timer){
  lcd.setCursor(0, 0);
  String colorVal = "COLOR: " + color + "     ";
  lcd.print(colorVal);
  lcd.setCursor(0, 1);
  String timerVal = "TIME: " + String(timer) + "           ";
  lcd.print(timerVal);

  Serial.println(colorVal);
  Serial.println(timerVal);
  
}
void loop() {
int btnState = digitalRead(btn);
if(btnFlag == 0){             
        if(btnState == 0){
          btnFlag = 1;
          delay(100);
        }
      }
    
      if(btnFlag == 1){              
       if(btnState == 1){
        btnFlag = 0;
        delay(10);
          counter = counter + 1;
        //send radio data
          radio.stopListening();
          String txt = "COUNTER: " + String(counter);
          char text[16];
          txt.toCharArray(text, 16);
          Serial.print("TEXT: ");
          Serial.print(text);
          Serial.println("\n");
          radio.write(&text, sizeof(text));
          Serial.println("Data Sent");
   // delay(1000);
        }
        
      }

      currentMillis = millis();

if(overrideState == true){
      if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    
     radio.stopListening();
     String txt = "COUNTER: " + String(counter);
//      char text[16];
 char text[32] = "";
      txt.toCharArray(text, 16);
      radio.write(&text, sizeof(text));
      Serial.println("Data Sent");
 
  }
}
   
  printTimer("RED", 0);
    

  radio.openReadingPipe(0, address_uno2Mega);
  radio.startListening();
  if (radio.available()) {
    while(radio.available()){

      overrideState = false;
      
    

    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
    String txt1 = "COLOR: GREEN";
    String txt2 = "COUNTER: " + String(timer);
    timer = 10;

  digitalWrite(greenLed, LOW);
    digitalWrite(redLed, HIGH);
    for(int x = timer; x >= 0; x = x - 1){
      String txt2 = "COUNTER: " + String(10);
      printTimer("GREEN", x);
      delay(1000);
    }

    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);

     previousMillis = currentMillis;
     overrideState = true;

 radio.stopListening();
     String txt = "COUNTER: " + String(counter);
//      char text[16];
      txt.toCharArray(text, 16);
      radio.write(&text, sizeof(text));
      Serial.println("Data Sent");
    
    

    
    }
  

  }

  
}
