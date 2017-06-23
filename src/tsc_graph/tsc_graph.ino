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
  CHSV getColor();
private:
  Adafruit_TCS34725 tcs;
  Metro readingTimer;
  bool enabled;
  int ledPin;
  CHSV color; // last committed color code
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

      uint32_t sum = clear;
      float r, g, b;
      r = red; r /= sum;
      g = green; g /= sum;
      b = blue; b /= sum;
      r *= 256; g *= 256; b *= 256;

      CRGB color = CRGB(r, g, b);
      this->color = rgb2hsv_approximate(color);

      // Bump up the saturation to get a richer color from the reading
      //this->color.s = 255; //qadd8(this->color.s, 40);

      CRGB approximated;
      approximated.setHSV(this->color.h, this->color.s, this->color.v);

      // TSC Read
      //Serial << F("R:\t") << color.r << F("\tG:\t") << color.g << F("\tB:\t") << color.b << endl;

      // HSV approximation
      Serial << F("H:\t") << this->color.h << F("\tS:\t") << this->color.s << F("\tV:\t") << this->color.v << F("\t");

      // RGB approximation
      Serial << F("R:\t") << color.r << F("\tG:\t") << color.g << F("\tB:\t") << color.b << endl;
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
CHSV RGBSensor::getColor() {
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