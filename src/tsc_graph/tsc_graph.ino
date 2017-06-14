#include <Metro.h>
#include <Streaming.h>

#include <FastLED.h>

// TCS45725 RGB Sensor
#include "Adafruit_TCS34725.h"
#define SENSOR_LED_PIN 13
// may want to replace this with interrupt logic instead of metronome?
class RGBSensor {
public:
  // initialize sensor in satndby state, setting led pin and metronome interval
  RGBSensor(int ledPin);
  bool begin(unsigned long intervalMillis);
  void enable();
  void disable();
  void update();
  bool isColor(); // returns true if the last reading was interesting; not black or white
  CRGB getColor();
private:
  Adafruit_TCS34725 tcs;
  Metro readingTimer;
  bool enabled;
  int ledPin;
  CRGB color; // last committed color code
};
RGBSensor::RGBSensor(int ledPin) {
  this->enabled = false;
  this->ledPin = ledPin;
  pinMode(this->ledPin, OUTPUT);
  this->tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
}
bool RGBSensor::begin(unsigned long intervalMillis) {
  this->readingTimer.interval( intervalMillis );
  return this->tcs.begin();
}
void RGBSensor::enable(){
  this->enabled = true;
  readingTimer.reset();
  digitalWrite(this->ledPin, HIGH);
}
void RGBSensor::disable() {
  this->enabled = false;
  digitalWrite(this->ledPin, LOW);
}
void RGBSensor::update() {
  if (this->enabled) {
    if (this->readingTimer.check()) {
      this->readingTimer.reset();
      
      uint16_t clear, red, green, blue;
      // actually read the sensor
      this->tcs.getRawData(&red, &green, &blue, &clear);

// coerce the 16 bit rgb values into a 32 bit colorcode
uint32_t sum = clear;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  
      this->color = CRGB(
          r,
          g,
          b
      );
      
      Serial << F("R:\t") << this->color.r << F("\tG:\t") << this->color.g << F("\tB:\t") << this->color.b << endl;
    }
  }
}
bool RGBSensor::isColor() {
  // TODO return if this->color != black or white (or anything returned when not actually touching a color)
  if (this->enabled) {
    return true;
  }
  return false;
}
CRGB RGBSensor::getColor() {
  return this->color;
}
RGBSensor Sensor = RGBSensor(SENSOR_LED_PIN);


void setup() {
  Serial.begin(9600);

  // start RGB sensor
  Sensor.begin(60);
  Sensor.enable();
}

void loop() {
  Sensor.update();
}

