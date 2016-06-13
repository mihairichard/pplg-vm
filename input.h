#pragma once

#include <stdint.h>

void set_input(uint8_t pad_key);
void clear_input(uint8_t pad_key);
void reset_input();

extern uint8_t pad1_DOWN;
extern uint8_t pad1_UP;
extern uint8_t pad1_LEFT;
extern uint8_t pad1_RIGHT;
extern uint8_t pad1_START;
extern uint8_t pad1_SELECT;
extern uint8_t pad1_A;
extern uint8_t pad1_B;
