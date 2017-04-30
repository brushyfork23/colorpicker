/*

    ------->  ready  <----------   confirm <------------------------------------
   |            v                                                               |
 reset    (color sensed)                                                        |
   ^        streaming  <---------------------------------------                 |
   |              v                                            |                |
(timeout)   (button released)                                  |                |
   ^              V                                            |                |
   ---- preview countdown                                      |                |
              v                                                |                |
          (button pressed)                                     |                |
              v                                                |                |
----> confirm debounce --> (confirm debounce timer expires) ---                 |
|            v                                                                  |
|      (button released, then pressed) --------> (confirm counter is above 2) --
|          v                                                
------- (increase confirm presses counter)                  


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
  void update();
  bool isColor(); // returns true if the last reading was interesting; not black or white
  uint32_t color(); // return the color code of the last reading
private:
  Adafruit_TCS34725 tcs;
  Metro readingTimer;
  bool enabled;
  int ledPin;
  uint32_t color; // last committed color code
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
bool RGBSensor::isColor() {
  // TODO return if this->color != black or white
  if (this->enabled) {

  }
}
uint32_t RGBSensor::color() {
  return this->color;
}



// SPST Pushbutton
#include <Bounce2.h>
#define BUTTON_PIN 12
Bounce Button = Bounce();

// TCS45725 RGB Sensor
#include "Adafruit_TCS34725.h"
#define SENSOR_LED_PIN 13

// NeoPixel FeatherWing
#include "Picker_Display.h"
#define DISPLAY_PIN 15
#define BRIGHTNESS 84
PickerDisplay Display;

#define RESET_TIMEOUT_MILIS 5000 // duration of reset timeout, in miliseconds
#define CONFRIM_TIMEOUT_MILIS 500 // duration of confirmation timeout, in miliseconds
#define CONFIRMATION_PRESSES 2 // number of button presses to confirm a color
Metro resetTimer;
Metro confirmTimer;

//http://www.arduino.cc/playground/uploads/Code/FSM_1-6.zip
#include <FiniteStateMachine.h>
// define state machine
State ready = State(readyEnter, readyUpdate, NULL);
State streaming = State(NULL, streamingUpdate, NULL);
State previewCountdown = State(previewCountdownEnter, previewCountdownUpdate, NULL);
State confirmDebounce = State(confirmDebounceEnter, confirmDebounceUpdate, NULL);
FSM picker = FSM(ready)

bool
  buttonReleased;

uint8_t
  buttonPresses;

uint32_t 
  color,
  previewColor;

void setup() {
  Serial.begin(115200);

  // Set up the button with an internal pull-up
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  Button.attach(BUTTON_PIN);
  Button.interval(5); // interval in ms

  // set duration of reset and confirm timers
  resetTimeout.interval(RESET_TIMEOUT_MILIS);
  confirmTimer.interval(CONFRIM_TIMEOUT_MILIS);

  // start neopixels
  Display.begin(DISPLAY_PIN);
  while(!Display);
  Display.PickerPulseSingle();
  Display.setBrightness(BRIGHTNESS);

  // start RGB sensor
  Sensor.begin(SENSOR_LED_PIN);
  while(!Sensor);

  // start wifi
  Network.begin();
  while(!Network);
}

void loop() {
  Button.update();
  Display.update();
  Sensor.update();
  Network.update();
  picker.update();
}


/*
  ALL the functions below are helper functions for the states of the program
*/

///[ready state:enter]
// fade in and out a sinlge pixel, rainbow if this->color == black
void readyEnter() {
}

///[ready state:update]
// continue to pulse until button is pressed and sensor detects a color
void readyUpdate() {
  // toggle sensor when button is pressed or released
  if ( Button.fell() ) {
    Sensor.disable();
  } else if ( Button.rose() ) {
    Sensor.enable();
  }

  // has a new color been scanned?
  if ( Sensor.isColor() && Sensor.color() != this->color) {
    this->previewColor = Sensor.color();
    Display.PickerPreviewInit(this->previewColor);
    //TODO fix callback assignment syntax
    Display.OnComplete(function(){
      Display.Solid(this->previewColor)
    });
    picker.transitionTo(streaming);
  }
}

///[streaming state:update]
//
void streamingUpdate() {
  // update color if new one is scanned
  if ( Sensor.isColor() && Sensor.color() != this->color) {
    this->previewColor = Sensor.color();
    Display.setColor(this->previewColor);
  }

  // if button is not pressed, transition to preview timeout
  if ( Button.read() == HIGH ) {
    Sensor.disable();
    picker.transitionTo(previewCountdown);
  }
}

///[previewCountdown state:enter]
//
void previewCountdownEnter() {
  Display.PickerPulse(this->previewColor);
  resetTimer.reset();
}

///[previewCountdown state:update]
//
void previewCountdownUpdate() {
  // if color was not confirmed in time, reset
  if ( resetTimer.check() ) {
    Display.PickerReset(this->color);
    //TODO fix callback assignment syntax
    Display.OnComplete(function(){
      Display.PickerPulseSingle(this->color)
    });
    picker.transitionTo(streaming);
  }

  // if button is pressed, confirm a double click
  if ( Button.fell() ){
    picker.transitionTo(confirmDebounce);
  }
}

void confirmDebounceEnter() {
  confirmTimer.reset();
  buttonReleased = false;
  buttonPresses = 1;
}

void confirmDebounceUpdate() {
  // if color was not confirmed in time, reset
  if ( resetTimer.check() ) {
    Display.PickerReset(this->color);
    //TODO fix callback assignment syntax
    Display.OnComplete(function(){
      Display.PickerPulseSingle(this->color)
    });
    picker.transitionTo(ready);
  }

  // if confirmation was not made, assume this is meant to continue streaming
  if ( confirmTimer.check() ) {
    Sensor.enable();
    picker.transitionTo(streaming);
  }

  // button must be clicked multiple times to confirm
  if ( Button.rose() ) {
    buttonReleased = true;
  } else if ( Button.fell() && buttonReleased ) {
    if ( buttonPresses >= CONFIRMATION_PRESSES ) {
      this->color = this->previewColor;
      Display.PickerConfirm(this->color);
      //TODO fix callback assignment syntax
      Display.OnComplete(function(){
        Display.PickerPulseSingle(this->color)
      });
      picker.transitionTo(ready);
    } else {
      buttonReleased = false;
      buttonPresses++;
    }
  }
}