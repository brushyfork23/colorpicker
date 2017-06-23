#include "RGBSensor.h"

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

      /*
        * This logic should be done with bitshifting, not floats.
        * Figure out what's wrong with this mask opperation (again)
        */
      // this->color = (red << 8) & 0x00ff0000 |
      //               (green ) & 0x0000ff00 |
      //               (blue >> 8) & 0x000000ff;

      uint32_t sum = clear;
      float r, g, b;
      r = red; r /= sum;
      g = green; g /= sum;
      b = blue; b /= sum;
      r *= 256; g *= 256; b *= 256;

      CRGB color = CRGB(r, g, b);
      this->color = rgb2hsv_approximate(color);

      this->color.s = qadd8(this->color.s, 80);

      // TSC Read
      //Serial << F("R:\t") << color.r << F("\tG:\t") << color.g << F("\tB:\t") << color.b << endl;

      // HSV approximation
      //Serial << F("H:\t") << this->color.h << F("\tS:\t") << this->color.s << F("\tV:\t") << this->color.v << endl;

      // RGB approximation
      //Serial << F("R:\t") << color.r << F("\tG:\t") << color.g << F("\tB:\t") << color.b << endl;
    }
  }
}
bool RGBSensor::isColor() {
  // TODO return if this->color != black or white (or anything returned when not actually touching a color)
  if (this->enabled
//    && !( // rgb 175, 71, 55 was the average color I got for "white", or "no color", when testing this around my apartment.
//      abs(175 - this->color.r) < 20
//      && abs(71 - this->color.g) < 20
//      && abs(55 - this->color.b) < 20
//    )
  ) {
    return true;
  }
  return false;
}
CHSV RGBSensor::getColor() {
  return this->color;
}