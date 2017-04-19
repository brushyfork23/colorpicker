/*

                                            initialize
                                                v
                                              passive
                                                v
                                          (color found)
                                         new reading animation
                                                |
                                                  ------> (color found)



                         ------------  (timeout)reset animation  <---------------  (button released) preview 
                        |      -->                                                                                                 |
                        v      |                                                                                                  |
State: initialize -> passive -- (button pressed) reading -> (color found) new color animation -> refresh ->
                        ^      |                                                  |
                        |      ->                                                   |
                         -------------- (double pressed) confirm  <--------------




initialize: start up neopixels, rgb sensor, and wifi.
passive:
  pulse neopixels (color cycle if none set)
  when button is pressed, update rbg sensor
  when rgb sensor detects new color, goto new_reading_animation
new_reading_animation:
  update animation: step through animation, filling display with sensed color
  

 

  animation.update()
  if (sensor.update()) {
  
  }
  animation.setColor(sensor.Color)



  Sensor class:
  sensor is initiallized with metronome millis, and in standby state (disabled)
  sensor has state: reading/standby, set by sensor.enable()/disable()
  when entering reading state, metronome is reset
  while in reading state, metronome is checked and new reading is taken every x millis
  on button press/release: call sensor.enable()/sensor.disable()
*/


// may want to replace this with interrupt logic instead of metronome?
class RGBSensor {
public:
  // initialize sensor in satndby state, setting led pin and metronome interval
  RGBSensor(int ledPin, unsigned long intervalMillis)
  void enable();
  void disable();
private:
  Adafruit_TCS34725 tcs;
  Metro readingTimer;
  bool enabled;
  int ledPin;
};
RGBSensor::RGBSensor(int ledPin, unsigned long intervalMillis) {
  this->enabled = false;
  this->ledPin = ledPin;
  pinMode(this->ledPin, OUTPUT);
  this->readingTimer.interval( intervalMillis );
  this->tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
}
void RGBSensor::enable(){
  this->enabled = true;
  readingTimer.reset()
  digitalWrite(this->ledPin, HIGH);
}
void RGBSensor::disable() {
  this->enabled = false;
  digitalWrite(this->ledPin, LOW);
}
void RGBSensor::update() {
  if (this->enabled) {

  }
}







#include <Metro.h>
#include <Bounce.h>

// Metronome wrapper class
class Beats {
  public:
    // set the interval in ms
    void tempo( unsigned long interval );
    // set start of interval
    void start();

    // return TRUE at beat start
    boolean isBeat();
    // return the number of ms until the next beat
    unsigned long nextBeat();
    // return the total cycle time in ms
    unsigned long getTempo();

  private:
    unsigned long interval, lastBeat;

    Metro beatCounter;
};
void Beats::tempo( unsigned long interval ) {
  this->interval = interval;
  this->beatCounter.interval( interval );
}
void Beats::start() {
  this->beatCounter.reset();
  this->lastBeat = millis();
}
boolean Beats::isBeat() {
  if( beatCounter.check() ) {
    this->lastBeat = millis();
    return( true );
  } else {
    return( false );
  }
}
unsigned long Beats::nextBeat() {
  return( this->interval - ( millis() - this->lastBeat ) );
}
unsigned long Beats::getTempo() {
  return( this->interval );
}
Beats Beat;


class RGBSensor {
  public:

  private:

};

void setup() {
  Serial.begin(115200);



  // Initialize RGB Sensor
  RGBSensor.begin( 461UL )

  // Initialize NeoPixels

  // Initialize Clock


}

void loop() {
  if (// when button is pressed
    // if this is a doulbe press
      // mark new 
      // change state to color selected
    Sensor.update()

    if 
}