#define FASTLED_INTERNAL

#include <FastLED.h>
#define NUM_LEDS 300
#define DATA_PIN 29

#define SEQ_SELECT 31
#define SEQ_INPUT  32

CRGB leds[NUM_LEDS * 2];

CLEDController *ctrl;

#define TOTAL_SEQ 10
volatile int currSequence = 9;

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
      case 6:
        initTrain2();
        break;
      case 7:
        initTwinkle();
        break;
      case 8:
        initMover();
        break;
      case 9:
        initDrops();
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
    case 6:
      loopTrain2();
      break;
    case 7:
      loopTwinkle();
      break;
    case 8:
      loopMover();
      break;
    case 9:
      loopDrops();
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
     trainEnd = NUM_LEDS - 1;
     trainInc = trainInc * -1;
  } else if (trainStart < 0) {
    trainStart = trainEnd + 2;
    hsv2rgb_rainbow(trainColor, leds[trainEnd + 1]);
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
    if (phaserFreq <= 0) {
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


int train2Idx = 0;
int train2Length = 0;
int train2Inc = 1;
uint8_t train2Hue1 = random8();
uint8_t train2Hue2 = random8();
void initTrain2() {
  pedalHi = true;
  train2Idx = 0;
  train2Length = 0;
  train2Inc = 1;
  memset8(leds, 0, NUM_LEDS * 2 * 3);
}


void loopTrain2() {
  if (train2Idx == 0) {
    train2Inc = 1;
    leds[NUM_LEDS - 1 - train2Length].setHSV(train2Hue1, 255, 200);
    leds[NUM_LEDS - 1 + train2Length].setHSV(train2Hue2, 255, 200);
    train2Length++;
  } else if (train2Idx == NUM_LEDS) {
    train2Inc = -1;
    leds[NUM_LEDS - 1 - train2Length].setHSV(train2Hue1, 255, 200);
    leds[NUM_LEDS - 1 + train2Length].setHSV(train2Hue2, 255, 200);
    train2Length++;
  }
  ctrl->show(leds + train2Idx, NUM_LEDS, 200);
  delay(5);
  train2Idx+=train2Inc; 
  int change = digitalRead(SEQ_INPUT);
  if (change == LOW && pedalHi) {
    pedalHi = false;
    train2Hue1 = random8();
    train2Hue2 = random8();
  } else if (change == HIGH && !pedalHi) {
    pedalHi = true;
  }
  if (train2Length == NUM_LEDS/2 - 1) {
    initTrain2();
  }
}


uint8_t twinkleHue = 64;
void initTwinkle() {
  twinkleHue = 64;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setHSV(twinkleHue, 255, 150);
  }
}

void loopTwinkle() {
  uint8_t v = random8(20, 150);
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t r = random8();
    if (r % 20 == 3) {
      leds[i].setHSV(twinkleHue, 255, v);  
    }
  }
  ctrl->show(leds, NUM_LEDS, 100);
  delay(200);
  int change = digitalRead(SEQ_INPUT);
  if (change == LOW && pedalHi) {
    pedalHi = false;
    twinkleHue = random8();
  } else if (change == HIGH && !pedalHi) {
    pedalHi = true;
  }
}


int moveIdx = 0;
uint8_t moveV = 150;
void initMover() {
  moveIdx = 0;
  memset8(leds, 0, NUM_LEDS * 2 * 3);
}

void loopMover() {
  leds[moveIdx].setHSV(64, 255, moveV);
  ctrl->show(leds, NUM_LEDS, 200);
  moveIdx++;
  delay(10);
  if (moveIdx == NUM_LEDS) {
    moveIdx = 0;
    if (moveV == 150) {
      moveV = 0;
    } else {
      moveV = 150;
    }
  }
}



int dropData[12][9] = {
  {0, 61, 113, 156, 200, 239, 268, 288, 299},
  {4, 65, 117, 160, 203, 241, 270, 289, 299},
  {10, 70, 120, 163, 206, 244, 271, 290, 299},
  {15, 74, 124, 167, 210, 246, 273, 291, 299},
  {21, 79, 127, 171, 213, 249, 275, 292, 299},
  {25, 83, 131, 174, 216, 251, 276, 293, 299},
  {30, 88, 135, 178, 219, 253, 278, 293, 299},
  {36, 82, 138, 182, 223, 256, 280, 294, 299},
  {41, 96, 142, 185, 226, 258, 281, 295, 299},
  {46, 101, 145, 189, 229, 261, 283, 296, 299},
  {52, 105, 149, 193, 232, 263, 284, 297, 299},
  {56, 110, 153, 196, 236, 266, 286, 298, 299},
};

bool activeDrops[10];
int dropDepth[10];

void initDrops() {
  for (int i = 0; i < 10; i++) {
    activeDrops[i] = false;
    dropDepth[i] = 0;
  }
  memset8(leds, 0, NUM_LEDS * 2 * 3);
}

void loopDrops() {
  uint8_t trig = random8();
  if (trig % 3 == 0) {
    int i = random8(0, 12);
    if (activeDrops[i] == false) {
      activeDrops[i] = true;
      dropDepth[i] = -1;
    }
  }
  for (int i = 0; i < 12; i++) {
    if (activeDrops[i]) {
      dropDepth[i]++;
      if (dropDepth[i] == 18) {
        activeDrops[i] = false;
      }
    }
    if (activeDrops[i]) {
      for (int j = dropDepth[i] - 9; j <= dropDepth[i]; j++) {
        if (j >= 0 && j < 9) {
          leds[dropData[i][8-j]].setHSV(16+(j * 3), 255, 255 - (dropDepth[i] - j) * 255 / 9);
        }
      }
    }
  }
  ctrl->show(leds, NUM_LEDS, 200);
  delay(50);
}
