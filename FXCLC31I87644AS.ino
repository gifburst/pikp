// APPLE ][ WATCH 
// Designed and Built for Instructables.com by DJ Harrigan in San Francisco, CA
// January 2, 2015
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SoftwareSerial.h>
#include "SOMO_II.h"
#include <Encoder.h>
#include <Bounce.h>
#include <Time.h>
#include <SPI.h>
#include <SD.h>

#define sclk 13  // SCLK can also use pin 14
#define mosi 11  // MOSI can also use pin 7
#define cs   10  // CS & DC can use pins 2, 6, 9, 10, 15, 20, 21, 22, 23
#define sdcs 9   // CS for SD card, can use any pin
#define dc   8   //  but certain pairs must NOT be used: 2+10, 6+9, 20+23, 21+22
#define rst  7   // RST can use any pin
#define ENC_A_PIN 5
#define ENC_B_PIN 6
#define BACKLIGHT_PIN 4
#define BUTTON_PIN 3
#define SOMO_RX 14
#define SOMO_TX 15
#define SOMO_BUSY_PIN 16
#define SHUTDOWN_PIN 20
#define BAT_STATUS_PIN 21
#define LED_L_PIN 22
#define LED_R_PIN 23

#define OFF 0
#define ON 1
#define OFF_FADE 2
#define ON_FADE 3
#define FADE_TIME 1000
#define APL_OB 0 // '['
#define APL_CB 1 // ']'
#define APL_FILL 2 // solid pixel apple logo
#define APL_STROKE 3 // outlined pixel apple logo

SoftwareSerial somoSS = SoftwareSerial(SOMO_TX, SOMO_RX);
SOMO_II mp3 = SOMO_II(&somoSS, SOMO_BUSY_PIN);
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);
Encoder myEnc(ENC_A_PIN, ENC_B_PIN);
Bounce rotaryButton = Bounce(BUTTON_PIN, 10);

const uint16_t APL_BLACK = RGB888toRGB565("000000"); // the ST7735 takes 16 bit 565 RGB color values (e.g. 0xFFFF = white) 
const uint16_t APL_GREEN = RGB888toRGB565("26C30F");
const uint16_t APL_WHITE = RGB888toRGB565("FFFFFF");
const int width = 160;
const int height = 128;
const int maxCharColumns = 26;
const int maxCharRows = 16;
const byte charWidth = 6;
const byte charHeight = 8;
const uint8_t screenRotation = 1; // landscape view with header on the right
const byte hourMax = 12;
const byte modeMax = 9;
int oldPosition  = -999;
int rotaryPosition = 0;
int rotarySign = 1;
const char* menuText[] = { // row/column
  "CLOCK",
  "FITNESS",
  "PICTURES",
  "PHONEBOOK",
  "WEATHER",
  "MUSIC",
  "UTILITY",
  "DISK MANAGER"
};
const char* nameText[] = { // row/column
  "R. SARAFAN",
  "R. GODSHAW",
  "P. RUSSEL",
  "M. WARREN",
  "D. SILVERMAN",
  "J. ELLEN",
  "J. ODOM",
  "J. BROWN"
};
int lastMinute = 0;
byte regHour = 1;
byte spkVolume = 30;
boolean buttonToggle = false;
boolean hourIsPM = false;
byte curMode = 0;
byte lastMode = 0;
byte selectedMode = 1;
byte scrollPos = 0;
byte lastScrollPos = 0;

