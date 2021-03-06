// TO-DOs:
// - put a low-pass filter on the current brightness level, so that it can't
//   jump around in the middle of transitions or flicker in general

// #define ROWS 10
// #define COLUMNS 11
#define NUMLEDS 114

// #define ROWS 2
// #define COLUMNS 3
// #define NUMLEDS 6

#include <Adafruit_DotStar.h>
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET

#define DATAPIN    11
#define CLOCKPIN   10
Adafruit_DotStar strip = Adafruit_DotStar(
  NUMLEDS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

// generated the following exponential increasing brightness levels by inputting
// x values (0,1,2...63) into the formula y = 256^(x/63)-1
uint8_t brightLevels[] = {0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,
                              0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x03,
                              0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x07,
                              0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,
                              0x10,0x11,0x13,0x15,0x17,0x19,0x1B,0x1E,
                              0x21,0x24,0x27,0x2B,0x2F,0x34,0x38,0x3E,
                              0x43,0x4A,0x51,0x58,0x60,0x69,0x73,0x7E,
                              0x89,0x96,0xA4,0xB3,0xC4,0xD6,0xE9,0xFF};

int frameDelay = 10;         // (milliseconds), time between transition screens
//uint32_t color = 0xFF0000; // 'On' color, full brightness, from strandtest code
int currentBrightness = 30;  // must be between 0 and 63 for lookup table above
int transitionUpLevel = 0;
int transitionDownLevel = 0;

int goingUpCounter = 0;      // which array position are the up pixels currently at
int goingDownCounter = 0;    // which array position are the down pixels currently at
bool transitioningNow = false; // track if we are still going to transition

bool currentScreen[NUMLEDS]; // which pixels are lit up right now
bool nextScreen[NUMLEDS];    // which pixels will be lit up in the next screen
bool stayingOn[NUMLEDS];     // tracks pixels staying at current brightness
bool stayingOff[NUMLEDS];    // tracks pixels staying at zero
bool goingUp[NUMLEDS];       // tracks pixels transitioning to current brightness
bool goingDown[NUMLEDS];     // tracks pixels transitioning to zero

uint8_t currentScreenLevel[NUMLEDS];
uint8_t nextScreenLevel[NUMLEDS];

bool screenITIS[NUMLEDS] =
  {1,1,0,1,1,0,0,0,0,0,0,           //  {I,T,_,I,S,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screen30[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,0,0,0,0,0,0,0,           //   H,A,L,F,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screen25[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,1,1,1,1,1,0,           //   T,W,E,N,T,Y,F,I,V,E,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screen20[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,1,0,0,0,0,0,           //   T,W,E,N,T,Y,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screen15[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   1,0,1,1,1,1,1,1,1,0,0,           //   A,_,Q,U,A,R,T,E,R,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screen10[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,1,1,1,0,0,0,           //   _,_,_,_,_,T,E,N,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screen5[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,0,           //   _,_,_,_,_,_,F,I,V,E,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenPAST[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,0,0,0,0,0,0,0,           //   P,A,S,T,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenTO[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,1,1,           //   _,_,_,_,_,_,_,_,_,T,O,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR1[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   1,1,1,0,0,0,0,0,0,0,0,           //   O,N,E,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR2[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,1,1,1,           //   _,_,_,_,_,_,_,_,T,W,O,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR3[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,T,H,R,E,E,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR4[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,0,0,0,0,0,0,0,           //   F,O,U,R,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR5[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,1,1,1,1,0,0,0,           //   _,_,_,_,F,I,V,E,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR6[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,1,1,1,0,0,0,0,0,           //   _,_,_,S,I,X,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR7[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   S,E,V,E,N,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR8[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   E,I,G,H,T,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR9[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,1,1,1,1,           //   _,_,_,_,_,_,_,N,I,N,E,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR10[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   1,1,1,0,0,0,0,0,0,0,0,0,0,0,0};  //   T,E,N,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR11[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,1,1,1,1,1,1,           //   _,_,_,_,_,E,L,E,V,E,N,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHOUR12[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,1,1,1,1,1,1,           //   _,_,_,_,_,T,W,E,L,V,E,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenOCLOCK[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,1,1,1,1,1,1,0,0,0,0};  //   _,_,_,_,_,O,C,L,O,C,K_,_,_,_,_}}


bool screenNUMLH0[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   A,D,Q,U,A,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   T,_,_,_,T,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   H,_,_,_,B,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   P,_,_,_,E,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   O,_,_,_,I,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   F,_,_,_,F,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   E,I,G,H,T,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMLH1[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,1,1,0,0,0,0,0,0,0,0,           //   _,D,Q,_,_,_,_,_,_,_,_,
   0,0,1,0,0,0,0,0,0,0,0,           //   _,_,E,_,_,_,_,_,_,_,_,
   0,0,1,0,0,0,0,0,0,0,0,           //   _,_,L,_,_,_,_,_,_,_,_,
   0,0,1,0,0,0,0,0,0,0,0,           //   _,_,S,_,_,_,_,_,_,_,_,
   0,0,1,0,0,0,0,0,0,0,0,           //   _,_,E,_,_,_,_,_,_,_,_,
   0,0,1,0,0,0,0,0,0,0,0,           //   _,_,U,_,_,_,_,_,_,_,_,
   0,0,1,0,0,0,0,0,0,0,0,           //   _,_,G,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMLH2[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   A,D,Q,U,A,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,T,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,B,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   P,A,S,T,E,_,_,_,_,_,_,
   1,0,0,0,0,0,0,0,0,0,0,           //   O,_,_,_,_,_,_,_,_,_,_,
   1,0,0,0,0,0,0,0,0,0,0,           //   F,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   E,I,G,H,T,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMLH3[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   A,D,Q,U,A,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,T,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,B,_,_,_,_,_,_,
   0,1,1,1,1,0,0,0,0,0,0,           //   _,A,S,T,E,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,I,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,F,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   E,I,G,H,T,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMLH4[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   A,_,_,_,A,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   T,_,_,_,T,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   H,_,_,_,B,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   P,A,S,T,E,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,I,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,F,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,T,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMLH5[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   A,D,Q,U,A,_,_,_,_,_,_,
   1,0,0,0,0,0,0,0,0,0,0,           //   T,_,_,_,_,_,_,_,_,_,_,
   1,0,0,0,0,0,0,0,0,0,0,           //   H,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   P,A,S,T,E,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,I,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,F,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   E,I,G,H,T,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMLH6[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   A,D,Q,U,A,_,_,_,_,_,_,
   1,0,0,0,0,0,0,0,0,0,0,           //   T,_,_,_,_,_,_,_,_,_,_,
   1,0,0,0,0,0,0,0,0,0,0,           //   H,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   P,A,S,T,E,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   O,_,_,_,I,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   F,_,_,_,F,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   E,I,G,H,T,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMLH7[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   A,D,Q,U,A,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,T,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,B,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,E,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,I,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,F,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,T,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMLH8[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   A,D,Q,U,A,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   T,_,_,_,T,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   H,_,_,_,B,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   P,A,S,T,E,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   O,_,_,_,I,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   F,_,_,_,F,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   E,I,G,H,T,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMLH9[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   A,D,Q,U,A,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   T,_,_,_,T,_,_,_,_,_,_,
   1,0,0,0,1,0,0,0,0,0,0,           //   H,_,_,_,B,_,_,_,_,_,_,
   1,1,1,1,1,0,0,0,0,0,0,           //   P,A,S,T,E,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,I,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,F,_,_,_,_,_,_,
   0,0,0,0,1,0,0,0,0,0,0,           //   _,_,_,_,T,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMRH0[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,T,E,R,E,M,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,F,_,_,_,X,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,E,_,_,_,O,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,U,_,_,_,E,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,T,_,_,_,E,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,V,_,_,_,O,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,L,E,V,E,N,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMRH1[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,1,1,0,           //   _,_,_,_,_,_,_,_,R,E,_,
   0,0,0,0,0,0,0,0,0,1,0,           //   _,_,_,_,_,_,_,_,_,E,_,
   0,0,0,0,0,0,0,0,0,1,0,           //   _,_,_,_,_,_,_,_,_,T,_,
   0,0,0,0,0,0,0,0,0,1,0,           //   _,_,_,_,_,_,_,_,_,N,_,
   0,0,0,0,0,0,0,0,0,1,0,           //   _,_,_,_,_,_,_,_,_,E,_,
   0,0,0,0,0,0,0,0,0,1,0,           //   _,_,_,_,_,_,_,_,_,W,_,
   0,0,0,0,0,0,0,0,0,1,0,           //   _,_,_,_,_,_,_,_,_,E,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMRH2[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,T,E,R,E,M,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,X,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,O,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,U,N,I,N,E,
   0,0,0,0,0,0,1,0,0,0,0,           //   _,_,_,_,_,_,T,_,_,_,_,
   0,0,0,0,0,0,1,0,0,0,0,           //   _,_,_,_,_,_,V,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,L,E,V,E,N,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMRH3NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,T,E,R,E,M,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,X,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,O,
   0,0,0,0,0,0,0,1,1,1,1,           //   _,_,_,_,_,_,_,N,I,N,E,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,E,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,O,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,L,E,V,E,N,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMRH4[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,T,_,_,_,M,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,F,_,_,_,X,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,E,_,_,_,O,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,U,N,I,N,E,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,E,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,O,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,N,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMRH5[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,T,E,R,E,M,
   0,0,0,0,0,0,1,0,0,0,0,           //   _,_,_,_,_,_,F,_,_,_,_,
   0,0,0,0,0,0,1,0,0,0,0,           //   _,_,_,_,_,_,E,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,U,N,I,N,E,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,E,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,O,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,L,E,V,E,N,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMRH6[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,T,E,R,E,M,
   0,0,0,0,0,0,1,0,0,0,0,           //   _,_,_,_,_,_,F,_,_,_,_,
   0,0,0,0,0,0,1,0,0,0,0,           //   _,_,_,_,_,_,E,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,U,N,I,N,E,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,T,_,_,_,E,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,V,_,_,_,O,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,L,E,V,E,N,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMRH7[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,T,E,R,E,M,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,X,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,O,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,E,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,E,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,O,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,N,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMRH8[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,T,E,R,E,M,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,F,_,_,_,X,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,E,_,_,_,O,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,U,N,I,N,E,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,T,_,_,_,E,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,V,_,_,_,O,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,L,E,V,E,N,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenNUMRH9[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,T,E,R,E,M,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,F,_,_,_,X,
   0,0,0,0,0,0,1,0,0,0,1,           //   _,_,_,_,_,_,E,_,_,_,O,
   0,0,0,0,0,0,1,1,1,1,1,           //   _,_,_,_,_,_,U,N,I,N,E,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,E,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,O,
   0,0,0,0,0,0,0,0,0,0,1,           //   _,_,_,_,_,_,_,_,_,_,N,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenMIKEEM[NUMLEDS] =
  {0,0,0,0,0,0,1,1,1,1,1,           //  {_,_,_,_,_,_,M,I,K,E,&,
   0,0,0,0,0,0,0,0,0,1,1,           //   _,_,_,_,_,_,_,_,_,E,M,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,           //   _,_,_,_,_,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,_,_,_,_,_,_,_,_,_,_}

bool screenHEARTFULL[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,1,1,0,0,0,1,1,0,0,           //   _,_,Q,U,_,_,_,E,R,_,_,
   0,1,1,1,1,0,1,1,1,1,0,           //   _,W,E,N,T,_,F,I,V,E,_,
   0,1,1,1,1,1,1,1,1,1,0,           //   _,A,L,F,B,T,E,N,F,T,_,
   0,1,1,1,1,1,1,1,1,1,0,           //   _,A,S,T,E,R,U,N,I,N,_,
   0,1,1,1,1,1,1,1,1,1,0,           //   _,N,E,S,I,X,T,H,R,E,_,
   0,0,1,1,1,1,1,1,1,0,0,           //   _,_,U,R,F,I,V,E,T,_,_,
   0,0,0,1,1,1,1,1,0,0,0,           //   _,_,_,H,T,E,L,E,_,_,_,
   0,0,0,0,1,1,1,0,0,0,0,           //   _,_,_,_,N,T,W,_,_,_,_,
   0,0,0,0,0,1,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,O,_,_,_,_,_,_,_,_,_}

bool screenHEARTLINE[NUMLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,           //  {_,_,_,_,_,_,_,_,_,_,_,
   0,0,1,1,0,0,0,1,1,0,0,           //   _,_,Q,U,_,_,_,E,R,_,_,
   0,1,0,0,1,0,1,0,0,1,0,           //   _,W,_,_,T,_,F,_,_,E,_,
   0,1,0,0,0,1,0,0,0,1,0,           //   _,A,_,_,_,T,_,_,_,T,_,
   0,1,0,0,0,0,0,0,0,1,0,           //   _,A,_,_,_,_,_,_,_,N,_,
   0,1,0,0,0,0,0,0,0,1,0,           //   _,N,_,_,_,_,_,_,_,E,_,
   0,0,1,0,0,0,0,0,1,0,0,           //   _,_,U,_,_,_,_,_,T,_,_,
   0,0,0,1,0,0,0,1,0,0,0,           //   _,_,_,H,_,_,_,E,_,_,_,
   0,0,0,0,1,0,1,0,0,0,0,           //   _,_,_,_,N,_,W,_,_,_,_,
   0,0,0,0,0,1,0,0,0,0,0,0,0,0,0};  //   _,_,_,_,_,O,_,_,_,_,_,_,_,_,_}


// bool TEST_A[NUMLEDS] =
//   {1,0,0,           
//    0,0,0};  

// bool TEST_B[NUMLEDS] =
//   {0,0,1,    
//    0,0,0};

// bool TEST_C[NUMLEDS] =
//   {0,0,0,         
//    0,1,0}; 

// bool TEST_D[NUMLEDS] =
//   {1,0,0,          
//    1,0,1}; 

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void loop() {
  zeroOutArray(currentScreen,NUMLEDS);
  Serial.println("zeroed currentScreen");
  printArray(currentScreen,NUMLEDS);

  zeroOutArray(nextScreen,NUMLEDS);
  Serial.println("zeroed nextScreen");
  printArray(nextScreen,NUMLEDS);

  // create temp array to hold the OR combined result
  bool tempCompiled[NUMLEDS];

  combineArrays(TEST_A, TEST_B, &tempCompiled[0], NUMLEDS);
  Serial.println("TEST_A and TEST_B");
  printArray(tempCompiled,NUMLEDS);

  combineArrays(tempCompiled, TEST_C, &tempCompiled[0], NUMLEDS);
  Serial.println("TEST_A and TEST_B and TEST_C");
  printArray(tempCompiled,NUMLEDS);

  memcpy(currentScreen, tempCompiled, NUMLEDS);
  Serial.println("currentScreen is now the compilation of A,B,C");
  printArray(currentScreen,NUMLEDS);

  memcpy(nextScreen, TEST_D, NUMLEDS);
  Serial.println("nextScreen is now TEST_D");
  printArray(nextScreen,NUMLEDS);

  // Compare nextscreen to currentscreen and build transition matrices
  for (int i = 0; i < NUMLEDS; i++)  {
    if ((currentScreen[i] == 1) && (nextScreen[i] == 1))  {
      stayingOn[i] = true; }
    else  { stayingOn[i] = false; }
    
    if ((currentScreen[i] == 0) && (nextScreen[i] == 1))  {
      goingUp[i] = true;
      transitioningNow = true; }
    else  { goingUp[i] = false; }
    
    if ((currentScreen[i] == 1) && (nextScreen[i] == 0))  {
      goingDown[i] = true;
      transitioningNow = true; }
    else  { goingDown[i] = false; }
  }

  Serial.println("currentScreenLevel:"); 

    setNextScreenLevels();          

  //------------------------------------------------------------------------------
  // HERE IS WHERE I AM AT
  //------------------------------------------------------------------------------

  // TODO: LIGHT UP SOME LEDS test
  // THEN WORRY ABOUT IMPLEMENTING ALL "SCREENS" FOR TIME AND TIME CODE: EASY WIN

  Serial.println("--------------------------------");
  delay(3000);
}

//--------------------------------------------------------------------------------------
// FUNCTIONS ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

void zeroOutArray(bool theArray[], int sizeOfArray)  {
  for (int i = 0; i < sizeOfArray; i++)  {
    theArray[i] = 0;
    }
}

void combineArrays(bool arrayA[], bool arrayB[], bool *parrayC, int sizeOfArray)  {
  // create temp arrays to be OR'd together
  bool tempAdditionA[sizeOfArray];
  bool tempAdditionB[sizeOfArray];

  // copy the first and second arrays to OR together into temp arrays
  memcpy(tempAdditionA, arrayA, sizeOfArray);
  memcpy(tempAdditionB, arrayB, sizeOfArray);

  for (int i = 0; i < sizeOfArray; i++)  {
    if ((tempAdditionA[i] == 1) || (tempAdditionB[i] == 1))  {
      parrayC[i] = 1;      
    }
    else  {
      parrayC[i] = 0;
    }
  }
}

void printArray(bool theArray[], int sizeOfArray)  {
  for (int i = 0; i < sizeOfArray; i++)  {
    Serial.print(theArray[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void printArrayByte(uint8_t theArray[], int sizeOfArray)  {
  for (int i = 0; i < sizeOfArray; i++)  {
    Serial.print(theArray[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void setNextScreenLevels()  {

  // set the transition indices to their starting points for the tranisitions
  transitionDownLevel = currentBrightness;
  transitionUpLevel = 0;
  
  while (transitioningNow == true)  {
    for (int i = 0; i < NUMLEDS; i++)  {
      if (stayingOn[i] == true)  {
        currentScreenLevel[i] = brightLevels[currentBrightness];   
      }
      else if (goingUp[i] == true)  {
        currentScreenLevel[i] = brightLevels[transitionUpLevel];
        }
      else if (goingDown[i] == true)  {
        currentScreenLevel[i] = brightLevels[transitionDownLevel];
      }
      else  {
        currentScreenLevel[i] = 0x00;
      }
    }
    
    // update the transition counters unless they've reached the end of transitioning
    if (transitionUpLevel == currentBrightness)  {
      transitioningNow = false;
      }
    else { 
      transitionUpLevel++; 
      transitionDownLevel--;
      }

    lightUpLEDs();
    printArrayByte(currentScreenLevel,NUMLEDS);
    
    // delay for 1 frame duration
    delay(frameDelay);                  
  }

// takes care of the LED strips being wired in "S"-shaped chain
void lightUpLEDs()  {
  bool reversed = false;  // track if we're on a Left-to-Right strip or R-to-L (= reversed)
  int j = 0;              // if on a reversed strip, track which position we're in

  for (int i = 0; i < NUMLEDS; i++)  {
    if (reversed == false)  {
      strip.setPixelColor(i, currentScreenLevel[i]);
      Serial.print(i);
      Serial.print("\t");
    }
    else {
      int temp = i + 10 - (2 * j);
      strip.setPixelColor(temp, currentScreenLevel[temp]);
      Serial.print(temp);
      Serial.print("\t");
      j++;
    }
    
    if (i%11 == 10)  {
      reversed = !reversed;
      j = 0;
      Serial.println("");
    }    
  }
}