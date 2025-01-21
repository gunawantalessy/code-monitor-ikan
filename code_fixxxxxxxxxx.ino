#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// LCD dan Keypad Setup
LiquidCrystal_I2C lcd(0x3F, 20, 4);

const int relayPin = A0;
const int proximityPins[] = {10, 11, 12, 13}; // Sensor pins
int fishCount = 0;
String manualInput = "";
bool isPaused = false;
bool isManualEntry = false;

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {5, 4, 3, 2};
byte colPins[COLS] = {9, 8, 7, 6};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  lcd.begin();
  lcd.backlight();

  pinMode(relayPin, OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(proximityPins[i], INPUT);
  }

  Serial.begin(9600);

  // Tampilan awal
  lcd.setCursor(3, 0);
  lcd.print("ALAT PENDETEKSI");
  lcd.setCursor(0, 2);
  lcd.print("JUMLAH BENIH IKAN");
  lcd.setCursor(0, 3);
  lcd.print("NEMO/CLOWNFISH");
  delay(2000);
  lcd.clear();

  lcd.setCursor(2, 0);
  lcd.print("DI BALAI PERIKANAN");
  lcd.setCursor(2, 1);
  lcd.print("BUDIDAYA");
  lcd.setCursor(2, 2);
  lcd.print("LAUT WAIHERU");
  delay(2000);
  lcd.clear();

  lcd.setCursor(4, 0);
  lcd.print("Disusun oleh:");
  lcd.setCursor(0, 2);
  lcd.print("1.Kala Talaohu");
  lcd.setCursor(0, 3);
  lcd.print("2.Inaya Ollong");
  delay(2000);
  lcd.clear();
}

void loop() {
  updateFishCount();
  updateLCD();

  char key = keypad.getKey();
  if (key) {
    handleKeypadInput(key);
  }

  controlPump(); // Kendalikan pompa air
}

void updateFishCount() {
  static bool lastState[4] = {LOW, LOW, LOW, LOW};
  static unsigned long lastDebounceTime[4] = {0, 0, 0, 0};
  const unsigned long debounceDelay = 50;

  for (int i = 0; i < 4; i++) {
    bool currentState = digitalRead(proximityPins[i]) == HIGH;

    if (currentState != lastState[i] && currentState == HIGH) {
      if ((millis() - lastDebounceTime[i]) > debounceDelay) {
        fishCount--;
        if (fishCount < 0) fishCount = 0;
        lastDebounceTime[i] = millis();
        updateLCD(); // Update LCD immediately
      }
    }

    lastState[i] = currentState;
  }
}

void updateLCD() {
  static int lastFishCount = -1;
  static bool lastIsPaused = false;

  if (isPaused != lastIsPaused) {
    lcd.clear();
    if (isPaused) {
      lcd.setCursor(0, 0);
      lcd.print("Monitor PAUSED");
    }
    lastIsPaused = isPaused;
  }

  if (!isPaused && fishCount != lastFishCount) {
    lcd.setCursor(2, 0);
    lcd.print("AKUARIUM MONITOR");
    lcd.setCursor(0, 1);
    lcd.print("Jumlah Ikan: ");
    lcd.setCursor(12, 1);
    lcd.print("   ");
    lcd.setCursor(12, 1);
    lcd.print(fishCount);
    lastFishCount = fishCount;
  }
}

void handleKeypadInput(char key) {
  if (isManualEntry) {
    if (key == '#') {
      if (manualInput.length() > 0) {
        fishCount = manualInput.toInt();
        updateLCD();
      }
      isManualEntry = false;
      manualInput = "";
      lcd.setCursor(0, 3);
      lcd.print("               ");
    } else if (key >= '0' && key <= '9') {
      if (manualInput.length() < 5) {
        manualInput += key;
        lcd.setCursor(0, 3);
        lcd.print("Input: ");
        lcd.print(manualInput);
      }
    }
  } else {
    switch (key) {
      case 'A':
        fishCount++;
        updateLCD();
        break;
      case 'B':
        if (fishCount > 0) {
          fishCount--;
          updateLCD();
        }
        break;
      case 'C':
        isPaused = !isPaused;
        updateLCD();
        break;
      case 'D':
        fishCount = 0;
        updateLCD();
        break;
      case '#':
        isManualEntry = true;
        manualInput = "";
        lcd.setCursor(0, 3);
        lcd.print("Input: ");
        break;
      default:
        break;
    }
  }
}

// Fungsi untuk mengontrol pompa air
void controlPump() {
  if (fishCount == 0) {
    digitalWrite(relayPin, LOW); // Matikan pompa
  } else {
    digitalWrite(relayPin, HIGH); // Hidupkan pompa
  }
}