void setup() { 
  
  pinMode(sdcs, INPUT_PULLUP);  // keep SD CS high when not using SD card
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  pinMode(LED_L_PIN, OUTPUT);
  pinMode(LED_R_PIN, OUTPUT);
  
  setScreen(OFF);
  SD.begin(sdcs);
  mp3.begin(); // fixed at 9600
  mp3.setVolume(spkVolume);
  Serial.begin(9600);
  //while(!Serial);
  //Serial.println("Apple ][ Watch");
  //setTime(10,48,30,26,1,15); // set time to (hour, minute, second, day of month, month, year
  //Teensy3Clock.set(now()); // update the internal RTC
  setSyncProvider(getTeensy3Time); // set the time provider to the internal RTC time
  setSyncInterval(3600); // update the provider every hour
  
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(APL_BLACK);
  tft.setTextColor(APL_GREEN, APL_BLACK);
  tft.setRotation(screenRotation);
  /*
  tft.invertDisplay(true);
  tft.drawLine(0, 0, tft.width()-1, y, color);
  tft.setTextSize(1);
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
  tft.drawChar(35, 0, ']', APL_GREEN, APL_BLACK, 1);
  tft.drawFastHLine(0, y, tft.width(), color1);
  tft.drawFastVLine(x, 0, tft.height(), color2);
  tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color);
  tft.fillRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color1);
  tft.fillCircle(x, y, radius, color);
  tft.drawCircle(x, y, radius, color);
  tft.drawTriangle(w, y, y, x, z, x, color);
  tft.drawRoundRect(x, y, w, h, 5, color);*/
  mp3.playTrack(1); // play first track on the SD card (tracks aren't zero indexed)
  setScreen(ON);
  for (int rr = 0; rr < 216; rr++){ // 
    tft.print("][");
  }
  delay(40);
  tft.fillScreen(APL_BLACK);
  tft.setCursor(54, 0);
  tft.print("APPLE");
  drawSymbol(APL_CB, 35+54, 0);
  drawSymbol(APL_OB, 41+54, 0);
  delay(50); // let the sound finish
  mp3.setVolume(26);
  mp3.playTrack(2); // disk calibration noise

  drawMenuScreen();

  
} // **************************************************************** END SETUP ****************************************************************

