#define FASTLED_INTERNAL

#include <FastLED.h>
#define NUM_LEDS 300
#define DATA_PIN 29

#define SEQ_SELECT 31
#define SEQ_INPUT  32

CRGB leds[NUM_LEDS * 2];

CLEDController *ctrl;

#define TOTAL_SEQ 6
volatile int currSequence = 5;

void setup() {
    delay(1000);
    ctrl = &FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
    pinMode(SEQ_SELECT, INPUT_PULLUP);
    pinMode(SEQ_INPUT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SEQ_SELECT), seqSelect, RISING);

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
      case 2:
        initShooter();
        break;
      case 3:
        initPhaser();
        break;
      case 4:
        initSine();
        break;
      case 5:
        initCanvas();
        break;
    }
}

void seqSelect() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 500ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 500) {
    currSequence = (currSequence + 1) % TOTAL_SEQ;
    initialize();  
  }
  last_interrupt_time = interrupt_time;
}

void loop() {
  switch (currSequence) {
    case 0:
      loopRainbow();
      break;
    case 1:
      loopTrain();
      break;
    case 2:
      loopShooter();
      break;
    case 3:
      loopPhaser();
      break;
    case 4:
      loopSine();
      break;
    case 5:
      loopCanvas();
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
void loopRainbow() {
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

void loopTrain() {
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



bool pedalHi = true;
int shooterIdx = 0;
void initShooter() {
  shooterIdx = 0;
  pedalHi = true;
  memset8(leds, 0, NUM_LEDS * 2 * 3);
}

void loopShooter() {
  int change = digitalRead(SEQ_INPUT);
  if (change == LOW && pedalHi) {
    pedalHi = false;
    uint8_t hue = random8();
    CHSV color1 = CHSV(hue, 200, 25);
    CHSV color2 = CHSV(hue, 200, 75);
    CHSV color3 = CHSV(hue, 200, 125);
    CHSV color4 = CHSV(hue, 200, 175);
    CHSV color5 = CHSV(hue, 200, 225);
    int pixIdx = shooterIdx + NUM_LEDS - 6;
    hsv2rgb_rainbow(color1, leds[pixIdx++]);
    hsv2rgb_rainbow(color2, leds[pixIdx++]);
    hsv2rgb_rainbow(color3, leds[pixIdx++]);
    hsv2rgb_rainbow(color4, leds[pixIdx++]);
    hsv2rgb_rainbow(color5, leds[pixIdx++]);
  } else if (change == HIGH && !pedalHi) {
    pedalHi = true;
  }
  ctrl->show(leds + shooterIdx, NUM_LEDS, 200);
  delay(5);
  shooterIdx++;
  if (shooterIdx == NUM_LEDS) {
    memcpy8(leds, leds+shooterIdx - 1, NUM_LEDS * 3);
    memset8(leds + NUM_LEDS, 0, NUM_LEDS * 3);
    shooterIdx = 0;
  }
}



int phaserFreq = 100;
int phaserClock = 0;
int phaserIdx = 0;
const int phaserLength = 25;
CHSV phaserColor[phaserLength];

void initPhaser() {
  phaserFreq = 200;
  phaserClock = 0;
  phaserIdx = 0;
  pedalHi = true;
  memset8(leds, 0, NUM_LEDS * 2 * 3);

  for (int i = 0; i < phaserLength; i++) {
    phaserColor[i] = CHSV(140 + (i * 3), 255, 255 - i * 255/phaserLength);
  }
}

void loopPhaser() {
  if (phaserClock == 0) {
    int pixIdx = phaserIdx + NUM_LEDS - 11;
    for (int i = 0; i < phaserLength; i++) {
      hsv2rgb_rainbow(phaserColor[i], leds[pixIdx++]);
    }
  }
  phaserClock = (phaserClock + 1) % phaserFreq;
  ctrl->show(leds + phaserIdx, NUM_LEDS, 255);
  delay(15);
  phaserIdx++;
  if (phaserIdx == NUM_LEDS) {
    memcpy8(leds, leds+phaserIdx - 1, NUM_LEDS * 3);
    memset8(leds + NUM_LEDS, 0, NUM_LEDS * 3);
    phaserIdx = 0;
  }
  int change = digitalRead(SEQ_INPUT);
  if (change == LOW && pedalHi) {
    pedalHi = false;
    phaserFreq = phaserFreq - 10;
    if (phaserFreq < 0) {
      phaserFreq = 200;
    }
  } else if (change == HIGH && !pedalHi) {
    pedalHi = true;
  }
}


int sineIdx = 0;
void initSine() {
  sineIdx = 0;
  setSineColor(random8());
}

void setSineColor(uint8_t hue) {
  uint8_t theta = 0;
  for (int i = 0; i < NUM_LEDS * 2; i++) {
    leds[i].setHSV(hue, 255, sin8(theta));
    theta+=2;
  }
}

void loopSine() {
  ctrl->show(leds + sineIdx, NUM_LEDS, 200);
  delay(20);
  sineIdx++;
  if (sineIdx == 255) {
    sineIdx = 0;
  }
  int change = digitalRead(SEQ_INPUT);
  if (change == LOW && pedalHi) {
    pedalHi = false;
    setSineColor(random8());
  } else if (change == HIGH && !pedalHi) {
    pedalHi = true;
  }
}

uint8_t canvasMoving[600];
uint8_t canvasStatic[300]; 
int canvasIdx = 0;
void initCanvas() {
  pedalHi = true;
  canvasIdx = 0;
  memset8(leds, 0, NUM_LEDS * 2 * 3);
  memset8(canvasMoving, 0, 600);
  memset8(canvasStatic, 0, 300);
  delay(1000);  
}

void loopCanvas() {
  int change = digitalRead(SEQ_INPUT);
  if (change == LOW && pedalHi) {
    pedalHi = false;
    canvasIdx = 0;
    int idx = NUM_LEDS - 1;
    canvasMoving[idx--] = 0xFF;
    canvasMoving[idx--] = 0xFF;
    canvasMoving[idx--] = 0xFF;
  } else if (change == HIGH && !pedalHi) {
    pedalHi = true;
    for (int i = 0; i < NUM_LEDS; i++) {
      canvasStatic[i] = canvasStatic[i] | canvasMoving[i + canvasIdx];
    }
  }
  if (!pedalHi) {
    canvasIdx++;
  }
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(200 & (canvasStatic[i] | canvasMoving[i + canvasIdx]), 0, 0);
  }
  ctrl->show(leds, NUM_LEDS, 150);
  delay(20);
}
