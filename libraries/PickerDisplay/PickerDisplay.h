/**
Runs animation on an Adafruit NeoPixel FeatherWing


*/
#ifndef Picker_Display_h
#define Picker_Display_h

#include <FastLED.h>
#include <Metro.h>

const uint8_t kMatrixWidth = 4;
const uint8_t kMatrixHeight = 8;
const bool    kMatrixSerpentineLayout = true;
// Set 'kMatrixSerpentineLayout' to false if your pixels are 
// laid out all running the same way, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//     .----<----<----<----'
//     |
//     5 >  6 >  7 >  8 >  9
//                         |
//     .----<----<----<----'
//     |
//    10 > 11 > 12 > 13 > 14
//                         |
//     .----<----<----<----'
//     |
//    15 > 16 > 17 > 18 > 19
//
// Set 'kMatrixSerpentineLayout' to true if your pixels are 
// laid out back-and-forth, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//                         |
//     9 <  8 <  7 <  6 <  5
//     |
//     |
//    10 > 11 > 12 > 13 > 14
//                        |
//                        |
//    19 < 18 < 17 < 16 < 15
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define BRIGHTNESS 32

// Available animations
enum animation {
	NONE,
    BLACK,
	SOLID,
	PICKER_PREVIEW_INIT,
	PICKER_RESET,
	PICKER_PULSE,
	PICKER_PULSE_SINGLE,
	PICKER_CONFIRM
};


class PickerDisplay {

	public:
		// initialize led strips
	    void begin(byte pin);
	    // which calls the following functions with their defaults:
	    // set frames per second
	    void setFPS(uint16_t framesPerSecond=30);
	    // set master brightness
	    void setBrightness(byte brightness=BRIGHTNESS);
	    // set the random seed for animation
    	void startSeed(uint16_t seed = 1337);
    	void setColor(uint32_t color);
    	// runs the animation; transitions to next 
    	void update();
    	// immediately change the current animation
    	void setAnimation(animation anim, bool clearQeue = true);
    	// queue an animation to display when the current completes
    	void queueAnimation(animation anim);

	private:

		CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
		CRGB* const leds( leds_plus_safety_pixel + 1);

	    animation ActiveAnimation;  // which animation is running
    
    	Metro pushNextFrame;

    	animation queue;

        void Solid();
		void SolidUpdate();
		void PickerPreviewInitUpdate();
		void PickerResetUpdate();
		void PickerPulseUpdate();
		void PickerPulseSingleUpdate();
		void PickerConfirmUpdate();
		void OnComplete();
    
    	uint16_t TotalSteps;  // total number of steps in the pattern
    	uint16_t Index;  // current step within the pattern
    	CRGB color;

    	// animation helper vars
    	uint8_t randCounter;
    	uint8_t actor1Index;
    	uint8_t actor2Index;
    	CRGB tmpCRGB;
    
};

#endif