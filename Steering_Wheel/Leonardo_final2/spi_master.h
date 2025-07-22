// Arduino UNO, NANO, Leonardo
// Он использует интрефейс SPI на пинах
// 13 - SCK, 12 - MISO, 11 - MOSI, 10 - SS
// SCK, MI, MO, SS для Leonardo
// Массивы могут иметь произвольную длинну
// Но должны совпадать по количеству байт
// Не использованные байты можно оставлять пустыми

// Массив корторый отправляем ведомому
uint8_t master_arr [4];
// Массив корторый получаем от ведомого
uint8_t slave_arr  [4];

void mater_init(){
  digitalWrite(SS, HIGH);  // ensure SS stays high for now
  SPI.begin ();
  // замедляем работу мастера
  SPI.setClockDivider(SPI_CLOCK_DIV8);
}

void refreshSPI ()          // процедура обмена массивами между платами
{
  digitalWrite(SS, LOW);    // enable Slave Select
  // отправляем "пустышку" чтобы загрузить первый байт массива slave_arr[]
  SPI.transfer (0xFF);
  // пауза чтобы успел подгрузиться байт на ведомом
  delayMicroseconds (20);
  // цикл по master_arr[]
  for (uint8_t i = 0; i < sizeof(master_arr); i++) {  
  slave_arr[i] = SPI.transfer(master_arr[i]);
  // пауза чтобы успел подгрузиться байт на ведомом
  delayMicroseconds (20);     
  }    
  digitalWrite(SS, HIGH);   // disable Slave Select
} // end of refreshSPI
