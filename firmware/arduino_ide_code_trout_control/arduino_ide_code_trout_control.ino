// Files must be be contained in SD card in specific structure.
// The welcome file must be in the root directory, and be the first file coppied onto the SD card. If you want to replace it, you
// must delete all other files first.
// The files to be randomly chosed at a button push must be in a folder called "02"
// /001.mp3 *Welcome file
// /02/001.mp3 *Random file
// /02/002.mp3 *Random file
#define SECONDS_BETWEEN_WELCOME_CLIPS 300

#define button_pin 0

//Following defined in motor.cpp. Listed here for visibility
//#define head_move_in_pin 14
//#define head_move_out_pin 12
//#define mouth_move_pin 13
//#define tail_move_pin 15
//Following defined in mp3.cpp. Listed here for visibility
//#define software_tx_pin 4
//#define software_rx_pin  5


#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//Following needed for WiFiManager library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Ticker.h> //for LED status (Still WiFiManger)

//Our Stuff
#include "mp3.h"
#include "motor.h"


#include <TaskScheduler.h>
#include <TaskSchedulerDeclarations.h>
Scheduler taskScheduler;
void play_welcome();
Task welcome_clip_task(SECONDS_BETWEEN_WELCOME_CLIPS * TASK_SECOND, TASK_FOREVER, &play_welcome);

volatile bool button_triggered;
bool currently_playing = false;

void setup()
{
  Serial.begin(115200); // All output uses this, leaving it in as debug is nice.
  //TODO wait for stable serial
  // Disable wifi until i have button stuff.
  //wifi_setup(); // Has all wifi stuff, including autoconfig. See wifi.ino
  //TODO Put in wifi reset button.
  mp3.setup(); // Initiallize mp3 player
  taskScheduler.init();
  taskScheduler.addTask(welcome_clip_task);
  welcome_clip_task.enable();
  //TODO Rest of setup
  pinMode(button_pin, INPUT_PULLUP);
  set_interrupt_active_state(false); // Attach interrupt.
  play_welcome();
}

void set_interrupt_active_state(bool set) {
  if (set) {
    detachInterrupt(digitalPinToInterrupt(button_pin));
    button_triggered = true;

  } else {
    button_triggered = false;
    attachInterrupt(digitalPinToInterrupt(button_pin), handle_button_interrupt, FALLING);
  }
}

void handle_button_interrupt() {
  set_interrupt_active_state(true);
}

void react_to_button() {
  Serial.println("Button Triggered");
  //delay(1000); //TODO Move head motor
  play_random();
}

void play_random() {
  currently_playing = true;
  mp3.play_random(&play_finished_callback);
}

void play_welcome() {
  currently_playing = true;
  mp3.play_welcome(&play_finished_callback);
}

void play_finished_callback() {
  currently_playing = false;
  //delay(1000); //TODO Move head motor
  set_interrupt_active_state(false); // Re-enable the button
}

void loop() {
  if (button_triggered && !currently_playing) {
    react_to_button(); // Plays a random clip
    welcome_clip_task.delay(); // Reset welcome clip delay to default delay
  }
  taskScheduler.execute(); // Run task scheduler tasks if needed.
  mp3.loop();
}
