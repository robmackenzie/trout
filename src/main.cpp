// Files must be be contained in SD card in specific structure.
// The welcome file must be in the root directory, and be the first file coppied
// onto the SD card. If you want to replace it, you must delete all other files
// first. The files to be randomly chosen at a button push must be in a folder
// called "02" 
// Example below:
// /001.mp3 *Welcome file
// /02/001.mp3 *Random file 
// /02/002.mp3 *Random file 
// /02/003.mp3 *Random file 

// Below are some variables that can be easily modified to change the behaviour of the fish
#define SECONDS_BETWEEN_WELCOME_CLIPS_MIN 300
#define SECONDS_BETWEEN_WELCOME_CLIPS_MAX 400

#define SECONDS_BETWEEN_IDLE_TAIL_FLAPS_MIN 5
#define SECONDS_BETWEEN_IDLE_TAIL_FLAPS_MAX 55

#define IDLE_TAIL_FLAP_EXTEND_LENGTH_MS_MIN 200
#define IDLE_TAIL_FLAP_EXTEND_LENGTH_MS_MAX 1000

#define TAIL_RETURN_TIME 200

#define TALKING_MOUTH_EXTEND_LENGTH_MS_MIN 100
#define TALKING_MOUTH_EXTEND_LENGTH_MS_MAX 400

#define TALKING_MOUTH_RETRACT_LENGTH_MS_MIN 50
#define TALKING_MOUTH_RETRACT_LENGTH_MS_MAX 250

#define TALKING_TAIL_FLAP_EXTEND_LENGTH_MS_MIN 200
#define TALKING_TAIL_FLAP_EXTEND_LENGTH_MS_MAX 400

#define TALKING_TAIL_FLAP_RETRACT_LENGTH_MS_MIN 150
#define TALKING_TAIL_FLAP_RETRACT_LENGTH_MS_MAX 250

#define HEAD_EXTEND_TIME_MS 300
#define HEAD_RETRACT_TIME_MS 220

#define MP3_TIMEOUT_TIME_MS 4000
#define VOLUME 18 // Set volume value (0~30)

// Input pin
#define button_pin 16

// Motor pins
#define head_move_in_pin 13
#define head_move_out_pin 15
#define mouth_move_pin 12
#define tail_move_pin 14

// Following defined in mp3.cpp. Listed here for visibility of pins used
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

// Timer to make pulsing tail/lip easier
#include <AsyncDelay.h>
enum motor_status
{
  RESTING,
  EXTENDING,
  RETRACTING
};
enum head_status
{
  H_RESTING,
  H_EXTENDING,
  H_RETRACTING,
  H_BRAKING
}; 
AsyncDelay mouth_delay;
AsyncDelay tail_delay;
uint8_t mouth_status = RESTING;
uint8_t tail_status = RESTING;

// Our Libs
#include "mp3.h"

// Finite State Machine lib
#include <FunctionFSM.h>

bool button_triggered = false; // This track button 
enum Trigger
{
  TOGGLE_SWITCH,
  MP3_FINISHED
};

