/*
 * TimeRTC.pde
 * example code illustrating Time library with Real Time Clock.
 * 
 */
#include <TimeLib.h>

int counter = 0;
#define DEFAULTTIME 1483228800 // seconds since epoch as of 1/1/17 00:00:00


void setup()  {
  // set the Time library to use Teensy 3.0's RTC to keep time
//  setSyncProvider(getTeensy3Time);
//  setSyncInterval(60);

  Serial.begin(115200);

  while (!Serial);  // Wait for Arduino Serial Monitor to open
  delay(100);
  if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }
  adjustTime(DEFAULTTIME);
}

void loop() {
  counter++;  
//  if (Serial.available()) {
//    time_t t = processSyncMessage();
//    if (t != 0) {
//      Teensy3Clock.set(t); // set the RTC
//      setTime(t);
//    }
//  }
//  digitalClockDisplay();  
//  Serial.println(now());
  if (counter%5 == 0)  {
    int temp = now()%60;  
    adjustTime(3600-temp);
  }

  digitalClockDisplay();
  Serial.println(now());
  delay(1000);
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     return pctime;
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
       pctime = 0L; // return 0 to indicate that the time is not valid
     }
  }
  return pctime;
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
