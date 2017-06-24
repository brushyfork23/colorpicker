/*

TODO:

Make a calibration mode.  When the system first boots (if color is still unset)
allow some button press sequence to initialize a scanning mode which takes
average rgb readings until released.  Then, give those values a little padding
and use them in isColor() like this:

!(abs(175 - this->color.r) < 20
  && abs(71 - this->color.g) < 20
  && abs(55 - this->color.b) < 20
)

*/










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
*/


#include <Metro.h>
#include <Streaming.h>
#include <FastLED.h>

// SPST Pushbutton
#include <Bounce2.h>
#define BUTTON_PIN 12
Bounce Button = Bounce();

// TCS45725 RGB Sensor
#include <RGBSensor.h>
#define SENSOR_LED_PIN 13
RGBSensor Sensor = RGBSensor(SENSOR_LED_PIN);

// NeoPixel FeatherWing
#include "PickerDisplay.h"
#define BRIGHTNESS 40

// Timers and Counters
#define SENSOR_UPDATE_MILIS 50
#define RESET_TIMEOUT_MILIS 2000 // duration of reset timeout, in miliseconds
#define CONFRIM_COUNTDOWN_MILIS 150 // duration of confirmation countdown timer, in miliseconds
#define CONFIRMATION_PRESSES 2 // number of button presses to confirm a color
Metro resetTimer;
Metro confirmDebounceTimer; // A debouncer for confirmation clicks.
                    // It starts when the button is pressed while a preview is pulsing.
                    // If the button is released again before time expires, count it as a confirm click.
                    // Otherwise, assume this was a request to re-enable sensor and detect new colors.

//http://www.arduino.cc/playground/uploads/Code/FSM_1-6.zip
#include <FiniteStateMachine.h>
// define state machine
void readyEnter();
void readyUpdate();
State ready = State(readyEnter, readyUpdate, NULL);
void previewStreamingEnter();
void previewStreamingUpdate();
State previewStreaming = State(previewStreamingEnter, previewStreamingUpdate, NULL);
void previewCountdownEnter();
void previewCountdownUpdate();
State previewCountdown = State(previewCountdownEnter, previewCountdownUpdate, NULL);
FSM picker = FSM(ready);

CHSV color, previewColor;
uint8_t confirmationPresses;

void setup() {
  Serial.begin(115200);

  // Set up the button with an internal pull-up
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  Button.attach(BUTTON_PIN);
  Button.interval(5); // interval in ms

  // set duration of reset and confirm countdown timers
  resetTimer.interval(RESET_TIMEOUT_MILIS);
  confirmDebounceTimer.interval(CONFRIM_COUNTDOWN_MILIS);

  // start neopixels
  Display.begin();
  //while(!Display);
  Display.setBrightness(BRIGHTNESS);
  Display.setFPS(32);

  // start RGB sensor
  if (Sensor.begin(60)) {
    Serial << F("Initialized TCS RGB sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    Display.setColor(CHSV(0,0,0));
    Display.setAnimation(SOLID);
    while (1); // halt!
  }

  // start wifi
  //Network.begin();
  //while(!Network);
}

void loop() {
  Button.update();
  Display.update();
  Sensor.update();
  //Network.update();
  picker.update();
}

/*
  ALL the functions below are helper functions for the states of the program
*/

///[ready state:enter]
// fade in and out a sinlge pixel, rainbow if this->color == 0
void readyEnter() {
  Serial << F("[state] ready") << endl;
  Display.queueAnimation(PICKER_PULSE_SINGLE);
}

///[ready state:update]
// continue to pulse until button is pressed and sensor detects a color
void readyUpdate() {
  // toggle sensor when button is pressed or released
  if ( Button.fell() ) {
    Sensor.enable();
  } else if ( Button.rose() ) {
    Sensor.disable();
  }

  // has a new color been scanned?
  if ( Sensor.isColor() && Sensor.getColor() != color) {
    Serial << F("detected color: H:\t") << Sensor.getColor().h << F("\tS:\t") << Sensor.getColor().s << F("\tV:\t") << Sensor.getColor().v << endl;
    previewColor = Sensor.getColor();
    Display.setColor(previewColor);
    //Display.setAnimation(PICKER_PREVIEW_INIT);
    picker.transitionTo(previewStreaming);
  }
}

///[previewStreaming state:enter]
// transition display to showing full bright whatever color is scanned
void previewStreamingEnter() {
  Serial << F("[state] previewStreaming") << endl;
  Sensor.enable();
  Display.setAnimation(CASCADE);
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
  if ( Sensor.isColor() && Sensor.getColor() != previewColor) {
    previewColor = Sensor.getColor();
    Display.setColor(previewColor);
  }
}

///[previewCountdown state:enter]
// set display to pulse the preview color, and start a countdown
void previewCountdownEnter() {
  Serial << F("[state] previewCountdown") << endl;
  Display.setColor(previewColor);
  Display.setAnimation(PICKER_PULSE);
  resetTimer.reset();
  confirmDebounceTimer.reset();
  confirmationPresses = 0;
}

///[previewCountdown state:update]
// If button is briefly pressed, check if there were enough recent presses to confirm.
// If button is held, go back to preview streaming.
// if button does not get pressed again in time, reset.
void previewCountdownUpdate() {
  // When button is released, increase count of presses and check if this is enough to confirm
  if ( Button.rose() ) {
    // button was released before debounce timer expired.
    // The last press was meant to be a confirmation
    confirmationPresses++;
    Serial << F("last press was a confirmation.  Confirmation count: ") << confirmationPresses << endl;
    if ( confirmationPresses >= CONFIRMATION_PRESSES ) {
      Serial << F("Confirmed! Setting new color.") << endl;
      // There have been enough presses; confirm color
      color = previewColor;
      Display.setColor(color);
      Display.setAnimation(PICKER_CONFIRM);
      //Network.publishColor(color);
    } else {
      // There are still more presses required; reset reset timer
      resetTimer.reset();
    }
  }

  // When button is pressed, reset confirmation debounce timer
  if ( Button.fell() ){
    Serial << F("button pressed; is this a confirmation press, or should we go back to streaming?") << endl;
    confirmDebounceTimer.reset();
  }


  if ( Button.read() == LOW ) {
    // check time elapsed on the confirmation timer since the last button change
    if (confirmDebounceTimer.check()) {
      // Button has been held for long enough, go back to preview streaming
      Serial << F("confirmation debounce timer expired; go back to streaming") << endl;
      //Display.setAnimation(PICKER_PREVIEW_INIT);
      picker.transitionTo(previewStreaming);
    }
  } else {
    // if color was not confirmed in time, reset
    if ( resetTimer.check() ) {
      Serial << F("not confirmed in time; resetting") << endl;
      Display.setColor(color);
      Display.setAnimation(PICKER_RESET);
      picker.transitionTo(ready);
    }
  }
}