void loop(){
  
  if(rotaryButton.update()){ // any button change?
    if (rotaryButton.fallingEdge()){ //rotaryButton.fallingEdge(); = press || rotaryButton.risingEdge(); = release
      if (curMode >= 1){ // pressing the button will return to the main menu in any submenu
        curMode = 0;
      } else {
        curMode = selectedMode;
      }
      //tft.setCursor(0, 8);
      //tft.print("CUR_MODE:");
      //tft.print(curMode);
    }
  }
  
  if (curMode == 0){ // update the screen every minute
    if (lastMinute != minute()){ 
      lastMinute = minute();
      tft.setCursor(54, 24);
      printTimeFormatted();
      tft.setCursor(54, 40);
      printDateFormatted();
    }
  
    scrollPos = getEncoder();
    if (scrollPos != lastScrollPos){
      tft.fillRect(0, 48 + (8*lastScrollPos), 6, 8, APL_BLACK); // clear old cursor
      lastScrollPos = scrollPos;
      selectedMode = scrollPos;
      drawSymbol(APL_CB, 0, 48 + (8*selectedMode)); // draw new cursor
    }  
  }//
  
  if (curMode != lastMode){ // only draw once
    lastMode = curMode;
    tft.fillScreen(APL_BLACK); // clear the last screen
    switch (curMode){
      case 0: // menu screen
        drawMenuScreen();
      break;
      case 1: // clock
        tft.drawCircle(79, 63, 60, APL_GREEN); // rounded clock face
        tft.fillCircle(79, 63, 3, APL_GREEN);
        //tft.drawLine(79, 63, (16*cos(45))+79, (16*sin(-45))+63, APL_GREEN);
        tft.drawLine(79, 63, (16*cos(360/regHour))+79, (16*sin(-360/regHour))+63, APL_GREEN); // hour line
        tft.drawLine(79, 63, (32*cos(360/minute()))+79, (32*sin(-360/minute()))+63, APL_GREEN); // minute line
      break;
      case 2: // fitness
      // random bar meters
        printAppleII();
        tft.setCursor(0, 48);
        tft.println("MOVE:\n");
        tft.println("EXERCISE:\n");
        tft.print("STAND:");
        for (int jj = 0; jj < 14; jj++){
          tft.setCursor(30 + (jj*6), 48);
          tft.print("|");
          tft.setCursor(54 + (jj*6), 64);
          tft.print("|");
          tft.setCursor(36 + (jj*6), 80);
          tft.print("|");
          delay(150);
        }
      break;
      case 3: // pictures
      //scroll through faces
        bmpDraw("face_1.bmp", 15, 0);
        delay(300);
        bmpDraw("face_2.bmp", 15, 0);
        delay(300);
        bmpDraw("face_3.bmp", 15, 0);
        delay(300);
        bmpDraw("face_4.bmp", 15, 0);
        delay(300);
        bmpDraw("face_5.bmp", 15, 0);
        delay(300);
        bmpDraw("face_6.bmp", 15, 0);
        delay(300);
        bmpDraw("face_7.bmp", 15, 0);
        delay(300);
        bmpDraw("face_8.bmp", 15, 0);
        delay(300);
        break;
        case 4: // phonebook
      // show list of names
        printAppleII();
        for (int ii = 0; ii < 8; ii++){
          tft.setCursor(6, 16 + (8*ii));
          tft.print(nameText[ii]);
        }
      break;
      case 5: // weather
      // show earth
        bmpDraw("earth.bmp", 0, 0);
      break;
      case 6: // music
      //cycle through flower graphic
      bmpDraw("flower_1.bmp", 0, 0);
      delay(100);
      bmpDraw("flower_2.bmp", 0, 0);
      delay(100);
       bmpDraw("flower_3.bmp", 0, 0);
      delay(100);
      bmpDraw("flower_4.bmp", 0, 0);
      delay(100);
      bmpDraw("flower_5.bmp", 0, 0);
      delay(100);
      bmpDraw("flower_6.bmp", 0, 0);
      delay(100);
      bmpDraw("flower_7.bmp", 0, 0);
      delay(100);
      bmpDraw("flower_8.bmp", 0, 0);
      delay(100);
      break;
      case 7: // utility
      //show butterfly graphic
        bmpDraw("bfly.bmp", 0, 0);
      break;
      case 8: // disk manager
        printAppleII();
        blinkLed(LED_L_PIN, 5, 500);
        blinkLed(LED_R_PIN, 5, 500);
      break;
    }
  } // END IF (curMode)
  
} // **************************************************************** END MAIN LOOP ****************************************************************

void drawMenuScreen(){
  
  printAppleII();
  tft.setCursor(54, 24);
  printTimeFormatted();
  tft.setCursor(54, 40);
  printDateFormatted();
  drawSymbol(APL_CB, 0, 48 + (8*selectedMode));
  for (int ii = 0; ii < 8; ii++){
    tft.setCursor(6, 56 + (8*ii));
    tft.print(menuText[ii]);
  }
  
}

void printAppleII(){
  tft.setCursor(54, 0);
  tft.print("APPLE");
  drawSymbol(APL_CB, 35+54, 0);
  drawSymbol(APL_OB, 41+54, 0);
}

void printTimeFormatted(){

  if (hour() > 12){ 
    hourIsPM = true;
    regHour = hour() - 12; // 12 hour time isn't zero indexed, but military time is 
  } else {
    hourIsPM = false;
    regHour = hour();
  }
  if (regHour < 10){
    tft.print(" "); // no leading '0' for hours
  }
  tft.print(regHour);
  tft.print(":");
  tftPrintDigits(minute());
  if (hourIsPM){
    tft.print(" PM");
  } else {
    tft.print(" AM");
  }
  
} // END FUNCTION printTimeFormatted()

void tftPrintDigits(int digits){
  
  if(digits < 10){
    tft.print('0'); 
  }
  tft.print(digits);
  
} // END FUNCTION tftPrintDigits()

