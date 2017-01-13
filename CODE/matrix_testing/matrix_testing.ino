#define ROWS 2
#define COLUMNS 11
#define NUMBEROFLEDS 22

byte currentBrightness = 0x05;

bool currentScreen[NUMBEROFLEDS];
bool nextScreen[NUMBEROFLEDS];
int8_t deltaScreen[NUMBEROFLEDS]; // +1 = pin is increasing, -1 = decreasing

//memset(currentScreen,0,sizeof(currentScreen));
//memset(nextScreen,0,sizeof(nextScreen));
//memset(deltaScreen,0,sizeof(deltaScreen));

bool IT_IS[NUMBEROFLEDS] =
  {1,1,0,1,1,0,0,0,0,0,0,   // {I,T,_,I,S,_,_,_,_,_,_,
   0,0,0,0,0,0,0,0,0,0,0};  //  _,_,_,_,_,_,_,_,_,_,_}

bool A_QUARTER[NUMBEROFLEDS] =
  {0,0,0,0,0,0,0,0,0,0,0,   // {_,_,_,_,_,_,_,_,_,_,_,
   1,0,1,1,1,1,1,1,1,0,0};  //  A,_,Q,U,A,R,T,E,R,_,_}

bool MIKE_AND_EM[NUMBEROFLEDS] =
  {0,0,0,0,0,0,1,1,1,1,1,   // {_,_,_,_,_,_,M,I,K,E,&,
   0,0,0,0,0,0,0,0,0,1,1};  //  _,_,_,_,_,_,_,_,_,E,M}

void setup() {
  Serial.begin(9600);
}

void loop() {
  zeroOutArray(currentScreen,NUMBEROFLEDS);
  printArray(currentScreen,NUMBEROFLEDS);

  // create temp array to hold the OR combined result
  bool tempCompiled[NUMBEROFLEDS];

  combineArrays(IT_IS, MIKE_AND_EM, &tempCompiled[0], NUMBEROFLEDS);
  printArray(tempCompiled,NUMBEROFLEDS);

  combineArrays(tempCompiled, A_QUARTER, &tempCompiled[0], NUMBEROFLEDS);
  printArray(tempCompiled,NUMBEROFLEDS);

  // TODO: LIGHT UP SOME LEDS WITH A SIMPLE ARRAY READ-IN FUNCTION, TEST THAT
  // THEN WORRY ABOUT TRANSITIONS
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

