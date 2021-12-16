# Greenhouse

Link to GitHub repository:

https://github.com/xhroch01/DE2-project

### Team members

* Aleš Horna (responsible for code writing and implementation/simulation)
* [GitHub reference](https://github.com/xhorna16/Digital-electronics-2)
* Jakub Hlaváček (responsible for README.md file completition and finishing touches)
* [GitHub reference](https://github.com/Jakubhl/Digital-electronics-2)
* Vojtěch Hroch (responsible for video presentation and diagrams)
* [GitHub reference](https://github.com/xhroch01/Digital-electronics-2)

### Table of contents

* [Project objectives](#objectives)
* [Hardware description](#hardware)
* [Libraries description](#libs)
* [Main application](#main)
* [Video](#video)
* [References](#references)

<a name="objectives"></a>

## Project objectives
The main objective is to implement techniques to control greenhouse. Explained by diagrams below.

### Scheme:

   ![your figure](IMAGES/celkoveschema.PNG)
   
### Flowchart
   ![your figure](IMAGES/stromfinal.PNG)

### Block diagram
   ![your figure](IMAGES/blokoveschema.png)

## Hardware description
For measuring we used capacitance and temperature sensors. When the temperature was higher than the desired value, the system turns on the fans to lower the temperature on the contrary when the temperature reached a value lower, than the desired value, the system turns on the heating. 

Soil moisture level measurement is based on the electric resistance of the soil, therefore the moisture sensor is very simple, it consists of two wires sticked into the soil. Based on data provided by moisture senzor are the valves to irrigation pumps controlled.

### Arduino uno

   ![your figure](IMAGES/Arduino.PNG)

Flash memory 32 KB, 14 digital pins a 6 analog pins, range USB a SPI.

### LCD

   ![your figure](IMAGES/displej.png)
   
2 rows, 16 characters 

### TC1046
High Precision Temperature-to-Voltage Converter

[TC1046 datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/21496C.pdf?fbclid=IwAR3JS0fTOTgRv-BpEHv4b_GTBlFUy0KrOrPso-AE79n30IDYJwicSIyW-h8)

### Moisture sensor

![your figure](IMAGES/moisture.PNG)

<a name="libs"></a>

## Libraries description
In this project we utilized libraries such as: "avr/io.h", "avr/interrupt.h", "util/delay.h", "stdlib.h", "math.h", "timer.h", "gpio.h", "lcd.h" and lcd_definitions.h

#### <avr/io.h>
This library contains IO (input/output) definitions for the devices based on AVR type of chip.

#### <avr/interrupt.h>
Library is used in the project to handle the interrupts

#### <util_delay.h>
Library "util_delay.h" is utilized for the definition of CPU frequency, also can be used for example as busy-wait function.

#### <stdlib.h>
This library defines variable types.

#### <math.h>
The math.h header defines various mathematical functions and one macro.

#### <timer.h>
In this library are defined overflows for interrupts.

#### <gpio.h>
This library is used for AVR pin configurations.

#### <lcd_definitions.h>, <lcd.h>
Those two libraries are utilized to control the LCD. Concretely the "lcd.h" library defines place (coordinates) to write the message on LCD (function: lcd_gotoxy), also the message itself (lcd_puts). The "lcd-definitions.h" library defines LCD pins.
## Main application

### Program
The first example shows the function that monitors whether the button was pressed. If the button is pressed, the demanded temperature is incremented.
```c
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
```
The second example shows the interrupt service routine that triggers the moisture measurement, AD conversion needed for the temperature measurement. It also gives orders to the appropriate peripherals.
```c
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
	if (timing == 75)                              // 5 min (75)
	{
		////PORTB = PORTB & ~(1<<PINB0); //heating turned off
		////PORTC = PORTC & ~(1<<PINC1); //heating turned off
		GPIO_write_low(&PORTC, HEAT);             // turn off the heating
	}
	if (timing == 45) // 3 min (45)
	{
		////PORTB = PORTB & ~(1<<PINB1); //fan turned off
		////PORTC = PORTC & ~(1<<PINC2); //fan turned off
		GPIO_write_low(&PORTC, FAN);             // turn off the fan
	}
	if (timing == 15) // 1 min (15)
	{
		////PORTB = PORTB & ~(1<<PINB2); //valve1 closed
		////PORTB = PORTB & ~(1<<PINB3); //valve2 closed
		GPIO_write_low(&PORTB, VALVE1);          // close the valves
		GPIO_write_low(&PORTB, VALVE2);
	}
}
```
The third example shows the interrupt service routine activated when the AD conversion is done. It shows also the method how the temperature is calculated and displayed.
```c
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
```
### Implementation

![your figure](IMAGES/ales_zapojeni.PNG)

<a name="video"></a>

## Video
The video describing our project was made in OpenShot software environment. The monolog in this video is made by Vojtěch Hroch.


<a name="references"></a>

## References

### 1. 
[YouTube - video presentation](https://www.youtube.com/watch?v=zIkkEqXffzc)

### 2. 
[Tomas-Fryza/Digital-electronics-2](https://github.com/tomas-fryza/Digital-electronics-2)
