/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <SoftwareSerial.h>
#include <FPM.h>

SoftwareSerial fserial(A8, A10);


#include <Keypad.h>

bool isDataRX = true;

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {43, 41, 39, 37}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {35, 33, 31}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

uint16_t num;
  String fidNum = "";

FPM finger(&fserial);
FPM_System_Params params;


bool isDirectionPrinted = false;

#include "SPI.h"
#define USE_SDFAT
#include <SdFat.h>           // Use the SdFat library
SdFatSoftSpi<50, 51, 52> SD; //Bit-Bang on the Shield pins
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h> 


// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

#define SD_CS     10

#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410



unsigned long currentMillis;

bool enrollNow = false;
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC

// If using the breakout, change pins as desired
MCUFRIEND_kbv tft;

#define NAMEMATCH ""        // "" matches any name
//#define NAMEMATCH "tiger"   // *tiger*.bmp
#define PALETTEDEPTH   0     // do not support Palette modes
//#define PALETTEDEPTH   8     // support 256-colour Palette

char namebuf[32] = "/";   //BMP files in root directory
//char namebuf[32] = "/bitmaps/";  //BMP directory e.g. files in /bitmaps/*.bmp

File root;
int pathlen;
String txtToPrint[10];
bool isHome = false;

int yCursor;

// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 10000; 

#define SCAN_MODE   true
#define NORMAL_MODE false

bool mode;

#define modePin 53

unsigned long testText(String txt, int yOffset ) {
  //tft.fillScreen(0x0000);
  
  int txtLen = txt.length();
  int xOffset = ((462 - (txtLen * 18)) / 2) + 6;

 
  
  tft.setCursor(xOffset, yOffset);
  
     
  tft.setTextSize(3.5);
  tft.println(txt);
  tft.println();
 
}

void setup() {

  pinMode(modePin, INPUT_PULLUP);
 
    Serial1.begin(115200);
    Serial.begin(9600);

    Serial.println("ENROLL test");
    fserial.begin(57600);

    if (finger.begin()) {
        finger.readParams(&params);
        Serial.println("Found fingerprint sensor!");
        Serial.print("Capacity: "); Serial.println(params.capacity);
        Serial.print("Packet length: "); Serial.println(FPM::packet_lengths[params.packet_len]);
    } else {
        Serial.println("Did not find fingerprint sensor :(");
        while (1) yield();
    }
    
  uint16_t ID;
  
  Serial.println("ILI9341 Test!"); 
   ID = tft.readID();
    Serial.println("Example: Font_simple");
    Serial.print("found ID = 0x");
    Serial.println(ID, HEX);
    if (ID == 0xD3D3) ID = 0x9481; //force ID if write-only display
    tft.begin(ID);
    tft.setRotation(1);

     tft.fillScreen(0x0000);

      bool good = SD.begin(TFT_CS);
    if (!good) {
        Serial.print(F("cannot start SD"));
        while (1);
    }
    root = SD.open(namebuf);
    pathlen = strlen(namebuf);
    
  //   showHomeScreen();

int modeVal = digitalRead(modePin);
     if(modeVal == 1){
      //scan mode

      mode = SCAN_MODE;
      
     }
     else if(modeVal == 0){
      //normal mode
      mode = NORMAL_MODE;
     }
 
}


