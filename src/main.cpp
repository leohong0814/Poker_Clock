#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

#define StartPin 0
#define ExtendPin 1
#define PausePin 2
#define BuzzerPin 3

const int segmentPins[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
const int digitPins[] = {18, 19};
const int deciamlNum = 2;
const int CountDownInterval = 30;
const int ExtenInterval = 90;
int CurrInterval = CountDownInterval;
int CurrTime = 0;

TaskHandle_t CountDownTaskHandle;
TaskHandle_t BuzzerTaskHandlel;

void CountDownTask(void *pvParameters);
void BuzzerTask(void *pvParameters);

enum Situation
{
  Pause,
  Running,
  Extend,
};
enum BuzzerType
{
  None,
  Hint,
  Alert,
};
enum Situation Status = Pause;
enum BuzzerType Buzzer = None;

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
  Buzzer = None;
}

void handleExtendPin()
{
  Status = Extend;
  Buzzer = None;
}

void handlePausePin()
{
  Status = Pause;
  Buzzer = None;
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
  xTaskCreate(CountDownTask, "CountDownrTask", 128, NULL, 1, &CountDownTaskHandle);
  xTaskCreate(BuzzerTask, "BuzzerTask", 128, NULL, 1, &BuzzerTaskHandlel);
  vTaskStartScheduler();
}

void loop()
{
  
}

void CountDownTask(void *pvParameters) {
  (void) pvParameters;

  while (true)
  {
    switch (Status)
    {
      case Pause:
      CurrTime = 0;
      Buzzer = None;
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
    if (CurrInterval - CurrTime == 5)
    {
      Buzzer = Hint;
    }
    else if (CurrInterval == CurrTime)
    {
      CurrTime = 0;
      Buzzer = Alert;
      Status = Pause;
    }
  }
}

void BuzzerTask(void *pvParameters) {
  (void) pvParameters;

  while (true)
  {
    switch (Buzzer)
    {
    case None:
    default:   
      break;
    case Hint:
      digitalWrite(BuzzerPin, HIGH);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      digitalWrite(BuzzerPin, LOW);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      digitalWrite(BuzzerPin, HIGH);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      digitalWrite(BuzzerPin, LOW);
      Buzzer = None;
      break;
    case Alert:
      digitalWrite(BuzzerPin, HIGH);
      vTaskDelay(2500 / portTICK_PERIOD_MS); 
      Buzzer = None;
      break;
    } 
  }
}
