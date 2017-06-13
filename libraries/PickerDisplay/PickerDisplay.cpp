#include "PickerDisplay.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* leds( leds_plus_safety_pixel + 1);

// startup
void PickerDisplay::begin() {
  FastLED.addLeds<NEOPIXEL, PIN_DATA>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  this->setFPS();
  this->setBrightness();

  this->startSeed();
  this->randCounter = random8();

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
  Serial << F("color set=") << color << endl;
}
void PickerDisplay::queueAnimation(animation anim) {
  if ( this->TotalSteps == 0 ) {
    // current animation will never "complete", so transition immediately
    this->setAnimation(anim);
  } else {
    this->queue = anim;
    Serial << F("animation queued=") << anim << endl;
  }
}

void PickerDisplay::setAnimation(animation anim, bool clearQueue) {
  Serial << F("set animation=");
  switch (anim) {
      case NONE:
        Serial << F("None");
        break;
    case BLACK:
      Serial << F("Black");
      break;
    case SOLID:
      Serial << F("Solid");
      break;
    case PICKER_PREVIEW_INIT:
      Serial << F("Picker Preview Init");
      break;
    case PICKER_RESET:
      Serial << F("Picker Reset");
      break;
    case PICKER_PULSE:
      Serial << F("Picker Pulse");
      break;
    case PICKER_PULSE_SINGLE:
      Serial << F("Picker Pulse Single");
      break;
    case PICKER_CONFIRM:
      Serial << F("Picker Confirm");
      break;
    default:
      Serial << F("UNDEFINED");
  }
  Serial << endl;

  this->ActivePattern = anim;
  this->Index = 0;
  if ( clearQueue ) {
    this->queue = NONE;
  }
  switch (this->ActivePattern) {
    case BLACK:
      this->Black();
      break;
    case SOLID:
      this->Solid();
      break;
    case PICKER_PREVIEW_INIT:
      this->PickerPreviewInit();
      break;
    case PICKER_RESET:
      this->PickerReset();
      break;
    case PICKER_PULSE:
      this->PickerPulse();
      break;
    case PICKER_PULSE_SINGLE:
      this->PickerPulseSingle();
      break;
    case PICKER_CONFIRM:
      this->PickerConfirm();
    break;
    default:
      this->TotalSteps = 0;
      break;
  }
  // Show now, otherwise none of the initializations will be
  // displayed, and will be overwritten by the next frame.
  FastLED.show();
}

// Change to the next animation in the queue
void PickerDisplay::OnComplete() {
  if ( this->queue != NONE ) {
    this->setAnimation(this->queue);
    this->queue = NONE;
  }
}

void PickerDisplay::update()
{
  if ( pushNextFrame.check() ) {
    switch(this->ActivePattern) {
      case SOLID:
        this->SolidUpdate();
        break;
      case PICKER_PREVIEW_INIT:
        this->PickerPreviewInitUpdate();
        break;
      case PICKER_RESET:
        this->PickerResetUpdate();
        break;
      case PICKER_PULSE:
        this->PickerPulseUpdate();
        break;
      case PICKER_PULSE_SINGLE:
        this->PickerPulseSingleUpdate();
        break;
      case PICKER_CONFIRM:
        this->PickerConfirmUpdate();
      default:
          break;
    }
    this->randCounter++;
    this->Increment();
    FastLED.show();
  }
}

// Increment the Index and reset at the end
void PickerDisplay::Increment()
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

void PickerDisplay::Black() {
  this->setFPS();
  fill_solid( leds, NUM_LEDS, CRGB::Black);
  // TotalSteps must be set to 0 so that when queue() is called next it will instantly transition
  this->TotalSteps = 0;
}

void PickerDisplay::Solid() {
  this->setFPS();
  fill_solid( leds, NUM_LEDS, this->color);
  // save color to tmp so changes can be detected and we don't have to write every update loop iteration
  this->tmpCRGB = this->color;
  // TotalSteps must be set to 0 so that when queue() is called next it will instantly transition
  this->TotalSteps = 0;
}

void PickerDisplay::SolidUpdate() {
  // update color if it has changed
  if (this->tmpCRGB != this->color) {
    this->tmpCRGB = this->color;
    fill_solid( leds, NUM_LEDS, this->color);
  }
}

