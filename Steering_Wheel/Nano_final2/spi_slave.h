// Данный код будет работать только на платах с чипом ATmega328P например
// Arduino UNO, NANO
// Он использует интрефейс SPI на пинах
// 13 - SCK, 12 - MISO, 11 - MOSI, 10 - SS

// Массивы могут иметь произвольную длинну
// Но должны совпадать по количеству байт
// Не использованные байты можно оставлять пустыми

// Массив корторый приходит от мастера
volatile uint8_t master_arr [4];
// Массив который отдаем от мастеру
volatile uint8_t slave_arr  [4];
// счетчик входящих байт
volatile int16_t countSPIb = -1;

void slave_init(){
  // Настройка SPI как SLAVE
  DDRB|=(1<<PB4);                        // Настроить вывод MISO на выход
  SPCR |= (1 << SPIE)|(1 << SPE);        // Прерывание включено и сам SPI как SLAVE
}

ISR (SPI_STC_vect)                       // Прерывание SPI - пришел байт
{
  if (countSPIb < 0) {                   // пришла "пустышка"
    countSPIb++;                         // увеличивам счетчик
    SPDR = slave_arr [countSPIb];        // подгружаем нулевой байт массива ведомого
    return;                              // выходим из процедуры
  }

  master_arr [countSPIb] = SPDR;         // получаем байт от мастера
  countSPIb++;                           // увеличиваем счетчик
  SPDR = slave_arr [countSPIb];          // отдаем байт ведомого (+1 индекс)

  if (countSPIb >= sizeof(master_arr)) { // если кончился массив
    countSPIb = -1;                      // обнуляем счетчик и ждем следующий обмен
  }
}                                        // Конец SPI - пришел байт
