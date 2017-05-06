#include "PickerDisplay.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

// startup
void PickerDisplay::begin(byte pin) {
  FastLED.addLeds<NEOPIXEL, pin>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  this->setFPS();
  this->setBrightness();

  this->startSeed();

  Serial << F("Display startup complete.") << endl;
}
// sets FPS
void PickerDisplay::setFPS(uint16_t framesPerSecond) {
  this->pushNextFrame.interval(1000UL/framesPerSecond);
  Serial << F("FPS= ") << framesPerSecond << F(". show update=") << 1000UL/framesPerSecond << F(" ms.") << endl;
}
// sets master brightness
void PickerDisplay::setBrightness(byte brightness) {
  // set master brightness control
  FastLED.setBrightness(brightness); 
  Serial << F("Brightness set= ") << brightness << endl;
}
void PickerDisplay::startSeed(uint16_t seed) {
  random16_set_seed( seed );  // FastLED/lib8tion
  Serial << F("random seed=") << seed << endl;
}
void PickerDisplay::setColor(uint32_t color) {
  this->color = CRGB(color);
}

void setAnimation(animation anim) {
  this->ActivePattern = anim;
  switch (anim) {
    case BLACK:
      fill_solid( this->leds, NUL_LEDS, CRGB::Black);
      break;
    case SOLID:
      fill_solid( this->leds, NUL_LEDS, this->color);
      break;
    case PICKER_PREVIEW_INIT:
      this->TotalSteps = 8;
      this->Index = 0;
      break;
    case PICKER_PULSE_SINGLE:
      this->TotalSteps = 20;
      this->Index = 0;
      break;
    default:
      break;
  }
}

// Change to the next animation in the queue
void OnComplete() {
  if ( this->queue != NONE ) {
    this->ActivePattern = this->queue;
    this->queue = NONE;
  }
}

// Update the pattern
void Update()
{
    if(pushNextFrame.check())
    {
        lastUpdate = millis();
        switch(ActivePattern)
        {
            case SOLID:
            	SolidUpdate();
            	break;
            case PICKER_PREVIEW_INIT:
            	PickerPreviewInitUpdate();
            	break;
            case PICKER_RESET:
            	PickerResetUpdate();
            	break;
            case PICKER_PULSE:
            	PickerPulseUpdate();
            	break;
            case PICKER_PULSE_SINGLE:
            	PickerPulseSingleUpdate();
            	break;
            case PICKER_CONFIRM:
              PickerConfirmUpdate();
            default:
                break;
        }
        show();
    }
}

// Increment the Index and reset at the end
void Increment()
{
   this->Index++;
   if (this->Index >= this->TotalSteps)
    {
        this->Index = 0;
        this->OnComplete(); // call the comlpetion callback
    }
}


/**
	ANIMATIONS
**/

// over the course of 8 steps, fill in the display with the new color.
void PickerPreviewInitUpdate()
{
	fill_solid()
}
void PickerPulseSingleUpdate() {
  // fade in, then out
  this->Increment();
}