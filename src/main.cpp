// Files must be be contained in SD card in specific structure.
// The welcome file must be in the root directory, and be the first file coppied
// onto the SD card. If you want to replace it, you must delete all other files
// first. The files to be randomly chosed at a button push must be in a folder
// called "02" /001.mp3 *Welcome file /02/001.mp3 *Random file /02/002.mp3
#define SECONDS_BETWEEN_WELCOME_CLIPS 300
#define button_pin 16

// Following defined in motor.cpp. Listed here for visibility
//#define head_move_in_pin 14
//#define head_move_out_pin 12
//#define mouth_move_pin 13
//#define tail_move_pin 15
// Following defined in mp3.cpp. Listed here for visibility
//#define software_tx_pin 4
//#define software_rx_pin  5

// clang-format off
/*
FSM States:
  A - Resting, waiting for button push. Will time out to play welcome message,
      flipping tail a bit randomly. 
  B - Activated, moving head out 
  C - Playing sound, moving lips and tail 
  D - Moving head back
*/
// clang-format on

// Because we still rely on the arduino env.
#include <Arduino.h>

// Our Stuff
#include "motor.h"
#include "mp3.h"

// Finite State Machine lib
#include <FunctionFSM.h>

// void handle_button_interrupt();
// void set_interrupt_active_state(bool set) {
//   if (set) {
//     detachInterrupt(digitalPinToInterrupt(button_pin));
//   } else {
//     button_triggered = false;
//     attachInterrupt(digitalPinToInterrupt(button_pin),
//     handle_button_interrupt,
//                     FALLING);
//   }
// }

// void handle_button_interrupt() {
//   button_triggered = true;
//   set_interrupt_active_state(true);
// }
bool button_triggered = false;
enum Trigger { TOGGLE_SWITCH, MP3_FINISHED };

// fsm state functions
void A_start() {
  Serial.println("Entering State A");
}
void A_loop() {}
void B_start() {
  Serial.println("Entering State B");
  // TODO Activate head motor out
}
void B_stop() {
  Serial.println("Leaving State B");
  // TODO Deactivate head motor out
}
void C_trigger();  // Defined later
void C_start() {
  Serial.println("Entering State C");
  if (button_triggered) {
    mp3.play_random(&C_trigger);
  } else {
    mp3.play_welcome(&C_trigger);
  }
}
void D_start() {
  Serial.println("Entering State D");
  // TODO Activate head motor in
}
void D_stop() {
  Serial.println("Leaving State D");
  // TODO Deactivate head motor in
  // set_interrupt_active_state(false);  // Re-enable the button
}
// fsm states (initialise in MyClass constructor)
FunctionState state_A(&A_start, &A_loop, nullptr);
FunctionState state_B(&B_start, nullptr, &B_stop);
FunctionState state_C(&C_start, nullptr, nullptr);
FunctionState state_D(&D_start, nullptr, &D_stop);

// fsm (initialise with MyClass constructor)
FunctionFsm fsm(&state_B);

void C_trigger() {
  Serial.println("Triggering end of state C");
  fsm.trigger(MP3_FINISHED);
};

void setup() {
  Serial.begin(
      115200);  // All output uses this, leaving it in as debug is nice.
  Serial.flush();
  mp3.setup();  // Initiallize mp3 player
  // TODO Rest of setup
  pinMode(button_pin, INPUT_PULLUP);
  // set_interrupt_active_state(false);  // Attach interrupt.

  // init fsm
  fsm.add_timed_transition(&state_A, &state_B,
                           SECONDS_BETWEEN_WELCOME_CLIPS * 1000,
                           nullptr);  // Timeout, have fish say welcome message
  fsm.add_transition(&state_A, &state_B, TOGGLE_SWITCH,
                     nullptr);  // Transition on button push
  fsm.add_timed_transition(&state_B, &state_C, 500,
                           nullptr);  // Wait for head to move out
  fsm.add_timed_transition(
      &state_C, &state_D, 10000,
      nullptr);  // Timeout in case something went wrong with the mp3 callback
  fsm.add_transition(&state_C, &state_D, MP3_FINISHED,
                     nullptr);  // Transition on MP3 finished playing.
  fsm.add_timed_transition(&state_D, &state_A, 1000,
                           nullptr);  // Wait for head to move back
}

void loop() {
  fsm.run_machine();
  mp3.loop();
  if (!digitalRead(button_pin) && fsm.is_in_state(state_A)) {
    button_triggered = true;
    //fsm.trigger(TOGGLE_SWITCH);
  }
}