void
head_motor(head_status status)
{
  switch (status) {
    case H_RESTING: {
      Serial.println("Entering head resting");
      digitalWrite(head_move_out_pin, LOW);
      digitalWrite(head_move_in_pin, LOW);
    }
    break;

    case H_EXTENDING: {
      Serial.println("Entering head extending");
      digitalWrite(head_move_out_pin, HIGH);
      digitalWrite(head_move_in_pin, LOW);
    }
    break;

    case H_RETRACTING: {
      Serial.println("Entering head retracting");
      digitalWrite(head_move_out_pin, LOW);
      digitalWrite(head_move_in_pin, HIGH);
    }
    break;

    case H_BRAKING: {
      Serial.println("Entering head braking");
      digitalWrite(head_move_out_pin, HIGH);
      digitalWrite(head_move_in_pin, HIGH);
    }
    break;

  }
}
// fsm state functions
void
A_start()
{
  Serial.println("Entering State A");
  tail_status = RETRACTING;
  tail_delay.start(random(SECONDS_BETWEEN_IDLE_TAIL_FLAPS_MIN,SECONDS_BETWEEN_IDLE_TAIL_FLAPS_MAX)*1000, AsyncDelay::MILLIS);
}
void
A_loop()
{
  if (tail_status == RESTING) {
    tail_delay.start(random(IDLE_TAIL_FLAP_EXTEND_LENGTH_MS_MIN, IDLE_TAIL_FLAP_EXTEND_LENGTH_MS_MAX), AsyncDelay::MILLIS);
    digitalWrite(tail_move_pin, HIGH);
    tail_status = EXTENDING;
  } else if (tail_status == EXTENDING && tail_delay.isExpired()) {
    tail_delay.start(random(SECONDS_BETWEEN_IDLE_TAIL_FLAPS_MIN,SECONDS_BETWEEN_IDLE_TAIL_FLAPS_MAX)*1000, AsyncDelay::MILLIS);
    digitalWrite(tail_move_pin, LOW);
    tail_status = RETRACTING;
  } else if (tail_status == RETRACTING && tail_delay.isExpired()) {
    tail_status = RESTING;
  }
}

void
B_start()
{
  Serial.println("Entering State B");
  head_motor(H_EXTENDING);
}
void
B_stop()
{
  Serial.println("Leaving State B");
  //head_motor(H_BRAKING);  
}

void
C_trigger(); // Defined later, declared here for use in C_start

void
C_start()
{
  Serial.println("Entering State C");
  
  digitalWrite(tail_move_pin, HIGH);
  delay(random(TALKING_TAIL_FLAP_EXTEND_LENGTH_MS_MIN, TALKING_TAIL_FLAP_EXTEND_LENGTH_MS_MAX));
  digitalWrite(tail_move_pin, LOW);
  delay(random(TALKING_TAIL_FLAP_RETRACT_LENGTH_MS_MAX,TALKING_TAIL_FLAP_RETRACT_LENGTH_MS_MAX));
  digitalWrite(tail_move_pin, HIGH);
  delay(random(TALKING_TAIL_FLAP_EXTEND_LENGTH_MS_MIN, TALKING_TAIL_FLAP_EXTEND_LENGTH_MS_MAX));
  digitalWrite(tail_move_pin, LOW);
  delay(random(TALKING_TAIL_FLAP_RETRACT_LENGTH_MS_MAX,TALKING_TAIL_FLAP_RETRACT_LENGTH_MS_MAX));
  tail_status = RESTING;
  
  if (button_triggered) {
    mp3.play_random(&C_trigger);
  } else {
    mp3.play_welcome(&C_trigger);
  }
}

void
C_loop()
{
  // Move mouth and tail.
  if (mouth_status == RESTING) {
    mouth_delay.start(random(TALKING_MOUTH_EXTEND_LENGTH_MS_MIN,TALKING_MOUTH_EXTEND_LENGTH_MS_MAX), AsyncDelay::MILLIS);
    digitalWrite(mouth_move_pin, HIGH);
    mouth_status = EXTENDING;
  } else if (mouth_status == EXTENDING && mouth_delay.isExpired()) {
    mouth_delay.start(random(TALKING_MOUTH_RETRACT_LENGTH_MS_MIN,TALKING_MOUTH_RETRACT_LENGTH_MS_MIN), AsyncDelay::MILLIS);
    digitalWrite(mouth_move_pin, LOW);
    mouth_status = RETRACTING;
  } else if (mouth_status == RETRACTING && mouth_delay.isExpired()) {
    mouth_status = RESTING;
  }

  if (tail_status == RESTING) {
    tail_delay.start(random(TALKING_TAIL_FLAP_EXTEND_LENGTH_MS_MIN, TALKING_TAIL_FLAP_EXTEND_LENGTH_MS_MAX), AsyncDelay::MILLIS);
    digitalWrite(tail_move_pin, HIGH);
    tail_status = EXTENDING;
  } else if (tail_status == EXTENDING && tail_delay.isExpired()) {
    tail_delay.start(random(TALKING_TAIL_FLAP_RETRACT_LENGTH_MS_MAX,TALKING_TAIL_FLAP_RETRACT_LENGTH_MS_MAX), AsyncDelay::MILLIS);
    digitalWrite(tail_move_pin, LOW);
    tail_status = RETRACTING;
  } else if (tail_status == RETRACTING && tail_delay.isExpired()) {
    tail_status = RESTING;
  }
}

