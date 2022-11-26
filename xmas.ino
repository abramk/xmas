#define FASTLED_INTERNAL

#include <FastLED.h>
#define NUM_LEDS 300
#define DATA_PIN 29

#define SEQ_SELECT 31
#define SEQ_INPUT  32

CRGB leds[NUM_LEDS * 2];

CLEDController *ctrl;

volatile int currSequence = 1;

void setup() {
    delay(1000);
    ctrl = &FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
    pinMode(SEQ_SELECT, INPUT_PULLUP);
    pinMode(SEQ_INPUT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SEQ_SELECT), seqSelect, FALLING);

    initialize();    
}

void initialize() {
  switch (currSequence) {
      case 0:
        initRainbow();
        break;
      case 1:
        initTrain();
        break;
    }
}

void seqSelect() {
  currSequence = (currSequence + 1) % 2;
  initialize();
}

void loop() {
  switch (currSequence) {
    case 0:
      rainbowLoop();
      break;
    case 1:
      train();
      break;
  }
}


void initRainbow() {
  for (int i = 0; i < NUM_LEDS * 2; i++) {
    leds[i].setHSV(i % 256, 200, 150);
  }
}

int rainbowIdx = 0;
int rainbowBrightness = 255;
void rainbowLoop() {
  ctrl->show(leds + rainbowIdx, NUM_LEDS, rainbowBrightness);
  
  delay(25);
  rainbowIdx = (rainbowIdx + 1) % 256;
  int change = digitalRead(SEQ_INPUT);
  if (change == LOW) {
    rainbowBrightness = rainbowBrightness - 20;
    if (rainbowBrightness < 0) {
      rainbowBrightness = 255;
    }
  }
}


int trainStart = 1; 
int trainEnd = 0;

void initTrain() {
  trainStart = 1;
  trainEnd = 0;  
  memset8(leds, 0, NUM_LEDS * 2 * 3);
}

int trainInc = 1;
int trainColorHue = 0;
CHSV trainColor = CHSV(trainColorHue, 175, 150);
void train() {
  hsv2rgb_rainbow(trainColor, leds[trainStart]);
  leds[trainEnd] = CRGB::Black;
  
  FastLED.show();
  delay(1);
  trainStart+=trainInc;
  trainEnd+=trainInc;
  
  if (trainStart == NUM_LEDS) {
     trainStart = trainEnd - 2;
     hsv2rgb_rainbow(trainColor, leds[trainEnd - 1]);
//     leds[trainEnd - 1] = CRGB::Red;
     trainEnd = NUM_LEDS - 1;
     trainInc = trainInc * -1;
  } else if (trainStart < 0) {
    trainStart = trainEnd + 2;
    hsv2rgb_rainbow(trainColor, leds[trainEnd + 1]);
//    leds[trainEnd + 1] = CRGB::Red;
    trainEnd = 0;
    trainInc = trainInc * -1;
  }
  if (abs(trainStart - trainEnd) >= NUM_LEDS) {
    initTrain();
  }
  int change = digitalRead(SEQ_INPUT);
  if (change == LOW) {
    trainColorHue = (trainColorHue + 1) % 256;
    trainColor = CHSV(trainColorHue, 175, 150);
  }
}
