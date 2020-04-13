
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
initOLED();
}

void loop() {
  // put your main code here, to run repeatedly:



if(Serial.available() > 0){
  String data = Serial.readStringUntil("\n");
  printToOLED(data);
}
}

void initOLED(){
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();
  delay(2000); // Pause for 2 seconds

  display.clearDisplay();

}

void printToOLED(String timeVal) {

  int x = timeVal.length();
  //x = x + 4;
  x = x / 2;

  float x_coordinate = 64 - (12.8 * x);
  int x_coord = x_coordinate;
  x_coord = x_coord + 5; // 5 is just the offset
//  Serial.print("TIME: "); Serial.print(timeVal); Serial.print("\t");
//  Serial.print("LEN: "); Serial.print(x);
//  Serial.print("\t");
  
//  Serial.println(x_coord);
  timeVal = timeVal + "";
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(x_coord, 10);             // Start at top-left corner
  display.println(timeVal);
  display.display();
}
