#include "FastLED.h"
#include <NESpad.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

// How many leds in your strip?
#define SIDE_PANEL 29 + 25 + 26 + 19
#define FRONT_PANEL 18 + 24 + 18 + 24
#define BACK_PANEL 25 + 25 + 59 + 59
#define NUM_LEDS SIDE_PANEL + SIDE_PANEL + FRONT_PANEL + BACK_PANEL

#define NUM_PANELS 4

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 5
#define CLOCK_PIN 13

int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() { 
  matrix.setIntensity(15); // Use a value between 0 and 15 for brightness
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(9600);  
  pinMode(13, OUTPUT);
}

// put your own strobe/clock/data pin numbers here -- see the pinout in readme.txt
NESpad nintendo = NESpad(2,3,4);

byte state = 0;
int beat;
int mode = 0;

void checkMode () {
  int x = 2;
  int y = 1;

  if (state & NES_UP) {
    mode++;
  }

  if (state & NES_DOWN) {
    mode--;
  }

  mode = max(mode, 0);
  mode = min(mode, 25);

  matrix.fillScreen(LOW);
  matrix.drawChar(x, y, mode + 'A', HIGH, LOW, 1);
  matrix.write(); // Send bitmap to display
}

CRGB color = CRGB(255,255,255);

bool panels[NUM_PANELS];
bool b;

void loop() { 
  state = nintendo.buttons();

  beat++;

  if (beat % 3 == 0) {
    checkMode();
  }
  
  // shows the shifted bits from the joystick
  // buttons are high (1) when up 
  // and low (0) when pressed
  // Serial.println(~state, BIN);
  
  digitalWrite(13, (beat % 60 < 30) ? HIGH : LOW); 
  
  byte brightness = sin(beat / 10.0) * 64 + 190;

  color.fadeToBlackBy(32);

  panels[0] = state & NES_LEFT;
  panels[1] = state & NES_UP;
  panels[2] = state & NES_RIGHT;
  panels[3] = state & NES_DOWN;
  
  if (state & NES_START){
    color.r = 255;
  }
  if (state & NES_B){
    color.g = 255;
  }
  if (state & NES_A){
    color.b = 255;
  }
  
//  if (state == 0){
//    beat = 0;
//  }
  
  int i;


  for(i;i<NUM_LEDS;i++){
    leds[i] = CRGB::Black;

    if (panels[0]){ // i * NUM_PANELS / NUM_LEDS]){
      leds[i] = color;
    }
  }
  FastLED.show();

  delay(1000 / 60);
}

