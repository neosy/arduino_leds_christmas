// Arduino Christmas tree garland
// Author: Neosy <neosy.dev@gmail.com>
//
//==================================
// Version 2
//==================================
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
  String led_name = "";
  for (byte i=1; i<=LED_LINES; i++) {
    led_name = "L" + String(i);
    mLeds->led(led_name.c_str())->dimmer->level_min_set(0);
    mLeds->led(led_name.c_str())->dimmer->level_max_set(255);
    mLeds->led(led_name.c_str())->off();
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
    //if (DEBUG_ON)
    //  Serial.println("freeMemory=" + String(freeMemory()));
    // Очистка всех сцен. Максимальное количество по числу линий светодиодов
    for (byte i=LED_LINES; i>=1; i--) {
      scena_name = F("S");
      scena_name += String(i);
      mScenas->del(scena_name.c_str());
    }
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
    String led_name = "";
    NLedScena *scena = NULL;
    scena = mScenas->add("S1");
    for (byte i=1; i<=LED_LINES; i++) {
      led_name = "L" + String(i);
      scena->add(mLeds->led(led_name.c_str()), &NLed::on);
    }
    scena->enable();
  }

  // Одновременно все цвета плавно загораются и тухнут
  if (mMode == 1) {
    const word fade_t = 3*1000;
    String scena_name = "";
    String led_name = "";
    NLedScena *scena = NULL;
    for (byte i=1; i<=LED_LINES; i++) {
      scena_name = "S" + String(i);
      led_name = "L" + String(i);
      scena = mScenas->add(scena_name.c_str());
      scena->add(mLeds->led(led_name.c_str()), &NLed::on, fade_t);
      scena->add(mLeds->led(led_name.c_str()), &NLed::off, fade_t);
      scena->addDelay(500);
      scena->enable();
    }
  }

  // Последовательно каждый цвет плавно загорается и затем последовательно каждый цвет тухнет
  if (mMode == 2) {
    const word fade_on_t = 6000;
    const word fade_off_t = 3000;
    String led_name = "";
    NLedScena *scena = NULL;
    scena = mScenas->add("S1");
    for (byte i=1; i<=LED_LINES; i++) {
      led_name = "L" + String(i);
      scena->add(mLeds->led(led_name.c_str()), &NLed::on, fade_on_t);
      //scena->addDelay(1000);
    }
    for (byte i=1; i<=LED_LINES; i++) {
      led_name = "L" + String(i);
      scena->add(mLeds->led(led_name.c_str()), &NLed::off, fade_off_t);
      scena->addDelay(1000);
    }
    scena->enable();
  }

  // Последовательно каждый цвет плавно зажигается и тухнет
  if (mMode == 3) {
    const word fade_t = 2*1000;
    String led_name = "";
    NLedScena *scena = NULL;
    scena = mScenas->add("S1");
    for (byte i=1; i<=LED_LINES; i++) {
      led_name = "L" + String(i);
      scena->add(mLeds->led(led_name.c_str()), &NLed::on, fade_t);
      scena->add(mLeds->led(led_name.c_str()), &NLed::off, fade_t);
      scena->addDelay(500);
    }
    scena->enable();
  }

  // Четные и нечетные последовательно плавно загораются и тухнут
  if (mMode == 4) {
    const word fade_on_t = 4*1000;
    const word fade_off_t = 2*1000;
    String scena_name = "";
    String led_name = "";
    NLedScena *scena = NULL;

    for (byte i=1; i<=LED_LINES; i++) {
      scena_name = "S" + String(i);
      led_name = "L" + String(i);
      scena = mScenas->add(scena_name.c_str());
      if (i % 2 == 0) {
        scena->add(mLeds->led(led_name.c_str()), &NLed::off, fade_off_t);
        scena->addDelay(1000);
        scena->add(mLeds->led(led_name.c_str()), &NLed::on, fade_on_t);
      } else {
        scena->add(mLeds->led(led_name.c_str()), &NLed::on, fade_on_t);
        scena->add(mLeds->led(led_name.c_str()), &NLed::off, fade_off_t);
        scena->addDelay(1000);
      }
      scena->enable();
    }
  }

  // Все лампочки всегда горят с малым накалом. Последовательно каждый цвет плавно загорается и тухнет
  if (mMode == 5) {
    const word fade_on_t = 3000;
    const word fade_off_t = 2000;
    const byte level_min = 3;
    String led_name = "";
    NLedScena *scena = NULL;
    scena = mScenas->add("S1");
    for (byte i=1; i<=LED_LINES; i++) {
      led_name = "L" + String(i);
      mLeds->led(led_name.c_str())->dimmer->level_min_set(level_min);
      scena->add(mLeds->led(led_name.c_str()), &NLed::on, fade_on_t);
      scena->addDelay(1000);
      scena->add(mLeds->led(led_name.c_str()), &NLed::off, fade_off_t);
      scena->addDelay(3000);
    }
    scena->enable();
  }

  // Все лампочки всегда горят с малым накалом. Последовательно каждый цвет 1 раз мигает
  if (mMode == 6) {
    const word fade_t = 120;
    const byte level_min = 3;
    String led_name = "";
    NLedScena *scena = NULL;
    scena = mScenas->add("S1");
    for (byte i=1; i<=LED_LINES; i++) {
      led_name = "L" + String(i);
      mLeds->led(led_name.c_str())->dimmer->level_min_set(level_min);
      scena->add(mLeds->led(led_name.c_str()), &NLed::on, fade_t);
      scena->addDelay(20);
      scena->add(mLeds->led(led_name.c_str()), &NLed::off, fade_t);
    }
    scena->enable();
  }

  // Последовательно каждый цвет 1 раз мигает
  if (mMode == 7) {
    String led_name = "";
    NLedScena *scena = NULL;
    scena = mScenas->add("S1");
    for (byte i=1; i<=LED_LINES; i++) {
      led_name = "L" + String(i);
      scena->add(mLeds->led(led_name.c_str()), &NLed::on);
      scena->addDelay(100);
      scena->add(mLeds->led(led_name.c_str()), &NLed::off);
      scena->addDelay(100);
    }
    scena->enable();
  }

  // Последовательно каждый цвет 4 раза мигает
  if (mMode == 8) {
    String led_name = "";
    String scena_name = "";
    NLedScena *scena = NULL;
    NLedScena *scenaGr = NULL;
    scena = mScenas->add("S1");
    scena->debugMode(DEBUG_ON);
    scena->enable();
    for (byte i=1; i<=LED_LINES; i++) {
      scena_name = String(scena->name_get()) + "-" + String(i);
      led_name = "L" + String(i);
      scenaGr = scena->addScena(scena_name.c_str(), 4);
      scenaGr->debugMode(DEBUG_ON);
      scenaGr->add(mLeds->led(led_name.c_str()), &NLed::on, 50);
      scenaGr->addDelay(50);
      scenaGr->add(mLeds->led(led_name.c_str()), &NLed::off, 50);
      scenaGr->addDelay(50);
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
  for (byte i=1; i<=LED_LINES; i++) {
    String led_name = "L" + String(i);
    mLeds->add(led_name.c_str(), LED_PIN_LIST[i-1]);
    mLeds->led(led_name.c_str())->dimmer->enable();
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

  delay(5);
}
