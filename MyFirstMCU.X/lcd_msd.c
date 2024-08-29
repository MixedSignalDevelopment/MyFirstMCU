// Includes
#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include <pic16f886.h>
#include <builtins.h>
#include "lcd_msd.h"

// Defines
#define _XTAL_FREQ          4000000                     // MCU Clock frequency

#define LCD_EN              PORTCbits.RC1               // Define Enable
#define LCD_EN_DIR          TRISCbits.TRISC1

#define LCD_RS              PORTBbits.RB5               // Define Register Select

#define LCD_RW              PORTCbits.RC3
#define LCD_RW_DIR          TRISCbits.TRISC3


#define LCD_DATA            PORTB                       // Define Data Register
#define LCD_DIR             TRISB                       // Define Direction Register

#define LCD_Enable()        ((LCD_EN=1),(LCD_EN=0))

// Local function prototypes

// Functions

uint32_t bit_config(uint32_t word, uint32_t bit_number, uint32_t bit_value)
{
    uint32_t word_temp = word;
    word_temp ^= (-bit_value ^ word_temp) & (1 << bit_number);
    return word_temp;
}

// Write one character to the LCD

void LCD_Write(uint8_t character, uint8_t RS)
{
    uint8_t data = LCD_DATA;

    data = ((character >> 4) & 0x0F); // Shift upper 4 bits
    LCD_DATA = bit_config(data, 5, RS);

    LCD_Enable(); // Write in LCD

    data = (character & 0x0F); // lower 4 bits
    LCD_DATA = bit_config(data, 5, RS);

    LCD_Enable(); // Write in LCD
}

// Errase LCD

void LCD_Clear(void)
{
    LCD_Write(0x01, 0); // Errase LCD's RAM and set the cursor
    // to position 0x00
    __delay_ms(2); // Wait for execution of the command
}

// Write a text into LCD

void LCD_Text(const char *s)
{
    while (*s)
    {
        LCD_Write(*s++, 1);
    }
}

// Write a single character to LCD

void LCD_Char(char c)
{
    LCD_Write(c, 1); // Sendet einzelnes Zeichen an LCD
}

// Set cursor position

void LCD_Goto(uint8_t row, uint8_t pos)
{
    uint8_t data;

    switch (row)
    {
    case 0: // 1. Row
        data = 0x80 + 0x00 + pos;
        break;

    case 1: // 2. Row
        data = 0x80 + 0x40 + pos;
        break;

    case 2: // 3. Row
        data = 0x80 + 0x10 + pos;
        break;

    case 3: // 4. Row
        data = 0x80 + 0x50 + pos;
        break;
    }
    LCD_Write(data, 0);
}

// Initializes the LCD Display. Call this function before any other LCD functions!

void LCD_Init()
{
    LCD_DIR = LCD_DIR & 0b11010000u;
    LCD_EN_DIR = 0;
    LCD_RW_DIR = 0;
    LCD_RW = 0;

    LCD_DATA = 0; // Reset Data bits
    LCD_EN = 0; // Reset EN pin

    __delay_ms(100);
    LCD_DATA = 0b00000011u; // Reset 3 times
    LCD_Enable();
    __delay_ms(5);
    LCD_Enable();
    __delay_ms(1);
    LCD_Enable();
    __delay_ms(1);

    LCD_DATA = 0b00000010u; // Enable 4 bit mode
    LCD_Enable();

    __delay_us(100);
    LCD_Write(0b00101000, 0);
    __delay_us(100);
    LCD_Clear();
    __delay_ms(4);
    LCD_Write(0b00000110, 0);
    __delay_us(100);
    LCD_Write(0b00001100, 0);
    __delay_us(100);
}

// Sends a long integer value to the LCD,
// value:       float value
// positions:   Number of positions to show (1-8 positions)
// zeros:       0 = zeros are not suppressed
//              1 = zeros are suppressed

