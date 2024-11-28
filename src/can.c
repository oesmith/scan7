#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "can.h"

void can_init(void) {
  gpio_init(PIN_CAN_CS);
  gpio_set_dir(PIN_CAN_CS, true);

  gpio_init(PIN_CAN_INT);
  gpio_set_dir(PIN_CAN_INT, false);

  sleep_ms(100);

  gpio_put(PIN_CAN_CS, false);
  sleep_us(1);

  const uint8_t reset_cmd = 0xc0;
  spi_write_blocking(spi0, &reset_cmd, sizeof(reset_cmd));

  gpio_put(PIN_CAN_CS, true);
  sleep_us(1);
}

uint8_t can_read_status(void) {
  gpio_put(PIN_CAN_CS, false);
  sleep_us(1);

  const uint8_t status_cmd = 0xa;
  spi_write_blocking(spi0, &status_cmd, sizeof(status_cmd));

  uint8_t status = 0;
  spi_read_blocking(spi0, 0, &status, sizeof(status));

  gpio_put(PIN_CAN_CS, true);
  sleep_us(1);

  return status;
}

void can_mode_normal(void) {
  can_bit_modify(0xf, 0xe0, 0);
}

void can_set_address(uint16_t addr) {
  static uint8_t data[4];
  data[0] = (uint8_t)((addr & 0x7f8) >> 3);
  data[1] = (uint8_t)((addr & 0x7) >> 5);
  data[2] = 0;
  data[3] = 0;
}

void can_set_extended_address(uint32_t addr) {
  static uint8_t data[4];
  data[0] = (uint8_t)((addr & 0x1fe00000) >> 21);
  data[1] = (uint8_t)(
      ((addr & 0x1c0000) >> 13) |
      ((addr & 0x300) >> 16) |
      0x8 /* EXIDE bit */);
  data[2] = (uint8_t)((addr & 0xff00) >> 8);
  data[3] = (uint8_t)(addr & 0xff);
}

void can_send_packet(uint8_t data[8]) {

}

uint8_t can_read_register(uint8_t addr) {
  gpio_put(PIN_CAN_CS, false);
  sleep_us(1);

  const uint8_t read_cmd[] = { 0x3, addr };
  spi_write_blocking(spi0, read_cmd, sizeof(read_cmd));

  uint8_t reg = 0;
  spi_read_blocking(spi0, 0, &reg, sizeof(reg));

  gpio_put(PIN_CAN_CS, true);
  sleep_us(1);

  return reg;
}

uint8_t can_bit_modify(uint8_t addr, uint8_t mask, uint8_t data) {
  gpio_put(PIN_CAN_CS, false);
  sleep_us(1);

  const uint8_t bitmod_cmd[] = { 0x5, addr, mask, data };
  spi_write_blocking(spi0, bitmod_cmd, sizeof(bitmod_cmd));

  uint8_t reg = 0;
  spi_read_blocking(spi0, 0, &reg, sizeof(reg));
  spi_read_blocking(spi0, 0, &reg, sizeof(reg));

  gpio_put(PIN_CAN_CS, true);
  sleep_us(1);

  return reg;
}
