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
#define NUM_LEDS BACK_PANEL + SIDE_PANEL + SIDE_PANEL + FRONT_PANEL
#define NUM_MODES 13

#define NUM_PANELS 4
#define DATA_PIN 5

#define BEATS_PER_BAR 4

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
    void drawWipeOut();
    void drawWipeIn();
    void drawStrobe();
    void drawRainbow();
    void drawSparkles();
    void drawChaser();
    void drawForest();
    void drawSolid(CRGB);
    void drawPulse();
    void drawRomance();
    void drawStoners();
    void drawDisco();
    void setFocus(byte);
    void setColor(CRGB);
    bool focus;
  
  private:
    int _start;
    int _end;
    int _length;
    byte _button;
    byte _counter;
    CRGB _color;
};

TestPanel::TestPanel(int s, int e, byte b){
  _start = s;
  _end = e;
  _length = _end - _start;
  _button = b;
  _color = CRGB(0,64,128);
  focus = true;

  assert(_start < _end);
  assert(_length > 0);
}

void TestPanel::setColor(CRGB c){
  _color = c;
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
  byte b = beatCompletion();
  if ((b / 8) % 4 == 0) {
    fill_solid(leds + _start, _length, _color); // CHSV(hue,255,c));
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
  leds[random(_start, _end)] = CRGB(255,255,255);
}

void TestPanel::drawSwipe(){
  byte b = (long) _length * beatCompletion() / 256;
  fill_solid(leds + _start, b, _color);
}

void TestPanel::drawWipeOut(){
  byte b = (long) _length * beatCompletion() / 256 / 2;
  fill_solid(leds + _start + b, (_length - b * 2), _color);
}

void TestPanel::drawWipeIn(){
  byte b = (long) _length * (256 - beatCompletion()) / 256 / 2;
  fill_solid(leds + _start + b, (_length - b * 2), _color);
}

void TestPanel::drawChaser(){
  long b = (long) (_length-4) * beatCompletion() / 256;
  
  fill_solid(leds + _start + b, 4, _color);

  b += (_length-4) / 2;
  
  if (b > _length){
    b -= _length;
  }
  
  fill_solid(leds + _start + b, 4, _color);
}

void TestPanel::drawRainbow(){
  fill_rainbow(leds + _start, _length, beat() * 16, 1);
}

void TestPanel::drawPulse(){
  byte i = beatsin8(bpm, 0, 255, timebase);
  CRGB c = _color;
  c.fadeToBlackBy(i);
  // CRGB c = CRGB(i,i,i);
  fill_solid(leds + _start, _length, c);
}

void TestPanel::drawRomance(){
  byte colorIndex = _counter;
  byte brightness = beatsin8(bpm / 8, 32, 92, timebase);
    
   _counter++;

  for( int i = _start; i < _end; i++) {
    leds[i] = ColorFromPalette(LavaColors_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void TestPanel::drawDisco(){
  byte colorIndex = _counter;
  byte brightness = beatsin8(bpm / 8, 32, 92, timebase);
    
   _counter++;

  for( int i = _start; i < _end; i++) {
    leds[i] = ColorFromPalette(ForestColors_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void TestPanel::drawStoners(){
  byte colorIndex = _counter;
  byte brightness = beatsin8(bpm / 8, 32, 92, timebase);
    
   _counter++;

  for( int i = _start; i < _end; i++) {
    leds[i] = ColorFromPalette(PartyColors_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

#define NUM_PANELS 4
TestPanel* panels[NUM_PANELS] = { 
    new TestPanel(0, 59, NES_DOWN),
    new TestPanel(59, 84, NES_LEFT),
    new TestPanel(84, 143, NES_UP),
    new TestPanel(143, 168, NES_RIGHT)
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

bool onRelease(byte button) {
  if ((lastState & button) && !(state & button)){
     return true;
  } else {
    return false;
  }
}


long arrowLastPushed = 0;
bool anyArrowsPressed() {
  return (state & NES_LEFT) || (state & NES_RIGHT) || (state & NES_UP) || (state & NES_DOWN);
}

bool anyArrowsPressedRecently() {
  if (anyArrowsPressed()) {
    arrowLastPushed = millis();
  }

  return (millis() - arrowLastPushed < 500);
}

// konami code
byte code[] = {
  NES_UP, NES_UP, NES_DOWN, NES_DOWN, NES_LEFT, NES_RIGHT, NES_LEFT, NES_RIGHT, NES_B, NES_A
};
byte codeIndex = 0;
long codeEnabledAt = -500000;

void checkButtons() {
  lastState = state;
  state = nintendo.buttons();

  digitalWrite(13, (state & NES_START) ? HIGH : LOW); 

  if (onPush(code[codeIndex])){
    codeIndex++;

    Serial.println(codeIndex, DEC);
    
    if (codeIndex == sizeof(code)){
      codeEnabledAt = millis();
      codeIndex = 0;
    }
  } else {
    if (onPush(NES_LEFT) || onPush(NES_RIGHT) || onPush(NES_A) || onPush(NES_B)){
      codeIndex = 0;
    }
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

  if (!anyArrowsPressedRecently()) {
    long b = beat();

    if (b % 3 == 0){
      panels[0]->focus = true;
    }
    
    if (b % 5 == 0){
      panels[1]->focus = true;
    }
    
    if (b % 7 == 0){
      panels[2]->focus = true;
    }
    
    if (b % 9 == 0){
      panels[3]->focus = true;
    }
    
    panels[b % NUM_PANELS]->focus = true;
  }
}

void setColor(CRGB c){
  for (int i=0;i<NUM_PANELS;i++){
    panels[i]->setColor(c);
  }
}

void onBeatChanged() {
}

void onBarChanged() {
  setColor(CHSV(random(256), 255, 128));
}

long lastBeat = 0;
void loop() { 
  checkButtons();

  long b = beat();
  if (b != lastBeat) {
    onBeatChanged();

    if ((b % BEATS_PER_BAR) == 0){
      onBarChanged();
    }
  }
  lastBeat = b;
  
  byte c = beatsin8(bpm, 0, 255, timebase);

  if (c < 192) {
    c = 0;
  }
  
  // byte hue = beatsin8(bpm / 4, 0, 4, timebase) * 63;

  // fill_solid(leds, NUM_LEDS, CRGB(c, 0, 0));
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  int i;

  if (millis() - codeEnabledAt < 5000){
    byte c = (long) 255 * (5000 - (millis() - codeEnabledAt)) / 5000;
    fill_solid(leds, NUM_LEDS, CRGB(c,c,c));
  } else if (mode % NUM_MODES == 0) {
    // Safety mode, red at the back, white at the front
    panels[0]->drawSolid(CRGB(128,0,0));
    panels[2]->drawSolid(CRGB(64,64,64));
  } else {
    byte m = mode;

    if (onRelease(NES_A) || onRelease(NES_B)){
      onBarChanged();
    }
    
    if (state & NES_A) {
      m += 23;
      setColor(CRGB(64,64,64));
    }
    if (state & NES_B) {
      m += 37;
      setColor(CRGB(255,0,0));
    }
    if ((state & NES_B) && (state & NES_A)) {
      setColor(CRGB(0,64,255));
    }

    m = m % NUM_MODES;

    // dont randomly go to safety mode
    if (m == 0){
      m++;
    }

    for (i=0;i<NUM_PANELS;i++){
      if ((panels[i]->focus) || (m > 9)) {
  
        switch (m) {
          case 1: panels[i]->drawForest(); break;
          case 2: panels[i]->drawRainbow(); break;
          case 3: panels[i]->drawSwipe(); break;
          case 4: panels[i]->drawChaser(); break;
          case 5: panels[i]->drawStrobe(); break;
          case 6: panels[i]->drawSparkles(); break;
          case 7: panels[i]->drawWipeOut(); break;
          case 8: panels[i]->drawWipeIn(); break;
          case 9: panels[i]->drawPulse(); break;
          case 10: panels[i]->drawRomance(); break;
          case 11: panels[i]->drawStoners(); break;
          case 12: panels[i]->drawDisco(); break;
        }
      }
    }
  }
    
  FastLED.show();
  delay(1000 / 60);
}


