#include <Metro.h>
#include <Streaming.h>

#include <FastLED.h>

// TCS45725 RGB Sensor
#include <RGBSensor.h>
#define SENSOR_LED_PIN 13
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