void PickerDisplay::PickerPulseSingle() {
  this->setFPS();
  fill_solid( leds, NUM_LEDS, 0);
  this->TotalSteps = 32;
  this->actor1Index = 0;
  this->actor2Index = 0;
}

// fade in a random pixel in and out.  When a pixel is partially faded out, start fading another random pixel in.
// first 3/4: fade in first actor, while fading out the second (previous) actor
// last 1/4: begin fading first actor out
void PickerDisplay::PickerPulseSingleUpdate() {
  // every new cycle, choose a new random actor between 1,3 and 2,4
  if ( this->Index == 0) {
    this->actor2Index = this->actor1Index;

    // select new actor
    uint16_t newAddr;
    do {
      uint8_t x =  1 + random8(2);
      uint8_t y =  3 + random8(2);
      //Serial << F("[animPickerPulse] chosing new actor: x=") << x << F(" y=") << y << endl;
      newAddr = this->XY(x, y);
    } while (this->actor1Index == newAddr);
    this->actor1Index = newAddr;
    //Serial << F("[animPickerPulse] moved actor1 to actor2. Now actor1=") << this->actor1Index << F(" actor2=") << this->actor2Index << endl;
  }

  // set tmp with a full-brightness target color
  if (this->color == CRGB(0)) {
    this->tmpCRGB.setHue(this->randCounter);
  } else {
    this->tmpCRGB = this->color;
  }

  // For first half of frames, fade old LED out
  // from the brightness it left off at at the end of the last cycle (2/3 max brightness)
  // to completely off by halfway through this cycle
  if (this->Index <= (this->TotalSteps / 2) ) {
    CRGB tmp = this->tmpCRGB;
    // 2/3 max brightness - (2/3 max brightness * (index/(totalsteps/2)))
    //(2/3 * 256) - (2/3 * 256 * (Index/ (totalsteps / 2)))
    uint8_t brightness = 170 - (170 * this->Index / (this->TotalSteps / 2) );
    //Serial << F("[animPickerPUlse] fading 2 out.  Index=") << this->Index << F(" brightness=") << brightness << endl;
    leds[this->actor2Index] = tmp.nscale8( brightness ); // 2/3 max brightness - (2/3 max brightness * (index/(totalsteps/2)))    (2/3 * 256) - (2/3 * 256 * (Index/ (totalsteps / 2)))
  }

  // For first 3/4, fade new LED in
  // from completely off to completely on.
  if (this->Index < this->TotalSteps * 3 / 4) {
    CRGB tmp = this->tmpCRGB;
    uint8_t brightness = 255 * this->Index / (this->TotalSteps * 3 / 4);
    //Serial << F("[animPickerPUlse] fading 1 in.  Index=") << this->Index << F(" brightness=") << brightness << endl;
    leds[this->actor1Index] = tmp.nscale8_video( brightness ); // max brightness * (index / (.75 * totalsteps))
  }

  // For last 1/4, fade new LED out
  // from completely on to 2/3 max brightness
  if (this->Index >= this->TotalSteps * 3 / 4) {
    CRGB tmp = this->tmpCRGB;
    // current brightness = max brightness - (percent to fade * # steps taken into segment / segment length
    // max brightness - (brightness / 3 * ( (index - ((totalsteps * .75) / (totalsteps - .75 * totalsteps)) ) )
    uint8_t brightness = 255 - 85 *  (this->Index - (this->TotalSteps * 3 / 4)) / ( this->TotalSteps - (this->TotalSteps * 3 / 4) );
    //Serial << F("[animPickerPUlse] fading 1 out.  Index=") << this->Index << F(" brightness=") << brightness << endl;
    leds[this->actor1Index] = tmp.nscale8( brightness ); 
  }
}

void PickerDisplay::PickerPreviewInit() {
  this->setFPS(16);
  this->TotalSteps = 8;
}