void printDateFormatted(){
  
  tftPrintDigits(month());
  tft.print(".");
  tftPrintDigits(day());
  tft.print(".");
  tft.print("15");
  
} // END FUNCTION printDateFormatted()

void setScreen(int screenState){
  
  int tempBrightness = 255;
  switch (screenState){
    case ON:
      digitalWrite(BACKLIGHT_PIN, HIGH);
    break;
    case OFF:
      digitalWrite(BACKLIGHT_PIN, LOW);
    break;
    case ON_FADE:
      for (int ll = 0; ll <= tempBrightness; ll++){
        analogWrite(BACKLIGHT_PIN, ll);
        delay(FADE_TIME / tempBrightness);
      }
    break;
    case OFF_FADE:
      for (int ll = tempBrightness; ll >= 0; ll--){
        analogWrite(BACKLIGHT_PIN, ll);
        delay(FADE_TIME / tempBrightness);
      }
    break;
  }
  
} // END FUNCTION setScreen()

int getEncoder(){
  
  int newPosition = myEnc.read();
  
  if (abs(newPosition - oldPosition) >= 4) {
    if (newPosition - oldPosition > -1){ // positive increase
      rotarySign = 1; // modify this function such that it either increaments or decreaments a global "scroll position" variable, the bounds are local to the given mode
    } else {
      rotarySign = -1;
    }
    rotaryPosition += rotarySign;
    oldPosition = newPosition;
    if (rotaryPosition >= 9){
      rotaryPosition = 1;
    } else if (rotaryPosition < 1){
      rotaryPosition = 8;
    }
  }
  
  return rotaryPosition;
  
} // END FUNCTION getEncoder()

void drawSymbol(int whichSymbol, int tempX, int tempY){
  //write a more compact function (use some pointers to refer to the arrays) if more symbols are added
  byte appleCB[][7] = { // 35 pixels
    {1, 1, 1, 1, 1}, 
    {0, 0, 0, 1, 1},
    {0, 0, 0, 1, 1},
    {0, 0, 0, 1, 1},
    {0, 0, 0, 1, 1},
    {0, 0, 0, 1, 1},
    {1, 1, 1, 1, 1},
  };
  byte appleOB[][7] = { // 35 pixels
    {1, 1, 1, 1, 1}, 
    {1, 1, 0, 0, 0},
    {1, 1, 0, 0, 0},
    {1, 1, 0, 0, 0},
    {1, 1, 0, 0, 0},
    {1, 1, 0, 0, 0},
    {1, 1, 1, 1, 1}
  };
  byte appleFill[][8] = { // 56 pixels
    {0, 0, 0, 0, 1, 0, 0}, 
    {0, 0, 0, 1, 0, 0, 0},
    {0, 1, 1, 0, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 0, 1, 1, 0}
  };
  byte appleStroke[][8] = { // 56 pixels
    {0, 0, 0, 0, 1, 0, 0}, 
    {0, 0, 0, 1, 0, 0, 0},
    {0, 1, 1, 0, 1, 1, 0},
    {1, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 0},
    {1, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 0, 0, 1},
    {0, 1, 1, 0, 1, 1, 0},
  };
  
  tft.setCursor(tempX, tempY);
  //Serial.print("(tempX, tempY):("); Serial.print(tempX);Serial.print(", ");Serial.print(tempY);Serial.println(")\n");
  
  if (whichSymbol == APL_OB || whichSymbol == APL_CB){
    for (int rr = 0; rr < 7; rr++){ 
      for (int cc = 0; cc < 5; cc++){
        if (whichSymbol == APL_OB){
          if (appleOB[rr][cc] == 1){
            tft.drawPixel(tempX+cc, tempY+rr, APL_GREEN);
          }
        }
        if (whichSymbol == APL_CB){
          if (appleCB[rr][cc] == 1){
            tft.drawPixel(tempX+cc, tempY+rr, APL_GREEN);
          }
        }
      }
    }
  } // END IF (whichSymbol)
  
  if (whichSymbol == APL_FILL || whichSymbol == APL_STROKE){
    for (int rr = 0; rr < 8; rr++){ 
      for (int cc = 0; cc < 7; cc++){
        if (whichSymbol == APL_FILL){
          if (appleFill[rr][cc] == 1){
            tft.drawPixel(tempX+cc, tempY+rr, APL_GREEN);
          }
        }
        if (whichSymbol == APL_STROKE){
          if (appleStroke[rr][cc] == 1){
            tft.drawPixel(tempX+cc, tempY+rr, APL_GREEN);
          } else {
            tft.drawPixel(tempX+cc, tempY+rr, APL_BLACK); // let the outline apple overwrite (for blinkin!)
          }
        }
      }
    }
  } // END IF (whichSymbol)

} // END FUNCTION drawSymbol()

