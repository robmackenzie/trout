// Files must be be contained in SD card in specific structure.
// The welcome file must be in the root directory, and be the first file coppied
// onto the SD card. If you want to replace it, you must delete all other files
// first. The files to be randomly chosed at a button push must be in a folder
// called "02" /001.mp3 *Welcome file /02/001.mp3 *Random file /02/002.mp3
#define SECONDS_BETWEEN_WELCOME_CLIPS 300
#define button_pin 16

// Motor pins
#define head_move_in_pin 14
#define head_move_out_pin 12
#define mouth_move_pin 13
#define tail_move_pin 15

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

//Timer to make pulsing tail/lip easier
#include <AsyncDelay.h>
enum motor_status {RESTING, EXTENDING, RETRACTING};
AsyncDelay mouth_delay;
AsyncDelay tail_delay;
uint8_t mouth_status=RESTING;
uint8_t tail_status=RESTING;

// Our Stuff
#include "mp3.h"

// Finite State Machine lib
#include <FunctionFSM.h>

bool button_triggered = false;
enum Trigger { TOGGLE_SWITCH, MP3_FINISHED };

// fsm state functions
void A_start() {
  Serial.println("Entering State A");
  tail_status = RETRACTING;
  tail_delay.start(12000, AsyncDelay::MILLIS);
}
void A_loop() {
	if (tail_status == RESTING) {
		tail_delay.start(600, AsyncDelay::MILLIS);
		digitalWrite(tail_move_pin,HIGH);
		tail_status = EXTENDING;
	} else if (tail_status == EXTENDING && tail_delay.isExpired()) {
		tail_delay.start(20000, AsyncDelay::MILLIS);
		digitalWrite(tail_move_pin,LOW);
		tail_status = RETRACTING;
	} else if (tail_status == RETRACTING && tail_delay.isExpired()) {
		tail_status = RESTING;
	}
}

void B_start() {
  Serial.println("Entering State B");
  digitalWrite(head_move_out_pin, HIGH);
}
void B_stop() {
  Serial.println("Leaving State B");
  digitalWrite(head_move_out_pin, LOW);
}

void C_trigger();  // Defined later, declared here for use in C_start

void C_start() {
  Serial.println("Entering State C");
  if (button_triggered) {
    mp3.play_random(&C_trigger);
  } else {
    mp3.play_welcome(&C_trigger);
  }
}

void C_loop() {
	// Move mouth and tail.
	if (mouth_status == RESTING) {
		mouth_delay.start(700, AsyncDelay::MILLIS);
		digitalWrite(mouth_move_pin,HIGH);
		mouth_status = EXTENDING;
	} else if (mouth_status == EXTENDING && mouth_delay.isExpired()) {
		mouth_delay.start(200, AsyncDelay::MILLIS);
		digitalWrite(mouth_move_pin,LOW);
		mouth_status = RETRACTING;
	} else if (mouth_status == RETRACTING && mouth_delay.isExpired()) {
		mouth_status = RESTING;
	}
	
	if (tail_status == RESTING) {
		tail_delay.start(600, AsyncDelay::MILLIS);
		digitalWrite(tail_move_pin,HIGH);
		tail_status = EXTENDING;
	} else if (tail_status == EXTENDING && tail_delay.isExpired()) {
		tail_delay.start(400, AsyncDelay::MILLIS);
		digitalWrite(tail_move_pin,LOW);
		tail_status = RETRACTING;
	} else if (tail_status == RETRACTING && tail_delay.isExpired()) {
		tail_status = RESTING;
	}
}
void D_start() {
  Serial.println("Entering State D");
  digitalWrite(head_move_in_pin, HIGH);
}
void D_stop() {
  Serial.println("Leaving State D");
  digitalWrite(head_move_in_pin, LOW);
}
// fsm states (initialise in MyClass constructor)
FunctionState state_A(&A_start, &A_loop, nullptr);
FunctionState state_B(&B_start, nullptr, &B_stop);
FunctionState state_C(&C_start, &C_loop, nullptr);
FunctionState state_D(&D_start, nullptr, &D_stop);

// fsm (initialise with MyClass constructor)
FunctionFsm fsm(&state_B);

void C_trigger() {
  Serial.println("Triggering end of state C");
  fsm.trigger(MP3_FINISHED);
};

void setup() {
  Serial.begin(115200);
  Serial.flush();
  Serial.println();
  Serial.println("Trout Control V2");
  Serial.println();
  Serial.flush(); // Get serial all nice and ready, with some new lines.
  mp3.setup();  // Initiallize mp3 player, sets up a link, will reboot if fails.
  //TODO Maybe put in a way of telling what's wrong with the system on fails.
  
  // Declare in/out pins
  pinMode(button_pin, INPUT);
  int outputs[4] = {head_move_in_pin, head_move_out_pin, mouth_move_pin, tail_move_pin};
  for (int i = 0; i < 4; i++) {
	 pinMode(outputs[i], OUTPUT);
  }

  // init fsm
  fsm.add_timed_transition(&state_A, &state_B,
                           SECONDS_BETWEEN_WELCOME_CLIPS * 1000,
                           nullptr);  // After timeout, have fish say welcome message.
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
  fsm.run_machine(); // Do Finite State Machine tasks
  mp3.loop(); // Do anything with talking to the MP3 Module
  if (!digitalRead(button_pin) && fsm.is_in_state(state_A)) {
    button_triggered = true;
    //fsm.trigger(TOGGLE_SWITCH);
  }
}