uint16_t readnumber(void) {
  uint16_t num = 0;
  
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}
/*
void serialEvent1(){
  previousMillis = currentMillis;
   if(Serial1.available() > 0){

    tft.fillScreen(0x00FF00);
    String data = Serial.readString();
    int startIndex_h = data.indexOf("110011");
    String header = data.substring(0, startIndex_h + 6);
    Serial.print("HEADER: ");
    Serial.println(header);

    tft.setTextColor(GREEN); 
    testText(data, 50);
    

  int index[4];
  String dataVal[3];

  index[0] = data.indexOf('|');
  index[1] = data.indexOf('|', index[0] + 1);
  index[2] = data.indexOf('|', index[1] + 1);
 

  

  dataVal[0] = data.substring(index[0] + 1, index[1]);
  dataVal[1] = data.substring(index[1] + 1, index[2]);
  dataVal[2] = data.substring(index[2] + 1, data.indexOf('!'));

  for(int x = 0; x <3; x++){
    Serial.print("DATA ");
    Serial.print(x);
    Serial.print(": ");
    Serial.println(dataVal[x]);
  }

  tft.fillScreen(0x0000);

  tft.setTextColor(GREEN); 
  testText(data, 50);
  tft.setTextColor(RED); 
  testText(dataVal[0], 100);
  tft.setTextColor(RED); 
  testText(dataVal[1], 150);
  tft.setTextColor(RED); 
  testText(dataVal[2], 200);

  isHome = false;
  previousMillis = currentMillis;
   
  }
}

*/
uint16_t readKeypad(void){
 
  while(enrollNow == false){
  
  char key = keypad.getKey();
 
  if (key){
    if((key != '#') && (key != '*')){
      fidNum  = fidNum + key;
     
     tft.fillScreen(0x0000);
      tft.setTextColor(WHITE); 
     testText("ENTER ID", 120);
     testText(fidNum, 170);
      Serial.println(fidNum);
    }
    else if(key == '#'){
    Serial.print("ID: ");
    uint16_t num = fidNum.toInt();
    Serial.println(num);
    enrollNow = true;
    fidNum = "";
  }
   else if(key == '*'){
    fidNum = "";
    
   tft.fillScreen(0x0000);
    testText("ENTER ID", 120);
  }
  return num;
}

}
}

void loop(void) {


if(mode == SCAN_MODE){
  if(!isDirectionPrinted){
    Serial.println("Please enter ID number where you want to store Finger Print Data");
    tft.fillScreen(0x0000);
     tft.setTextColor(WHITE); 
     testText("ENTER ID", 120);
     delay(1000);
     
    isDirectionPrinted = !isDirectionPrinted;
  }
    

     tft.setTextColor(WHITE); 
    
     
      int16_t fid = readKeypad();
      if(enrollNow == true){

      
     

      enroll_finger(fid);
      delay(2000);
      isDirectionPrinted = !isDirectionPrinted;
      }
      enrollNow = false;
  
}

else if(mode == NORMAL_MODE){

  
 /* 
if(isDataRX){
  Serial1.println("1");
  isDataRX = false;
}
  
 */
 search_database();
  
  currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
  
    showHomeScreen();
    previousMillis = currentMillis;
  }
  
}

  

  
  

 
  
  /*
 int delayVal = 1000;

    testText("DREAM IT");
    delay(delayVal);
    testText("GEORGE MANIBO");
    delay(delayVal);
    testText("NOEL FERNANDEZ");
    delay(delayVal);
    testText("CHARLES KIM KABILING");
    delay(delayVal);
  
   
    //drawGrid();  //this one's for Calibration routine
    */
}

int search_database(void) {
    int16_t p = -1;

    /* first get the finger image */
    Serial.println("Waiting for valid finger");
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {
            case FPM_OK:
                Serial.println("Image taken");
                break;
            case FPM_NOFINGER:
                Serial.println(".");
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error");
                break;
            case FPM_IMAGEFAIL:
                Serial.println("Imaging error");
                break;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                break;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                break;
            default:
                Serial.println("Unknown error");
                break;
        }
        yield();
    }

    /* convert it */
    p = finger.image2Tz();
    switch (p) {
        case FPM_OK:
            Serial.println("Image converted");
            break;
        case FPM_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FPM_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FPM_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_TIMEOUT:
            Serial.println("Timeout!");
            return p;
        case FPM_READ_ERROR:
            Serial.println("Got wrong PID or length!");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    Serial.println("Remove finger");
    p = 0;
    while (p != FPM_NOFINGER) {
        p = finger.getImage();
        yield();
    }
    Serial.println();

    /* search the database for the converted print */
    uint16_t fid, score;
    p = finger.searchDatabase(&fid, &score);
    if (p == FPM_OK) {
        Serial.println("Found a print match!");
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FPM_NOTFOUND) {
        Serial.println("Did not find a match");
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }

    // found a match!
    Serial.print("Found ID #"); Serial.print(fid);
    Serial.print(" with confidence of "); Serial.println(score);
}

