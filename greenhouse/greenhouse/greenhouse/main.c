/*************************************************
 * 
 * Greenhouse electronic system
 * based on ATmega328P (Arduino Uno), 16 MHz
 *
 * J. Hlavacek, A. Horna, V. Hroch
 * Brno University of Technology, Czechia
 * 
 *************************************************/
/* defines --------------------------------------*/
#ifndef F_CPU
# define F_CPU 16000000
#endif
#define HEAT   PC1
#define FAN    PC2
#define VALVE1 PB2
#define VALVE2 PB3
#define MOIST1 PB4
#define MOIST2 PB5
#define BUTTON PD2

/* includes -------------------------------------*/
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <math.h>
#include "timer.h"
#include "gpio.h"
#include "lcd.h"
#include "lcd_definitions.h"
uint8_t temperature_set = 25;        // (default value of) user defined temperature to be kept
uint16_t timing = 0;                 // timer
float temperature = 0;               // measured temperature
/* function definitions -------------------------*/
int main(void)
{
    //DDRB = DDRB | (1<<PB0);        // output - heating
    //PORTB = PORTB & ~(1<<PB0);     // heating turned off
    //DDRB = DDRB | (1<<PB1);        // output - fan
    //PORTB = PORTB & ~(1<<PB1);     // fan turned off
    //DDRB = DDRB | (1<<PB2);        // output - valve1
    //PORTB = PORTB & ~(1<<PB2);     // valve1 closed
    //DDRB = DDRB | (1<<PB3);        // output - valve2
    //PORTB = PORTB & ~(1<<PB3);     // valve2 closed
    //uint8_t temperature_set = 25;  // default value of user defined temperature to be kept
    //uint16_t timing = 0;           // timer
	//float temperature = 0;         // measured temperature
    GPIO_config_output(&DDRC, HEAT);          // heating output is set
    GPIO_write_low(&PORTC, HEAT);             // heating turned off
    GPIO_config_output(&DDRC, FAN);           // fan output is set
    GPIO_write_low(&PORTC, FAN);              // fan is turned off
    GPIO_config_output(&DDRB, VALVE1);        // 1st valve output is set
    GPIO_write_low(&PORTB, VALVE1);           // 1st valve is closed
    GPIO_config_output(&DDRB, VALVE2);		  // 2nd valve output is set
    GPIO_write_low(&PORTB, VALVE2);           // 2nd valve is closed
    GPIO_config_input_nopull(&PORTB, MOIST1); // external pull-up resistor will be added
    GPIO_config_input_nopull(&PORTB, MOIST2); // external pull-up resistor will be added
    GPIO_config_input_pullup(&PORTD, BUTTON); // temperature setting
    
    lcd_init(LCD_DISP_ON);            // LCD initialization
    lcd_gotoxy(0, 0); 
    lcd_puts("SET 25,0C M1 M2 ");
	lcd_gotoxy(0, 1);
    lcd_puts("MEA --,-C -- -- ");

    TIM1_overflow_4s();               //set the overflow prescaler to 4 s
    TIM1_overflow_interrupt_enable(); // enables interrupt caused by overflow
    sei();                            // enables interrupts at all
	
	// AD conversion
    ADMUX |= (1<<REFS0);              // reference set to internal 1.1V
    ADMUX |= (1<<REFS1);

    ADMUX = ADMUX &~ (1<<MUX3);       // input channel set to PC0
    ADMUX = ADMUX &~ (1<<MUX2);
    ADMUX = ADMUX &~ (1<<MUX1);
    ADMUX = ADMUX &~ (1<<MUX0);

    ADCSRA = ADCSRA | (1<<ADEN);      // ADC module is enabled
    ADCSRA |= (1<<ADIE);              // conversion complete interrupt is enabled

    ADCSRA = ADCSRA | (1<<ADPS2);     // clock prescaler is set to 128
    ADCSRA = ADCSRA | (1<<ADPS1);
    ADCSRA = ADCSRA | (1<<ADPS0);
    
    // Infinite loop
    while (1)
    {
		    if ((GPIO_read(&PIND, BUTTON)) == 0)                 // button is pushed
		    {
			    temperature_set = temperature_set + 1;
			    lcd_gotoxy(4, 0);
				char temperature_set_itoa[2] = "--";
				itoa(temperature_set, temperature_set_itoa, 10); // temperature is converted into a string
			    lcd_puts(temperature_set_itoa);                  // temperature is displayed
			    if (temperature_set == 41)                       // maximum demandable temperature is exceeded
			    {
				    temperature_set = 10;
				    lcd_gotoxy(4, 0);
					char temperature_set_itoa[2] = "--";
					itoa(temperature_set, temperature_set_itoa, 10);
				    lcd_puts(temperature_set_itoa);
			    }
		    }
    }

    // This will be never reached
    return 0;
}

