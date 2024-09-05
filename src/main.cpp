#include <Arduino.h>

#define StartPin 0
#define ExtendPin 1
#define PausePin 2
#define BuzzerPin 3
#define EnterSetupThreshold 5

const int segmentPins[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
const int digitPins[] = {18, 19};
const int deciamlNum = 2;
const int CountDownInterval = 30;
const int ExtenInterval = 90;
int CurrInterval = CountDownInterval;
int CurrTime = 0;

enum Situation
{
  Pause,
  Running,
  Extend,
};

enum Situation Status = Pause;

const byte digitToSegments[] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};

void handleStartPin()
{
  CurrTime = 0;
  Status = Running;
}

void handleExtendPin()
{
  CurrTime = 0;
  Status = Extend;
}

void handlePausePin()
{
  CurrTime = 0;
  Status = Pause;
}

void alartReachThreshold()
{
  digitalWrite(BuzzerPin, HIGH);
  delay(1500);
  digitalWrite(BuzzerPin, LOW);
}

void lightDigit(int digit, int value)
{
  for (int i = 0; i < deciamlNum; i++)
  {
    digitalWrite(digitPins[i], HIGH);
  }

  digitalWrite(digitPins[digit], LOW);

  for (int i = 0; i < 8; i++)
  {
    digitalWrite(segmentPins[i], (digitToSegments[value] >> i) & 0x01);
  }
}

void displayNumber(int num)
{
  for (int digit = 0; digit < deciamlNum; digit++)
  {
    int digitValue = num % 10;
    num /= 10;

    lightDigit(digit, digitValue);
    delay(5);
  }
}

void showCountDown()
{
  switch (Status)
  {
  case Pause:
    return;
    break;
  case Running:
    CurrInterval = CountDownInterval;
    break;
  case Extend:
    CurrInterval = ExtenInterval;
    break;
  default:
    break;
  }
  displayNumber(CurrInterval - CurrTime);
  delay(1000);
  CurrTime = CurrTime + 1;
  if (CurrInterval - CurrTime == 5)
  {
    alartReachThreshold();
  }
  else if (CurrInterval == CurrTime)
  {
    CurrTime = 0;
    alartReachThreshold();
    Status = Pause;
  }
}

void setup()
{
  pinMode(StartPin, INPUT);
  pinMode(ExtendPin, INPUT);
  pinMode(PausePin, INPUT);
  pinMode(BuzzerPin, OUTPUT);
  for (int i = 0; i < 16; i++)
  {
    pinMode(segmentPins[i], OUTPUT);
  }
  for (int i = 0; i < deciamlNum; i++)
  {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH);
  }
  attachInterrupt(digitalPinToInterrupt(StartPin), handleStartPin, FALLING);
  attachInterrupt(digitalPinToInterrupt(ExtendPin), handleExtendPin, FALLING);
  attachInterrupt(digitalPinToInterrupt(PausePin), handlePausePin, FALLING);
}

void loop()
{
  showCountDown();
}
