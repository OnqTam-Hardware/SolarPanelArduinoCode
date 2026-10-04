/*
  Соларен панел - 2 серва SG90 + LCD 16x2 I2C
  -------------------------------------------
  1. Двата мотора отиват на 90
  2. М1 отива на 180, М2 на 0
  3. За 6 минути: М1 180 -> 0, М2 0 -> 180 (синхронно)
  4. Двата мотора плавно на 90

  Свързване:
    LCD:      GND -> GND, VCC -> 5V, SDA -> A4, SCL -> A5
    Мотор 1:  Кафяв -> GND, Червен -> 5V, Оранжев -> D9
    Мотор 2:  Кафяв -> GND, Червен -> 5V, Оранжев -> D10
*/

#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Пинове ---
const int SERVO1_PIN = 9;
const int SERVO2_PIN = 10;

// --- Настройки ---
const unsigned long TOTAL_TIME = 6UL * 60UL * 1000UL;  // 6 минути
const int SPEED = 15;    // мс на градус при преместване (по-голямо = по-бавно)

LiquidCrystal_I2C lcd(0x27, 16, 2);   // ако екранът е празен, пробвай 0x3F
Servo servo1;
Servo servo2;

int angle = 90;   // ъгъл на мотор 1 (мотор 2 е винаги 180 - angle)

// ---------- Мотори ----------

// Слага двата мотора огледално: М1 = a, М2 = 180 - a
void setPanel(int a) {
  servo1.write(a);
  servo2.write(180 - a);
}

// Ред 1 на екрана: градусите на моторите
void showAngles() {
  char line[17];
  snprintf(line, sizeof(line), "M1:%3d   M2:%3d ", angle, 180 - angle);
  lcd.setCursor(0, 0);
  lcd.print(line);
}

// Плавно преместване на двата мотора заедно
void moveSlow(int target) {
  while (angle != target) {
    angle += (target > angle) ? 1 : -1;
    setPanel(angle);
    showAngles();
    delay(SPEED);
  }
}

// ---------- Екран ----------

// Ред 2 на екрана: текст
void showText(const char* text) {
  char line[17];
  snprintf(line, sizeof(line), "%-16s", text);
  lcd.setCursor(0, 1);
  lcd.print(line);
}

// Ред 2 на екрана: оставащо време
void showTimeLeft(unsigned long msLeft) {
  unsigned long s = (msLeft + 999) / 1000;
  char line[17];
  snprintf(line, sizeof(line), "Time left: %lu:%02lu   ", s / 60, s % 60);
  lcd.setCursor(0, 1);
  lcd.print(line);
}

// ---------- Програма ----------

void setup() {
  lcd.init();
  lcd.backlight();

  // 1. Двата мотора на 90
  angle = 90;
  setPanel(angle);
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  showAngles();
  showText("Middle - 90");
  delay(2000);

  // 2. М1 на 180, М2 на 0
  showText("Going to start");
  moveSlow(180);
  delay(1000);

  // 3. 6 минути: М1 180 -> 0, М2 0 -> 180
  unsigned long start = millis();
  unsigned long lastLcd = 0;

  while (true) {
    unsigned long elapsed = millis() - start;
    if (elapsed >= TOTAL_TIME) break;

    int target = 180 - (int)(elapsed * 180UL / TOTAL_TIME);
    if (target != angle) {
      angle = target;
      setPanel(angle);
    }

    if (millis() - lastLcd >= 250) {
      lastLcd = millis();
      showAngles();
      showTimeLeft(TOTAL_TIME - elapsed);
    }
  }
  angle = 0;
  setPanel(angle);
  showAngles();
  delay(1000);

  // 4. Двата мотора плавно на 90
  showText("Back to 90");
  moveSlow(90);

  // Край
  showText("Done!");
}

void loop() {
  // Край - моторите стоят на 90
}
