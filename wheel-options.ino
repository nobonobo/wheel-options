#include <movingAvg.h>

class Analog {
  static const int AverageN = 8;

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
    new Analog(A0, -1, 3, -1, 2), new Analog(A1, -1, 2, -1, 1),
    new Analog(A2, 0, 32767),     new Analog(A3, 0, 32767),
    new Analog(A4, 0, 32767),     new Analog(A5, 0, 32767),
};

void setup() {
  //
  Serial.begin(57600);
}

int cnt = 0;

void loop() {
  delay(1);
  cnt++;
  if (cnt % 20 == 0) {
    Serial.print(inputs[0]->Get());  // シフト左..右: 619..777
    Serial.print(",");
    Serial.print(inputs[1]->Get());  // シフト手前..奥: 450..610
    Serial.print(",");
    Serial.print(inputs[2]->Get());  // サイド: 48..256
    Serial.print(",");
    Serial.print(inputs[3]->Get());  // スロットル: 105..245
    Serial.print(",");
    Serial.print(inputs[4]->Get());  // ブレーキ: 50..212
    Serial.print(",");
    Serial.print(inputs[5]->Get());  // クラッチ: 540..800
    Serial.println();
  } else {
    inputs[0]->Get();
    inputs[1]->Get();
    inputs[2]->Get();
    inputs[3]->Get();
    inputs[4]->Get();
    inputs[5]->Get();
  }
}
