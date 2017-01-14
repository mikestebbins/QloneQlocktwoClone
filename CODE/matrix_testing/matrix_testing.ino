// TO-DOs:
// - put a low-pass filter on the current brightness level, so that it can't
//   jump around in the middle of transitions or flicker in general

//#define ROWS 2
//#define COLUMNS 11
//#define NUMBEROFLEDS 22

#define ROWS 2
#define COLUMNS 3
#define NUMBEROFLEDS 6

//uint32_t color = 0xFF0000;      // 'On' color (starts red), from strandtest code
uint8_t currentBrightness = 0x15;
uint8_t transitionUpLevel = 0x00;
uint8_t transitionDownLevel = 0x00;

uint8_t transitionLevels[] = {0x00,0x01,0x03,0x07,0x15};
int goingUpCounter = 0;    // which array position are the up pixels currently at
int goingDownCounter = 0;  // which array position are the down pixels currently at
bool currentlyTransitioning = false; // track if we are still going to transition

bool currentScreen[NUMBEROFLEDS]; // which pixels are lit up right now
bool nextScreen[NUMBEROFLEDS];    // which pixels will be lit up in the next screen
bool stayingOn[NUMBEROFLEDS];     // tracks pixels staying at current brightness
bool stayingOff[NUMBEROFLEDS];    // tracks pixels staying at zero
bool goingUp[NUMBEROFLEDS];       // tracks pixels transitioning to current brightness
bool goingDown[NUMBEROFLEDS];     // tracks pixels transitioning to zero

uint8_t currentScreenLevel[NUMBEROFLEDS];
uint8_t nextScreenLevel[NUMBEROFLEDS];

//bool IT_IS[NUMBEROFLEDS] =
//  {1,1,0,1,1,0,0,0,0,0,0,   // {I,T,_,I,S,_,_,_,_,_,_,
//   0,0,0,0,0,0,0,0,0,0,0};  //  _,_,_,_,_,_,_,_,_,_,_}
//
//bool A_QUARTER[NUMBEROFLEDS] =
//  {0,0,0,0,0,0,0,0,0,0,0,   // {_,_,_,_,_,_,_,_,_,_,_,
//   1,0,1,1,1,1,1,1,1,0,0};  //  A,_,Q,U,A,R,T,E,R,_,_}
//
//bool MIKE_AND_EM[NUMBEROFLEDS] =
//  {0,0,0,0,0,0,1,1,1,1,1,   // {_,_,_,_,_,_,M,I,K,E,&,
//   0,0,0,0,0,0,0,0,0,1,1};  //  _,_,_,_,_,_,_,_,_,E,M}

bool TEST_A[NUMBEROFLEDS] =
  {1,0,0,   //
   0,0,0};  // 

bool TEST_B[NUMBEROFLEDS] =
  {0,0,1,   //
   0,0,0};  // 

bool TEST_C[NUMBEROFLEDS] =
  {0,0,0,   //
   0,1,0};  // 

bool TEST_D[NUMBEROFLEDS] =
  {1,0,0,   //
   1,0,1};  // 

void setup() {
  Serial.begin(9600);
}

void loop() {
  zeroOutArray(currentScreen,NUMBEROFLEDS);
  Serial.println("zeroed currentScreen");
  printArray(currentScreen,NUMBEROFLEDS);

  zeroOutArray(nextScreen,NUMBEROFLEDS);
  Serial.println("zeroed nextScreen");
  printArray(nextScreen,NUMBEROFLEDS);

  // create temp array to hold the OR combined result
  bool tempCompiled[NUMBEROFLEDS];

//  combineArrays(IT_IS, MIKE_AND_EM, &tempCompiled[0], NUMBEROFLEDS);
  combineArrays(TEST_A, TEST_B, &tempCompiled[0], NUMBEROFLEDS);
  Serial.println("TEST_A and TEST_B");
  printArray(tempCompiled,NUMBEROFLEDS);

//  combineArrays(tempCompiled, A_QUARTER, &tempCompiled[0], NUMBEROFLEDS);
  combineArrays(tempCompiled, TEST_C, &tempCompiled[0], NUMBEROFLEDS);
  Serial.println("TEST_A and TEST_B and TEST_C");
  printArray(tempCompiled,NUMBEROFLEDS);

  memcpy(currentScreen, tempCompiled, NUMBEROFLEDS);
  Serial.println("currentScreen is now the compilation of A,B,C");
  printArray(currentScreen,NUMBEROFLEDS);

  memcpy(nextScreen, TEST_D, NUMBEROFLEDS);
  Serial.println("nextScreen is now TEST_D");
  printArray(nextScreen,NUMBEROFLEDS);

  // Compare next to current and build transition matrices
  for (int i = 0; i < NUMBEROFLEDS; i++)  {
//    if ((currentScreen[i] == 0) && (nextScreen[i] == 0))  {
//      stayingOff[i] = true;
//    }
//    else  {
//      stayingOff[i] = false;
//    }

    if ((currentScreen[i] == 1) && (nextScreen[i] == 1))  {
      stayingOn[i] = true;
    }
    else  {
      stayingOn[i] = false;
    }
    
    if ((currentScreen[i] == 0) && (nextScreen[i] == 1))  {
      goingUp[i] = true;
      currentlyTransitioning = true;
    }
    else  {
      goingUp[i] = false;
    }
    
    if ((currentScreen[i] == 1) && (nextScreen[i] == 0))  {
      goingDown[i] = true;
      currentlyTransitioning = true;
    }
    else  {
      goingDown[i] = false;
    }
  }

//------------------------------------------------------------------------------
// HERE IS WHERE I AM AT
//------------------------------------------------------------------------------
  Serial.println("currentScreenLevel:");

  while (currentlyTransitioning == true)  {
    for (int i = 0; i < NUMBEROFLEDS; i++)  {
      if (stayingOn[i] == true)  {
        currentScreenLevel[i] = currentBrightness;   
      }
      else if (goingUp[i] == true) {
        // find the current transition up array index and set the brightness to that
        // if you've reached the end of the transition (which they both should do at
        // the same time), then set currentlyTransitioning to false
        currentlyTransitioning = false;  // REMOVE ME, JUST KEPT FROM WHILE LOOP TRAP
      }
      else if (goingDown[i] == true) {
        // find the current transition down array index and set the brightness to that
        // if you've reached the end of the transition (which they both should do at
        // the same time), then set currentlyTransitioning to false
      }
      else  {
        currentScreenLevel[i] = 0x00;
      }
    }
    // leds.show here
      printArrayByte(currentScreenLevel,NUMBEROFLEDS);
    // delay for 1 frame duration
  }

  // keep worrying about transitions up and down. Have fake steps and trackers above

  // TODO: LIGHT UP SOME LEDS WITH A SIMPLE ARRAY READ-IN FUNCTION, TEST THAT
  // THEN WORRY ABOUT IMPLEMENTING ALL "SCREENS" FOR TIME AND TIME CODE: EASY WIN

  Serial.println("--------------------------------");
  delay(3000);
}

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
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
    Serial.print(theArray[i]);
    Serial.print(" ");
  }
  Serial.println();
}
