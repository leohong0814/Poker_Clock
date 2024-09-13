#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

#define StartPin 2
#define ExtendPin 3
#define BuzzerPin 13

const int segmentPins[7] = { 11,6,10,8,9,5,7}; // Pins for 7-segment display segments
const int digitPins[] = {12, 4}; // Pins for selecting each digit
const int countdownSeconds = 20;
const int ExtendSeconds = 40;

int currentNumber = countdownSeconds;

TaskHandle_t DisplayTaskHandle;
TaskHandle_t CountdownTaskHandle;
TaskHandle_t BuzzerTaskHandle;

void DisplayTask(void *pvParameters);
void CountdownTask(void *pvParameters);
void BuzzerTask(void *pvParameters);

enum CountDownSituation {
  Pause,
  Running
};
enum BuzzerSituation {
  None,
  Hint,
  Alert,
};
static enum CountDownSituation Status = Pause;
static enum BuzzerSituation Buzzer = None;

// Segment encoding for digits 0-9 (common cathode)
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

void handleStartPin() {
  static unsigned long lastPressTime = 0;
  unsigned long currentTime = millis();
  if(currentTime - lastPressTime > 50)
  {
    if (Status == Pause) {
      currentNumber = countdownSeconds;
      Status = Running;  
    } else {
      Status = Pause;
    }
  }
}

void handleExtendPin() {
  static unsigned long lastPressTime = 0;
  unsigned long currentTime = millis();
  if(currentTime - lastPressTime > 50)
  {
    if(Status == Running)
    {
        currentNumber+=ExtendSeconds;
    }
  }
}

void setup() {
  // Setup pins for 7-segment display
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], LOW);
  }
  for (int i = 0; i < 2; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH);  // Turn off both digits initially
  }
  
  // Setup pins for Start button and Buzzer
  pinMode(StartPin, INPUT_PULLUP);
  pinMode(ExtendPin, INPUT_PULLUP);
  pinMode(BuzzerPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(StartPin), handleStartPin, FALLING);
  attachInterrupt(digitalPinToInterrupt(ExtendPin), handleExtendPin, FALLING);

  // Create FreeRTOS tasks
  xTaskCreate(DisplayTask, "DisplayTask", 256, NULL, 1, &DisplayTaskHandle);
  xTaskCreate(CountdownTask, "CountdownTask", 256, NULL, 1, &CountdownTaskHandle);
  xTaskCreate(BuzzerTask, "BuzzerTask", 128, NULL, 1, &BuzzerTaskHandle);
  
  vTaskStartScheduler();  // Start the FreeRTOS scheduler
}

void lightDigit(int digit, int value) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], (digitToSegments[value] >> i) & 0x01 ? HIGH : LOW);
  }
  digitalWrite(digitPins[digit], LOW);  // Enable the current digit
}

void displayNumber(int num) {
  int tens = num / 10;
  int ones = num % 10;

  // Display the tens digit
  digitalWrite(digitPins[1], HIGH);  // Turn off ones
  lightDigit(0, tens);
  vTaskDelay(5 / portTICK_PERIOD_MS);
  digitalWrite(digitPins[0], HIGH);  // Turn off tens
  
  // Display the ones digit
  lightDigit(1, ones);
  vTaskDelay(5 / portTICK_PERIOD_MS);
  digitalWrite(digitPins[1], HIGH);  // Turn off ones
}

void DisplayTask(void *pvParameters) {
  (void)pvParameters;

  while (true) {
    displayNumber(currentNumber);
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Refresh display every 10ms
  }
}

void CountdownTask(void *pvParameters) {
  (void)pvParameters;

  while (true) {
    if (Status == Running && currentNumber > 0) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 1 second
      currentNumber--;  // Decrease the number every second
      if (currentNumber == 0) {
        Status = Pause;  // Stop countdown when reaching zero
        Buzzer = Alert;
      }
      else if (currentNumber == 5)
      {
        Buzzer = Hint;
      }
    } else {
      vTaskDelay(10 / portTICK_PERIOD_MS);  // Pause mode, minimal delay
    }
  }
}

void BuzzerTask(void *pvParameters) {
  (void)pvParameters;

  while (true) {
    if (Buzzer == Hint) {
      digitalWrite(BuzzerPin, HIGH);  // Buzzer on when countdown finishes
      vTaskDelay(100 / portTICK_PERIOD_MS);
      digitalWrite(BuzzerPin, LOW);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      digitalWrite(BuzzerPin, HIGH);  // Buzzer on when countdown finishes
      vTaskDelay(100 / portTICK_PERIOD_MS);
      digitalWrite(BuzzerPin, LOW);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      Buzzer = None;
    }
    else if (Buzzer == Alert)
    {
      digitalWrite(BuzzerPin, HIGH);  // Buzzer on when countdown finishes
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      digitalWrite(BuzzerPin, LOW);
      Buzzer = None;
    }  
     else {
      vTaskDelay(100 / portTICK_PERIOD_MS);  // Check buzzer every 100ms
    }
  }
}

void loop() {
  // Nothing to do in loop
}
