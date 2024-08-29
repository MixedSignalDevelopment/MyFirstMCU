
// Includes
#include <xc.h>
#include <stdint.h>
#include "lcd_msd.h"
#include "dht11.h"

// Configuration settings
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF   // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON   // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON   // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF    // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF    // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF    // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF     // Flash Program Memory Code Protection bit (Code protection off)

#define _XTAL_FREQ 4000000  // Define the frequency of the crystal oscillator

#define SBIT_PS1  5
#define SBIT_PS0  4

#define TMR_H_10MS  0xFB
#define TMR_L_10MS  0x1E

// Define button pins
#define BUTTON_RA0 PORTAbits.RA0
#define BUTTON_RA1 PORTAbits.RA1
#define BUTTON_RA2 PORTAbits.RA2

#define LED_RA3 PORTAbits.RA3
#define LED_RA4 PORTAbits.RA4

uint16_t duty_cycle = 0;
uint16_t pot_value = 0;
uint16_t led_on = 1;

volatile uint8_t tick = 1;
uint32_t tick_counter = 0;

float hum, temp;

void __interrupt() isr(void) {
    if (TMR1IF) {
        TMR1H = 0xF6;
        TMR1L = 0x3B;
        tick = 0;
        TMR1IF = 0; //Clear the Timer 1 interrupt flag
    }
}


// Init ADC

void ADC_Init() {
    ANSEL = 0b00010000;
    ADCON0 = 0b10010001; // Select AN4 as analog input
    ADCON1 = 0b00000000; // Vref+ is connected to VDD, Vref- is connected to VSS
}

// Read ADC value

unsigned int ADC_Read() {
    GO_nDONE = 1; // Start ADC conversion
    while (GO_nDONE); // Wait for conversion to complete
    return ((ADRESH << 8) + ADRESL); // Return 10-bit ADC result
}

// Initialize PWM module

void PWM_Init() {
    TRISCbits.TRISC2 = 0; // Set RB5 as output for PWM

    // Configure CCP module for PWM mode
    CCP1CONbits.CCP1M = 0b1100; // PWM mode, active-high

    // Configure Timer2 for PWM operation
    PR2 = 255; // Set PWM period (PR2 + 1) * 4 * ToscxPrescale
    T2CONbits.T2CKPS = 0b01; // Prescaler 1:1
    T2CONbits.TMR2ON = 1; // Turn on Timer2

    // Initialize duty cycle to 0
    CCPR1L = 0;
}

// Main Function

void main(void) {
    // Set button pins as inputs
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;
    // Set button pins as digital inputs
    ANSELbits.ANS0 = 0;
    ANSELbits.ANS1 = 0;
    ANSELbits.ANS2 = 0;

    ADC_Init(); // Initialize ADC module
    PWM_Init(); // Initialize PWM module
    LCD_Init(); // Initialize LCD display
    dht11_config(); // Initialize DHT11 sensor

    TRISAbits.TRISA3 = 0;
    TRISAbits.TRISA4 = 0;
    TRISAbits.TRISA7 = 0;
    PORTAbits.RA7 = 0;

    TMR1H = 0xF6;
    TMR1L = 0x3B;
    T1CON = 0b00010000; // Prescalar = 1:4
    PIE1 = 0b00000001;
    INTCON = 0b11000000;
    TMR1ON = 1;

    LED_RA3 = 0;
    LED_RA4 = 0;

    while (1) {

        // this code is getting executed exactly every 10ms


        while (tick);   // 10ms interval
        tick = 1;
        tick_counter++;

        // Execute every 1s
        if ((tick_counter % 100) == 0) {
            LED_RA3 = ~LED_RA3; //Toggle the LED
            dht11_read(&hum, &temp);

            LCD_Goto(0, 0);
            LCD_Float(temp, 5, 2);
            LCD_Goto(1, 0);
            LCD_Float(hum, 5, 2);
        }

        // Execute every 10ms
        if (led_on == 1) {
            // Read potentiometer value
            pot_value = ADC_Read();
            // Map potentiometer value to duty cycle (0-255)
            duty_cycle = pot_value / 256; // Adjust the mapping factor as per your requirement   
            // Set duty cycle for PWM
            CCPR1L = duty_cycle;
        }
        
        if (BUTTON_RA1 == 0) {
            LED_RA4 = ~LED_RA4; //Toggle the LED
        }
        if (BUTTON_RA2 == 0) {
            // Button RA0 is pressed
            led_on = led_on * (-1);
            CCPR1L = 0;
        }
    }


    return;
}
