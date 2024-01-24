// Arduino Christmas tree garland
// Author: Neosy <neosy.dev@gmail.com>
//
//==================================
// Version 3
//==================================
// v3 (2024-01-24)
//    1) Оптимизация кода
//    2) Настройка граничных значений яркости в зависимости от цвета лампочек. Проблема: зеленые и синие светодиоды обычно светят ярче
// v2 (2024-01-23)
//    1) Скетч по умолчанию настроен на 4 линии светодиодов (пины: D3, D5, D6 и D9)
//    2) Добавлены 2 новых режима
//       - Все лампочки всегда горят с малым накалом. Последовательно каждый цвет плавно загорается и тухнет
//       - Последовательно каждый цвет плавно загорается и затем последовательно каждый цвет тухнет
// v1
//    1) Поддержка до 6 независимых линий светодиодов. D пины для подключения: 3, 5, 6, 9, 10, 11. 
//    2) По умолчанию скетч настрое на 2 линии светодиодов (пины: D3 и D5).
//    3) 7 режимов. Переключение с помощью кнопки (пин D7).
//    4) Сохранение последнего режима.
//    5) Протестировано на Arduino Nano
//==================================
// Константы
// LED_LINES - количество независимых линий (цветов) гирлянды
// MODES_NUMBER - количество доступных режимов
// MODE_DEF - режима по умолчанию при первом старте.
// LED_PIN_LIST - список D пинов для подключения независимых линий гирлянды
// BTN_PIN - D пин для подключения кнопки 
//==================================

#include <EEPROM.h>

#include "nLeds.h"
#include "nButton.h"
#include "MemoryFree.h"

// Arduino Nano
#define LED_LINES 4
static const byte LED_PIN_LIST[] = {3, 5, 6, 9, 10, 11};
static const char *LED_COLOR_LIST[] = {"B", "Y", "G", "R", "L1", "L2"}; // Blue, Yellow, Green, Red
static const byte LED_LEVEL_MAX_LIST[] = {150, 255, 50, 255, 255, 255}; // Максимальная яркость по цветам
static const byte LED_LEVEL_WEAK_LIST[] = {3, 3, 1, 3, 3, 3}; // Слабое свечение по цветам
static const NLed::Dimmer::Method LED_METHOD_LIST[] = { NLed::Dimmer::Method::Degree, // Метод увеличения\уменьшения яркости
                                                        NLed::Dimmer::Method::Degree, 
                                                        NLed::Dimmer::Method::Exp, 
                                                        NLed::Dimmer::Method::Degree, 
                                                        NLed::Dimmer::Method::Degree, 
                                                        NLed::Dimmer::Method::Degree};
#define BTN_PIN 7

#define MODE_DEF 0
#define MODES_NUMBER 9
#define DEBUG_ON false

NLeds*          mLeds;
NButton*        mButton;
NLedScenarios*  mScenas;

byte mMode = 255;

void handlerButtonClicked();
void leds_reset();
void mode_set(byte _mode);
void mode_next();

// Обработка нажания кнопки
void handlerButtonClicked() {
  mode_next();
}

// Сброс настроек светодиодов в исходное состояние. Выключение всех светодиодов
void leds_reset() {
  NLed::Dimmer *dimmer;
  for (byte i=0; i<mLeds->number_get(); i++) {
    dimmer = mLeds->led(i)->dimmer;
    dimmer->level_min_set(dimmer->level_min_def_get());
    dimmer->level_max_set(dimmer->level_max_def_get());
    dimmer->method_set(LED_METHOD_LIST[i]);
    mLeds->led(i)->off();
  }
}

