#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "can.h"
#include "oled.h"

#define SPI_BAUD 8000000 // 8Mbit

#define PIN_SPI_SCK 2
#define PIN_SPI_MOSI 3
#define PIN_SPI_MISO 4

int main() {
  stdio_init_all();

  gpio_set_function(PIN_SPI_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_SPI_MOSI, GPIO_FUNC_SPI);
  gpio_set_function(PIN_SPI_MISO, GPIO_FUNC_SPI);

  spi_init(spi0, SPI_BAUD);
  spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

  oled_init();

  oled_print_line(7, "     SCAN-7     ", 0xf);
  oled_print_line(9, "    v.241124    ", 0x4);

  oled_display();

  can_init();
  can_mode_normal();

  uint16_t cnt = 0;
  while(1) {
    sleep_ms(1000);
    uint8_t status = can_bit_modify(0xf, 0xe0, 0x0);
    uint8_t status2 = can_read_register(0xf);
    printf("%04x status %02x %02x\n", cnt++, status, status2);
  }
}
