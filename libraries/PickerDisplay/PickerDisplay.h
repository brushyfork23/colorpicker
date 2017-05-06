/**
Runs animation of an Adafruit NeoPixel FeatherWing



*/
#ifndef Picker_Display_h
#define Picker_Display_h

#include <FastLED.h>
#include <Metro.h>

#define NUM_LEDS 32
#define BRIGHTNESS 32

// Animation types supported:
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
    	void setAnimation(animation anim);
    	// queue an animation to display next
    	void queueAnimation(animation anim);

	private:
		CRGB leds[NUM_LEDS];

	    animation ActiveAnimation;  // which animation is running
    
    	Metro pushNextFrame;

    	animation queue;

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
    
};

#endif