// Установка режима работы гирлянды
void mode_set(byte _mode) {
  if (DEBUG_ON) {
    Serial.print(F("Mode "));
    Serial.println(String(_mode));
  }

  if (mMode != _mode) {
    String scena_name = "";
    // Очистка всех сцен. Максимальное количество по числу линий светодиодов
    mScenas->clear();
  }
  if (DEBUG_ON) {
    Serial.print(F("mode_set() - freeMemory="));
    Serial.println(String(freeMemory()));
  }

  leds_reset();

  mMode = _mode;
  // Сохранение режима в памяти
  EEPROM.put(0, mMode);

 
  // Все цвета постоянно горят
  if (mMode == 0) {
    NLedScena *scena = NULL;
    scena = mScenas->add("S1");
    for (byte i=0; i<mLeds->number_get(); i++) {
      scena->add(mLeds->led(i), &NLed::on);
    }
    scena->enable();
  }

  // Одновременно все цвета плавно загораются и тухнут
  if (mMode == 1) {
    const word fade_t = 5*1000;
    const word delay = 1000;
    String scena_name = "";
    NLedScena *scena = NULL;
    NLed *led = NULL;

    for (byte i=0; i<mLeds->number_get(); i++) {
      led = mLeds->led(i);
      scena_name = "S" + String(i);
      scena = mScenas->add(scena_name.c_str());
      scena->add(led, &NLed::on, fade_t);
      //scena->addDelay(delay);
      scena->add(led, &NLed::off, fade_t);
      scena->addDelay(delay);
      scena->enable();
    }
  }

  // Последовательно каждый цвет плавно загорается и затем последовательно каждый цвет тухнет
  if (mMode == 2) {
    const word fade_on_t = 6000;
    const word fade_off_t = 3000;
    const word delay = 1000;
    NLedScena *scena = NULL;
    NLed *led = NULL;

    scena = mScenas->add("S");
    for (byte i=0; i<mLeds->number_get(); i++) {
      led = mLeds->led(i);
      scena->add(led, &NLed::on, fade_on_t);
      //scena->addDelay(delay);
    }
    for (byte i=0; i<mLeds->number_get(); i++) {
      led = mLeds->led(i);
      scena->add(led, &NLed::off, fade_off_t);
      scena->addDelay(delay);
    }
    scena->enable();
  }

  // Последовательно каждый цвет плавно зажигается и тухнет
  if (mMode == 3) {
    const word fade_on_t = 4*1000;
    const word fade_off_t = 4*1000;
    NLedScena *scena = NULL;
    NLed *led = NULL;

    scena = mScenas->add("S1");
    for (byte i=0; i<mLeds->number_get(); i++) {
      led = mLeds->led(i);
      scena->add(led, &NLed::on, fade_on_t);
      scena->add(led, &NLed::off, fade_off_t);
      scena->addDelay(500);
    }
    scena->enable();
  }

  // Четные и нечетные последовательно плавно загораются и тухнут
  if (mMode == 4) {
    const word fade_on_t = 6*1000;
    const word fade_off_t = 3*1000;
    const word delay = 1000;
    String scena_name = "";
    NLedScena *scena = NULL;
    NLed *led = NULL;

    for (byte i=0; i<mLeds->number_get(); i++) {
      led = mLeds->led(i);
      scena_name = "S" + String(i);
      scena = mScenas->add(scena_name.c_str());
      if ((i + 1) % 2 == 0) {
        scena->add(led, &NLed::off, fade_off_t);
        scena->addDelay(delay);
        scena->add(led, &NLed::on, fade_on_t);
      } else {
        scena->add(led, &NLed::on, fade_on_t);
        scena->add(led, &NLed::off, fade_off_t);
        scena->addDelay(delay);
      }
      scena->enable();
    }
  }

  // Все лампочки всегда горят с малым накалом. Последовательно каждый цвет плавно загорается и тухнет
  if (mMode == 5) {
    const word fade_on_t = 3000;
    const word fade_off_t = 3000;
    NLedScena *scena = NULL;
    NLed *led = NULL;

    scena = mScenas->add("S");
    for (byte i=0; i<mLeds->number_get(); i++) {
      led = mLeds->led(i);
      led->dimmer->level_min_set(LED_LEVEL_WEAK_LIST[i]);
      scena->addDelay(3000);
      scena->add(led, &NLed::on, fade_on_t);
      scena->addDelay(1000);
      scena->add(led, &NLed::off, fade_off_t);
    }
    scena->enable();
  }

  // Все лампочки всегда горят с малым накалом. Последовательно каждый цвет 1 раз мигает
  if (mMode == 6) {
    const word fade_t = 350;
    NLedScena *scena = NULL;
    NLed *led = NULL;

    scena = mScenas->add("S");
    for (byte i=0; i<mLeds->number_get(); i++) {
      led = mLeds->led(i);
      led->dimmer->level_min_set(LED_LEVEL_WEAK_LIST[i]);
      scena->add(led, &NLed::on, fade_t);
      scena->addDelay(20);
      scena->add(led, &NLed::off, fade_t);
    }
    scena->enable();
  }

  // Последовательно каждый цвет 1 раз мигает
  if (mMode == 7) {
    const word fade_t = 50;
    const word delay = 100;
    NLedScena *scena = NULL;
    scena = mScenas->add("S");
    NLed *led = NULL;

    for (byte i=0; i<mLeds->number_get(); i++) {
      led = mLeds->led(i);
      scena->add(led, &NLed::on, fade_t);
      scena->addDelay(delay);
      scena->add(led, &NLed::off, fade_t);
      scena->addDelay(delay);
    }
    scena->enable();
  }

  // Последовательно каждый цвет 4 раза мигает
  if (mMode == 8) {
    const word fade_t = 70;
    const word delay = 50;
    String scena_name = "";
    NLedScena *scena = NULL;
    NLedScena *scenaGr = NULL;
    NLed *led = NULL;

    scena = mScenas->add("S");
    scena->debugMode(DEBUG_ON);
    scena->enable();
    for (byte i=0; i<mLeds->number_get(); i++) {
      led = mLeds->led(i);
      scena_name = String(scena->name_get()) + "-" + String(i);
      scenaGr = scena->addScena(scena_name.c_str(), 4);
      scenaGr->debugMode(DEBUG_ON);
      scenaGr->add(led, &NLed::on, fade_t);
      scenaGr->addDelay(delay);
      scenaGr->add(led, &NLed::off, fade_t);
      scenaGr->addDelay(delay);
      scenaGr->enable();
    }
  }

  if (DEBUG_ON) {
    Serial.print(F("mode_set() End - freeMemory="));
    Serial.println(String(freeMemory()));
  }
}

