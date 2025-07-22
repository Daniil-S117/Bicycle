#include <SPI.h>
#include <Wire.h>
#include "spi_master.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Joystick.h>

Adafruit_SSD1306 display(128, 32, &Wire, -1);


Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   15, 0,                  // Button Count, Hat Switch Count
                   true, true, false,     // X and Y, but no Z Axis
                   false, false, false,   // No Rx, Ry, or Rz
                   false, false,           // No rudder or throttle
                   false, false, false);  // No accelerator, brake, or steering


int16_t X = 0;          // значение потенциометра руля
int16_t XL = 0;         // значение потенциометра левой подрульки
int16_t XR = 0;         // значение потенциометра правой подрульки
int16_t XF = 0;         // значение оси X готовое (руль)
int16_t XLm = 490;      // мертвая зона в лево
int16_t XRm = 500;      // мертвая зона в право
int16_t wheelAngle = 0; // угол поворота колеса

int8_t Y = 0;

// переменные для хранения Часов, Минут, Секунд
uint8_t hours = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;
// создаем объект класса long для хранения счетчика
long previousMillis = 0;        // храним время последнего переключения светодиода
long interval = 1000;           // интервал между включение/выключением светодиода (1 секунда)
long lastTime = 0;

void setup (void)
{
  mater_init();
  // отладка
  Serial.begin (9600);
  Serial.println ();
  //кнопки на руле
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  // встроеный светодиод (если включен - режим настройки)
  pinMode(13, OUTPUT);

  master_arr [0] = 0; // n/a
  master_arr [1] = 0; // n/a
  master_arr [2] = 0; // n/a

  // запускаем библиотеку джойстик (автопосыл выключен)
  Joystick.begin(false);
  Joystick.setYAxisRange(-64, 64);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}


void loop (void)
{
  refreshSPI ();

  unsigned long currentMillis = millis();

  //проверяем не прошел ли нужный интервал, если прошел то
  if (currentMillis - previousMillis > interval) {
    // сохраняем время последнего переключения
    previousMillis = currentMillis;

    //...обновляем  lastTime и добавляем к счетчику Секунд +1
    lastTime = millis();
    seconds++;
    // как только счетчик секунд достигнет 60, обнуляем его и добавляем к счетчику Минут +1...
    if (seconds >= 60) {
      seconds = 0;
      minutes++;
    }
    // ...тоже самое для Часов...
    if (minutes >= 60) {
      minutes = 0;
      hours++;
    }
    // ... и обнуляем счетчик Часов в конце дня
    if (hours >= 24) {
      hours = 0;
    }
  }

  //сброс часов
  if (!digitalRead(7)&&!digitalRead(4)) {
  seconds = 0;
  minutes = 0;
  hours = 0;  
  }


  //режим настройка/игра
  digitalWrite(13, bitRead(slave_arr [1], 7));

  wheelAngle = map(analogRead(A5), 2, 1020, 90, 20);
  wheelAngle = wheelAngle * 10;

  Joystick.setXAxisRange(-wheelAngle / 2, wheelAngle / 2);

  X =  map(analogRead(A0), 0, 1023, 1023, 0);

  XL = map(analogRead(A1), 470, 720, 0, 350);
  if (XL < 0) XL = 0;
  if (XL > 350) XL = 350;

  XR = map(analogRead(A2), 580, 340, 0, 350);
  if (XR < 0) XR = 0;
  if (XR > 350) XR = 350;

  if (X >= XLm && X <= XRm) {
    // мертвая зона
    XF = 0;
  } else if (X > XRm) {
    // рулим вправо
    int16_t x = X - XRm;
    if (x > 300)x = 300;
    XF = x / 3;
  } else if (X < XLm) {
    // рулим влево
    int16_t x = X - XLm;
    if (x < -292) x = -300;
    XF = x / 3;
  }

  XF = XF + XR - XL;

  Y = slave_arr [0];

  // Присваиваем значения кнопок руля
  if (!digitalRead(10)) Joystick.pressButton (0); else Joystick.releaseButton (0);    // rudder BT1
  if (!digitalRead(11)) Joystick.pressButton (1); else Joystick.releaseButton (1);    // rudder BT2
  if (!digitalRead(9))  Joystick.pressButton (2); else Joystick.releaseButton (2);    // rudder BT3
  if (!digitalRead(8))  Joystick.pressButton (3); else Joystick.releaseButton (3);    // rudder BT4
  if (!digitalRead(7))  Joystick.pressButton (4); else Joystick.releaseButton (4);    // rudder BT5
  if (!digitalRead(5))  Joystick.pressButton (5); else Joystick.releaseButton (5);    // rudder BT6
  if (!digitalRead(6))  Joystick.pressButton (6); else Joystick.releaseButton (6);    // rudder BT7
  if (!digitalRead(4))  Joystick.pressButton (7); else Joystick.releaseButton (7);    // rudder BT8

  if (bitRead(slave_arr [1], 0)) Joystick.pressButton (8); else Joystick.releaseButton (8);   // rudder BT9
  if (bitRead(slave_arr [1], 1)) Joystick.pressButton (9); else Joystick.releaseButton (9);   // rudder BT10
  if (bitRead(slave_arr [1], 2)) Joystick.pressButton (10); else Joystick.releaseButton (10); // rudder BT11
  if (bitRead(slave_arr [1], 3)) Joystick.pressButton (11); else Joystick.releaseButton (11); // rudder BT12
  if (bitRead(slave_arr [1], 4)) Joystick.pressButton (12); else Joystick.releaseButton (12); // rudder BT13
  if (bitRead(slave_arr [1], 5)) Joystick.pressButton (13); else Joystick.releaseButton (13); // rudder BT14
  if (bitRead(slave_arr [1], 6)) Joystick.pressButton (14); else Joystick.releaseButton (14); // rudder BT15

  Joystick.setXAxis(XF);
  Joystick.setYAxis(Y);


  Serial.println(XL);
  //Serial.print('=');
  //Serial.println(XF);


  // отображаем значения на дисплее
  display.clearDisplay();
  display.setTextSize(2);                     // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);                    // Start at top-left corner
  display.print(XF);
  display.setCursor(64, 0);                   // Start at top-left corner
  display.print(Y);

  if (!digitalRead(13)) {
    // если светодиод не горит - отображаем параметры настройки
    // угол поворота руля X
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.print (wheelAngle);
    // отображаем калибровку оси Y
    display.setCursor(64, 16);
    // печатаем сырое время
    display.print(slave_arr [2]);
    // печатаем значение потенциометра
    display.setCursor(64, 24);
    display.print(slave_arr [3]);
  } else {
    //светодиод горит - отображаем время проведенное в игре
    display.setTextSize(2);
    display.setCursor(0, 16);

    if (hours < 10) display.print(0);
    display.print(hours);
    display.print(':');

    if (minutes < 10) display.print(0);
    display.print(minutes);
    display.print(':');

    if (seconds < 10) display.print(0);
    display.print(seconds);
  }

  display.display();

  //посылаем данные в джойстик
  Joystick.sendState();
}
