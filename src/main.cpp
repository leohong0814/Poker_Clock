#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

#define StartPin 2
#define ExtendPin 3
#define BuzzerPin 18

const int segmentPins[2][7] = {{4,5,6,7,8,9, 10},{ 11,12,13,14,15,16,17}};
const int CountDownInterval = 25;
const int ExtenInterval = 65;
const int decimalNum = 2;
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
  if(Status==Running || Status == Extend)
  {
    Status = Pause;
  }
  else
  {
    Status = Running;
  }  
}

void handleExtendPin()
{
  if(Status == Running)
  {
    Status = Extend;
  }
  else if (Status == Extend)
  {
    Status = Pause;
  }
}

void lightDigit(int digit,int value)
{
  for (int i = 0; i < 7; i++)
  {
    digitalWrite(segmentPins[digit][i], (digitToSegments[value] >> i) & 0x01? HIGH : LOW);
  }
}

void displayNumber(int num)
{
  for(int digit =0; digit<decimalNum;digit++)
  {
    int vlaue = num % 10;
    num/=10;
    lightDigit(digit, vlaue);   
  }
      
}

void setup()
{
  pinMode(StartPin, INPUT);
  pinMode(ExtendPin, INPUT);
  pinMode(BuzzerPin, OUTPUT);

  for(int digit = 0;digit<decimalNum;digit++)
  {
      for(int i =0;i <7;i++)
      {
        pinMode(segmentPins[digit][i], OUTPUT);
        digitalWrite(segmentPins[digit][i], LOW);
      }
  }
  
  attachInterrupt(digitalPinToInterrupt(StartPin), handleStartPin, FALLING);
  attachInterrupt(digitalPinToInterrupt(ExtendPin), handleExtendPin, FALLING);
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
      CurrInterval = CountDownInterval;
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