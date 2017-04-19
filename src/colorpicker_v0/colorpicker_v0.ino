/*

                                            initialize
                                                v
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
    set animation to pulse single
  update:
    pulse neopixels (color cycle if none set)
    button.update
    if button.fallingEdge {
      Sensor.enable()
    }
    if button.risingEdge {
      Sensor.disable()
    }
    if ( Sensor.color() != this->color && Sensor.color() != black ) {
      this->previewColor = Sensor.color();
      goto state preview
    }

preview:
  enter:
    set animation sequence(picker_init_preview, solid) (fill display with this->previewColor, then hold preview color)
      (animation internally sets animation.transitioning = true)
    reset timeout timer
  update:
    if ( Sensor.color() != this->color && Sensor.color() != black ) {
      this->previewColor = Sensor.color()
    }
    button.update
    if button.pressed = false && timeout.expired {
      goto reset
    }
    // todo problem here.  on falling edge, set reset timer for 250 millis.  enable or confirm after 250 millis
    if button.fallingEdge {
      if button pushed for more than 250 millis {
        if button.isDoubleClick {
          goto confirm
        } else {
          Sensor.enable()
          set animation to solid
        }
      }
    }
    if button.risingEdge {
      Sensor.disable()
      set animation to pulse full
      reset timeout timer
    }


  reset:
    enter:
      set animation to reset (clear display and set back to this->color)
        (animation internally sets animation.transitioning = true)
    update:
      clearing display, going back to single pixel
      if !animation.Transitioning() { // updated when display has been emptied to only a single pixel
        goto ready
      }

  confirm
    enter:
      this->color = this->previewColor
      Network.publish(this->color) // initiate network sendWithAck event
      set animation to picker_confirm
    update:
      





  loop{
    Animation.update()
    Sensor.update()
    Network.update()
    state.update()
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
