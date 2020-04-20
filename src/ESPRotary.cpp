/////////////////////////////////////////////////////////////////
/*
  ESP8266/Arduino Library for reading rotary encoder values.
  Copyright 2017-2020 Lennart Hennigs.
*/
/////////////////////////////////////////////////////////////////

#include "ESPRotary.h"

/////////////////////////////////////////////////////////////////

ESPRotary::ESPRotary(int pin1, int pin2, int moves_per_click /* = 1 */, int lower_bound /* = -32768 */, int upper_bound /* = 32768 */) {
  this->pin1 = pin1;
  this->pin2 = pin2;
  if (moves_per_click < 1) {
    #pragma message("At least one move per click required, reverting to 1")
    moves_per_click = 1;
  }
  this->moves_per_click = moves_per_click;
  this->lower_bound = lower_bound;
  this->upper_bound = upper_bound;

  pinMode(pin1, INPUT_PULLUP);
  pinMode(pin2, INPUT_PULLUP);

  loop();
  resetPosition();
  last_read_ms = 0;
}

/////////////////////////////////////////////////////////////////

void ESPRotary::setChangedHandler(CallbackFunction f) {
  change_cb = f;
}

/////////////////////////////////////////////////////////////////

void ESPRotary::setRightRotationHandler(CallbackFunction f) {
  right_cb = f;
}

/////////////////////////////////////////////////////////////////

void ESPRotary::setLeftRotationHandler(CallbackFunction f) {
  left_cb = f;
}

/////////////////////////////////////////////////////////////////

void ESPRotary::resetPosition(int p /* = 0 */) {
  last_position = (lower_bound > p) ? lower_bound * moves_per_click : p;
  position = last_position;
  direction = 0;
}

/////////////////////////////////////////////////////////////////

byte ESPRotary::getDirection() {
  return direction;
}

/////////////////////////////////////////////////////////////////

String ESPRotary::directionToString(byte direction) {
  if (direction == RE_LEFT) {
    return "LEFT";
  } else if (direction == RE_RIGHT) {
    return "RIGHT";
    }
}

/////////////////////////////////////////////////////////////////

int ESPRotary::getPosition() {
  return position / moves_per_click;
}

/////////////////////////////////////////////////////////////////

void ESPRotary::loop() {
  int s = state & 3;
  if (digitalRead(pin1)) s |= 4;
  if (digitalRead(pin2)) s |= 8;

  switch (s) {
    case 0: case 5: case 10: case 15:
      break;
    case 1: case 7: case 8: case 14:
        position++; break;
    case 2: case 4: case 11: case 13:
      position--; break;
    case 3: case 12:
      position += 2; break;
    default:
      position -= 2; break;
  }
  state = (s >> 2);
  
  if (getPosition() >= lower_bound && getPosition() <= upper_bound) {
    if (position != last_position) {
      if (abs(position - last_position) >= moves_per_click) {
        if (position > last_position) {
          direction = RE_RIGHT;
          if (right_cb != NULL) right_cb (*this);
        } else {
          direction = RE_LEFT;
          if (left_cb != NULL) left_cb (*this);
        }
        last_position = position;      
        if (change_cb != NULL) change_cb (*this);
      }
    }
  } else position = last_position;
}

/////////////////////////////////////////////////////////////////