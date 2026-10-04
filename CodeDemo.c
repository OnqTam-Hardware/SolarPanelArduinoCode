/*
  Соларен панел на 2 серва SG90 + LCD 16x2 I2C
  ---------------------------------------------
  1. Отива на 90
  2. Връща се в началото (С1: 0, С2: 180)
  3. За 7 минути се завърта до другата страна (С1: 180, С2: 0)
  На екрана: градусите на моторите и оставащото време

  Свързване:
    LCD:      GND -> GND, VCC -> 5V, SDA -> A4, SCL -> A5
    Серво 1:  Кафяв -> GND, Червен -> 5V, Оранжев -> D9
    Серво 2:  Кафяв -> GND, Червен -> 5V, Оранжев -> D10
*/

#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const unsigned long TOTAL_TIME = 7UL * 60UL * 1000UL;   // 7 минути

LiquidCrystal_I2C lcd(0x27, 16, 2);   // ако екранът е празен, пробвай 0x3F
Servo servo1;
Servo servo2;

int angle = 90;   // ъгъл на серво 1 (серво 2 е винаги 180 - angle)

// Слага двете серва огледално
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

// Бавно завъртане на панела
void moveSlow(int target) {
  while (angle != target) {
    angle += (target > angle) ? 1 : -1;
    setPanel(angle);
    showAngles();
    delay(15);
  }
}

void setup() {
  lcd.init();
  lcd.backlight();

  // 1. Отива на 90
  setPanel(90);
  servo1.attach(9);
  servo2.attach(10);
  showAngles();
  showText("Middle - 90");
  delay(2000);

  // 2. Връща се в началото (С1: 0, С2: 180)
  showText("Going to start");
  moveSlow(0);
  delay(1000);

  // 3. За 7 минути до другата страна (С1: 180, С2: 0)
  unsigned long start = millis();
  unsigned long lastLcd = 0;

  while (true) {
    unsigned long elapsed = millis() - start;
    if (elapsed >= TOTAL_TIME) break;

    int target = elapsed * 180UL / TOTAL_TIME;
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

  // Край
  angle = 180;
  setPanel(angle);
  showAngles();
  showText("Done!");
}

void loop() {
  // Стоят на крайна позиция
}
