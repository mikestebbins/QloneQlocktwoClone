// TO-DOs:
// - put a low-pass filter on the current brightness level, so that it can't
//   jump around in the middle of transitions or flicker in general

//#define ROWS 2
//#define COLUMNS 11
//#define NUMPIXELS 22

#include <Adafruit_DotStar.h>
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET

#define ROWS 2
#define COLUMNS 3
#define NUMPIXELS 6

#define DATAPIN    11
#define CLOCKPIN   10
Adafruit_DotStar strip = Adafruit_DotStar(
  NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

//uint32_t color = 0xFF0000;      // 'On' color, full brightness, from strandtest code
int currentBrightness = 63;    // must be between 0 and 63 for lookup table below
int transitionUpLevel = 0;
int transitionDownLevel = 0;

// generated the following exponential increasing brightness levels by inputting
// x values (0,1,2...63) into the formula y = 256^(x/63)-1
uint8_t transitionLevels[] = {0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,
                              0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x03,
                              0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x07,
                              0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,
                              0x10,0x11,0x13,0x15,0x17,0x19,0x1B,0x1E,
                              0x21,0x24,0x27,0x2B,0x2F,0x34,0x38,0x3E,
                              0x43,0x4A,0x51,0x58,0x60,0x69,0x73,0x7E,
                              0x89,0x96,0xA4,0xB3,0xC4,0xD6,0xE9,0xFF};
int goingUpCounter = 0;    // which array position are the up pixels currently at
int goingDownCounter = 0;  // which array position are the down pixels currently at
bool currentlyTransitioning = false; // track if we are still going to transition

bool currentScreen[NUMPIXELS]; // which pixels are lit up right now
bool nextScreen[NUMPIXELS];    // which pixels will be lit up in the next screen
bool stayingOn[NUMPIXELS];     // tracks pixels staying at current brightness
bool stayingOff[NUMPIXELS];    // tracks pixels staying at zero
bool goingUp[NUMPIXELS];       // tracks pixels transitioning to current brightness
bool goingDown[NUMPIXELS];     // tracks pixels transitioning to zero

uint8_t currentScreenLevel[NUMPIXELS];
uint8_t nextScreenLevel[NUMPIXELS];

//bool IT_IS[NUMPIXELS] =
//  {1,1,0,1,1,0,0,0,0,0,0,   // {I,T,_,I,S,_,_,_,_,_,_,
//   0,0,0,0,0,0,0,0,0,0,0};  //  _,_,_,_,_,_,_,_,_,_,_}
//
//bool A_QUARTER[NUMPIXELS] =
//  {0,0,0,0,0,0,0,0,0,0,0,   // {_,_,_,_,_,_,_,_,_,_,_,
//   1,0,1,1,1,1,1,1,1,0,0};  //  A,_,Q,U,A,R,T,E,R,_,_}
//
//bool MIKE_AND_EM[NUMPIXELS] =
//  {0,0,0,0,0,0,1,1,1,1,1,   // {_,_,_,_,_,_,M,I,K,E,&,
//   0,0,0,0,0,0,0,0,0,1,1};  //  _,_,_,_,_,_,_,_,_,E,M}

bool TEST_A[NUMPIXELS] =
  {1,0,0,   //
   0,0,0};  // 

bool TEST_B[NUMPIXELS] =
  {0,0,1,   //
   0,0,0};  // 

bool TEST_C[NUMPIXELS] =
  {0,0,0,   //
   0,1,0};  // 

bool TEST_D[NUMPIXELS] =
  {1,0,0,   //
   1,0,1};  // 

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
  zeroOutArray(currentScreen,NUMPIXELS);
  Serial.println("zeroed currentScreen");
  printArray(currentScreen,NUMPIXELS);

  zeroOutArray(nextScreen,NUMPIXELS);
  Serial.println("zeroed nextScreen");
  printArray(nextScreen,NUMPIXELS);

  // create temp array to hold the OR combined result
  bool tempCompiled[NUMPIXELS];

//  combineArrays(IT_IS, MIKE_AND_EM, &tempCompiled[0], NUMPIXELS);
  combineArrays(TEST_A, TEST_B, &tempCompiled[0], NUMPIXELS);
  Serial.println("TEST_A and TEST_B");
  printArray(tempCompiled,NUMPIXELS);

//  combineArrays(tempCompiled, A_QUARTER, &tempCompiled[0], NUMPIXELS);
  combineArrays(tempCompiled, TEST_C, &tempCompiled[0], NUMPIXELS);
  Serial.println("TEST_A and TEST_B and TEST_C");
  printArray(tempCompiled,NUMPIXELS);

  memcpy(currentScreen, tempCompiled, NUMPIXELS);
  Serial.println("currentScreen is now the compilation of A,B,C");
  printArray(currentScreen,NUMPIXELS);

  memcpy(nextScreen, TEST_D, NUMPIXELS);
  Serial.println("nextScreen is now TEST_D");
  printArray(nextScreen,NUMPIXELS);

  // Compare nextscreen to currentscreen and build transition matrices
  for (int i = 0; i < NUMPIXELS; i++)  {
    if ((currentScreen[i] == 1) && (nextScreen[i] == 1))  {
      stayingOn[i] = true; }
    else  { stayingOn[i] = false; }
    
    if ((currentScreen[i] == 0) && (nextScreen[i] == 1))  {
      goingUp[i] = true;
      currentlyTransitioning = true; }
    else  { goingUp[i] = false; }
    
    if ((currentScreen[i] == 1) && (nextScreen[i] == 0))  {
      goingDown[i] = true;
      currentlyTransitioning = true; }
    else  { goingDown[i] = false; }
  }

  Serial.println("currentScreenLevel:"); 
  // set the transition indices to their starting points for the tranisitions
  transitionDownLevel = currentBrightness;
  transitionUpLevel = 0;
  
  while (currentlyTransitioning == true)  {
    for (int i = 0; i < NUMPIXELS; i++)  {
      if (stayingOn[i] == true)  {
        currentScreenLevel[i] = transitionLevels[currentBrightness];   
      }
      else if (goingUp[i] == true)  {
        currentScreenLevel[i] = transitionLevels[transitionUpLevel];
        }
      else if (goingDown[i] == true)  {
        currentScreenLevel[i] = transitionLevels[transitionDownLevel];
      }
      else  {
        currentScreenLevel[i] = 0x00;
      }
    }
    
    // update the transition counters unless they've reached the end of transitioning
    if (transitionUpLevel == currentBrightness)  {
      currentlyTransitioning = false;
      }
    else { 
      transitionUpLevel++; 
      transitionDownLevel--;
      }
      
    // light up some LEDs per the currentScreenLevel values just set
    for (int i = 0; i < NUMPIXELS; i++)  {
        strip.setPixelColor(i, currentScreenLevel[i]);
    }
    strip.show();  
     
    printArrayByte(currentScreenLevel,NUMPIXELS);
    
    // delay for 1 frame duration
    delay(10);                  
  }

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

  // printArray(tempAdditionA, sizeOfArray);
  // printArray(tempAdditionB, sizeOfArray);

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
