#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

#define StartPin 2
#define BuzzerPin 4

const int segmentPins[7] = { 7,8,9,10,11,12,13};
const int CountDownInterval = 15;
const int ExtenInterval = 90;
int CurrInterval = CountDownInterval;
int CurrTime = 0;

TaskHandle_t CountDownTaskHandle;

void CountDownTask(void *pvParameters);

enum Situation
{
  Pause,
  Running,
  Extend,
};

static enum Situation Status = Running;

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
  Status = Status==Running?Pause:Running;
}

void lightDigit(int value)
{
  for (int i = 0; i < 7; i++)
  {
    digitalWrite(segmentPins[i], (digitToSegments[value] >> i) & 0x01? HIGH : LOW);
  }
}

void displayNumber(int num)
{
    int digitValue = num % 10;
    lightDigit(digitValue);  
}

void setup()
{
  pinMode(StartPin, INPUT);
  pinMode(BuzzerPin, OUTPUT);
  digitalWrite(BuzzerPin, LOW);

    for(int i =0;i <7;i++)
    {
      pinMode(segmentPins[i], OUTPUT);
      digitalWrite(segmentPins[i], LOW);
    }
  
  attachInterrupt(digitalPinToInterrupt(StartPin), handleStartPin, FALLING);
  xTaskCreate(CountDownTask, "CountDownrTask", 128, NULL, 1, &CountDownTaskHandle);
  vTaskStartScheduler();
}

void loop()
{
}

void CountDownTask(void *pvParameters)
{
  (void)pvParameters;

  while (true)
  {
    switch (Status)
    {
    case Pause:
      CurrTime = 0;
      continue;
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
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    CurrTime = CurrTime + 1;
    if (CurrInterval < CurrTime)
    {
      CurrTime = 0;
      Status = Pause;
    }
  }
}