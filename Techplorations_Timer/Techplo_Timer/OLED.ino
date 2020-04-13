/*void initOLED(){
  
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
  x = x + 4;
  x = x / 2;

  float x_coordinate = 64 - (12.8 * x);
  int x_coord = x_coordinate;
  x_coord = x_coord + 5; // 5 is just the offset
//  Serial.print("TIME: "); Serial.print(timeVal); Serial.print("\t");
//  Serial.print("LEN: "); Serial.print(x);
//  Serial.print("\t");
  
//  Serial.println(x_coord);
  timeVal = timeVal + "Hrs";
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(x_coord, 27);             // Start at top-left corner
  display.println(timeVal);
  display.display();
}*/
