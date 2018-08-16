/* Adrian Doran
*  ID: 217195412
* Soil Moisture detection and LED Warning Light. 
* Planter Watering Aid.
*/ 

#include "RTClib.h"
#include "SD.h"
#include <Wire.h>

#define LOG_INTERVAL  1000 // mills between entries. 
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   1 // echo data to serial port. 


//Variables
char activeMotion [] = "Active";
char inactiveMotion [] = "Inactive";
char* state ;

RTC_PCF8523 RTC; // define the Real Time Clock

const int chipSelect = 10; // for data logging, this connects the SD Card, Digital Pin 10

File logfile; // defines the file for logging

void setup()
{
  Serial.begin(9600);
  initSDcard(); // initialize the SD card for recording
  createFile(); // create a new file each startup

  initRTC(); // Intialize the Real Time Clock, this is for time accuracy.

  logfile.println("millis,stamp,datetime,moisture");// CSV Header for the first row
#if ECHO_TO_SERIAL
  Serial.println("millis,stamp,datetime,moisture"); // Serial Monitor Header for display
#endif //ECHO_TO_SERIAL

  pinMode(A0, INPUT);
  pinMode(2, OUTPUT);
}

void loop()
{
  int val;
  DateTime now;

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));

  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m);           // milliseconds since start
  logfile.print(", ");
#if ECHO_TO_SERIAL
  Serial.print(m);         // milliseconds since start
  Serial.print(", ");
#endif

  // fetch the time
  now = RTC.now();
  // log time
  logfile.print(now.unixtime()); // seconds since 2000
  logfile.print(", ");
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
#if ECHO_TO_SERIAL
  Serial.print(now.unixtime()); // seconds since 2000
  Serial.print(", ");
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
#endif //ECHO_TO_SERIAL
  
  val = analogRead(0); //connect sensor to Analog 0

if (val < 300)
  digitalWrite(2, HIGH);
else
  digitalWrite(2, LOW);

  logfile.print(", ");
  logfile.println(val); // outputs the data to the CSV

#if ECHO_TO_SERIAL
  Serial.print(", "); // outputs the data to the serial monitor.
  Serial.println(val);

#endif //ECHO_TO_SERIAL

  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();

  logfile.flush();
}

void error(char const *str) // Error checking function
{
  Serial.print("error: ");
  Serial.println(str);

  while (1);
}

void initSDcard()
{
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

}

void createFile()
{
  //file name must be in 8.3 format (name length at most 8 characters, follwed by a '.' and then a three character extension.
  char filename[] = "SM_LOG00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[4] = i / 10 + '0';
    filename[5] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }

  if (! logfile) {
    error("couldnt create file");
  }

  Serial.print("Logging to: ");
  Serial.println(filename);
}

void initRTC()
{
  Wire.begin();
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL

  }
}

