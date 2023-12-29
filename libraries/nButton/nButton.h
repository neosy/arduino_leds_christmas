// Arduino lib nButton.h
// Author: Neosy <neosy.dev@gmail.com>
//
//==================================
// Version 2
//==================================

#ifndef nButton_h
#define nButton_h

#include "Arduino.h" 

#define DEBUG_STATE false
#define BUTTON_LOOP_DELAY 50

class NButton {
  private:
    NButton       *prev, *next;
    uint16_t      loop_delay = BUTTON_LOOP_DELAY;
    unsigned long loop_timer;
    
    void init() {
    }

  protected:
    char    *name;
    byte    pin;
    boolean state = HIGH;
    boolean debugState = DEBUG_STATE;
  
  public:
    void (*callBackClicked)();
    
    NButton(const char *_name, byte _pin);
    ~NButton();
    void setCallBackClicked(void (*_callBack)());
    char *name_get();
    virtual void down();
    virtual void up();
    virtual void clicked();
    boolean debugMode(boolean _state);
    boolean debugMode();
    void loop_run();
    
    friend class NButtons;
};

class NButtons {
  private:
    NButton       *first, *last;
    uint8_t       number;

  public:
    NButtons();
    ~NButtons();
    NButton* add(NButton* _button);
    NButton* add(const char *_name, byte _pin);
    NButton* button(uint8_t _num);
    NButton* button(const char *_name);
    void loop_run();
};
#endif
