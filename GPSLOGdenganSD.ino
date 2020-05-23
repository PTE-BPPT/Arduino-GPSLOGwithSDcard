
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

SoftwareSerial mySerial(3, 2);
File myFile;

Adafruit_GPS GPS(&mySerial);


// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences. 
#define GPSECHO  false

boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

void setup()  
{
    
  Serial.begin(9600);
  Serial.println("GPS LOGGING");
  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  GPS.begin(9600);
  
  useInterrupt(true);

  delay(1000);
}


// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
#endif
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

uint32_t timer = millis();
void loop()                     // run over and over again
{

  if (! usingInterrupt) {
    char c = GPS.read();
  }
  
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    myFile = SD.open("gpslog.txt", FILE_WRITE);
    if (myFile) {
      //format print #GPSLOG,TIME,DATE,FIX,QUALITY,LATITUDE,LONGITUDE,ALTITUDE,SATELLITES
    myFile.print("#GPSLOG,");
    myFile.print(",");
    myFile.print(GPS.hour+7, DEC); myFile.print(':');
    myFile.print(GPS.minute, DEC); myFile.print(':');
    myFile.print(GPS.seconds, DEC);
    myFile.print(",");
    myFile.print(GPS.day, DEC); myFile.print('/');
    myFile.print(GPS.month, DEC); myFile.print("/20");
    myFile.print(GPS.year, DEC);
    myFile.print(",");
    myFile.print((int)GPS.fix);
    myFile.print(",");
    myFile.print((int)GPS.fixquality); 
    myFile.print(",");
    if (GPS.fix) {
      myFile.print(GPS.latitudeDegrees, 8);
      myFile.print(","); 
      myFile.print(GPS.longitudeDegrees, 8);
      myFile.print(","); 
      myFile.print(GPS.altitude);
      myFile.print(","); 
      myFile.print((int)GPS.satellites);
      myFile.print(",");
    }
    myFile.println(" ");
    // close the file:
    myFile.close();

    }
  
  }
}

