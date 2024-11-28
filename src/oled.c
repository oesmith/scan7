#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <string.h>

#include "oled.h"
#include "font.h"

static uint8_t oled_buffer[64 * 128];

void oled_write(uint8_t* data, size_t len, bool is_data) {
  gpio_put(PIN_OLED_CS, false);
  sleep_us(10);
  gpio_put(PIN_OLED_DC, is_data);
  spi_write_blocking(spi0, data, len);
  sleep_us(10);
  gpio_put(PIN_OLED_CS, true);
}

void oled_write_data(uint8_t* data, size_t len) {
  oled_write(data, len, /* is_data= */ true);
}

void oled_write_cmd(uint8_t* data, size_t len) {
  oled_write(data, len, /* is_data= */ false);
}

void oled_reset(void) {
  gpio_put(PIN_OLED_RST, true);
  sleep_us(10);
  gpio_put(PIN_OLED_RST, false);
  sleep_us(10);
  gpio_put(PIN_OLED_RST, true);
  sleep_us(10);

  static uint8_t cmdlist[] = {
    0xab, 0x1, // regulator
  };

  oled_write_cmd(cmdlist, sizeof(cmdlist));

  sleep_ms(100);

  static uint8_t cmdlist2[] = {
    0xaf, // display on
    0x81, 0x2f, // set contrast
    0xa0, 0x51
  };

  oled_write_cmd(cmdlist2, sizeof(cmdlist2));
}

void oled_init(void) {
  gpio_init(PIN_OLED_CS);
  gpio_set_dir(PIN_OLED_CS, true);
  gpio_init(PIN_OLED_DC);
  gpio_set_dir(PIN_OLED_DC, true);
  gpio_init(PIN_OLED_RST);
  gpio_set_dir(PIN_OLED_RST, true);

  gpio_put(PIN_OLED_CS, true);
  gpio_put(PIN_OLED_DC, true);
  gpio_put(PIN_OLED_RST, true);

  oled_reset();

  oled_clear();

  oled_display();
}

void oled_display(void) {
  static uint8_t cmdlist[] = {
    0x15, 0x0, 0x3f, // set cols
    0x75, 0x0, 0x7f, // set rows
  };
  oled_write_cmd(cmdlist, sizeof(cmdlist));
  oled_write_data(oled_buffer, sizeof(oled_buffer));
}

void oled_print_char(int row, int col, char ch, char colour) {
  // Each character sprite is 32 bytes -- 4 bytes per row * 8 bytes.
  const uint8_t* bitmap = font_char(ch);
  for (int r = 0; r < 8; r++) {
    uint8_t *buf = &oled_buffer[((row * 8 + r) * 64) + col * 4];
    for (int c = 0; c < 8; c+=2) {
      *(buf++) =
        (bitmap[r] & (0x80 >> c) ? (colour << 4) : 0)
        | (bitmap[r] & (0x40 >> c) ? colour : 0);
    }
  }
}

void oled_print_line(int row, char* str, char colour) {
  bool end_of_string = false;
  for (int c = 0; c < 16; c++) {
    end_of_string = end_of_string || str[c] == 0;
    oled_print_char(row, c, end_of_string ? ' ' : str[c], colour);
  }
}

void oled_clear() {
  memset(oled_buffer, 0, sizeof(oled_buffer));
}
