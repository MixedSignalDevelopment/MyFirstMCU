#ifndef LCD_MSD_H
#define LCD_MSD_H

#include <stdint.h>
#include <xc.h>

// Function Prototypes
void LCD_Write(uint8_t character, uint8_t RS);
void LCD_Clear(void);
void LCD_Text(const char *s);
void LCD_Char(char);
void LCD_Goto(uint8_t row, uint8_t pos);
void LCD_Init(void);
void LCD_Int(uint32_t value, uint8_t positions, uint8_t zeros);
void LCD_Hex(uint16_t value);
void LCD_Bin(uint8_t value);
void LCD_Float(float value, uint8_t positions, uint8_t decimal_pos);

#endif