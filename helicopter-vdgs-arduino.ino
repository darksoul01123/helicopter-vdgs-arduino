/*
  Helicopter VDGS & Ultrasonic Proximity System (Arduino Uno R3)
  Built & calibrated from live hardware testing on COM14.

  Verified Hardware Pinout:
   - HC-SR04 Ultrasonic : TRIG = D9, ECHO = D10
   - Green LED          : D5  (via 330 ohm resistor to GND)
   - Yellow LED         : D6  (via 330 ohm resistor to GND)
   - Red LED            : D7  (via 330 ohm resistor to GND)
   - Buzzer             : D8  (+ to D8, - to GND)
   - Model Button       : D2  (to GND, uses INPUT_PULLUP)
   - 0.96" I2C OLED     : SDA = A4, SCL = A5 (auto-detects 0x3C / 0x3D)

  Verified Distance Behavior:
   - Above 100 cm (or no echo) : Green LED ON, Buzzer OFF  -> SAFE
   - 51 - 100 cm               : Yellow LED ON, Buzzer OFF -> CAUTION
   - 21 - 50 cm                : Yellow LED ON + Slow Beep -> WARNING!
   - 20 cm or less             : Red LED ON + Fast Alarm   -> DANGER!
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Verified Pins ----------
const int TRIG_PIN   = 9;
const int ECHO_PIN   = 10;
const int GREEN_LED  = 5;
const int YELLOW_LED = 6;
const int RED_LED    = 7;
const int BUZZER     = 8;
const int BTN_PIN    = 2;

// ---------- Buzzer Mode ----------
// true = active buzzer (digitalWrite HIGH/LOW), false = passive buzzer (tone)
const bool BUZZER_ACTIVE = true;

// ---------- Distance Thresholds (cm) ----------
const int DIST_GREEN_MIN  = 101; // > 100 cm
const int DIST_YELLOW_MIN = 51;  // 51 - 100 cm
const int DIST_WARN_MIN   = 21;  // 21 - 50 cm
const int BAR_MAX_CM      = 120; // OLED progress bar scale (0..120 cm)

// ---------- Helicopter Models (cycle with D2 button) ----------
const char* models[] = {"Bell 407", "Airbus H125", "ALH Dhruv", "Mi-17"};
const int MODEL_COUNT = sizeof(models) / sizeof(models[0]);
int currentModel = 0;

// ---------- State & Timing ----------
bool oledAvailable = false;
bool lastBtn = HIGH;
unsigned long lastDebounce = 0;
unsigned long lastBeep = 0;
bool buzzerState = false;
unsigned long lastSerialLog = 0;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER, LOW);

  Wire.begin();
  Wire.setClock(400000);

  // Auto-detect OLED at 0x3C or 0x3D; continue running LEDs/buzzer even if OLED is unplugged
  oledAvailable = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (!oledAvailable) {
    oledAvailable = display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  }

  if (oledAvailable) {
    Serial.println(F("OLED: Ready"));
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 14);
    display.println(F("Helicopter VDGS"));
    display.setCursor(10, 32);
    display.println(F("System Ready"));
    display.display();
    delay(1200);
  } else {
    Serial.println(F("OLED: Not detected (running LEDs + Buzzer + Serial)"));
  }
}

// Median of 5 ultrasonic readings in cm (filters out stray spikes)
long readDistanceCm() {
  long readings[5];
  for (int i = 0; i < 5; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
    readings[i] = (duration == 0) ? 999 : (long)(duration * 0.0343f / 2.0f);
    delay(10);
  }

  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (readings[j] < readings[i]) {
        long t = readings[i];
        readings[i] = readings[j];
        readings[j] = t;
      }
    }
  }
  return readings[2];
}

void setLeds(bool g, bool y, bool r) {
  digitalWrite(GREEN_LED, g ? HIGH : LOW);
  digitalWrite(YELLOW_LED, y ? HIGH : LOW);
  digitalWrite(RED_LED, r ? HIGH : LOW);
}

void stopBuzzer() {
  buzzerState = false;
  if (BUZZER_ACTIVE) {
    digitalWrite(BUZZER, LOW);
  } else {
    noTone(BUZZER);
  }
}

// intervalMs: 0 = off, >0 = toggle every intervalMs
void handleBuzzer(unsigned long intervalMs, uint16_t freqHz) {
  if (intervalMs == 0) {
    if (buzzerState) stopBuzzer();
    return;
  }

  unsigned long now = millis();
  if (now - lastBeep >= intervalMs) {
    lastBeep = now;
    buzzerState = !buzzerState;
    if (BUZZER_ACTIVE) {
      digitalWrite(BUZZER, buzzerState ? HIGH : LOW);
    } else {
      if (buzzerState) tone(BUZZER, freqHz);
      else noTone(BUZZER);
    }
  }
}

void checkButton() {
  bool reading = digitalRead(BTN_PIN);
  if (reading != lastBtn && (millis() - lastDebounce) > 200) {
    lastDebounce = millis();
    if (reading == LOW) {
      currentModel = (currentModel + 1) % MODEL_COUNT;
    }
  }
  lastBtn = reading;
}

void drawScreen(const char* status, const char* ledText, long dist) {
  if (!oledAvailable) return;

  display.clearDisplay();

  // Top bar: helicopter model + active indicator
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(models[currentModel]);
  display.setCursor(82, 0);
  display.print(ledText);
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Main status
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print(status);

  // Distance
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print(F("Dist: "));
  if (dist >= 999) display.print(F(">400"));
  else display.print(dist);
  display.print(F(" cm"));

  // Proximity bar (0..120 cm)
  display.drawRect(0, 56, 128, 8, SSD1306_WHITE);
  if (dist < 999) {
    int clamped = constrain((int)dist, 0, BAR_MAX_CM);
    int fill = map(clamped, BAR_MAX_CM, 0, 0, 124);
    display.fillRect(2, 58, fill, 4, SSD1306_WHITE);
  }

  display.display();
}

void loop() {
  checkButton();

  long dist = readDistanceCm();
  const char* status;
  const char* ledText;

  if (dist >= DIST_GREEN_MIN) {
    setLeds(true, false, false);
    handleBuzzer(0, 0);
    status = "SAFE";
    ledText = "GRN";
  } else if (dist >= DIST_YELLOW_MIN) {
    setLeds(false, true, false);
    handleBuzzer(0, 0);
    status = "CAUTION";
    ledText = "YLW";
  } else if (dist >= DIST_WARN_MIN) {
    setLeds(false, true, false);
    handleBuzzer(400, 1000);
    status = "WARNING!";
    ledText = "YLW+BUZ";
  } else {
    setLeds(false, false, true);
    handleBuzzer(100, 2000);
    status = "DANGER!";
    ledText = "RED+BUZ";
  }

  drawScreen(status, ledText, dist);

  if (millis() - lastSerialLog >= 250) {
    lastSerialLog = millis();
    Serial.print(models[currentModel]);
    Serial.print(F(" | Dist: "));
    if (dist >= 999) {
      Serial.print(F("Out of range"));
    } else {
      Serial.print(dist);
      Serial.print(F(" cm"));
    }
    Serial.print(F(" | "));
    Serial.print(status);
    Serial.print(F(" ("));
    Serial.print(ledText);
    Serial.println(F(")"));
  }
}
