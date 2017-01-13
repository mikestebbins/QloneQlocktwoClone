//    DELETE ALL CODE THAT BEGINS WITH ////
/*
    Copyright 2017 Michael Stebbins
    Licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

    Based on code from Marcus Liang's QlockTwo Clone, 
    Copyright 2009, information: http://www.flickr.com/photos/19203306@N00/sets/72157622998814956/ 
    Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
    except in compliance with the License.  You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software distributed under the 
    License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
    either express or implied. See the License for the specific language governing permissions 
    and limitations under the License.

    Ran version 14 for some time, with LEDs continuing to fail.  Decided to replace individual, inexpensive
    white LEDs with Adafruit Dotstar individually-addressable Cool White color temp lights.
    version_reboot starts from version 14, with changes to move to Teensy 3.2 (with soldered-on chip for 
    Teensy RealTimeClock), Dotstar lights and 3.3 to 5 volt level shifter. This completely changes the 
    "charlie-plexed" work and LED drivers necessary in the previous version.
 
*  LAST UPDATED: 07-24-2016
*  Copyright 2016 Mike Stebbins
*  Arduino X.X.X and Windows 10 used to upload

TODO's:

//-------------------------------------------------------------------------------------------------------------
//PIN ASSIGNMENTS
//-------------------------------------------------------------------------------------------------------------
                          
Teensy 3.1 / 3.2
                                  |      |
                          --------|      |--------
        Power Supply (-) [] Gnd              Vin [] Power Supply (+)
                         [] RX1             AGND []
                         [] TX1             3.3V []             
                         [] 02                23 []
                         [] 03                22 []
                         [] 04                21 []
                         [] 05                20 []
                         [] 06                19 [] SCL
                         [] 07                18 [] SDA
                         [] 08                17 []
                         |  09                16 []
                         [] 10                15 []
                         [] 11                14 []
                         [] 12                13 []
                          |    (+)   (-)         |
                          |     [] [] [] [] []   |
                          ------------------------
                                3 volt coin cell for maintaining RTC


 
 74AHCT125 Voltage Level Shifter
                     --------------
                GND [] 1OE    VCC [] 5V input
   Strip 1 CLOCK In [] 1A     4OE [] 
  Strip 1 CLOCK Out [] 1Y      4A [] 
                GND [] 2OE     4Y [] 
    Strip 1 DATA In [] 2A     3OE [] 
   Strip 1 DATA Out [] 2Y      3A [] 
         Teensy GND [] GND     3Y [] 
                     --------------

 Power Supply - 5V, 2.4A
                        [] +
                        [] -
                                        
//-------------------------------------------------------------------------------------------------------------       
*/

/*
LETTER to LED strip NUMBER decoder

111                                                                    112
      000   001   002   003   004   005   006   007   008   009   010 
      021   020   019   018   017   016   015   014   013   012   011
      022   023   024   025   026   027   028   029   030   031   032   
      043   042   041   040   039   038   037   036   035   034   033
      044   045   046   047   048   049   050   051   052   053   054   
      065   064   063   062   061   060   059   058   057   056   055
      066   067   068   069   070   071   072   073   074   075   076   
      087   086   085   084   083   082   081   080   079   078   077
      088   089   090   091   092   093   094   095   096   097   098   
      109   108   107   106   105   104   103   102   101   100   099
110                                                                    113      


                      
min1                                                                   min2                    
       I     T     L     I     S     Y     M     I     K     E     &
       A     D     Q     U     A     R     T     E     R     E     M
       T     W     E     N     T     Y     F     I     V     E     X
       H     A     L     F     B     T     E     N     F     T     O
       P     A     S     T     E     R     U     N     I     N     E
       O     N     E     S     I     X     T     H     R     E     E
       F     O     U     R     F     I     V     E     T     W     O
       E     I     G     H     T     E     L     E     V     E     N
       S     E     V     E     N     T     W     E     L     V     E
       T     E     N     S     E     O     C     L     O     C     K
min4                                                                   min3  

*/

//-------------------------------------------------------------------------------------------------------------
//INCLUDES
//-------------------------------------------------------------------------------------------------------------
//// #include <Arduino.h>
//// #include <Wire.h>

#include <Adafruit_DotStar.h>
#include <binary.h>

//-------------------------------------------------------------------------------------------------------------
//CONSTANTS
//-------------------------------------------------------------------------------------------------------------