// over the course of 8 steps, fill in the display with the new color.
void PickerDisplay::PickerPreviewInitUpdate()
{
  // draw the next frame
  switch (this->Index) {
    case 0:
      leds[this->XY(0,7)] = this->color;
      leds[this->XY(1,7)] = this->color;
      leds[this->XY(2,7)] = this->color;
      leds[this->XY(3,7)] = this->color;

      leds[this->XY(0,6)] = this->color;
      leds[this->XY(3,6)] = this->color;
      break;
    case 1:
      leds[this->XY(0,7)] = this->color;
      leds[this->XY(1,7)] = this->color;
      leds[this->XY(2,7)] = this->color;
      leds[this->XY(3,7)] = this->color;

      leds[this->XY(0,6)] = this->color;
      leds[this->XY(1,6)] = this->color;
      leds[this->XY(2,6)] = this->color;
      leds[this->XY(3,6)] = this->color;

      leds[this->XY(0,5)] = this->color;
      leds[this->XY(3,5)] = this->color;
      break;
    case 2:
      leds[this->XY(0,7)] = this->color;
      leds[this->XY(1,7)] = this->color;
      leds[this->XY(2,7)] = this->color;
      leds[this->XY(3,7)] = this->color;

      leds[this->XY(0,6)] = this->color;
      leds[this->XY(1,6)] = this->color;
      leds[this->XY(2,6)] = this->color;
      leds[this->XY(3,6)] = this->color;

      leds[this->XY(0,5)] = this->color;
      leds[this->XY(3,5)] = this->color;

      leds[this->XY(0,4)] = this->color;
      leds[this->XY(3,4)] = this->color;
      break;
    case 3:
      leds[this->XY(0,7)] = this->color;
      leds[this->XY(1,7)] = this->color;
      leds[this->XY(2,7)] = this->color;
      leds[this->XY(3,7)] = this->color;

      leds[this->XY(0,6)] = this->color;
      leds[this->XY(1,6)] = this->color;
      leds[this->XY(2,6)] = this->color;
      leds[this->XY(3,6)] = this->color;

      leds[this->XY(0,5)] = this->color;
      leds[this->XY(1,5)] = this->color;
      leds[this->XY(2,5)] = this->color;
      leds[this->XY(3,5)] = this->color;

      leds[this->XY(0,4)] = this->color;
      leds[this->XY(3,4)] = this->color;

      leds[this->XY(0,3)] = this->color;
      leds[this->XY(3,3)] = this->color;

      leds[this->XY(0,2)] = this->color;
      leds[this->XY(3,2)] = this->color;
      break;
    case 4:
      // move center 4 pixels down one
      leds[this->XY(1,2)] = leds[this->XY(1,4)];
      leds[this->XY(2,2)] = leds[this->XY(2,4)];

      leds[this->XY(0,7)] = this->color;
      leds[this->XY(1,7)] = this->color;
      leds[this->XY(2,7)] = this->color;
      leds[this->XY(3,7)] = this->color;

      leds[this->XY(0,6)] = this->color;
      leds[this->XY(1,6)] = this->color;
      leds[this->XY(2,6)] = this->color;
      leds[this->XY(3,6)] = this->color;

      leds[this->XY(0,5)] = this->color;
      leds[this->XY(1,5)] = this->color;
      leds[this->XY(2,5)] = this->color;
      leds[this->XY(3,5)] = this->color;

      leds[this->XY(0,4)] = this->color;
      leds[this->XY(1,4)] = this->color;
      leds[this->XY(2,4)] = this->color;
      leds[this->XY(3,4)] = this->color;

      leds[this->XY(0,3)] = this->color;
      leds[this->XY(3,3)] = this->color;

      leds[this->XY(0,2)] = this->color;
      leds[this->XY(3,2)] = this->color;

      leds[this->XY(0,1)] = this->color;
      leds[this->XY(3,1)] = this->color;

      leds[this->XY(0,0)] = this->color;
      leds[this->XY(3,0)] = this->color;
      break;
    case 5:
      // move down again
      leds[this->XY(1,1)] = leds[this->XY(1,3)];
      leds[this->XY(2,1)] = leds[this->XY(2,3)];

      leds[this->XY(0,7)] = this->color;
      leds[this->XY(1,7)] = this->color;
      leds[this->XY(2,7)] = this->color;
      leds[this->XY(3,7)] = this->color;

      leds[this->XY(0,6)] = this->color;
      leds[this->XY(1,6)] = this->color;
      leds[this->XY(2,6)] = this->color;
      leds[this->XY(3,6)] = this->color;

      leds[this->XY(0,5)] = this->color;
      leds[this->XY(1,5)] = this->color;
      leds[this->XY(2,5)] = this->color;
      leds[this->XY(3,5)] = this->color;

      leds[this->XY(0,4)] = this->color;
      leds[this->XY(1,4)] = this->color;
      leds[this->XY(2,4)] = this->color;
      leds[this->XY(3,4)] = this->color;

      leds[this->XY(0,3)] = this->color;
      leds[this->XY(1,3)] = this->color;
      leds[this->XY(2,3)] = this->color;
      leds[this->XY(3,3)] = this->color;

      leds[this->XY(0,2)] = this->color;
      leds[this->XY(3,2)] = this->color;

      leds[this->XY(0,1)] = this->color;
      leds[this->XY(3,1)] = this->color;

      leds[this->XY(0,0)] = this->color;
      leds[this->XY(3,0)] = this->color;
      break;
    case 6:
      // move down again
      leds[this->XY(1,0)] = leds[this->XY(1,2)];
      leds[this->XY(2,0)] = leds[this->XY(2,2)];

      leds[this->XY(0,7)] = this->color;
      leds[this->XY(1,7)] = this->color;
      leds[this->XY(2,7)] = this->color;
      leds[this->XY(3,7)] = this->color;

      leds[this->XY(0,6)] = this->color;
      leds[this->XY(1,6)] = this->color;
      leds[this->XY(2,6)] = this->color;
      leds[this->XY(3,6)] = this->color;

      leds[this->XY(0,5)] = this->color;
      leds[this->XY(1,5)] = this->color;
      leds[this->XY(2,5)] = this->color;
      leds[this->XY(3,5)] = this->color;

      leds[this->XY(0,4)] = this->color;
      leds[this->XY(1,4)] = this->color;
      leds[this->XY(2,4)] = this->color;
      leds[this->XY(3,4)] = this->color;

      leds[this->XY(0,3)] = this->color;
      leds[this->XY(1,3)] = this->color;
      leds[this->XY(2,3)] = this->color;
      leds[this->XY(3,3)] = this->color;

      leds[this->XY(0,2)] = this->color;
      leds[this->XY(1,2)] = this->color;
      leds[this->XY(2,2)] = this->color;
      leds[this->XY(3,2)] = this->color;

      leds[this->XY(0,1)] = this->color;
      leds[this->XY(3,1)] = this->color;

      leds[this->XY(0,0)] = this->color;
      leds[this->XY(3,0)] = this->color;
      break;
    case 7:
      fill_solid( leds, NUM_LEDS, this->color);
      break;
  }
}

