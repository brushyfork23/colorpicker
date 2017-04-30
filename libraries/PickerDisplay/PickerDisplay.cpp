#include "PickerDisplay.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

CRGB leds[NUM_LEDS];

// startup
void PickerDisplay::begin(byte pin) {
  FastLED.addLeds<NEOPIXEL, pin>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  this->setFPS();
  this->setBrightness();

  this->None();

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
  this->brightVal = brightness;
  FastLED.setBrightness(brightness); 
  Serial << F("Brightness set= ") << brightness << endl;
}
void PickerDisplay::startSeed(uint16_t seed) {
  random16_set_seed( seed );  // FastLED/lib8tion
  Serial << F("random seed=") << seed << endl;
}
void PickerDisplay::setCallback(void (*callback)()) {
	this->OnComplete = callback;
}
void PickerDisplay::setColor(uint32_t color) {
  this->color = color;
}

// Update the pattern
    void Update()
    {
        if(pushNextFrame.check())
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                case SOLID:
                	SolidUpdate();
                	break;
                case PICKER_PREVIEW_Init:
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
                default:
                    break;
            }
        }
    }
  
    // Increment the Index and reset at the end
    void Increment()
	{
       this->Index++;
       if (this->Index >= this->TotalSteps)
        {
            this->Index = 0;
            if (this->OnComplete != NULL)
            {
                this->OnComplete(); // call the comlpetion callback
            }
        }
    }


    /**
    	ANIMATIONS
    **/

   	void None()
   	{
   		this->ActivePattern = NONE;
   		fill_solid( this->leds, NUL_LEDS, CRGB::Black);
   	}

   	void Solid(uint32_t color)
    {
    	this->ActivePattern = SOLID;
      this->color = CRGB(color)
    }

    void SolidUpdate() {
    	ColorSet(Color1);
    	show();
    }

    void PickerPreviewInit(uint32_t color)
    {
    	this->ActivePattern = PICKER_PREVIEW_INIT;
    	this->TotalSteps = 8;
    	this->Index = 0;
    	this->previewColor = color;
    }

    // over the course of 8 steps, fill in the display with the new color.
    void PickerPreviewInitUpdate()
    {
    	fill_solid()
    }