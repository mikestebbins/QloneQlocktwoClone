//-------------------------------------------------------------------------------------------------------------
//INCLUDES
//-------------------------------------------------------------------------------------------------------------
#include <Adafruit_DotStar.h>
#include <SPI.h>
#include <TimeLib.h>

//-------------------------------------------------------------------------------------------------------------
//CONSTANTS
//-------------------------------------------------------------------------------------------------------------
#define COLUMNS 11
#define NUMLEDS 10

#define DEFAULTTIME 1483228800 // seconds since epoch as of 1/1/17 00:00:00

#define DATAPIN    11
#define CLOCKPIN   10

//-------------------------------------------------------------------------------------------------------------
//VARIABLES
//-------------------------------------------------------------------------------------------------------------
Adafruit_DotStar strip = Adafruit_DotStar(
  NUMLEDS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

uint8_t brightLevels[] = {
                          0x00,
                          0x00,
                          0x00,
                          0x00,
                          0x00,
                          0x01,
                          0x01,
                          0x01,
                          0x01,
                          0x01,
                          0x01,
                          0x02,
                          0x02,
                          0x02,
                          0x02,
                          0x03
                              };

int delayMultiple = 100; // (microsecs)
int numberOfLoops = 100; //
int bigDelay = 50;


//-------------------------------------------------------------------------------------------------------------
//SETUP
//-------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
}

//-------------------------------------------------------------------------------------------------------------
//LOOP
//-------------------------------------------------------------------------------------------------------------
void loop() {
  for (int j = 0; j < 16; j++)  {
    ramp (j);
  }
  for (int j = 16; j > 0; j--)  {
    ramp (j);
  }
}

void ramp (int i)  {
    if (i == 0) {
      setColor(0x00);
      delay(bigDelay);
      }
    if (i == 1) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x00);
      delayMicroseconds(4*delayMultiple);
      setColor(0x01);
      delayMicroseconds(1*delayMultiple);                 
      }
    }
    if (i == 2) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x00);
      delayMicroseconds(3*delayMultiple);
      setColor(0x01);
      delayMicroseconds(2*delayMultiple);                 
      }
    } 
    if (i == 3) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x00);
      delayMicroseconds(2*delayMultiple);
      setColor(0x01);
      delayMicroseconds(3*delayMultiple);                 
      }
    }
    if (i == 4) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x00);
      delayMicroseconds(1*delayMultiple);
      setColor(0x01);
      delayMicroseconds(4*delayMultiple);                 
      }
    }
    if (i == 5) {
      setColor(0x01);
      delay(bigDelay);                 
      }
    if (i == 6) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x01);
      delayMicroseconds(4*delayMultiple);
      setColor(0x02);
      delayMicroseconds(1*delayMultiple);                 
      }
    }
    if (i == 7) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x01);
      delayMicroseconds(3*delayMultiple);
      setColor(0x02);
      delayMicroseconds(2*delayMultiple);                 
      }
    }                              
    if (i == 8) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x01);
      delayMicroseconds(2*delayMultiple);
      setColor(0x02);
      delayMicroseconds(3*delayMultiple);                 
      }
    }
    if (i == 9) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x01);
      delayMicroseconds(1*delayMultiple);
      setColor(0x02);
      delayMicroseconds(4*delayMultiple);                 
      }
    }
    if (i == 10) {
      setColor(0x02);
      delay(bigDelay);                 
      }
    if (i == 11) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x02);
      delayMicroseconds(4*delayMultiple);
      setColor(0x03);
      delayMicroseconds(1*delayMultiple);                 
      }
    }  
    if (i == 12) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x02);
      delayMicroseconds(3*delayMultiple);
      setColor(0x03);
      delayMicroseconds(2*delayMultiple);                 
      }
    } 
    if (i == 13) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x02);
      delayMicroseconds(2*delayMultiple);
      setColor(0x03);
      delayMicroseconds(3*delayMultiple);                 
      }
    }  
     if (i == 14) {
      for (int k = 0; k < numberOfLoops; k++)  {
      setColor(0x02);
      delayMicroseconds(1*delayMultiple);
      setColor(0x03);
      delayMicroseconds(4*delayMultiple);                 
      }
    }   
    if (i == 15) {
      setColor(0x03);
      delay(bigDelay);                
      }                              
  }

void setColor(byte newColor)  {
  for (int j = 0; j < NUMLEDS; j++)  {
    strip.setPixelColor(j, newColor);
    }
    strip.show();
}

