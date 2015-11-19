/*******************************************************************************
* This file provides the functions for the RFID Sample code

* Author: ANG JIA YI @ Cytron Technologies Sdn. Bhd.
*******************************************************************************/
// This file is written for PIC16F877A, if other model of PIC is uses, please modify accordingly


#include <P18F2550.h>
#include "lcd.h"		
/*******************************************************************************
* DEVICE CONFIGURATION WORDS                                                   *
*******************************************************************************/
#pragma config LVP=OFF
#pragma config PBADEN=OFF
#pragma config WDT=OFF
#pragma config FOSC=HS
#pragma config MCLRE=ON

//		 WDTDIS &		// Disable Watchdog Timer.
//		 PWRTEN &		// Enable Power Up Timer.
//		 BORDIS &		// Disable Brown Out Reset.
//		 LVPDIS);		// Disable Low Voltage Programming.

/*******************************************************************************
* Defination of label		                                                  *
*******************************************************************************/

						
/*******************************************************************************
* PRIVATE FUNCTION PROTOTYPES                                                  *
*******************************************************************************/
void adc_initialize(void);
void delay_ms(unsigned int num);
void uart_initialize(void);
unsigned char uc_uart_receive(void);
void uart_transmit(unsigned char uc_data);
void uart_putstr(rom const char* csz_string);
void sendsms(void);
/*******************************************************************************
* Global Variables                                                             *
*******************************************************************************/

/*******************************************************************************
* MAIN FUNCTION                                                                *
*******************************************************************************/
void main()
{
	unsigned int check = 0;	// declare a variable to store 
	// ensure all the hardware port in zero initially
	PORTA = 0;
	PORTB = 0;
	PORTC = 0;

	// Initialize the I/O port direction, this must be configured according to circuit
	TRISA = 0b00000011;
	TRISB = 0b00000001;
	TRISC = 0b10010011;

	//Initialize uart to be use
	uart_initialize();
	//Initialize lcd to be use
	lcd_init();
	//Initialize adc to be use
	adc_initialize();
	
	lcd_goto(0x80);
	lcd_putstr("initial");
	delay_ms(500);			//to let the PIR sensor to calibrate with the environment
	lcd_clear_msg("ready\rSW1=rst");
	while(1) 								// create an infinite loop
	{
		if(PORTAbits.RA1==1)
		{
			PORTBbits.RB1=1;		//to check if LED is inside the routin
			delay_ms(100);
			if(check==0)		//recheck the status weather it is really triggle
			{
			sendsms();			//send the sms to notice someone is there 
			check++;			//increase the check number so that it wont resent the sms for lot of time
			}
		}			
		else if(PORTBbits.RB0==0)	
		{	
			check=0;			//use for checking the time of sms send. if more than 1 dont send	
		}	
		else if(PORTAbits.RA1==0)	
		PORTBbits.RB1=0;		//turn off the LED in the SK28A when in initialize condition
	}	
		while(1) continue;	// infinite loop to prevent PIC from reset if there is no more program	
}

/*******************************************************************************
* PRIVATE FUNCTION: delay_ms
*
* PARAMETERS:
* ~ ui_value	- The period for the delay in miliseconds.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Delay in miliseconds.
*
*******************************************************************************/

/*******************************************************************************
* PRIVATE FUNCTION: beep
*
* PARAMETERS:
* ~ uc_count	- How many times we want to beep.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Beep for the specified number of times.
*
*******************************************************************************/

void delay_ms(unsigned int num)
{
	unsigned int lop,i1,j1;
	for(lop=0;lop<=num;lop++)
	{
	for(i1=0;i1<200;i1++)
		for(j1=0;j1<25;j1++);	
	}	
}	

void adc_initialize(void)
{
	ADCON1=0b00001110;
	ADCON2=0b10101010;
	ADCON0=0b00000000;	
}	

void uart_initialize(void)
{
	unsigned char dummy = 0;
	TXSTAbits.BRGH = 1;									// Select high speed baud rate.
	SPBRG = (20000000 / 16 / 9600) - 1;	// Configure the baud rate.
	RCSTAbits.SPEN = 1;									// Enable serial port.
	RCSTAbits.CREN = 1;									// Enable reception.
	TXSTAbits.TXEN = 1;									// Enable transmission.
	dummy = RCREG;		// dummy read, to clear the receive buffer
	dummy = RCREG; 
}


void uart_transmit(unsigned char uc_data)
{
	// Wait until the transmit buffer is ready for new data.
	while (PIR1bits.TXIF == 0);
	
	// Transmit the data.
	TXREG = uc_data;
}


unsigned char uc_uart_receive(void)
{
	// If there is overrun error...
	if (RCSTAbits.OERR == 1) {
		// Clear the flag by disable and enable back the reception.
		RCSTAbits.CREN = 0;
		RCSTAbits.CREN = 1;
	}	
	
	// Wait until there is data available in the receive buffer.
	while (PIR1bits.RCIF == 0);
	
	// Return the received data.
	return RCREG;
}

void uart_putstr(rom const char* csz_string)
{
	// Loop until the end of string.
	while (*csz_string != '\0') 
		{
		uart_transmit(*csz_string);
			
		// Point to next character.
		csz_string++;
	}
}


void sendsms(void)
{
	uart_putstr("AT\r");
	delay_ms(10);
		
	uart_putstr("AT+CMGF=1\r");	
	delay_ms(10);
			
	uart_putstr("AT+CSCA=+60162999902\r");
	delay_ms(10);	
	
	uart_putstr("AT+CMGS=0125651453\r");	
	delay_ms(10);
	
	uart_putstr("ALERT!!!");
	uart_transmit(0x1A);			
}	