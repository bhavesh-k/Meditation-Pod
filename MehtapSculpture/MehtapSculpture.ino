// libraries
#include <SPI.h>              // SPI interface to music decoder chip
#include <SD.h>               // SD card will store music
#include <Adafruit_VS1053.h>  // music decoder chip interface library

// pins
#define PIR_P          25     // digital input from PIR sensor on pin 25 of Adafruit HUZZAH32 Feather
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)
#define VS1053_CS      32     // VS1053 chip select pin (output)
#define VS1053_DCS     33     // VS1053 Data/command select pin (output)
#define CARDCS         14     // Card chip select pin
#define VS1053_DREQ    15     // VS1053 Data request, ideally an Interrupt pin

// occupancy status from PIR sensor (0 means unoccupied)
int occupancy = 0;

// instantiate the music decoder chip interface object
Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

void setup() {
  Serial.begin(115200);   // set up serial port to PC
  pinMode(PIR_P, INPUT);  // set up PIR/occupancy digital signal pin as input

  // initialize the music player module
  if (!musicPlayer.begin()) {
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("VS1053 found"));

  // initialize the SD card module
  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD OK!");

  // print list of files on the SD card
  printDirectory(SD.open("/"), 0);
  
  // Set volume for left, right channels. lower numbers => louder volume!
  musicPlayer.setVolume(0, 0);
}

void loop() {

  // check for occupancy
  occupancy = digitalRead(PIR_P);

  // if occupancy is detected, play the music/rain sounds
  if(occupancy)
  {
    musicPlayer.startPlayingFile("/raindrop.mp3"); 
    while(occupancy)
    {
      // buffer in the next block of music data while occupancy is being detected
      musicPlayer.feedBuffer();
      delay(5);
      
      // check for occupancy from the PIR pin and stop playing music
      // if the person has left (occupancy reports zero)
      occupancy = digitalRead(PIR_P);
    }

    musicPlayer.stopPlaying();
  }

  delay(50); // 50 ms delay between occupancy checks
}


// Utility function to list files present on SD card (used in setup())
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}
