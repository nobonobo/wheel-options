#include <Joystick.h>
#include <movingAvg.h>

Joystick_ js(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, 8, 1, false,
             false, true, false, false, false, false, true, true, true, false);

class Analog {
  static const int AverageN = 32;

 public:
  Analog(int pin, int tmin, int tmax, int lmin = 0, int lmax = 0);
  int Get();

 private:
  int pin;
  int targetMin;
  int targetMax;
  int limitMin;
  int limitMax;
  int vmin;
  int vmax;
  movingAvg average;
};

Analog ::Analog(int pin, int tmin, int tmax, int lmin = 0, int lmax = 0)
    : pin(pin),
      targetMin(tmin),
      targetMax(tmax),
      limitMin(lmin),
      limitMax(lmax),
      vmin(1023),
      vmax(0),
      average(AverageN) {
  pinMode(pin, INPUT);
  average.begin();
}

int Analog ::Get() {
  int v = this->average.reading(analogRead(this->pin));
  if (v < this->vmin) {
    this->vmin = v;
  }
  if (v > this->vmax) {
    this->vmax = v;
  }
  if (this->vmax - this->vmin < 10) return 0;
  v = map(v, this->vmin, this->vmax, this->targetMin, this->targetMax);
  if (this->limitMin && v < this->limitMin) v = this->limitMin;
  if (this->limitMax && v > this->limitMax) v = this->limitMax;
  return v;
}

Analog *inputs[6] = {
    new Analog(A0, -1, 3, -1, 2), new Analog(A1, -2, 3, -2, 2),
    new Analog(A2, 0, 32767),     new Analog(A3, 0, 32767),
    new Analog(A4, 0, 32767),     new Analog(A5, 0, 32767),
};

void setup() {
  Serial.begin(230400);
  js.setZAxisRange(0, 32767);
  js.setThrottleRange(0, 32767);
  js.setAcceleratorRange(0, 32767);
  js.setBrakeRange(0, 32767);
  js.begin(false);
}

void loop() {
  static int cnt = -1;
  static int prev[6] = {0};
  static int idleBegin = 0;
  static bool sleepMode = false;
  int next[6];
  delay(1);
  for (int i = 0; i < 6; i++) {
    next[i] = inputs[i]->Get();
  }
  cnt++;
  if (cnt % 10 != 0) return;
  bool changed = false;
  bool active = false;
  for (int i = 0; i < 6; i++) {
    changed |= prev[i] != next[i];
    switch (i) {
      case 0:
      case 1:
        active |= next[i] != prev[i];
        break;
      default:
        active |= abs(next[i] - prev[i]) > 1600;
        break;
    }
    prev[i] = next[i];
  }
  int now = millis();
  if (idleBegin == 0) idleBegin = now;
  if (!active) {
    if (!sleepMode && now - idleBegin > 30000) {
      Serial.println("sleep mode");
      sleepMode = true;
      return;
    }
  } else {
    if (sleepMode) {
      sleepMode = false;
      Serial.println("wakeup");
    }
    idleBegin = now;
  }
  if (sleepMode || !changed) return;
  int x = next[0];
  int y = next[1];
  js.setButton(0, false);
  js.setButton(1, false);
  js.setButton(2, false);
  js.setButton(3, false);
  js.setButton(4, false);
  js.setButton(5, false);
  js.setButton(6, false);
  js.setButton(7, false);
  js.setHatSwitch(0, -1);
  if (y == 0) {
    switch (x) {
      case -1:
        js.setHatSwitch(0, 270);
        break;
      case 1:
      case 2:
        js.setHatSwitch(0, 90);
        break;
    }
  }
  switch (x) {
    case -1:
      switch (y) {
        case 2:
          js.setButton(0, true);
          break;
        case -2:
          js.setButton(1, true);
          break;
      }
      break;
    case 0:
      switch (y) {
        case 2:
          js.setButton(2, true);
          break;
        case 1:
          js.setHatSwitch(0, 0);
          break;
        case -1:
          js.setHatSwitch(0, 180);
          break;
        case -2:
          js.setButton(3, true);
          break;
      }
      break;
    case 1:
      switch (y) {
        case 2:
          js.setButton(4, true);
          break;
        case -2:
          js.setButton(5, true);
          break;
      }
      break;
    case 2:
      switch (y) {
        case 2:
          js.setButton(6, true);
          break;
        case -2:
          js.setButton(7, true);
          break;
      }
      break;
  }
  js.setZAxis(next[2]);        // サイド: 48..256
  js.setThrottle(next[3]);     // スロットル: 105..245
  js.setBrake(next[4]);        // ブレーキ: 50..212
  js.setAccelerator(next[5]);  // クラッチ: 540..800
  js.sendState();
}