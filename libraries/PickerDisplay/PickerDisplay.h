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
	FADE,
	SOLID,
	PICKER_INIT_PREVIEW,
	PICKER_RESET,
	PICKER_PULSE,
	PICKER_PULSE_SINGLE
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
    	void setCallback(void (*callback)())
    	// runs the animation
    	void update();

    	// animations
    	void None();
    	void Solid(uint32_t color);
    	void PickerInitPreview(uint32_t color)

	private:
		CRGB leds[NUM_LEDS];

	    animation ActiveAnimation;  // which animation is running
    
    	Metro pushNextFrame;

		void SolidUpdate();
		void PickerInitPreviewUpdate();
    
    	uint16_t TotalSteps;  // total number of steps in the pattern
    	uint16_t Index;  // current step within the pattern
    	uint32_t color;
    	uint32_t previewColor;
    
    	void (*OnComplete)();  // Callback on completion of pattern
    
};

#endif