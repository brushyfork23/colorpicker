# Color Picker
Color Picker is a handheld device capable of scanning colors from objects in the physical world, displaying them on a screen, and transfering them over WiFi to other projects.

Powering on the device displays a standby animation on the screen.  When the button is held, a front-facing RGB sensor is activated and begins scanning for colors.  When a bright color is detected, it is displayed on the screen.  Double pressing the button will then confirm that color, and it will be broadcast over WiFi to any projects listening.

## wiring
* Neopixel board jumper moved to 15
* TCS345725 GND, 3V, SDA, and SDL wired to corrosponding feather pins
* TCS345725 LED wired to feather 13
* Momentary pushbutton wired between feather 12 and feather GND
* Rocker switch wired between positive battery lead and feather