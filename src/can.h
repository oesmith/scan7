#include "pico/stdlib.h"

#define PIN_CAN_CS 9
#define PIN_CAN_INT 11

void can_init(void);
uint8_t can_read_status(void);
uint8_t can_read_register(uint8_t addr);
void can_mode_normal(void);
void can_set_address(uint16_t addr);
void can_set_extended_address(uint32_t addr);
uint8_t can_bit_modify(uint8_t addr, uint8_t mask, uint8_t data);
