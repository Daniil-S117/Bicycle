#include "spi_slave.h"

volatile uint8_t filter = 0;           // переменная фильтра
volatile uint32_t fDisk18 = 0;         // оборотов вперед/18
volatile uint32_t bDisk18 = 0;         // оборотов назад /18
volatile uint32_t OLDfDisk18 = 0;      // предыдущее оборотов вперед/18
volatile uint32_t OLDbDisk18 = 0;      // предыдущее оборотов назад /18
volatile uint32_t startTime = 0;       // предыдущее значение
volatile uint32_t endTime = 0;         // время 1/18 оборота
boolean buttonWasUp = true;            // BTSet была ли кнопка отпущена ?

void setup (void)
{
  slave_init();                          // инициализируем SPI как SLAVE
  pinMode (2, INPUT_PULLUP);             // yellow hall
  pinMode (3, INPUT_PULLUP);             // green  hall
  pinMode (4, OUTPUT);                   // yellow LED !!!
  pinMode (5, OUTPUT);                   // green  LED !!!
  // включаем внешнее прерывание на порту D
  bitSet(PCICR, 2);
  // вызов прерывания на пинах 2 и 3
  bitSet(PCMSK2, 2);
  bitSet(PCMSK2, 3);
  slave_arr [0] = 0;                     // ось газ/тормоз int8_t
  slave_arr [1] = 0;                     // кнопки
  slave_arr [2] = 255;                   // сырое значение времени
  slave_arr [3] = 0;                     // регулировка макс скорости вращения для макс Y
  pinMode (14, INPUT_PULLUP); // BT9
  pinMode (15, INPUT_PULLUP); // BT10
  pinMode (16, INPUT_PULLUP); // BT11
  pinMode (17, INPUT_PULLUP); // BT12
  pinMode (9, INPUT_PULLUP);  // BT13
  pinMode (8, INPUT_PULLUP);  // BT14
  pinMode (7, INPUT_PULLUP);  // BT15
  pinMode (6, INPUT_PULLUP);  // BTSet
}


ISR (PCINT2_vect) {                       // Обработчик запросов прерывания от пинов D0..D7

  // В режиме настройки мигаем светодиодами
  if (!bitRead(slave_arr [1], 7)) {
    bitWrite(PORTD, 4, bitRead(PIND, 2));
    bitWrite(PORTD, 5, bitRead(PIND, 3));
  }

  filter = filter << 1;
  bitWrite(filter, 0, bitRead(PIND, 2));
  filter = filter << 1;
  bitWrite(filter, 0, bitRead(PIND, 3));

  // Обнаружено движение вперед 2,3 B10110100
  if (filter == B10110100) {
    // сохраняем старое значение
    OLDfDisk18 = fDisk18;
    // увеличиваем счетчик оборотов вперед
    fDisk18++;
    endTime =  millis() - startTime;
    startTime = millis();
  }

  // Обнаружено движение назад 2,3 B10000111
  if (filter == B10000111) {
    // сохраняем старое значение
    OLDbDisk18 = bDisk18;
    // увеличиваем счетчик оборотов назад
    bDisk18++;
    endTime =  millis() - startTime;
    startTime = millis();
  }

  // маленькая скорость не интересна
  if (endTime>255) endTime = 255;

}

void loop (void)
{
  if (digitalRead (SS) == HIGH) countSPIb = -1;  //сброс в случае глюка связи

  // кнопка настройки 0 настройка / 1 игра
  boolean buttonIsUp = digitalRead(6);
  // если «кнопка была отпущена и (&&) не отпущена сейчас»...
  if (buttonWasUp && !buttonIsUp) {
    delay(10);
    // считываем сигнал снова
    buttonIsUp = digitalRead(6);
    if (!buttonIsUp) {  // если она всё ещё нажата...
      // ...это клик! 0 настройка / 1 игра
      bitWrite(slave_arr [1], 7, !bitRead(slave_arr [1], 7));
    }
  }
  // запоминаем последнее состояние кнопки для новой итерации
  buttonWasUp = buttonIsUp;

  // переписываем состояние кнопок
  bitWrite(slave_arr [1], 0, !digitalRead(14)); // BT9
  bitWrite(slave_arr [1], 1, !digitalRead(15)); // BT10
  bitWrite(slave_arr [1], 2, !digitalRead(16)); // BT11
  bitWrite(slave_arr [1], 3, !digitalRead(17)); // BT12
  bitWrite(slave_arr [1], 4, !digitalRead(9));  // BT13
  bitWrite(slave_arr [1], 5, !digitalRead(8));  // BT14
  bitWrite(slave_arr [1], 6, !digitalRead(7));  // BT15

  // потенциометр регулировки максимальной скорости вращения
  slave_arr [3] = map(analogRead(A7),0,1023,1,254);

  //проверяем не остановились ли педали
  if (millis() - startTime > 255) {
    slave_arr[0] = 0;
    endTime = 255;
  }


  if (OLDfDisk18 != fDisk18) {  // педали крутятся вперед
    OLDfDisk18 = fDisk18;       // обнуляем значение
    if (endTime < slave_arr [3]) slave_arr [0] = 64;
    else slave_arr [0] = map(endTime, 255, slave_arr [3], 0, 64);
  }

  if (OLDbDisk18 != bDisk18) {   // педали крутятся назад
    OLDbDisk18 = bDisk18;        // обнуляем значение   
    if (endTime < slave_arr [3]) slave_arr [0] = -64;
    else slave_arr [0] = map(endTime, 255, slave_arr [3], 0, -64);
  }

  //отдаем сырое значение на дисплей
  slave_arr [2] = endTime;
}

// отладка
// Serial.begin(9600);
//long previousMillis = 0;               // переменные для отладки
//long interval = 100;                   // через порт
/*
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    // сохраняем время последнего переключения
    previousMillis = currentMillis;
    int8_t testik = slave_arr [0];
    Serial.println(slave_arr [3]);
  }
*/