// How many leds are in the strip?
#define NUM_LEDS 114

#define DATAPIN    11
#define CLOCKPIN   10

// CHECK OR UPDATE
#define BUT3 = 9   // minute++
#define BUT2 = 8   // hour++
#define BUT1 = 10  // change mode++

// LED intensity  
// CHECK OR UPDATE
//// // Four levels of light intensity plus off
//// const int LEDOFF  = 0;  // not really off
//// const int LEDINT1 = 1;
//// const int LEDINT2 = 7;
//// const int LEDINT3 = 11;
//// const int LEDINT4 = 15;

// Photo resistor cell setup
// CHECK OR UPDATE
int photocellPin = 0; // the cell and 10K pulldown are connected to A0

// MODE
const int MODEDEFAULT = 0;
const int MODEDEFAULTSEC = 1;
const int MODESECONDS = 2;
const int MODETEST = 3;
const int MODELOVE = 4;

// update/debounce delays
const int ledDelay = 50;           //(milliseconds)
const int buttonPressDelay = 400;   //(milliseconds)

//-------------------------------------------------------------------------------------------------------------
//VARIABLES
//-------------------------------------------------------------------------------------------------------------
Adafruit_DotStar strip = Adafruit_DotStar(
  NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

int photocellReading; // the analog reading from the sensor divider
unsigned long ledLastUpdate = 0;
int currentMode = MODEDEFAULT;
boolean forceUpdate = true;

int cHour;
int cMin;
int cSec;

long previousMillis = 0;      // will store last time LED was updated
long pause = 10;              // (millis) interval at which to blink individual letters, test case
long longpause = 25;          // (millis) interval at which to blink whole words, test case
int testCase = -1;            // initialize case number for test case

unsigned long but1LastPress = 0;  
unsigned long but2LastPress = 0;  
unsigned long but3LastPress = 0;  

int currentLEDIntensity = 0;   // how bright the LEDs are 
char loveCase = 'a';      //initial case for LOVE switchcase
int dly = 1500;      //length of pause between Mike&Em and heart

//-------------------------------------------------------------------------------------------------------------
//SETUP
//-------------------------------------------------------------------------------------------------------------
void setup(void) {
  delay(1000); 

  //CHECK OR UPDATE
  pinMode (BUT1, INPUT);
  pinMode (BUT2, INPUT);
  pinMode (BUT3, INPUT);  

  Serial.begin(38400);
}

//-------------------------------------------------------------------------------------------------------------
//LOOP
//-------------------------------------------------------------------------------------------------------------
void loop() {

//TODO: ADD IN CODE TO LIGHT UP LEDS, AMONGST OTHER THINGS

// update LEDs and choose run mode    
  if ((millis() - ledLastUpdate) > ledDelay) {
    ledLastUpdate = millis();
      if (currentMode == MODEDEFAULT) mode_default();
      else if (currentMode == MODEDEFAULTSEC) mode_defaultsec();
      else if (currentMode == MODESECONDS) mode_seconds();
      else if (currentMode == MODETEST) mode_test();
      else if (currentMode == MODELOVE) mode_love();
  }
  
// Read and serial print brightness of photocell
    photocellReading = analogRead(photocellPin);
    //Serial.print("Analog reading = ");
    //Serial.println(photocellReading); // the raw analog reading

// modified non-linear mapping of photoresitor to LED intensity
if (photocellReading > 499 && photocellReading < 600 ) {currentLEDIntensity = 2;  }
else if (photocellReading > 599 && photocellReading < 700 ) {currentLEDIntensity = 3;  }
else if (photocellReading > 699 && photocellReading < 800 ) {currentLEDIntensity = 4;  }
else if (photocellReading > 799 && photocellReading < 900 ) {currentLEDIntensity = 6;  }
else if (photocellReading > 899 && photocellReading < 1000 ) {currentLEDIntensity = 9;  }
else if (photocellReading > 999) {currentLEDIntensity = 13;  }
else {currentLEDIntensity = 1;  }

// update the LEDs intensity
LC1.setIntensity(0,currentLEDIntensity);
LC2.setIntensity(0,currentLEDIntensity);
}

void checkButtons()  {
  int but1read = digitalRead(BUT1);
  int but2read = digitalRead(BUT2);
  int but3read = digitalRead(BUT3);

  if ((but1read == HIGH) && ((millis() - but1LastPress) > buttonPressDelay)) {
    but1LastPress = millis();
    doButton1();
  }
  else if ((but1read == LOW) && ((millis() - but1LastPress) > buttonPressDelay ))
    but1LastPress = 0;  // reset
  
  if ((but2read == HIGH) && ((millis() - but2LastPress) > buttonPressDelay)) {
    but2LastPress = millis();
    doButton2();
  }
  else if ((but2read == LOW) && ((millis() - but2LastPress) > buttonPressDelay ))
    but2LastPress = 0;  // reset
    
  if ((but3read == HIGH) && ((millis() - but3LastPress) > buttonPressDelay)) {
    but3LastPress = millis();
    doButton3();
  }
  else if ((but3read == LOW) && ((millis() - but3LastPress) > buttonPressDelay ))
   but3LastPress = 0;  // reset  
}

void doButton1() {
  // mode change

  if (currentMode == MODEDEFAULT) currentMode = MODEDEFAULTSEC;
  else if (currentMode == MODEDEFAULTSEC) currentMode = MODESECONDS;
  else if (currentMode == MODESECONDS) currentMode = MODETEST;
  else if (currentMode == MODETEST) currentMode = MODELOVE;
  else if (currentMode == MODELOVE) currentMode = MODEDEFAULT;
  
  LED_CLEAR();
  forceUpdate = true; 
}

void doButton2() {
  // update hour
  int hour = RTC.get(DS1307_HR,true);
  if (hour == 23) hour = 0;
  else hour = hour + 1;
  RTC.set(DS1307_HR,hour);
}

void doButton3() {
  // update minutes
  int min = RTC.get(DS1307_MIN,true);
  if (min == 59) min = 0;
  else min = min+1;
  RTC.set(DS1307_MIN,min);
  RTC.set(DS1307_SEC,0);              // zero the seconds.
}


// MODE DEFAULT: This is the default mode of operation.  The words light up based on the time and the 4 corners light up 
// based on the minutes past 5 minutes.

void mode_default() {

   int hour = RTC.get(DS1307_HR,true);
   int min = RTC.get(DS1307_MIN,true);
   int sec = RTC.get(DS1307_SEC,false); // kinda redundant?
   
   if ((hour == cHour) && (min == cMin) && (forceUpdate == false))
		return;
   	
   int tpast5mins = min % 5; // remainder
   int t5mins = min - tpast5mins;
   int tHour = hour;
   
   if (tHour > 12) tHour = tHour - 12;
   else if (tHour == 0) tHour = 12;
   
   LED_CLEAR();
   W_ITIS();
   
   if (t5mins == 5 || t5mins == 55)     M_FIVE();        // 5 past or 5 to..
   else if (t5mins == 10 || t5mins == 50)    M_TEN();        // 10 past or 10 to..
   else if (t5mins == 15 || t5mins == 45)    M_AQUARTER();    // etc..
   else if (t5mins == 20 || t5mins == 40)    M_TWENTY();
   else if (t5mins == 25 || t5mins == 35)    M_TWENTYFIVE();
   else if (t5mins == 30)    M_HALF();

   // past or to or o'clock?
   if (t5mins == 0)	W_OCLOCK();
   else if (t5mins > 30)	W_TO();
   else W_PAST();
   
   if (t5mins > 30)	{
		tHour = tHour+1;
		if (tHour > 12) tHour = 1;
   }

   // light up the hour word
   if (tHour == 1) H_ONE(); else if (tHour == 2) H_TWO(); else if (tHour == 3) H_THREE(); else if (tHour == 4) H_FOUR();
   else if (tHour == 5) H_FIVE(); else if (tHour == 6) H_SIX(); else if (tHour == 7) H_SEVEN(); else if (tHour == 8) H_EIGHT();
   else if (tHour == 9) H_NINE(); else if (tHour == 10) H_TEN(); else if (tHour == 11) H_ELEVEN(); else if (tHour == 12) H_TWELVE();
   
   // light up aux minute LED
   // ugly but quicker 
   if (tpast5mins == 0 ) { }
   else if (tpast5mins == 1) { P_ONE(); }
   else if (tpast5mins == 2) { P_ONE(); P_TWO(); }
   if (tpast5mins == 3) { P_ONE(); P_TWO(); P_THREE(); }
   if (tpast5mins == 4) { P_ONE(); P_TWO(); P_THREE(); P_FOUR(); }

   // save last updated time
   cHour = hour;
   cMin = min;
   cSec = sec;
   forceUpdate = false;
}

// MODE DEFAULTSEC: This is like Mode default except that the dots in the four corners will change every second.
void mode_defaultsec() {
   int hour = RTC.get(DS1307_HR,true);
   int min = RTC.get(DS1307_MIN,false);
   int sec = RTC.get(DS1307_SEC,true); 
   
   // minute or hour has changed,..
   if ((hour != cHour) || (min != cMin) || (forceUpdate == true))  {
  
	   int tpast5mins = min % 5; // remainder
	   int t5mins = min - tpast5mins;
	   int tHour = hour;

           // we don't do 24H clocks
           if (tHour > 12) tHour = tHour - 12;
           else if (tHour == 0) tHour = 12;
  
	   
	   LED_CLEAR();
	   W_ITIS();
  	   
       if (t5mins == 5 || t5mins == 55)     M_FIVE();        // 5 past or 5 to..
       else if (t5mins == 10 || t5mins == 50)    M_TEN();        // 10 past or 10 to..
       else if (t5mins == 15 || t5mins == 45)    M_AQUARTER();    // etc..
       else if (t5mins == 20 || t5mins == 40)    M_TWENTY();
       else if (t5mins == 25 || t5mins == 35)    M_TWENTYFIVE();
       else if (t5mins == 30)    M_HALF();
           
	   // past or to or o'clock?
	   if (t5mins == 0)	W_OCLOCK();
	   else if (t5mins > 30)	W_TO();
	   else W_PAST();
	   
	   if (t5mins > 30)	{
			tHour = tHour+1;
			if (tHour > 12) tHour = 1;
	   }
		
	   // light up the hour word
	   if (tHour == 1) H_ONE(); else if (tHour == 2) H_TWO(); else if (tHour == 3) H_THREE(); else if (tHour == 4) H_FOUR();
	   else if (tHour == 5) H_FIVE(); else if (tHour == 6) H_SIX(); else if (tHour == 7) H_SEVEN(); else if (tHour == 8) H_EIGHT();
	   else if (tHour == 9) H_NINE(); else if (tHour == 10) H_TEN(); else if (tHour == 11) H_ELEVEN(); else if (tHour == 12) H_TWELVE();
	}
	
	if (sec != cSec) {
		// update the seconds;
		//P_CLEAR();
		int r = sec % 10;

                if (r > 5) r = r - 5;

		// 4 patterns.
		// 0-4,   5-9,   10-14, 15-19 then repeat.
		// 20-24, 25-29, 30-34, 35-39
		// 40-44, 45-49, 50-54, 55-59
	
		if (((sec > 0) && (sec < 5)) || ((sec > 20) && (sec < 25)) || ((sec > 40) && (sec < 45))) {
			if (r == 1) { P_ONE(); }
			else if (r == 2) {  P_ONE();  P_TWO(); }
			else if (r == 3) {  P_ONE(); P_TWO();  P_THREE();}
			else if (r == 4) {  P_ONE(); P_TWO(); P_THREE(); P_FOUR();}
		}
	
		else if (((sec > 5) && (sec < 10)) || ((sec > 25) && (sec < 30)) || ((sec > 45) && (sec < 50))) {
			if (r == 1) { P_TWO(); }
			else if (r == 2) { P_TWO(); P_THREE(); }
			else if (r == 3) { P_TWO(); P_THREE(); P_FOUR();}
			else if (r == 4) { P_TWO(); P_THREE(); P_FOUR(); P_ONE();}
		}
		
		else if (((sec > 10) && (sec < 15)) || ((sec > 30) && (sec < 35)) || ((sec > 50) && (sec < 55)))  {
			if (r == 1) { P_THREE(); }
			else if (r == 2) { P_THREE(); P_FOUR(); }
			else if (r == 3) { P_THREE(); P_FOUR(); P_ONE();}
			else if (r == 4) { P_THREE(); P_FOUR(); P_ONE(); P_TWO();}
		}
				
		else if (((sec > 15) && (sec < 20)) || ((sec > 35) && (sec < 40)) || ((sec > 55) && (sec < 60)))  {
			if (r == 1) { P_FOUR(); }
			else if (r == 2) { P_FOUR(); P_ONE(); }
			else if (r == 3) { P_FOUR(); P_ONE(); P_TWO();}
			else if (r == 4) { P_FOUR(); P_ONE(); P_TWO(); P_THREE();}
		}
		
		else if ((r == 0)  || (r == 5)) {
    		      P_CLEAR();
		}
	}

	// save last updated time
   cHour = hour;
   cMin = min;
   cSec = sec;
   forceUpdate = false;   
}


// MODE SECONDS: The entire face will show the "seconds" the clock is on
void mode_seconds() {
   
   int hour = RTC.get(DS1307_HR,true);
   int min = RTC.get(DS1307_MIN,false);
   int sec = RTC.get(DS1307_SEC,true); 

   // no seconds change, do nothing
   if (sec == cSec) return;
 
   int tsec = sec;
    
   // decide if we only want to draw the right number of both numbers.
   // reduce the apparentness of the flicker of the non changing digit.
     if ((tsec - (tsec % 10) != cSec - (cSec % 10)) || (forceUpdate == true)) {
     LED_CLEAR();   
     if (tsec < 10) L_ZERO();
     else if (tsec < 20)  L_ONE();
     else if (tsec < 30)  L_TWO();
     else if (tsec < 40)  L_THREE();
     else if (tsec < 50)  L_FOUR();
     else L_FIVE();
   }
   else {
    R_CLEAR();
   }  
   
   // seconds have changed, draw the seconds.
	
	tsec = tsec % 10;

	if (tsec == 0) R_ZERO();
	if (tsec == 1) R_ONE();
	if (tsec == 2) R_TWO();
	if (tsec == 3) R_THREE();
	if (tsec == 4) R_FOUR();
	if (tsec == 5) R_FIVE();
	if (tsec == 6) R_SIX();
	if (tsec == 7) R_SEVEN();
	if (tsec == 8) R_EIGHT();
	if (tsec == 9) R_NINE();
   
   // save last updated time
   cHour = hour;
   cMin = min;
   cSec = sec;   
   forceUpdate = false;   
   
}

// MODE TEST: Cycle through individual words and then LEDs
void mode_test() {
    
    unsigned long currentMillis = millis();
    
    if (testCase < 114)   {
          if(currentMillis - previousMillis > pause) {
              // save the last time you switched cases
              previousMillis = currentMillis;  
              // Serial.println(currentMillis);

              // increment the case number by one, unless it has reached 
              //the last case, then go back to zero:
              if (testCase < 113)   {
              testCase ++; } 
              else     {
            testCase = 0;  }
          }
        }     

    switch (testCase) { 
        case 0:
          LED_CLEAR();     LC1.setLed(0,0,0,true); 
        break;
        case 1:
          LED_CLEAR();     LC1.setLed(0,0,1,true); 
        break;
    }
}

// MODE LOVE: The face flashes between "Mike & Em" and a Heart shape
void mode_love() {
  
unsigned long currentMillis = millis();

if ((currentMillis - previousMillis > dly) || (forceUpdate == true))
    {
    // save the last time you switched cases
    previousMillis = currentMillis;

    // switch the case from it's previous value
    if (loveCase == 'b') {
      loveCase = 'a'; 
          }
    else {
      loveCase = 'b';
          }       

    switch (loveCase) { 
    case 'a':
    LED_CLEAR();     LOVE();  
    break;
    case 'b':
    LED_CLEAR();     HEART();  
break;
}
// ensure forceUpdate is set to false
forceUpdate = false;  
}
else
{}
}

/* For the LED code below to work, the LEDs need to be wired in a very specific way
   
   Normally a MAX7219's matrix is wired this way (8x8)

    dp| a| b| c| d| e| f| g
  0
  1
  2
  4
  5
  6
  7
   
  However, since the clock is 10x11 we wire 2x MAX7219 this way.
  Essentially, row 5,6,7 is transposed into column 9,10,11
    
  [dp,5]                         [g,5]
  -----------------------------------
   dp| a| b| c| d| e| f| g| 5| 6| 7
  0                       |         e - 5
  1                       |         d - 4
  2                       |         c - 3
  3                       |         b - 2
  4                       |         a - 1
  -----------------------------------
   dp| a| b| c| d| e| f| g| 5| 6| 7
  0                       |         e
  1                       |         d
  2                       |         c
  3                       |         b
  4                       |         a
  ------------------------------------
  [dp,5]                         [g,5]
 
*/

// LED Turn on/off procedures
void LED_CLEAR() {
  LC1.clearDisplay(0);
  LC2.clearDisplay(0);
}
void R_CLEAR() {
  LC1.setColumn(0,6,B00000000);
  LC1.setColumn(0,7,B00000000);
  LC1.setRow(0,5,B00000000);
  LC1.setRow(0,6,B00000000);
  LC1.setRow(0,7,B00000000);
  LC2.setColumn(0,6,B00000000);
  LC2.setColumn(0,7,B00000000);
  LC2.setRow(0,5,B00000000);
  LC2.setRow(0,6,B00000000);
  LC2.setRow(0,7,B00000000);
  
}

void M_FIVE() {
	LC1.setRow(0,2,B00000011); // FI
	LC1.setLed(0,5,3, true); // V
	LC1.setLed(0,6,3, true); // E
}
void M_TEN() {
	LC1.setRow(0,3,B00000111);
}
void M_AQUARTER() {
	LC1.setRow(0,1,B10111111); // A QUARTE
	LC1.setLed(0,5,4, true); // R
}
void M_TWENTY() {
	LC1.setRow(0,2,B11111100); // TWENTY
}
void M_TWENTYFIVE() {
	LC1.setRow(0,2,B11111111); // TWENTYFI
	LC1.setLed(0,5,3, true); // V
	LC1.setLed(0,6,3, true); // E
}
void M_HALF() {
	LC1.setRow(0,3,B11110000); // HALF
}
void W_ITIS() {
	// Row0 "IT IS" (R0=216) OO.OO.......
	LC1.setRow(0,0,B11011000);  // IT IS
}
void W_OCLOCK() {
	LC2.setLed(0,4,5,true); // O'
	LC2.setLed(0,4,6,true); // C
	LC2.setLed(0,4,7,true);	// L
	LC2.setLed(0,5,1,true); // O
	LC2.setLed(0,6,1,true); // C
	LC2.setLed(0,7,1,true);	// K
}
void W_TO() {
	LC1.setLed(0,6,2,true); // T
	LC1.setLed(0,7,2,true); // O
}

void W_PAST(){
	//LC1.setRow(0,0,B11110000); // PAST
	LC1.setLed(0,4,0,true); // P
	LC1.setLed(0,4,1,true); // A
	LC1.setLed(0,4,2,true); // S
	LC1.setLed(0,4,3,true); // T
}

void H_ONE(){
	LC2.setRow(0,0,B11100000); // ONE
}

void H_TWO(){
	LC2.setLed(0,5,4,true); // T
	LC2.setLed(0,6,4,true); // W
	LC2.setLed(0,7,4,true);  // O
}
void H_THREE(){
	LC2.setRow(0,0,B00000011); // TH
	LC2.setLed(0,5,5, true); //R
	LC2.setLed(0,6,5, true); //E
	LC2.setLed(0,7,5, true); //E
}
void H_FOUR(){
	LC2.setRow(0,1,B11110000); // FOUR
}
void H_FIVE(){
	LC2.setRow(0,1,B00001111); // FIVE
}
void H_SIX(){
	LC2.setRow(0,0,B00011100); // SIX
}
void H_SEVEN(){
	LC2.setRow(0,3,B11111000); // SEVEN...
}

void H_EIGHT(){
	LC2.setRow(0,2,B11111000);  //EIGHT...
}

void H_NINE(){
	LC1.setLed(0,4,7,true); // N
   	LC1.setLed(0,5,1,true); // I
  	LC1.setLed(0,6,1,true); // N
  	LC1.setLed(0,7,1,true); // E
}

void H_TEN(){
	LC2.setLed(0,4,0,true); // T
	LC2.setLed(0,4,1,true); // E
	LC2.setLed(0,4,2,true);	// N
}

void H_ELEVEN(){
	LC2.setRow(0,2,B00000111); //ELE
	LC2.setLed(0,5,3,true); //V
	LC2.setLed(0,6,3,true); //E
	LC2.setLed(0,7,3,true); //N
}
void H_TWELVE(){
	LC2.setRow(0,3,B00000111); // TWE
	LC2.setLed(0,5,2,true); //L
	LC2.setLed(0,6,2,true); //V
	LC2.setLed(0,7,2,true); //E
}

void P_ONE() {
    LC1.setLed(0,5,0,true); // top left
}
void P_TWO() {
	LC1.setLed(0,5,7,true); // top right
}
void P_THREE() {
	LC2.setLed(0,5,7,true);// bottom right

}
void P_FOUR() {
	LC2.setLed(0,5,0,true); // bottom left
}


// SECONDS COUNTER MODE
void L_ZERO(){
	LC1.setRow(0,2,B01110000);
	LC1.setRow(0,3,B10001000);
	LC1.setRow(0,4,B10011000);
	LC2.setRow(0,0,B10101000);
	LC2.setRow(0,1,B11001000);
	LC2.setRow(0,2,B10001000);
	LC2.setRow(0,3,B01110000);
}

void L_ONE(){
	LC1.setRow(0,2,B00100000);
	LC1.setRow(0,3,B01100000);
	LC1.setRow(0,4,B00100000);
	LC2.setRow(0,0,B00100000);
	LC2.setRow(0,1,B00100000);
	LC2.setRow(0,2,B00100000);
	LC2.setRow(0,3,B01110000);
}
void L_TWO(){
	LC1.setRow(0,2,B01110000);
	LC1.setRow(0,3,B10001000);
	LC1.setRow(0,4,B00001000);
	LC2.setRow(0,0,B00010000);
	LC2.setRow(0,1,B00100000);
	LC2.setRow(0,2,B01000000);
	LC2.setRow(0,3,B11111000);
}
void L_THREE(){
	LC1.setRow(0,2,B11111000);
	LC1.setRow(0,3,B00010000);
	LC1.setRow(0,4,B00100000);
	LC2.setRow(0,0,B00010000);
	LC2.setRow(0,1,B00001000);
	LC2.setRow(0,2,B10001000);
	LC2.setRow(0,3,B01110000);
}
void L_FOUR(){
	LC1.setRow(0,2,B00010000);
	LC1.setRow(0,3,B00110000);
	LC1.setRow(0,4,B01010000);
	LC2.setRow(0,0,B10010000);
	LC2.setRow(0,1,B11111000);
	LC2.setRow(0,2,B00010000);
	LC2.setRow(0,3,B00010000);
}
void L_FIVE(){
	LC1.setRow(0,2,B11111000);
	LC1.setRow(0,3,B10000000);
	LC1.setRow(0,4,B10000000);
	LC2.setRow(0,0,B11110000);
	LC2.setRow(0,1,B00001000);
	LC2.setRow(0,2,B10001000);
	LC2.setRow(0,3,B01110000);
}
void R_ZERO(){
	LC1.setColumn(0,6,B00011000);
	LC1.setLed(0,2,7,true);
        LC1.setLed(0,5,3,true);
	LC1.setRow(0,6,B01010000);
	LC1.setRow(0,7,B01100000);
	LC2.setColumn(0,6,B11100000);
	LC2.setColumn(0,7,B01010000);
	LC2.setRow(0,5,B00100100);
	LC2.setRow(0,6,B00100000);
        LC2.setRow(0,7,B00011100);
}
void R_ONE(){
	LC1.setLed(0,3,7,true);
	LC1.setRow(0,5,B01110000);
	LC2.setRow(0,5,B00111100);
	LC2.setLed(0,3,7,true);
	LC2.setLed(0,6,2,true);
}
void R_TWO(){
	LC1.setLed(0,3,6,true);
	LC1.setLed(0,2,7,true);
	LC1.setLed(0,5,3,true);
	LC1.setLed(0,6,3,true);
	LC1.setRow(0,7,B01100000);
	
	LC2.setLed(0,3,6,true);
	LC2.setColumn(0,7,B00110000);
	LC2.setRow(0,5,B00101000);
	LC2.setRow(0,6,B00100100);
	LC2.setLed(0,7,2,true);
}

void R_THREE(){
	LC1.setLed(0,2,6,true);
	LC1.setLed(0,2,7,true);
	LC1.setRow(0,5,B01010000);
	LC1.setRow(0,6,B00110000);
	LC1.setLed(0,7,3,true);

	LC2.setLed(0,2,6,true);
	LC2.setLed(0,3,7,true);
	LC2.setLed(0,5,2,true);
	LC2.setRow(0,6,B00100100);
	LC2.setRow(0,7,B00011000);
}

void R_FOUR(){
	LC1.setLed(0,4,7,true);
	LC1.setLed(0,5,2,true);
	LC1.setRow(0,6,B01110000);
	
	LC2.setColumn(0,6,B11000000);
	LC2.setLed(0,1,7,true);
	LC2.setLed(0,5,4,true);
	LC2.setRow(0,6,B00111100);
	LC2.setLed(0,7,4,true);
}

void R_FIVE(){
	LC1.setColumn(0,6,B00111000);
	LC1.setLed(0,2,7,true);
	LC1.setLed(0,5,3,true);
	LC1.setLed(0,6,3,true);
	LC1.setLed(0,7,3,true);
	
	LC2.setColumn(0,6,B10100000);
	LC2.setColumn(0,7,B10010000);
	LC2.setRow(0,5,B00100100);
	LC2.setRow(0,6,B00100100);
	LC2.setRow(0,7,B00011000);
}

void R_SIX(){
	LC1.setLed(0,4,6,true);
	LC1.setLed(0,3,7,true);
	LC1.setLed(0,5,3,true);
	LC1.setLed(0,6,3,true);
	
	LC2.setColumn(0,6,B11100000);
	LC2.setColumn(0,7,B10010000);
	LC2.setRow(0,5,B00100100);
	LC2.setRow(0,6,B00100100);
	LC2.setRow(0,7,B00011000);	
}

void R_SEVEN(){
	LC1.setLed(0,2,6,true);
	LC1.setLed(0,2,7,true);
	LC1.setLed(0,5,3,true);
	LC1.setRow(0,6,B01010000);
	LC1.setRow(0,7,B00110000);
	
	LC2.setColumn(0,7,B01110000);
	LC2.setLed(0,5,5,true);
}

void R_EIGHT(){
	LC1.setColumn(0,6,B00011000);
	LC1.setLed(0,2,7,true);
	LC1.setLed(0,5,3,true);
	LC1.setLed(0,6,3,true);
	LC1.setRow(0,7,B01100000);
	
	LC2.setColumn(0,6,B01100000);
	LC2.setColumn(0,7,B10010000);
	LC2.setRow(0,5,B00100100);
	LC2.setRow(0,6,B00100100);
	LC2.setRow(0,7,B00011000);	
}

void R_NINE(){
	LC1.setColumn(0,6,B00011000);
	LC1.setLed(0,2,7,true);
	LC1.setLed(0,5,3,true);
	LC1.setLed(0,6,3,true);
	LC1.setRow(0,7,B01100000);
	
	LC2.setColumn(0,7,B10010000);
	LC2.setRow(0,5,B00100100);
	LC2.setRow(0,6,B00010100);
	LC2.setRow(0,7,B00001100);
}

// for dot mode - clear the 4 dots only.
void P_CLEAR() {
	LC1.setLed(0,5,0,false); // top left
	LC1.setLed(0,5,7,false); // top right
	LC2.setLed(0,5,0,false); // top left	
	LC2.setLed(0,5,7,false); // bottom right
}

void LOVE() {
	LC1.setRow(0,0,B00000011); // MI
	LC1.setLed(0,5,5, true); // K
	LC1.setLed(0,6,5, true); // E
	LC1.setLed(0,7,5, true); // &
	LC1.setLed(0,6,4, true); // E
	LC1.setLed(0,7,4, true); // M
}

void HEART() {
	LC1.setRow(0,1,B00010001);  // top line
	LC1.setRow(0,2,B00101010);  // line 2
	LC1.setLed(0,5,3, true); // line 2 - right half
	LC1.setRow(0,3,B01000100);  // line 3
	LC1.setLed(0,6,2, true); // line 3 - right half
	LC1.setRow(0,4,B01000000);  // line 4
	LC1.setLed(0,6,1, true); // line 4 - right half
	LC2.setRow(0,0,B01000000);  // line 5
	LC2.setLed(0,6,5, true); // line 5 - right half
	LC2.setRow(0,1,B00100000);  // line 6
	LC2.setLed(0,5,4, true); // line 6 - right half
	LC2.setRow(0,2,B00010001);  // line 7
	LC2.setRow(0,3,B00001010);  // line 8
	LC2.setRow(0,4,B00000100);  // line 9
}