time_t getTeensy3Time(){
  
  return Teensy3Clock.get();
  
} // END FUNCTION getTeensy3Time()

uint16_t RGB888toRGB565(const char *rgb32_str_){ // Credit to Tom Carpenter on the Arduino forum for an RGB888 to RGB565 function!

  typedef union {
    uint16_t integer;
    struct{
      uint8_t low;
      uint8_t high;
    };
  } Byter;
  
  byte red;
  byte green;
  Byter rgb16;
                          
  green = hexToNibble(rgb32_str_[2])<<4;
  green |= hexToNibble(rgb32_str_[3])&0xC;
  rgb16.low = hexToNibble(rgb32_str_[4])<<4;
  rgb16.low |= hexToNibble(rgb32_str_[5])&0x8;
  rgb16.high = hexToNibble(rgb32_str_[0])<<4;
  rgb16.high |= hexToNibble(rgb32_str_[1])&0x8;
  rgb16.low >>= 3;
  rgb16.integer |= (green << 3);
  
  return rgb16.integer;
}
inline byte hexToNibble(char hex) {
  if (hex & _BV(6)){
    hex += 9;
  }
  return hex;
  
} // END FUNCTION RGB888toRGB565()

void blinkLed(byte whichLed, int numBlinks, int blinkPeriod){
  
  int timeDelay = blinkPeriod/2;
  
  if (numBlinks < 0){ // -1 or less sets the LED to constant ON
    digitalWrite(whichLed, ON); // leds are wired to be active LOW
  } else if (numBlinks == 0){ // 0 turns it off
    digitalWrite(whichLed, OFF);
  } else { // 1 or greater sets how many times to blink
    for (int bb = 0; bb < numBlinks; bb++){
      digitalWrite(whichLed, ON);
      delay(timeDelay);
      digitalWrite(whichLed, OFF);
      delay(timeDelay);
    }
  } // END IF ELSE
   
} // END FUNCTION blinkLed();
#define BUFFPIXEL 60

void bmpDraw(char *filename, uint8_t x, uint8_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print("File not found");
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print("File size: "); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: "); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: "); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print("Loaded in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
/*
void printCenter(char* textToCenter, byte tempY){
  
  byte charCount = sizeof(textToCenter); // the sizeof operator is returning the value "4" for any parameter but a byte, apparently in C, chars have an integer value of 4 ugh
  //Serial.print("\nsize: "); //Serial.println(charCount);
  byte charPixelCount = charCount * charWidth;
  //Serial.print("charPixelCount: "); //Serial.println(charPixelCount);
  byte newCursorX = width - charPixelCount; // subtract from the total width, then divide by 2
  //Serial.print("newCursorX: "); //Serial.println(newCursorX);
  newCursorX = newCursorX >> 1;
  //Serial.print("newCursorX: "); //Serial.println(newCursorX);
  tft.setCursor(newCursorX, tempY);
  tft.print(textToCenter);
} // END FUNCTION printCenter()
*/
