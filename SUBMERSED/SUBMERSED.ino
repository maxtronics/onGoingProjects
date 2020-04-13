#define phPin A0


void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
int x = analogRead(phPin);
Serial.print("RAW: ");
Serial.print(x); Serial.print("\t");

float phVal = map_float(x, 214, 350, 4, 7);
Serial.print("PH VAL: "); Serial.println(phVal);

delay(1000);
}

float map_float(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
