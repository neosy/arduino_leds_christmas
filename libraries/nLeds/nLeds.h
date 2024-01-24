// Arduino library nLeds.h
// Author: Neosy <neosy.dev@gmail.com>
//
//==================================
// Version 3
//==================================

#ifndef nLeds_h
#define nLeds_h

#include "Arduino.h" 

#define SCENA_INFINITY 0
#define PROGRESS_BLINK_DELAY 1000
#define PROGRESS_LED_ROUND_MORE true // При расчете количество горящих лампочек использовать округление в большую сторону 
#define DEBUG_STATE false

class NLed;
class NLeds;
class NLedProgress;
class NLedScenarios;
class NLedScena;

class NLed {  
  private:
    NLed      *prev, *next;
  
  protected:
    char    *name;
    uint8_t pin;
    boolean isOn;
  
  public:
    struct Dimmer {
      enum class Method 
      { 
          Line, //Линейная
          Degree, //Степенная
          Exp //Экспоненциальная
      };

      private:
        boolean enabled   = false;
        byte      level   = 0;
      protected:
        byte      level_min_def = 0;
        byte      level_max_def = 255;
        byte      level_min = level_min_def;
        byte      level_max = level_max_def;
        Method    method = Method::Line;

      public:
        void enable();
        void disable();
        byte level_min_def_get();
        void level_min_def_set(byte _level);
        byte level_max_def_get();
        void level_max_def_set(byte _level);       
        byte level_min_get();
        void level_min_set(byte _level);
        byte level_max_get();
        void level_max_set(byte _level);       
        byte level_get();
        void level_set(byte _level);
        Method method_get();
        void method_set(Method _method);

      friend class NLed;
    };
    Dimmer *dimmer;
    NLed(const char *_name, uint8_t _pin);
    ~NLed();
    char *name_get();
    boolean isOn_get();
    void switchLight();
    boolean switchLight(boolean _isOn);
    void on();
    void on(byte _level);
    void off();
    void blink(int _delay);
    void blink();
    
    friend class NLeds;
};

class NLeds {
  private:
    uint8_t   number = 0;
    NLed      *first, *last;

  public:
    NLeds();
    ~NLeds();
    uint8_t number_get();
    NLed *add(NLed *_led);
    NLed *add(const char *_name, uint8_t _pin);
    NLed *led(uint8_t _num);
    NLed *led(const char *_name);
};

class NLedProgress {
  private:
    struct Led {
      NLed  *led;
      Led   *prev, *next;
    };
  
    uint8_t   number        = 0;
    Led       *first, *last;
    NLedScena *ledScena;
    uint8_t   cntLedOn      = 0;
    uint8_t   numIncomplete = 0; // Последняя неполная лампочка
    boolean   blinkLast     = false; // Мигать последней неполной лампочкой

  protected:
    float calcExactCntLedOn();
    
  public:
    uint32_t  count = 0;
    uint32_t  total = 0;
    
    NLedProgress(NLedScena *_ledScena);
    ~NLedProgress();
    NLed *add(NLed *_led);
    NLed *led(uint8_t _num);
    NLed *ledIncomplete();
    void init(uint32_t _total);
    void init(uint32_t _total, long _count);
    boolean parmBlinkLast();
    boolean parmBlinkLast(boolean _blinkLast);
    uint8_t number_get();
    uint32_t incCount(uint32_t _v);
    uint32_t incCount();
    void setCount(long _count);
    void setTotal(uint32_t _total);
    void reset();
    void update(boolean _force = false);
    void hideAll();
};

class NLedScena {
  private:
    NLedScena *prev, *next;  
    uint16_t  iterations      = SCENA_INFINITY;
    uint16_t  iterationCount  = 0;
    //uint16_t  startDelay      = 0;

  protected:
    char      *name;
    uint8_t   numberSteps = 0;
    boolean   enabled     = false;
    boolean   debugState  = DEBUG_STATE;

    void init();

  private:
    struct Step;  
    byte retryNumber = 1;
    byte retryCount = 0;
    NLedScena::Step *first, *last, *active, *lastRun, *parentStep;
    NLedScena *parent;
    void run_init_steps(uint32_t _timer);
    void run_reset();
    void run_steps(NLedScena::Step *_stepStart = NULL);

  public:
    NLedScena(const char *_name);
    NLedScena(const char *_name, uint16_t _iterations);
    //NLedScena(const char *_name, uint16_t _iterations, uint16_t _startDelay);
    ~NLedScena();
    char *name_get();
    boolean enabled_get();
    void reset();
    void enable();
    void disable();
    void disable(boolean _hideAll);
    void add(NLed *_led, void (NLed::*_fn)(), uint32_t _dimmer_delay = 0);
    NLedScena *addScena(const char *_name, byte _retryNumber = 1);
    void addDelay(uint16_t _time);
    void clear();
    void hideAll();
    boolean debugMode(boolean _state);
    boolean debugMode();
    void run();
    
  friend class NLedScenarios;
};

struct NLedScena::Step {
  enum class Type 
  { 
      Null,
      Scena,
      Led,
      Delay 
  };
  struct LedFn {
      NLed *led;
      void (NLed::*fn)();
  };
  

  Step      *prev, *next;
  NLedScena *scena;
  Type      type = Type::Null;
  LedFn     *ledFn;
  uint16_t  delayTime = 0;
  uint16_t  delayTimeAccumulated = 0;
  uint32_t  tmrTime = 0;
  bool      done = false;

  Step();
  ~Step();
  uint16_t calcDelayTimeAccumulated();
};

class NLedScenarios {
  private:
    uint8_t   number  = 0;
    NLedScena *first, *last;
  
  public:
    NLedScenarios();
    ~NLedScenarios();
    uint8_t number_get();
    NLedScena *add(NLedScena *_scena);
    NLedScena *add(const char *_name);
    NLedScena *add(const char *_name, uint16_t _iterations);
    //NLedScena *add(const char *_name, uint16_t _iterations, uint16_t _startDelay);
    void del(uint8_t _num);
    void del(const char *_name);
    void clear();
    NLedScena *scena(uint8_t _num);
    NLedScena *scena(const char *_name);
    void loop_run();
};

#endif
