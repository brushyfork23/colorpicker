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


#include <Metro.h>

// SPST Pushbutton
#include <Bounce2.h>
#define BUTTON_PIN 12
Bounce Button = Bounce();

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
  uint32_t getColor();
private:
  Adafruit_TCS34725 tcs;
  Metro readingTimer;
  bool enabled;
  int ledPin;
  uint32_t color; // last committed color code
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
      this->color = (red << 8) & 0x00ff0000 |
                    (green ) & 0x0000ff00 |
                    (blue >> 8) & 0x000000ff;
    }
  }
}
bool RGBSensor::isColor() {
  // TODO return if this->color != black or white (or anything returned when not actually touching a color)
  if (this->enabled
    && this->color > 0) {
    return true;
  }
  return false;
}
uint32_t RGBSensor::getColor() {
  return this->color;
}
RGBSensor Sensor = RGBSensor(SENSOR_LED_PIN);


// NeoPixel FeatherWing
#include "PickerDisplay.h"
#define BRIGHTNESS 84

#define SENSOR_UPDATE_MILIS 50
#define RESET_TIMEOUT_MILIS 3000 // duration of reset timeout, in miliseconds
#define CONFRIM_COUNTDOWN_MILIS 300 // duration of confirmation countdown timer, in miliseconds
#define CONFIRMATION_PRESSES 3 // number of button presses to confirm a color
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

uint32_t color, previewColor;
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
    Display.setColor(0x00FF0000);
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
    Serial << F("detected color: ") << Sensor.getColor() << endl;
    previewColor = Sensor.getColor();
    Display.setColor(previewColor);
    Display.setAnimation(PICKER_PREVIEW_INIT);
    picker.transitionTo(previewStreaming);
  }
}

///[previewStreaming state:enter]
// transition display to showing full bright whatever color is scanned
void previewStreamingEnter() {
  Serial << F("[state] previewStreaming") << endl;
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
      Serial << F("confirmation debounce timer expired") << endl;
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