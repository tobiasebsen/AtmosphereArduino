//#define PIN_RESET       8 // Uncomment this to use reset

void ethernetInit() {

#ifdef PIN_RESET
  pinMode(PIN_RESET, OUTPUT);
  digitalWrite(PIN_RESET, LOW); delayMicroseconds(2);
  digitalWrite(PIN_RESET, HIGH); delay(150);
#endif

  // Speed up SPI
  SPI.setClockDivider(SPI_CLOCK_DIV2);
}

void ethernetMaximize() {
  // Set memory sizes
#if defined(W5100_ETHERNET_SHIELD)
  uint16_t sizes[4] = {(4<<10),(4<<10),0,0};
  W5100.setRXMemorySizes(sizes);
#elif defined(W5200_ETHERNET_SHIELD)
  uint16_t sizes[8] = {(8<<10),(8<<10),0,0,0,0,0,0};
  W5100.setRXMemorySizes(sizes);
#elif defined(W5500_ETHERNET_SHIELD)
  uint16_t sizes[8] = {(8<<10),(8<<10),0,0,0,0,0,0};
  W5100.setRXMemorySizes(sizes);
#endif
}