void drawGrid(){
  for(int x = 0; x<=480; x = x + 18){
    tft.drawRect(x, 20, 18, 10, 0xFFFF);
  }
  
}

#define BMPIMAGEOFFSET 54

#define BUFFPIXEL      20

uint16_t read16(File& f) {
    uint16_t result;         // read little-endian
    f.read((uint8_t*)&result, sizeof(result));
    return result;
}

uint32_t read32(File& f) {
    uint32_t result;
    f.read((uint8_t*)&result, sizeof(result));
    return result;
}

uint8_t showBMP(char *nm, int x, int y)
{
    File bmpFile;
    int bmpWidth, bmpHeight;    // W+H in pixels
    uint8_t bmpDepth;           // Bit depth (currently must be 24, 16, 8, 4, 1)
    uint32_t bmpImageoffset;    // Start of image data in file
    uint32_t rowSize;           // Not always = bmpWidth; may have padding
    uint8_t sdbuffer[3 * BUFFPIXEL];    // pixel in buffer (R+G+B per pixel)
    uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL;
    uint8_t bitmask, bitshift;
    boolean flip = true;        // BMP is stored bottom-to-top
    int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx;
    uint32_t pos;               // seek position
    boolean is565 = false;      //

    uint16_t bmpID;
    uint16_t n;                 // blocks read
    uint8_t ret;

    if ((x >= tft.width()) || (y >= tft.height()))
        return 1;               // off screen

    bmpFile = SD.open(nm);      // Parse BMP header
    bmpID = read16(bmpFile);    // BMP signature
    (void) read32(bmpFile);     // Read & ignore file size
    (void) read32(bmpFile);     // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile);       // Start of image data
    (void) read32(bmpFile);     // Read & ignore DIB header size
    bmpWidth = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    n = read16(bmpFile);        // # planes -- must be '1'
    bmpDepth = read16(bmpFile); // bits per pixel
    pos = read32(bmpFile);      // format
    if (bmpID != 0x4D42) ret = 2; // bad ID
    else if (n != 1) ret = 3;   // too many planes
    else if (pos != 0 && pos != 3) ret = 4; // format: 0 = uncompressed, 3 = 565
    else if (bmpDepth < 16 && bmpDepth > PALETTEDEPTH) ret = 5; // palette 
    else {
        bool first = true;
        is565 = (pos == 3);               // ?already in 16-bit format
        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
        if (bmpHeight < 0) {              // If negative, image is in top-down order.
            bmpHeight = -bmpHeight;
            flip = false;
        }

        w = bmpWidth;
        h = bmpHeight;
        if ((x + w) >= tft.width())       // Crop area to be loaded
            w = tft.width() - x;
        if ((y + h) >= tft.height())      //
            h = tft.height() - y;

        if (bmpDepth <= PALETTEDEPTH) {   // these modes have separate palette
            bmpFile.seek(BMPIMAGEOFFSET); //palette is always @ 54
            bitmask = 0xFF;
            if (bmpDepth < 8)
                bitmask >>= bmpDepth;
            bitshift = 8 - bmpDepth;
            n = 1 << bmpDepth;
            lcdbufsiz -= n;
            palette = lcdbuffer + lcdbufsiz;
            for (col = 0; col < n; col++) {
                pos = read32(bmpFile);    //map palette to 5-6-5
                palette[col] = ((pos & 0x0000F8) >> 3) | ((pos & 0x00FC00) >> 5) | ((pos & 0xF80000) >> 8);
            }
        }

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
        for (row = 0; row < h; row++) { // For each scanline...
            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            uint8_t r, g, b, *sdptr;
            int lcdidx, lcdleft;
            if (flip)   // Bitmap is stored bottom-to-top order (normal BMP)
                pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
            else        // Bitmap is stored top-to-bottom
                pos = bmpImageoffset + row * rowSize;
            if (bmpFile.position() != pos) { // Need seek?
                bmpFile.seek(pos);
                buffidx = sizeof(sdbuffer); // Force buffer reload
            }

            for (col = 0; col < w; ) {  //pixels in row
                lcdleft = w - col;
                if (lcdleft > lcdbufsiz) lcdleft = lcdbufsiz;
                for (lcdidx = 0; lcdidx < lcdleft; lcdidx++) { // buffer at a time
                    uint16_t color;
                    // Time to read more pixel data?
                    if (buffidx >= sizeof(sdbuffer)) { // Indeed
                        bmpFile.read(sdbuffer, sizeof(sdbuffer));
                        buffidx = 0; // Set index to beginning
                        r = 0;
                    }
                    switch (bmpDepth) {          // Convert pixel from BMP to TFT format
                        case 24:
                            b = sdbuffer[buffidx++];
                            g = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            color = tft.color565(r, g, b);
                            break;
                        case 16:
                            b = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            if (is565)
                                color = (r << 8) | (b);
                            else
                                color = (r << 9) | ((b & 0xE0) << 1) | (b & 0x1F);
                            break;
                        case 1:
                        case 4:
                        case 8:
                            if (r == 0)
                                b = sdbuffer[buffidx++], r = 8;
                            color = palette[(b >> bitshift) & bitmask];
                            r -= bmpDepth;
                            b <<= bmpDepth;
                            break;
                    }
                    lcdbuffer[lcdidx] = color;

                }
                tft.pushColors(lcdbuffer, lcdidx, first);
                first = false;
                col += lcdidx;
            }           // end cols
        }               // end rows
        tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); //restore full screen
        ret = 0;        // good render
    }
    bmpFile.close();
    return (ret);
}


  
int16_t enroll_finger(int16_t fid) {
  tft.fillScreen(0x0000);
  
    int16_t p = -1;
    Serial.println("Waiting for valid finger to enroll");
    yCursor = 150;
     tft.setTextColor(WHITE); 
  testText("SCAN FINGER", yCursor);
  yCursor = 80;
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {

          
            case FPM_OK:
                tft.fillScreen(0x0000);
                Serial.println("Image taken");

                 tft.setTextColor(GREEN); 
                 testText("IMAGE TAKEN", yCursor);
                break;
            case FPM_NOFINGER:
                
                Serial.println(".");
                break;
            case FPM_PACKETRECIEVEERR:
                tft.fillScreen(0x0000);
                Serial.println("Communication error");
                tft.setTextColor(RED); 
                testText("Communication error", yCursor);
                break;
            case FPM_IMAGEFAIL:
                tft.fillScreen(0x0000);
                Serial.println("Imaging error");
                tft.setTextColor(RED); 
                testText("Imaging Error", yCursor);
                break;
            case FPM_TIMEOUT:
                tft.fillScreen(0x0000);
                Serial.println("Timeout!");
                tft.setTextColor(RED);
                testText("Timeout!", yCursor);
                break;
            case FPM_READ_ERROR:
                tft.fillScreen(0x0000);
                Serial.println("Got wrong PID or length!");
                tft.setTextColor(RED);
                testText("Got wrong PID or length!", yCursor);
                break;
            default:
                tft.fillScreen(0x0000);
                Serial.println("Unknown error");
                tft.setTextColor(RED);
                testText("Unknown error", yCursor);
                break;
        }
        yield();
    }
    // OK success!
    yCursor = 120;
    p = finger.image2Tz(1);
    switch (p) {
        case FPM_OK:
            Serial.println("Image converted");
             tft.setTextColor(GREEN);
             testText("IMAGE CONVERTED", yCursor);
            break;
        case FPM_IMAGEMESS:
            Serial.println("Image too messy");
            tft.setTextColor(RED);
            testText("ERROR CAPTURING IMAGE!", yCursor);
            return p;
        case FPM_PACKETRECIEVEERR:
            Serial.println("Communication error");
            tft.setTextColor(RED);
            testText("ERROR CAPTURING IMAGE!", yCursor);
            return p;
        case FPM_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            tft.setTextColor(RED);
            testText("ERROR CAPTURING IMAGE!", yCursor);
            return p;
        case FPM_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            tft.setTextColor(RED);
            testText("ERROR CAPTURING IMAGE!", yCursor);
            return p;
        case FPM_TIMEOUT:
            Serial.println("Timeout!");
            tft.setTextColor(RED);
            testText("ERROR CAPTURING IMAGE!", yCursor);
            return p;
        case FPM_READ_ERROR:
            Serial.println("Got wrong PID or length!");
            tft.setTextColor(RED);
            testText("ERROR CAPTURING IMAGE!", yCursor);
            return p;
        default:
            Serial.println("Unknown error");
            tft.setTextColor(RED);
            testText("ERROR CAPTURING IMAGE!", yCursor);
            return p;
    }

    
    Serial.println("Remove finger");
     yCursor = 180;
     tft.setTextColor(GREEN);
     testText("REMOVE FINGER", yCursor);
    delay(2000);
    p = 0;
    while (p != FPM_NOFINGER) {
        p = finger.getImage();
        yield();
    }

    p = -1;

    tft.fillScreen(0x0000);
    Serial.println("Place same finger again");
    yCursor = 150;
     tft.setTextColor(WHITE);
     testText("PLACE SAME FINGER AGAIN", yCursor);
     yCursor = 60;
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {
            case FPM_OK:
                Serial.println("Image taken");
                  tft.fillScreen(0x0000);
                 tft.setTextColor(GREEN); 
                 testText("IMAGE TAKEN", yCursor);
                break;
            case FPM_NOFINGER:
                Serial.print(".");
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error");
                 tft.fillScreen(0x0000);
                 tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
                break;
            case FPM_IMAGEFAIL:
                Serial.println("Imaging error");
                tft.fillScreen(0x0000);
                 tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
                break;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                tft.fillScreen(0x0000);
                 tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
                break;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                tft.fillScreen(0x0000);
                 tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
                break;
            default:
                Serial.println("Unknown error");
                tft.fillScreen(0x0000);
                 tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
                break;
        }
        yield();
    }

    // OK success!

    p = finger.image2Tz(2);

     yCursor = 100;
    switch (p) {
        case FPM_OK:
            Serial.println("Image converted");
            tft.setTextColor(GREEN);
            testText("IMAGE CONVERTED", yCursor);
            break;
        case FPM_IMAGEMESS:
            Serial.println("Image too messy");
             tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
            return p;
        case FPM_PACKETRECIEVEERR:
            Serial.println("Communication error");
             tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
            return p;
        case FPM_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
             tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
            return p;
        case FPM_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
            return p;
        case FPM_TIMEOUT:
            Serial.println("Timeout!");
            tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
            return false;
        case FPM_READ_ERROR:
            Serial.println("Got wrong PID or length!");
             tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
            return false;
        default:
            Serial.println("Unknown error");
            tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
            return p;
    }


    // OK converted!
    yCursor = 150;
    p = finger.createModel();
    if (p == FPM_OK) {
        Serial.println("Prints matched!");
         tft.setTextColor(GREEN);
                 testText("MATCHED PRINT", yCursor);
        
        
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
         tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
        return p;
    } else if (p == FPM_ENROLLMISMATCH) {
        Serial.println("Fingerprints did not match");
         tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
         tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
         tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
        return p;
    } else {
        Serial.println("Unknown error");
         tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
        return p;
    }

yCursor = 200;
    Serial.print("ID "); Serial.println(fid);
    p = finger.storeModel(fid);
    if (p == FPM_OK) {
        Serial.println("Stored!");
         tft.setTextColor(GREEN);
                 testText("STORED!", yCursor);
        return 0;
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
         tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
        return p;
    } else if (p == FPM_BADLOCATION) {
        Serial.println("Could not store in that location");
         tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
        return p;
    } else if (p == FPM_FLASHERR) {
        Serial.println("Error writing to flash");
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
         tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
         tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
        return p;
    } else {
        Serial.println("Unknown error");
         tft.setTextColor(RED);
                 testText("ERROR!!!", yCursor);
        return p;
    }

    delay(3000);
    tft.fillScreen(0x0000);
}
void showHomeScreen(){
  if(!isHome){
    tft.fillScreen(0x0000);
    isHome = !isHome;
  }
  showBMP("/csa_3.bmp", 90, 5);

  
}

