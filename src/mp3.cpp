#define software_tx_pin 4
#define software_rx_pin  5

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "mp3.h"

typedef void (*voidFuncPtr)(void); // Needed to track callback function
void do_nothing() {};
voidFuncPtr callback_function = do_nothing;
bool callback_needed = false;

SoftwareSerial mySoftwareSerial(software_rx_pin, software_tx_pin); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
int num_random_files;
void printDetail(uint8_t type, int value);


sclass::sclass() {
}

void sclass::setup() {
  {
    mySoftwareSerial.begin(9600);
    Serial.println();
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
    if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
      Serial.println(F("Unable to begin:"));
      Serial.println(F("1.Please recheck the connection!"));
      Serial.println(F("2.Please insert the SD card!"));
      while (true);
    }
    Serial.println(F("DFPlayer Mini online."));

    myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms

    //----Set volume----
    myDFPlayer.volume(20);  //Set volume value (0~30).
    num_random_files = myDFPlayer.readFileCountsInFolder(2); //Read total of random files in folder "/02/"
    randomSeed(analogRead(0));
  }
}

void sclass::loop() {
  static unsigned long timer = millis();
  if (myDFPlayer.available()) {
    uint8_t type = myDFPlayer.readType();
    int value = myDFPlayer.read();
    if (type == DFPlayerPlayFinished && callback_needed) {
      callback_needed = false;
      callback_function();
    } else if (type != DFPlayerPlayFinished) {
      printDetail(type, value); //Print the detail message from DFPlayer to handle different errors and states.
    }
  }
}

void sclass::play_welcome(void (*userFunc)(void)) {
  callback_function = userFunc;
  callback_needed = true;
  play_welcome();
}
void sclass::play_random(void (*userFunc)(void)) {
  callback_function = userFunc;
  callback_needed = true;
  play_random();
}
void sclass::play_welcome() {
  myDFPlayer.play(1);
}
void sclass::play_random() {
  myDFPlayer.playFolder(2, random(num_random_files) + 1);
}

sclass mp3 = sclass();

void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

}
