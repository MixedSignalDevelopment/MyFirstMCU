/*============================================================================
 * Licencia:
 * Autor:       Nahuel Espinosa
 * Fecha:       20/10/2019
 *===========================================================================*/
/** @file
 * @brief	Contiene las definiciones para el m?dulo DHT11
 * 
 * Aqui se encuentra la implementaci?n de funciones, los defines, macros, 
 * datos internos y externos, declaraciones y definiciones de funciones
 * internas y definiciones de funciones externas
 */

/*==================[Includes]============================================*/
#include <xc.h>
#include <stdint.h>

/*==================[Macros and defs]==================================*/

//#define PIN_DHT11   PORTBbits.RB4
//#define TRIS_DHT11  TRISBbits.TRISB4


#define PIN_DHT11           PORTAbits.RA6
#define TRIS_DHT11          TRISAbits.TRISA6

#define DHT11_TIMEOUT     (100)     // Timeout in microseconds
#define DHT11_DATA_SIZE   (5)

#define TRUE  1
#define FALSE 0

#define _XTAL_FREQ        (4000000L)

#define dht11_GPIO_Low()      PIN_DHT11  = 0; TRIS_DHT11 = 0;
#define dht11_GPIO_High()     TRIS_DHT11  = 1;
#define dht11_GPIO_Read()     PIN_DHT11

// Utiliza un timer para identificar los tiempos de cada bit de datos y el timeout
// Uses a timer to identify the times of the bits and the timeout
#define dht11_TMR_Reset()     TMR0= 0
#define dht11_TMR_Read()      TMR0
#define dht11_TMR_Config()    OPTION_REGbits.T0CS = 0; OPTION_REGbits.PSA = 1;

/*==================[definiciones de datos internos]=========================*/
/*==================[Internal data defs]=========================*/
static uint8_t dht11_byte[DHT11_DATA_SIZE];
static uint8_t dht11_aux;

/*==================[definiciones de funciones internas]=====================*/
/*==================[Internal function def]=========================*/

/**
 * @brief       Lee un byte enviado por el m?dulo DHT11 / Reads a byte sent by the DHT11
 * @return      1 si la recepci?n fue correcta / 1 if the reception was correct
 *              0 si hubo timeout / 0 there was timeout
 * @note        Save the result in a global variable (dht11_aux)
 */
static uint8_t dht11_read_byte()
{
    uint8_t i;
    dht11_aux = 0;

    // Recieve 8 bits
    for (i = 0; i < 8; i++)
    {
        // Wait for a raising edge, low pulse is always 50us
        dht11_TMR_Reset();
        while (!dht11_GPIO_Read())
        {
            if (dht11_TMR_Read() > DHT11_TIMEOUT) return FALSE;
        }

        //Wait for raising edge and measure the high time
        //If the pulse is high 28us it means 0, if it lasts 70us its a 1
        dht11_TMR_Reset();
        while (dht11_GPIO_Read())
        {
            if (dht11_TMR_Read() > DHT11_TIMEOUT) return FALSE;
        }

        //If the pulse is longer than 50us then its a logic 1, otherwise a 0
        // Recieve first most significant bits
        //if( dht11_TMR_Read() > 50 ) dht11_aux |= 0x01;
        //byte_aux <<= 1;
        // Writen in assembler to not lose the data for the delay on execution       
        asm("  movlw  0x32          ");
        asm("  subwf  TMR0,w        ");
        asm("  rlf    _dht11_aux,f  ");
    }
    return TRUE;
}

/*==================[External function define]=====================*/

/**
 * @brief       Init and config te timer
 * @return      void
 */
void dht11_config(void)
{
    dht11_GPIO_Low();
    __delay_ms(18);
    dht11_GPIO_High();
    __delay_us(30);
    dht11_TMR_Config()
    
}

//void dht11_config()
//{
//    DHT11_Data_Pin_Direction = 0; //Configure RD0 as output
//    DHT11_Data_Pin = 0; //RD0 sends 0 to the sensor
//    delay_ms(18);
//    DHT11_Data_Pin = 1; //RD0 sends 1 to the sensor
//    delay_us(30);
//    DHT11_Data_Pin_Direction = 1; //Configure RD0 as input
//}

/**
 * @brief       Reads data from the DHT11
 * @param[in]   *phum:  Pointer to save hum
 * @param[in]   *ptemp: Pointer to save temp
 * @return      1 if the readout was correct
 *              0 there was timeout or error checksum
 */
uint8_t dht11_read(float *phum, float *ptemp)
{
    uint8_t i;

    // Start signal of at least 18ms
    dht11_GPIO_Low();
    __delay_ms(18);
    dht11_GPIO_High();

    // Wait for falling edge as a response
    dht11_TMR_Reset();
    while (dht11_GPIO_Read())
    {
        if (dht11_TMR_Read() > DHT11_TIMEOUT) return FALSE; // Return FALSE if there was a timeout
    }

    // Wait for response rising edge (80 us)
    dht11_TMR_Reset();
    while (!dht11_GPIO_Read())
    {
        if (dht11_TMR_Read() > DHT11_TIMEOUT) return FALSE; // Return FALSE if there was a timeout
    }

    // Wait for response falling edge (80 us)
    dht11_TMR_Reset();
    while (dht11_GPIO_Read())
    {
        if (dht11_TMR_Read() > DHT11_TIMEOUT) return FALSE; // Return FALSE if there was a timeout
    }

    // Module start sending data (40 bits):
    //     8 bits int part of RH, 8 decimal part of RH
    //     8 bits int part T , 8 bits decimal part T
    //     8 bits checksum
    for (i = 0; i < DHT11_DATA_SIZE; i++)
    {
        if (!dht11_read_byte()) return FALSE; // Return FALSE if there was a timeout
        dht11_byte[i] = dht11_aux;
    }

    //    // Check the integrity of the data
    //    uint8_t checksum;
    //	checksum = dht11_byte[0] + dht11_byte[1] + dht11_byte[2] + dht11_byte[3];
    //	if( checksum != dht11_byte[4] ) {
    //		return FALSE;   // Return FALSE if there was a timeout
    //	}

    // Format the data
    *phum = ((float) dht11_byte[0]) + ((float) dht11_byte[1]) / 10;
    *ptemp = ((float) dht11_byte[2]) + ((float) dht11_byte[3]) / 10;
    return TRUE;
}

/*==================[fin del archivo]========================================*/

