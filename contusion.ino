#include <SD.h>
#include <TMRpcm.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SD_CARD_CS_PIN 4
#define BUTTON_PIN 2
#define ANALOG_BUTTON_PIN A0
#define AUDIO_PIN_LEFT 9
#define AUDIO_PIN_RIGHT 10

TMRpcm audio;
LiquidCrystal_I2C lcd(0x20, 16, 2); // LCD екран

int bpm = 80;
int noteDuration = 4; // 1/4 by default
int currentBank = 0; // 0 - Bank A, 1 - Bank B
int currentSound = 1; // 1-8
unsigned long lastPlayTime = 0;
bool isPlaying = false;

int buttonValues[6] = {843, 920, 762, 689, 603, 506};

bool buttonHeld = false;
unsigned long buttonPressTime = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ANALOG_BUTTON_PIN, INPUT);
  audio.speakerPin = AUDIO_PIN_RIGHT;
  audio.setVolume(5);
  audio.quality(1);

  if (!SD.begin(SD_CARD_CS_PIN)) {
    lcd.print("SD Card Fail");
    return;
  }

    lcd.init();                       //  Инициируем работу с LCD дисплеем
    lcd.setCursor(0, 0);              //  Устанавливаем курсор в позицию (0 столбец, 0 строка)
  lcd.setBacklight(1);
  lcd.print("CONTUSION");
  delay(1000);
  lcd.clear();
}

void loop() {
  handleButtonPress();
  handleAnalogButtons();
  playSound();
  updateDisplay();
}

void handleButtonPress() {
  static unsigned long lastDebounceTime = 0;
  static int lastButtonState = LOW;
  int buttonState = digitalRead(BUTTON_PIN);
    unsigned long currentTime = millis();

  // Логіка кнопки зміни банку і BPM
  if (buttonState == HIGH && lastButtonState == LOW) {
    buttonPressTime = currentTime;
    buttonHeld = false;
  }

  if (buttonState == HIGH && !buttonHeld && (currentTime - buttonPressTime > 500)) {
    buttonHeld = true;
  }

  if (buttonState == LOW && lastButtonState == HIGH) {
    if (!buttonHeld) {
      currentBank = !currentBank;
      updateDisplay();
    }
    buttonHeld = false;
  }

  if (buttonHeld) {
    bpm = (bpm == 120) ? 80 : bpm + 1;
    updateDisplay();
    delay(500);
  }

  lastButtonState = buttonState;
}

void handleAnalogButtons() {
  int analogValue = analogRead(ANALOG_BUTTON_PIN);

  for (int i = 0; i < 6; i++) {
    if (abs(analogValue - buttonValues[i]) < 10) {
      if (i < 2) {
        // First 2 buttons - change note duration
        noteDuration = (i == 0) ? 4 : 8;
      } else {
        // Next 4 buttons - select sound
        currentSound = (currentBank * 4) + (i - 1);
        isPlaying = true;
      }
      break;
    }
  }
}

void playSound() {
  if (!isPlaying) return;

  unsigned long currentTime = millis();
  unsigned long soundInterval = (60000 / bpm) / noteDuration;

  if (currentTime - lastPlayTime >= soundInterval) {
    lastPlayTime = currentTime;

    char soundFile[10];
    sprintf(soundFile, "%d.wav", currentSound);
    audio.play(soundFile);

    // Додатковий відлік часу для синхронізації
    while (millis() - lastPlayTime < soundInterval) {
      // Чекаємо наступного такту
    }
  }
}

void updateDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("BPM: ");
  lcd.print(bpm);
  lcd.print(" ");
  lcd.print(noteDuration == 4 ? "1/4" : "1/8");

  lcd.setCursor(0, 1);
  lcd.print("Bank: ");
  lcd.print(currentBank == 0 ? "A" : "B");
  lcd.print(" Sound: ");
  lcd.print(currentSound);
}