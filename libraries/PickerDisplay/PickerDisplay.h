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
    	void setCallback(void (*callback)());
    	void setColor(uint32_t color);
    	// runs the animation
    	void update();

    	// animations
    	void None();
    	void Solid(uint32_t color);
    	void PickerPreviewInit(uint32_t color);
    	void PickerReset(uint32_t color);
    	void PickerPulse(uint32_t color);
    	void PickerPulseSingle(uint32_t color);
    	void PickerConfirm(uint32_t color);

	private:
		CRGB leds[NUM_LEDS];

	    animation ActiveAnimation;  // which animation is running
    
    	Metro pushNextFrame;

		void SolidUpdate();
		void PickerPreviewInitUpdate();
		void PickerResetUpdate();
		void PickerPulseUpdate();
		void PickerPulseSingleUpdate();
		void PickerConfirmUpdate();
    
    	uint16_t TotalSteps;  // total number of steps in the pattern
    	uint16_t Index;  // current step within the pattern
    	uint32_t color;
    	uint32_t previewColor;
    
    	void (*OnComplete)();  // Callback on completion of pattern
    
};

#endif