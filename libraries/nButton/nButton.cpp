// Arduino lib nButton
// Author: Neosy <neosy.dev@gmail.com>
//
//==================================
// Version 2
//  1. Добавлен debugMode()
//==================================
// Version 1
//==================================

#include "nButton.h"

// ********************************* Functions for class Button ****************************
NButton::NButton(const char *_name, byte _pin) {
  this->name = new char[strlen(_name) + 1];
  strcpy(this->name, _name);
  this->pin = _pin;
  this->state = HIGH;
  this->loop_timer  = millis();
  
  pinMode(pin, INPUT_PULLUP);
  
  init();
}

NButton::~NButton() {
  delete []name;
}

void NButton::setCallBackClicked(void (*_callBack)()){
  this->callBackClicked = _callBack;
}

char *NButton::name_get() {
  return this->name;
}

void NButton::down() {
  if (this->debugState == true) {
    Serial.print(F("Button name \""));
    Serial.print(String(this->name));
    Serial.println(F("\" Down"));
  }
}

void NButton::up() {
  if (this->debugState == true) {
    Serial.print(F("Button name \""));
    Serial.print(String(this->name));
    Serial.println(F("\" Up"));
  }
}

void NButton::clicked() {
  if (this->callBackClicked != NULL) {
    (*this->callBackClicked)();
  }    
}

boolean NButton::debugMode(boolean _state) {
  this->debugState = _state;

  return this->debugState;
}

boolean NButton::debugMode() {
  return this->debugState;
}

void NButton::loop_run() {
  boolean btn_state;

  if (millis() - this->loop_timer > this->loop_delay) {
    this->loop_timer = millis();

    btn_state = digitalRead(this->pin);
    if (btn_state != this->state) {
      this->state = btn_state; 
      if (this->state == HIGH) {
        this->up();
        this->clicked();
      } else {
        this->down();
      }
    }
  }
}

// ********************************* Functions for class Buttons ****************************
NButtons::NButtons() {
  this->first       = NULL;
  this->last        = NULL;
  this->number      = 0;
}

NButtons::~NButtons() {
  NButton *cur = this->first;
  NButton *tmp;

  if (cur != NULL) {
    while (cur) {
      tmp = cur;
      cur = cur->next;
      delete tmp;
    }
  }
}

NButton *NButtons::add(NButton *_button) {
  NButton  *ret = NULL;

  this->number++;

  if (_button != NULL) {
    if (this->first == NULL) {
      _button->prev  = NULL;
      this->first = _button;
    } else {
      _button->prev  = last;
      last->next  = _button;  
    }
    _button->next  = NULL;
    this->last  = _button;
    ret = this->last;
  }    

  return ret;
}

NButton* NButtons::add(const char *_name, byte _pin) {
  return add(new NButton(_name, _pin));
}

NButton *NButtons::button(uint8_t _num) {
  NButton   *cur  = this->first;
  uint8_t   i     = 0;

  while (cur) {
    if (i == _num) {
      break;
    }
    i++;
    cur = cur->next;
  }

  return cur;
}

NButton *NButtons::button(const char *_name) {
  NButton  *cur = this->first;

  while (cur) {
    if (strcmp(cur->name, _name) == 0) {
      break;
    }
    cur = cur->next;
  }
  
  return cur;
}

void NButtons::loop_run() {
  NButton   *cur  = this->first;

  while (cur != NULL) {
    cur->loop_run();
    cur = cur->next;
  }
}
