/* todo: 
 *  
 *  * stop cycling panels for 2 seconds after hitting arrow key (black 
      when arrow not pressed)
    * only go back to mode 0 when press and hold select
    * alternate between mode 0 and mode 1 (all black) when hold select
    * go to alternate color scheme when pressing A or B
    * white strobe when holding A+B (?)
    * add konami code for insane white pulse
 */
  
#include "FastLED.h"
#include <NESpad.h>

/* Asserts */

#define __ASSERT_USE_STDERR
#include <assert.h>

// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link. 
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
    // abort program execution.
    abort();
}

// How many leds in your strip?
#define SIDE_PANEL 29 + 25 + 26 + 19
#define FRONT_PANEL 18 + 24 + 18 + 24
#define BACK_PANEL 25 + 25 + 59 + 59
#define NUM_LEDS SIDE_PANEL + SIDE_PANEL + FRONT_PANEL + BACK_PANEL
#define NUM_MODES 7

#define NUM_PANELS 4
#define DATA_PIN 5

// Define the array of leds
CRGB leds[NUM_LEDS];

uint32_t timebase = 0;
byte bpm = 120;


#define TEST_LENGTH 35

class TestPanel
{
  public:
    TestPanel(int, int, byte);
    void drawSwipe();
    void drawStrobe();
    void drawRainbow();
    void drawSparkles();
    void drawChaser();
    void drawForest();
    void drawSolid(CRGB);
    void setFocus(byte);
    bool focus;
  
  private:
    int _start;
    int _end;
    int _length;
    byte _button;
    byte _counter;
};

TestPanel::TestPanel(int s, int e, byte b){
  _start = s;
  _end = e;
  _length = _end - _start;
  _button = b;
  focus = true;

  assert(_start < _end);
  assert(_length > 0);
}

void TestPanel::setFocus(byte state){
  if (state & _button) {
    focus = true;
  }
}

// How far through the beat are we?
byte beatCompletion() {
  long m = millis() - timebase;
  return (long) m * 256 * bpm / 60 / 1000;
}

byte beat() {
  return (long) (millis() - timebase) * bpm / 60 / 1000;
}

void TestPanel::drawSolid(CRGB c){
  fill_solid(leds + _start, _length, c);
}

void TestPanel::drawStrobe(){
  _counter++;

  if (_counter % 4 == 0) {
    fill_solid(leds + _start, _length, CRGB(0,255,0)); // CHSV(hue,255,c));
  }
}

void TestPanel::drawForest(){
   _counter++;

  int i;
  
  for(i=_start;i<_end;i++){
    byte b = sin8((i + _counter) * 256 * 4 / _length);
    leds[i] = CRGB(0, b, 256 - b);
  }
}

void TestPanel::drawSparkles(){
  leds[random(_start, _end)] = CRGB(255,255,255);
  leds[random(_start, _end)] = CRGB(255,255,0);
  leds[random(_start, _end)] = CRGB(255,0,255);
  leds[random(_start, _end)] = CRGB(0,255,255);
}

void TestPanel::drawSwipe(){
  fill_solid(leds + _start, _length * beatCompletion() / 256, CRGB(0,255,0)); // CHSV(hue,255,c));
}

void TestPanel::drawChaser(){
  fill_solid(leds + _start + _length * beatCompletion() / 256, 1, CRGB(255,0,192)); // CHSV(hue,255,c));
}

void TestPanel::drawRainbow(){
  fill_rainbow(leds + _start, _length, beat() * 16, 1);
}

#define NUM_PANELS 4
TestPanel* panels[NUM_PANELS] = { 
    new TestPanel(TEST_LENGTH  * 0, TEST_LENGTH * 1 - 1, NES_LEFT),
    new TestPanel(TEST_LENGTH  * 1, TEST_LENGTH * 2 - 1, NES_UP),
    new TestPanel(TEST_LENGTH  * 2, TEST_LENGTH * 3 - 1, NES_RIGHT),
    new TestPanel(TEST_LENGTH  * 3, TEST_LENGTH * 4 - 1, NES_DOWN)
};

void setup() { 
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(9600);  
  pinMode(13, OUTPUT);
  FastLED.setBrightness(255);
   //  set_max_power_in_volts_and_milliamps(5, 500);

}

// put your own strobe/clock/data pin numbers here -- see the pinout in readme.txt
NESpad nintendo = NESpad(2,3,4);

byte state = 0;
byte lastState = 0;
byte mode = 0;

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
}

CRGB color = CRGB(255,255,255);

bool b;

long firstTap = 0;
long lastTap = 0;
byte numTaps = 0;

#define WAIT_TO_RESET_BPM 2000

void tap() {
  long  t = millis();
  
  if (t - lastTap > WAIT_TO_RESET_BPM) {
    timebase = t;
    firstTap = t;
    lastTap = t;
    numTaps = 0;
    Serial.println("Reset!");
  } else {
    lastTap = t;
    numTaps++;

    long millisPerBeat = (lastTap - firstTap) / numTaps;
    bpm = (long) 60000 / millisPerBeat;
    Serial.println(firstTap, DEC);
    Serial.println(lastTap, DEC);
    Serial.println(numTaps, DEC);
    Serial.println(bpm, DEC);
  }
}

bool onPush(byte button) {
  if (!(lastState & button) && (state & button)){
     return true;
  } else {
    return false;
  }
}

bool anyArrowsPressed() {
  return (state & NES_LEFT) || (state & NES_RIGHT) || (state & NES_UP) || (state & NES_DOWN);
}

void checkButtons() {
  lastState = state;
  state = nintendo.buttons();

  digitalWrite(13, (state & NES_START) ? HIGH : LOW); 
  
  if (onPush(NES_START)){
    tap();
  }

  if (onPush(NES_SELECT)){
    mode++;
  }

  int i;
  
  for (i=0;i<NUM_PANELS;i++){
    panels[i]->focus = false;
  }

  for (i=0;i<NUM_PANELS;i++){
    panels[i]->setFocus(state);
  }

  if (!anyArrowsPressed()) {
    panels[beat() % NUM_PANELS]->focus = true;
  }
}

void loop() { 
  checkButtons();
  
  byte c = beatsin8(bpm, 0, 255, timebase);

  if (c < 192) {
    c = 0;
  }
  
  // byte hue = beatsin8(bpm / 4, 0, 4, timebase) * 63;

  // fill_solid(leds, NUM_LEDS, CRGB(c, 0, 0));
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  int i;
  
  if (mode % NUM_MODES == 0) {
    // Safety mode, red at the back, white at the front
    panels[0]->drawSolid(CRGB(128,0,0));
    panels[1]->drawSolid(CRGB(64,64,64));
  } else {
    for (i=0;i<NUM_PANELS;i++){
      if (panels[i]->focus) {
  
        switch (mode % NUM_MODES) {
          case 1: panels[i]->drawForest(); break;
          case 2: panels[i]->drawRainbow(); break;
          case 3: panels[i]->drawSwipe(); break;
          case 4: panels[i]->drawChaser(); break;
          case 5: panels[i]->drawStrobe(); break;
          case 6: panels[i]->drawSparkles(); break;
        }
      }
    }
  }
    
  FastLED.show();
  delay(1000 / 60);
}