void LCD_Int(uint32_t value, uint8_t positions, uint8_t zeros)
{
    uint16_t i; // Counter
    uint8_t buf[8]; // char-String f?r Ergebnis der Umwandlung

    for (i = 0; i < positions; i++) // n-stellige LongInt-number verarbeiten
    {
        buf[positions - 1 - i] = (value % 10) + 0x30; // Eine Stelle in char umwandeln
        value = value / 10; // value um eine Stelle reduzieren
    }

    for (i = 0; i < positions - 1; i++) // F?hrende Nullen entfernen
    {
        if (zeros && (buf[i] == 0x30)) // Test ob buf[i] = 0
            buf[i] = 0x20; // char[i] durch Leerschlag ersetzen
        else
            zeros = 0; // sobald erste Ziffer gefunden, Flag l?schen
    }

    for (i = 0; i < positions; i++) // 5-stellige Integer-number auf Display ausgeben
        LCD_Write(buf[i], 1); // einzelne Ziffer ausgeben
}

// Writes an unsigned integer value to LCD and displays it in Hex

void LCD_Hex(uint16_t value)
{
    uint16_t i; // Counter
    uint8_t buf[4]; // char-String f?r Ergebnis der Umwandlung

    for (i = 0; i < 4; i++) // Int-number verarbeiten
    {
        buf[3 - i] = (value & 0x000F); // unterstes Nibble isolieren (1 HEX-Stelle)
        if (buf[3 - i] <= 9) // Test, ob Ziffer 0-9 ist
            buf[3 - i] = buf[3 - i] + 0x30; // Ziffern 0-9 in Char umwandeln
        else
            buf[3 - i] = buf[3 - i] + 0x37; // Ziffern A-F in Char umwandeln
        value = value >> 4; // value um ein Nibble (1 HEX-Stelle) reduzieren
    }

    for (i = 0; i < 4; i++) // 4-stellige HEX-number auf Display ausgeben
        LCD_Write(buf[i], 1); // einzelne Ziffer ausgeben
}

// Writes an unsigned integer value to LCD and displays it in Binary

void LCD_Bin(uint8_t value)
{
    uint16_t i; // Counter
    uint8_t buf[8]; // char-String f?r Ergebnis der Umwandlung

    for (i = 0; i < 8; i++) // 8-Bit-number verarbeiten
    {
        buf[7 - i] = (value & 0x01) + 0x30; // unterstes Bit isolieren und in ASCII umwandeln
        value = value >> 1; // value um ein Bit reduzieren
    }

    for (i = 0; i < 8; i++) // 8-stellige Bin?r-number auf Display ausgeben
        LCD_Write(buf[i], 1); // einzelne Ziffer ausgeben
}

// Sends a float value to the LCD,
// value:       float value
// positions:   positions before decimal point (min.2)
// decimal_pos: positions after decimal point (min.1)

void LCD_Float(float value, uint8_t positions, uint8_t decimal_pos)
{
    uint8_t i; // Z?hlvariable
    uint32_t number; // Ganzzahliger Teil der Float-Variablen
    float Bruch; // Ziffern nach dem Komma

    if (value >= 0) // Vorzeichen bestimmen und ausgeben
        LCD_Write('+', 1);
    else
    {
        LCD_Write('-', 1);
        value = -value; // bei neg. number invertieren
    }

    number = (int) value; // number enth?lt nur ganzzahligen Teil
    Bruch = value - number; // Stellen vor dem Komma abtrennen

    LCD_Int(number, positions - decimal_pos, 1); // Ziffern vor dem Komma ausgeben
    LCD_Write('.', 1); // Dezimalpunkt ausgeben

    for (i = 1; i <= decimal_pos; i++) // Dezimalbruch in ganze number umwandeln
        Bruch = Bruch * 10;
    LCD_Int((int) Bruch, decimal_pos, 0); // Dezimalstellen ausgeben
}
