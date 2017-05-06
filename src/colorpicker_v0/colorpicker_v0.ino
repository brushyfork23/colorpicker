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
  On button press/release: call sensor.enable()/sensor.disable()
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
#define CONFRIM_COUNTDOWN_MILIS 500 // duration of confirmation countdown timer, in miliseconds
#define CONFIRMATION_PRESSES 2 // number of button presses to confirm a color
Metro resetTimer;
Metro confirmCountdownTimer; // A debouncer for confirmation clicks.
                    // It starts when the button is pressed while a preview is pulsing.
                    // If the button is released again before time expires, count it as a confirm click.
                    // Otherwise, assume this was a request to re-enable sensor and detect new colors.

//http://www.arduino.cc/playground/uploads/Code/FSM_1-6.zip
#include <FiniteStateMachine.h>
// define state machine
State ready = State(readyEnter, readyUpdate, NULL);
State previewStreaming = State(previewStreamingEnter, streamingUpdate, NULL);
State previewCountdown = State(previewCountdownEnter, previewCountdownUpdate, NULL);
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

  // set duration of reset and confirm countdown timers
  resetTimeout.interval(RESET_TIMEOUT_MILIS);
  confirmCountdownTimer.interval(CONFRIM_COUNTDOWN_MILIS);

  // start neopixels
  Display.begin(DISPLAY_PIN);
  while(!Display);
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
// fade in and out a sinlge pixel, rainbow if this->color == 0
void readyEnter() {
  Display.queueAnimation(PICKER_PULSE_SINGLE);
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
    Display.setAnimation(PICKER_PREVIEW_INIT);
    Display.setColor(this->previewColor);
    picker.transitionTo(streaming);
  }
}

///[previewStreaming state:enter]
// transition display to showing full bright whatever color is scanned
void previewStreamingEnter() {
  Sensor.enable();
  Display.queueAnimation(SOLID);
}
///[previewStreaming state:update]
// update desplayed color with scanned color until button is released
void previewStreamingUpdate() {
  // if button has been released, turn off sensor and start timeout countdown
  if ( Button.read() == HIGH ) {
    Sensor.disable();
    picker.transitionTo(previewCountdown);
  }

  // update color if new one is scanned
  if ( Sensor.isColor() && Sensor.color() != this->previewColor) {
    this->previewColor = Sensor.color();
    Display.setColor(this->previewColor);
  }
}

///[previewCountdown state:enter]
// set display to pulse the preview color, and start a countdown
void previewCountdownEnter() {
  Display.setAnimation(PICKER_PULSE);
  resetTimer.reset();
  confirmCountdownTimer.reset();
  confirmationPresses = 0;
}

///[previewCountdown state:update]
// If button is briefly pressed, check if there were enough recent presses to confirm.
// If button is held, go back to preview streaming.
// if button does not get pressed again in time, reset.
void previewCountdownUpdate() {
  // When button is released, increase count of presses and check if this is enough to confirm
  if ( Button.rose() ) {
    confirmationPresses++;
    if ( confirmationPresses >= CONFIRMATION_PRESSES ) {
      // There have been enough presses; confirm color
      this->color = this->previewColor;
      Display.setAnimation(PICKER_CONFIRM);
      Network.publishColor(this->color);
    } else {
      // There are still more presses required; reset confirmation timeout
      confirmCountdownTimer.reset();
    }
  }

  // When button is pressed, reset confirmation debounce timer
  if ( Button.fell() ){
    confirmCountdownTimer.reset();
  }

  // check time elapsed on the confirmation timer since the last button change
  if ( confirmCountdownTimer.check() ) {
    if ( Button.check() == LOW ) {
      // Button has been held for long enough, go back to preview streaming
      picker.transitionTo(previewStreaming);
    } else if ( confirmationPresses > 0 ) {
      // Too much time has passed between confirmation clicks; reset confirmation press counter
      confirmationPresses = 0;
    }
  }

  // if color was not confirmed in time, reset
  if ( resetTimer.check() ) {
    Display.setAnimation(PICKER_RESET);
    picker.transitionTo(ready);
  }
}
}