void PickerDisplay::PickerPulse() {
  //this->setFPS(16);
  //this->TotalSteps = 16;
  //this->tmpCRGB = this->color;
  //this->tmpCRGB.nscale8_video(16);
  fill_solid( leds, NUM_LEDS, this->color);
}

// fade entire display from bright to dim
void PickerDisplay::PickerPulseUpdate() {
  //if (this->Index < 8) {
  //  nscale8_video(leds, NUM_LEDS, 32);
  // } else {
  //   this->tmpCRGB += this->tmpCRGB.nscale8_video(32);
  //   fill_solid( leds, NUM_LEDS, this->tmpCRGB);
  // }
}

void PickerDisplay::PickerReset() {
  this->setFPS();
  this->TotalSteps = 8;
}

// push out old color
void PickerDisplay::PickerResetUpdate() {
  // draw black pixel in center, expanding outward
}

void PickerDisplay::PickerConfirm() {
  this->setFPS();
  this->TotalSteps = 20;
}

// do a sweet confimation animation.  Make it super cool and poofy
void PickerDisplay::PickerConfirmUpdate() {

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
uint16_t PickerDisplay::XY( uint8_t x, uint8_t y)
{
  return (x * kMatrixHeight) + y;
}

uint16_t PickerDisplay::XYsafe( uint8_t x, uint8_t y)
{
  if( x >= kMatrixWidth) return -1;
  if( y >= kMatrixHeight) return -1;
  return this->XY(x,y);
}

PickerDisplay Display;