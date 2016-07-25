/*
   Copyright 2011 Michael Stebbins
   http://students.washington.edu/mikesteb/projects/Qlock_LED_Clock/Qlock_LED_clock.html
   Licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
      
   Based on code from Marcus Liang's QlockTwo Clone, 
   Copyright 2009, information: http://www.flickr.com/photos/19203306@N00/sets/72157622998814956/ 
   Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
   except in compliance with the License.  You may obtain a copy of the License at
   http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software distribserialserialuted under the 
   License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
   either express or implied. See the License for the specific language governing permissions 
   and limitations under the License.
   
   Use "Diecimila/Duemilanove with 328P" in Arduino IDE to upload code.
   
   Ran versino 12 for some time in clock, 13 is first attempt at Arduino 1.0.* IDE.
   Changed #included <WProgram.h> to <Arduino.h>, updated DS1307, LedControl libraries.
 */

#include <Arduino.h>
#include <Wire.h>
#include <DS1307.h>  
#include <LedControl.h>
#include <binary.h>

// Photo resistor cell setup
int photocellPin = 0; // the cell and 10K pulldown are connected to a0
int photocellReading; // the analog reading from the sensor divider

void setup()  {
Serial.begin(38400);
}
void loop() {

// Read and serial print brightness of photocell
    photocellReading = analogRead(photocellPin);
//Serial.print("Analog reading = ");
Serial.println(photocellReading); // the raw analog reading
delay(200);
}