void
C_stop()
{
  digitalWrite(tail_move_pin, LOW);
  digitalWrite(mouth_move_pin, LOW);
}
void
D_start()
{
  Serial.println("Entering State D");
  head_motor(H_RETRACTING);
}

void
D_stop()
{
  Serial.println("Leaving State D");
  head_motor(H_BRAKING);
  button_triggered=false;
}

// fsm states (initialise in MyClass constructor)
FunctionState state_A(&A_start, &A_loop, nullptr);
FunctionState state_B(&B_start, nullptr, &B_stop);
FunctionState state_C(&C_start, &C_loop, &C_stop);
FunctionState state_D(&D_start, nullptr, &D_stop);

// fsm (initialise with MyClass constructor)
FunctionFsm fsm(&state_B);

void
C_trigger()
{
  Serial.println("Triggering end of state C");
  fsm.trigger(MP3_FINISHED);
};

void
setup()
{
  Serial.begin(115200);
  Serial.flush();
  Serial.println();
  Serial.println("Trout Control V2");
  Serial.println();
  Serial.flush(); // Get serial all nice and ready, with some new lines.
  mp3.setup();  // Initiallize mp3 player, sets up a link, will loop, forcing WDT reset if fails.
  //TODO Maybe put in a way of telling what's wrong with the system on fails
  mp3.set_volume(VOLUME);
  
  // Set up in/out pins
  pinMode(button_pin, INPUT);
  int outputs[4] = {
    head_move_in_pin, head_move_out_pin, mouth_move_pin, tail_move_pin
  };
  for (int i = 0; i < 4; i++) {
    pinMode(outputs[i], OUTPUT);
    digitalWrite(outputs[i], LOW);
  }
  // //Cycle motors to test.
  // for (int i = 0; i < 4; i++) {
  //   digitalWrite(outputs[i], HIGH);
  //   delay(500);
  //   digitalWrite(outputs[i], LOW);
  // }

  // init fsm
  fsm.add_timed_transition(
    &state_A,
    &state_B,
    random(SECONDS_BETWEEN_WELCOME_CLIPS_MIN,SECONDS_BETWEEN_WELCOME_CLIPS_MAX) * 1000, // 1000 ms in a second
    nullptr); // After timeout, have fish say welcome message.
  fsm.add_transition(&state_A,
                     &state_B,
                     TOGGLE_SWITCH,
                     nullptr); // Transition on button push or welcome timeout
  fsm.add_timed_transition(&state_B,
                           &state_C,
                           HEAD_EXTEND_TIME_MS,
                           nullptr); // Wait for head to move out
  fsm.add_timed_transition(
    &state_C,
    &state_D,
    MP3_TIMEOUT_TIME_MS, // Max length of any audio file
    nullptr); // Timeout in case something went wrong with the mp3 callback
  fsm.add_transition(&state_C,
                     &state_D,
                     MP3_FINISHED,
                     nullptr); // Transition on MP3 finished playing.
  fsm.add_timed_transition(&state_D,
                           &state_A,
                           HEAD_RETRACT_TIME_MS,
                           nullptr); // Wait for head to move back
}

void loop()
{
  fsm.run_machine(); // Do Finite State Machine tasks
  mp3.loop(); // Do anything with talking to the MP3 Module

  // Check the button push. Could be moved to a interrupt, but no real need. This works well.
  if (fsm.is_in_state(state_A) && !button_triggered && !digitalRead(button_pin)) {
    Serial.println("Button Triggered");
    button_triggered = true;
    fsm.trigger(TOGGLE_SWITCH);
  }
}
