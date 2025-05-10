#ifndef _LCD_H_
#define _LCD_H_

#include "hardware/i2c.h"

//*************************************************************************************************

#define MAX_LINES      2
#define MAX_CHARS      16

//*************************************************************************************************

void lcd_init(i2c_inst_t* port, uint sda_pin, uint scl_pin);

void lcd_set_cursor(int line, int position);
void lcd_set_string(const char *s);
void lcd_clear(void);

#endif