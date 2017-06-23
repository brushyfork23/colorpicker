/*
  Sensor class:
  sensor is initiallized with metronome millis, and in standby state (disabled)
  sensor has state: reading/standby, set by sensor.enable()/disable()
  when entering reading state, metronome is reset
  while in reading state, metronome is checked and new reading is taken every x millis
  On button press/release: call sensor.enable()/sensor.disable()
*/

#ifndef RGBSensor_h
#define RGBSensor_h

#include <Metro.h>
#include <Streaming.h>

// https://github.com/FastLED/FastLED
#include <FastLED.h>

// TCS45725 RGB Sensor
#include "Adafruit_TCS34725.h"

class RGBSensor {
public:
  // initialize sensor in satndby state, setting led pin and metronome interval
  RGBSensor(int ledPin);
  bool begin(unsigned long intervalMillis);
  void enable();
  void disable();
  void update();
  bool isColor(); // returns true if the last reading was interesting; not black or white
  CHSV getColor();
private:
  Adafruit_TCS34725 tcs;
  Metro readingTimer;
  bool enabled;
  int ledPin;
  CHSV color; // last committed color code
};

#endif