/* Interrupt service routines -------------------*/
ISR(TIMER1_OVF_vect)
{	timing = timing + 1;
	if ((GPIO_read(&PINB, MOIST1)) == 1)      // insufficient moisture
	{
		lcd_gotoxy(10, 1);
		lcd_puts("LO");
	}
	else if ((GPIO_read(&PINB, MOIST1)) == 0) // sufficient moisture
	{
		lcd_gotoxy(10, 1);
		lcd_puts("HI");
	}
	if ((GPIO_read(&PINB, MOIST2)) == 1)      // insufficient moisture
	{
		lcd_gotoxy(13, 1);
		lcd_puts("LO");
	}
	else if ((GPIO_read(&PINB, MOIST2)) == 0) // sufficient moisture
	{
		lcd_gotoxy(13, 1);
		lcd_puts("HI");
	}
	ADCSRA = ADCSRA | (1<<ADSC);                  //start AD conversion
 	if (timing == 150)                            // 10 min interval (150)
	{
		if (temperature <= (temperature_set - 1)) // lower temperature
		{
		//PORTB |= (1<<PB0); // turn on the heating
		GPIO_write_high(&PORTC, HEAT);            // turn on the heating
		}
		if (temperature >= (temperature_set + 1)) // higher temperature
		{
			//PORTB |= (1<<PB1); // turn on the fan
			GPIO_write_high(&PORTC, FAN);         // turn on the fan
		}
		if ((GPIO_read(&PINB, MOIST1)) == 1)      // insufficient moisture
		{
			////PORTB |= (1<<PB2); // open valve1
			GPIO_write_high(&PORTB, VALVE1);      // open the 1st valve
		}
		if ((GPIO_read(&PINB, MOIST2)) == 1)      // insufficient moisture
		{
			////PORTB |= (1<<PB3); // open valve2
			GPIO_write_high(&PORTB, VALVE1);      // open the 2nd valve
		}
		timing = 0;                               // timer reset
	}
	if (timing == 3)                              // 5 min (75)
	{
		////PORTB = PORTB & ~(1<<PINB0); //heating turned off
		////PORTC = PORTC & ~(1<<PINC1); //heating turned off
		GPIO_write_low(&PORTC, HEAT);             // turn off the heating
	}
	if (timing == 3) // 3 min (45)
	{
		////PORTB = PORTB & ~(1<<PINB1); //fan turned off
		////PORTC = PORTC & ~(1<<PINC2); //fan turned off
		GPIO_write_low(&PORTC, FAN);             // turn off the fan
	}
	if (timing == 3) // 1 min (15)
	{
		////PORTB = PORTB & ~(1<<PINB2); //valve1 closed
		////PORTB = PORTB & ~(1<<PINB3); //valve2 closed
		GPIO_write_low(&PORTB, VALVE1);          // close the valves
		GPIO_write_low(&PORTB, VALVE2);
	}
}

ISR(ADC_vect)
{
	//float temperature = 0;
	char temperature_string [2] = "--";
	char temperature_dp_string [1] = "-";
	//uint16_t AD = 480;
	temperature = (((ADC*(1.1/1.023))-424)/6.25); // calculates the temperature from the output voltage
	itoa(temperature, temperature_string, 10);    // converts the integer part into a string
	itoa((floor(10*(temperature - (floor(temperature))))), temperature_dp_string, 10); //converts the first decimal point into a string
	//temperature_string = gcvt (temperature, 3, buf); //cuts the excessive decimal points and converts the resultant number into a string
	lcd_gotoxy(4, 1);
	lcd_puts("--,-");
	lcd_gotoxy(4, 1);
	lcd_puts(temperature_string);                // integer part is displayed
	lcd_gotoxy(6, 1);
	lcd_puts(",");
	lcd_gotoxy(7, 1);
	lcd_puts(temperature_dp_string);             // decimal part is displayed
	//char x [1] = "0";
	//itoa(ADC, x, 10);
	//lcd_gotoxy(4, 1);
	//lcd_puts(x);
}
//ISR(INT0_vect)
//{
//temperature_set++
//lcd_gotoxy(4,0);
//lcd_puts(temperature_set);
//	if temperature_set == 41 {
//	temperature_set = 10;
//	lcd_gotoxy(4,0);
//	lcd_puts(temperature_set);
//	}
//}