// Выбор следующего режима
void mode_next() {
  byte mode_new = mMode;

  mode_new++;
  if (mode_new > MODES_NUMBER - 1)
    mode_new = 0;

  mode_set(mode_new);
}

void setup() {
  NLed *led;

  if (DEBUG_ON)
    Serial.begin(9600);

  if (DEBUG_ON) {
    Serial.print(F("Setup Begin - freeMemory()="));
    Serial.println(String(freeMemory()));
  }

  // Инициализация кнопки
  mButton = new NButton("B", BTN_PIN);
  mButton->setCallBackClicked(handlerButtonClicked);
  mButton->debugMode(DEBUG_ON); //DEBUG_ON

  mLeds = new NLeds();

  // Инициализация светодиодов
  for (byte i=0; i<LED_LINES; i++) {
    led = mLeds->add(LED_COLOR_LIST[i], LED_PIN_LIST[i]);
    led->dimmer->enable();
    led->dimmer->level_max_def_set(LED_LEVEL_MAX_LIST[i]);
  }

  mScenas = new NLedScenarios();

  // Чтение из памяти последнего режима. Установка режима
  byte mode_save = 0;
  EEPROM.get(0, mode_save);
  if (mode_save > MODES_NUMBER - 1)
    mode_save = MODE_DEF;
  mode_set(mode_save);

  if (DEBUG_ON) {
    Serial.print(F("Setup End - freeMemory()="));
    Serial.println(String(freeMemory()));
  }
}

void loop() {
  mButton->loop_run();
  mScenas->loop_run();

  delay(10);
}
