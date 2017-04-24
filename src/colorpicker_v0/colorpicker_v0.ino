/*
      State machine

      --------------->  ready <--------------------------
     |                    |                              |
(timeout) reset           |             (double click) confirm
     ^                    |                              ^
     |                    v                              |
      ---------------- preview --------------------------





initialize: start up neopixels, rgb sensor, and network.
  set vars: 
    color = black
    previewColor = black
ready:
  enter:
    Animation.setAnim(picker_pulse_single); // fade in and out a sinlge pixel, rainbow if this->color == black
  update:
    if ( Sensor.color() != this->color && Sensor.color() != black ) {
      this->previewColor = Sensor.color();
      goto state preview
    }
    button.update
    if button.risingEdge {
      Sensor.disable()
    } else if button.fallingEdge {
      Sensor.enable()
    }

preview:
  enter:
    Animation.setSequence(picker_init_preview, picker_solid) (fill display with this->previewColor, then hold preview color)
      (animation internally sets animation.transitioning = true)
    reset timeout timer
  update:
    if ( Sensor.color() != this->color && Sensor.color() != black ) {
      this->previewColor = Sensor.color()
    }
    button.update
    if button.pressed = false && timeout.expired {
      Animation.setSequence(picker_reset, picker_pulse_single) // clear display and set back to single pixel of this->color
      goto state ready
    }
    // todo problem here.  on falling edge, set reset timer for 250 millis.  enable or confirm after 250 millis
    if button.fallingEdge {
      if button pushed for more than 250 millis {
        if button.isDoubleClick { // confirm selection
          this->color = this->previewColor
          Network.publish(this->color) // initiate network sendWithAck event; Maybe include some retry logic?
          Animation.setSequence(picker_transmit, picker_pulse_single) // perform cool "broadcasting" animation, then settle in to a single pixel
          goto state ready
        } else {
          Sensor.enable()
          set animation to solid
        }
      }
    }
    if button.risingEdge {
      Sensor.disable()
      Animation.setAnim(pulse) // fade brightness of display in and out
      reset timeout timer
    }

  loop{
    Animation.update()
    Sensor.update()
    Network.update()
    picker.update() // execute update() for current state
  }

 



  // set a sequence of transitional animatinos, ending with a stable one.
 animation.setSequence(anim1, anim2, ...){
  this->seq = [anim2, ...]
  this->anim = anim1
  this->transitioning = this->anim->transitional
 }
 anim.update() {
  if this->anim->transitional
    if this->transitioning == false
      if this->seq->hasNextAnim
        this->setAnim(this->seq->nextAnim)
 }



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

class Color {
  public:
    byte red, green, blue;

  Color::Color(byte r, g, b){
    this->red = r;
    this->green = g;
    this->blue = b;
  }
};

//http://www.arduino.cc/playground/uploads/Code/FSM_1-6.zip
#include <FiniteStateMachine.h>

// define state machine
State ready = State(readyEnter, readyUpdate, NULL);
State preview = State(previewEnter, previewUpdate, NULL);
FSM picker = FSM(initialize)


// TCS45725 RGB Sensor
#include "Adafruit_TCS34725.h"
#define LED_PIN 13

// NeoPixel FeatherWing
#include "Picker_Display.h"
#define DISPLAY_PIN 15
#define BRIGHTNESS 84
PickerDisplay Display

uint32_t 
  color,
  previewColor;

void setup() {
  Serial.begin(115200);

  //start up neopixels, rgb sensor, and network.
  Display.begin(DISPLAY_PIN);
  Display.setBrightness(BRIGHTNESS);
}

void loop() {
  Display.update();
  Sensor.update();
  Network.update();
  picker.update();
}


/*
  ALL the functions below are helper functions for the states of the program
*/

///[ready state:update]
// fade in and out a sinlge pixel, rainbow if this->color == black
void readyEnter() {
  Animation.setAnim(picker_pulse_single);
  Display.PickerPulseSingle(this->color);
}