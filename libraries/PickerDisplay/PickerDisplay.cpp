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
void queueAnimation(animation anim) {
  if ( this->TotalSteps == 0 ) {
    // current animation will never "complete", so transition immediately
    this->setAnimation(anim);
  } else {
    this->queue = anim;
  }
}

void setAnimation(animation anim, bool clearQueue) {
  this->ActivePattern = anim;
  this->Index = 0;
  if ( clearQueue ) {
    this->queue = NONE;
  }
  switch (anim) {
    case BLACK:
      Black();
      break;
    case PICKER_PREVIEW_INIT:
      PickerPreviewInit();
      break;
    case PICKER_RESET:
      PickerReset();
      break;
    case PICKER_PULSE:
      PickerPulse();
      break;
    case PICKER_PULSE_SINGLE:
      PickerPulseSingle();
      break;
    case PICKER_CONFIRM:
      PickerConfirm();
    break;
    default:
      this->TotalSteps = 0;
      break;
  }
}

// Change to the next animation in the queue
void OnComplete() {
  if ( this->queue != NONE ) {
    this->setAnimation(this->queue);
    this->queue = NONE;
  }
}

void Update()
{
  if ( pushNextFrame.check() ) {
    lastUpdate = millis();
    switch(ActivePattern) {
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
    this->Increment();
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

void Black() {
    fill_solid( this->leds, NUL_LEDS, CRGB::Black);
    // TotalSteps must be set to 0 so that when queue() is called next it will instantly transition
    this->TotalSteps = 0;
}

void SolidUpdate() {
  fill_solid( this->leds, NUL_LEDS, this->color);
}

void PickerPulseSingle() {
  this->TotalSteps = 16;
  this->randCounter = random8();
}

// fade in a random pixel in and out.  When a pixel is 50% faded out, start fading another random pixel in.
// step 1 - 12: fade in actor in; all others out
// step 13 - 16: fade all out
void PickerPulseSingleUpdate() {
  // every new cycle, choose a new random actor between 2,3 and 3,4
  if ( this->Index = 0) {
    this->actor2Index = this->actor1Index;
    // clear previous tmp
    this->tmp = CRGB(0,0,0);
    // select new actor
    uint16_t newAddr;
    do {
      newAddr = XY(2 + random8(1), 3 + random8(1));
    } while (this->actor1Index == newAddr);
    this->actor1Index = newAddr;
  }

  // if no color is set, cycle through a rainbow
  if (!this->isColor()) {
    // cycle previous actor's color
    leds[this->actor2Index].setHue(this->randCounter);
    // cycle new actor
    this->tmp.setHue(this->randCounter++);
  } else {
    this->tmp = CRGB(this->color);
  }

  // fade all LEDs out
  this->leds.fadeToBlackBy(16);

  if (this->Index < 12) {
    // for the first 3/4, fade actor in
    this->tmp.addToRGB(16);
  } else {
    // for the last 1/4, fade actor out
    this->tmp.fadeToBlackBy(16);
  }

  // update actor
  leds[this->actor1Index] = this->tmp;
}

void PickerPreviewInit() {
  this->TotalSteps = 8;
  this->tmp = CRGB(this->color);
}

// over the course of 8 steps, fill in the display with the new color.
void PickerPreviewInitUpdate()
{
  // update fade-in color
  if (this->isColor()) {
    this->tmp = CRGB(this->color);
  }

  // draw the next frame
  switch (this->Index) {
    case 0:
      leds[XY(0,7)] = this->tmp;
      leds[XY(1,7)] = this->tmp;
      leds[XY(2,7)] = this->tmp;
      leds[XY(3,7)] = this->tmp;

      leds[XY(0,6)] = this->tmp;
      leds[XY(3,6)] = this->tmp;
      break;
    case 1:
      leds[XY(0,7)] = this->tmp;
      leds[XY(1,7)] = this->tmp;
      leds[XY(2,7)] = this->tmp;
      leds[XY(3,7)] = this->tmp;

      leds[XY(0,6)] = this->tmp;
      leds[XY(1,6)] = this->tmp;
      leds[XY(2,6)] = this->tmp;
      leds[XY(3,6)] = this->tmp;

      leds[XY(0,5)] = this->tmp;
      leds[XY(3,5)] = this->tmp;
      break;
    case 2:
      leds[XY(0,7)] = this->tmp;
      leds[XY(1,7)] = this->tmp;
      leds[XY(2,7)] = this->tmp;
      leds[XY(3,7)] = this->tmp;

      leds[XY(0,6)] = this->tmp;
      leds[XY(1,6)] = this->tmp;
      leds[XY(2,6)] = this->tmp;
      leds[XY(3,6)] = this->tmp;

      leds[XY(0,5)] = this->tmp;
      leds[XY(3,5)] = this->tmp;

      leds[XY(0,4)] = this->tmp;
      leds[XY(3,4)] = this->tmp;
      break;
    case 3:
      leds[XY(0,7)] = this->tmp;
      leds[XY(1,7)] = this->tmp;
      leds[XY(2,7)] = this->tmp;
      leds[XY(3,7)] = this->tmp;

      leds[XY(0,6)] = this->tmp;
      leds[XY(1,6)] = this->tmp;
      leds[XY(2,6)] = this->tmp;
      leds[XY(3,6)] = this->tmp;

      leds[XY(0,5)] = this->tmp;
      leds[XY(1,5)] = this->tmp;
      leds[XY(2,5)] = this->tmp;
      leds[XY(3,5)] = this->tmp;

      leds[XY(0,4)] = this->tmp;
      leds[XY(3,4)] = this->tmp;

      leds[XY(0,3)] = this->tmp;
      leds[XY(3,3)] = this->tmp;

      leds[XY(0,2)] = this->tmp;
      leds[XY(3,2)] = this->tmp;
      break;
    case 4:
      // move center 4 pixels down one
      leds[XY(1,2)] = leds[XY(1,4)];
      leds[XY(2,2)] = leds[XY(2,4)];

      leds[XY(0,7)] = this->tmp;
      leds[XY(1,7)] = this->tmp;
      leds[XY(2,7)] = this->tmp;
      leds[XY(3,7)] = this->tmp;

      leds[XY(0,6)] = this->tmp;
      leds[XY(1,6)] = this->tmp;
      leds[XY(2,6)] = this->tmp;
      leds[XY(3,6)] = this->tmp;

      leds[XY(0,5)] = this->tmp;
      leds[XY(1,5)] = this->tmp;
      leds[XY(2,5)] = this->tmp;
      leds[XY(3,5)] = this->tmp;

      leds[XY(0,4)] = this->tmp;
      leds[XY(1,4)] = this->tmp;
      leds[XY(2,4)] = this->tmp;
      leds[XY(3,4)] = this->tmp;

      leds[XY(0,3)] = this->tmp;
      leds[XY(3,3)] = this->tmp;

      leds[XY(0,2)] = this->tmp;
      leds[XY(3,2)] = this->tmp;

      leds[XY(0,1)] = this->tmp;
      leds[XY(3,1)] = this->tmp;

      leds[XY(0,0)] = this->tmp;
      leds[XY(3,0)] = this->tmp;
      break;
    case 5:
      // move down again
      leds[XY(1,1)] = leds[XY(1,3)];
      leds[XY(2,1)] = leds[XY(2,3)];

      leds[XY(0,7)] = this->tmp;
      leds[XY(1,7)] = this->tmp;
      leds[XY(2,7)] = this->tmp;
      leds[XY(3,7)] = this->tmp;

      leds[XY(0,6)] = this->tmp;
      leds[XY(1,6)] = this->tmp;
      leds[XY(2,6)] = this->tmp;
      leds[XY(3,6)] = this->tmp;

      leds[XY(0,5)] = this->tmp;
      leds[XY(1,5)] = this->tmp;
      leds[XY(2,5)] = this->tmp;
      leds[XY(3,5)] = this->tmp;

      leds[XY(0,4)] = this->tmp;
      leds[XY(1,4)] = this->tmp;
      leds[XY(2,4)] = this->tmp;
      leds[XY(3,4)] = this->tmp;

      leds[XY(0,3)] = this->tmp;
      leds[XY(1,3)] = this->tmp;
      leds[XY(2,3)] = this->tmp;
      leds[XY(3,3)] = this->tmp;

      leds[XY(0,2)] = this->tmp;
      leds[XY(3,2)] = this->tmp;

      leds[XY(0,1)] = this->tmp;
      leds[XY(3,1)] = this->tmp;

      leds[XY(0,0)] = this->tmp;
      leds[XY(3,0)] = this->tmp;
      break;
    case 6:
      // move down again
      leds[XY(1,0)] = leds[XY(1,2)];
      leds[XY(2,0)] = leds[XY(2,2)];

      leds[XY(0,7)] = this->tmp;
      leds[XY(1,7)] = this->tmp;
      leds[XY(2,7)] = this->tmp;
      leds[XY(3,7)] = this->tmp;

      leds[XY(0,6)] = this->tmp;
      leds[XY(1,6)] = this->tmp;
      leds[XY(2,6)] = this->tmp;
      leds[XY(3,6)] = this->tmp;

      leds[XY(0,5)] = this->tmp;
      leds[XY(1,5)] = this->tmp;
      leds[XY(2,5)] = this->tmp;
      leds[XY(3,5)] = this->tmp;

      leds[XY(0,4)] = this->tmp;
      leds[XY(1,4)] = this->tmp;
      leds[XY(2,4)] = this->tmp;
      leds[XY(3,4)] = this->tmp;

      leds[XY(0,3)] = this->tmp;
      leds[XY(1,3)] = this->tmp;
      leds[XY(2,3)] = this->tmp;
      leds[XY(3,3)] = this->tmp;

      leds[XY(0,2)] = this->tmp;
      leds[XY(1,2)] = this->tmp;
      leds[XY(2,2)] = this->tmp;
      leds[XY(3,2)] = this->tmp;

      leds[XY(0,1)] = this->tmp;
      leds[XY(3,1)] = this->tmp;

      leds[XY(0,0)] = this->tmp;
      leds[XY(3,0)] = this->tmp;
      break;
    case 7:
      fill_solid( this->leds, NUL_LEDS, this->tmp);
      break;
  }
}

void PickerReset() {
  this->TotalSteps = 8;
}

// push out old color
void PickerResetUpdate() {
  // draw black pixel in center, expanding outward
}

void PickerPulse() {
  this->TotalSteps = 20;
}

// fade entire display from bright to dim
void PickerPulseUpdate() {
}

void PickerConfirm() {
  this->TotalSteps = 20;
}

// do a sweet confimation animation.  Make it super cool and poofy
void PickerConfirmUpdate() {

}




/**
  XY grid helper methods taken from FastLED example:
  https://github.com/FastLED/FastLED/blob/master/examples/XYMatrix/XYMatrix.ino
**/

// Helper functions for an two-dimensional XY matrix of pixels.
// Simple 2-D demo code is included as well.
//
//     XY(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
//             No error checking is performed on the ranges of x and y.
//
//     XYsafe(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
//             Error checking IS performed on the ranges of x and y, and an
//             index of "-1" is returned.  Special instructions below
//             explain how to use this without having to do your own error
//             checking every time you use this function.  
//             This is a slightly more advanced technique, and 
//             it REQUIRES SPECIAL ADDITIONAL setup, described below.

// XY grid helper method initialization


// Param for different pixel layouts

//
// Bonus vocabulary word: anything that goes one way 
// in one row, and then backwards in the next row, and so on
// is call "boustrophedon", meaning "as the ox plows."

// This function will return the right 'led index number' for 
// a given set of X and Y coordinates on your matrix.  
// IT DOES NOT CHECK THE COORDINATE BOUNDARIES.  
// That's up to you.  Don't pass it bogus values.
//
// Use the "XY" function like this:
//
//    for( uint8_t x = 0; x < kMatrixWidth; x++) {
//      for( uint8_t y = 0; y < kMatrixHeight; y++) {
//      
//        // Here's the x, y to 'led index' in action: 
//        leds[ XY( x, y) ] = CHSV( random8(), 255, 255);
//      
//      }
//    }
//
//
uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }

  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  
  return i;
}

uint16_t XYsafe( uint8_t x, uint8_t y)
{
  if( x >= kMatrixWidth) return -1;
  if( y >= kMatrixHeight) return -1;
  return XY(x,y);
}