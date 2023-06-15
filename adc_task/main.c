/*
 * adc_task.c
 * Multichannel ADC example with oversampling
 * Reference voltage = 5V
 * Sample = 5/1023 = 4,88 mV
 * Allowing ADC channel measurement in 100 uS period
 * Created: 10.08.2017 14:37:46
 * Author : Mazinov
 * To print float numders via standart C io lib
 * Put the -lprintf_flt to Settings->Toolchain->AVR/GNU Linker->Miscellaneous 
 * And check Use vprintf library(-Wl,-u,vfprintf) in Settings->Toolchain->AVR/GNU Linker->General
 */ 

#define F_CPU 8000000UL // MHz Clock
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd_lib.h"
#include <stdio.h>
// Adc channels from 0 to 4
#define FIRST_ADC_INPUT 0
#define LAST_ADC_INPUT 4
// Voltage Reference: AREF pin
#define ADC_VREF_TYPE ((0<<REFS1) | (0<<REFS0) | (0<<ADLAR))
#define BEGIN_CONVERT ADCSRA|=(1<<ADSC)

typedef unsigned char byte;

volatile uint16_t adc_data[LAST_ADC_INPUT-FIRST_ADC_INPUT+1];
volatile static byte input_index=0;

// Function prototypes
float convert_to_voltage(uint16_t* adc_data);
void print_result(void);
uint16_t adc_read(byte adc_input);
void mcu_init(void);



ISR( TIMER0_OVF_vect )
{
	// 100 us oveflow
	TCNT0=0x9C;
	BEGIN_CONVERT;
}


ISR (ADC_vect)
{
	//static byte input_index=0;
	// Read the AD conversion result
	adc_data[input_index]=ADC; // ADCL+ADCH, sum += (ADCH<<8)|ADCL;
	// Select next ADC input
	if (++input_index > (LAST_ADC_INPUT-FIRST_ADC_INPUT))
	input_index=0;
	ADMUX=(FIRST_ADC_INPUT % 3)| (ADC_VREF_TYPE+input_index);
	// Delay needed for the stabilization of the ADC input voltage
	_delay_us(10);
	// Begin convert
	//ADCSRA|=(1<<ADSC);
	
}

int main(void)
{
	mcu_init();
	lcd_init();
	sei(); // Enable global interrupts
    
    while (1) 
    {
		print_result();
    }
	return 0;
}

float convert_to_voltage(uint16_t* adc_data)
{
	float voltage; 
    voltage = (float)adc_data[0] * 0.0048828; // (float)* data*5.00/1024.00;
	return voltage;               
}

void print_result(void)
{
	char voltage[10];
	lcd_gotoxy(0,0);
	lcd_string("Oversample ", 11);
	lcd_gotoxy(12, 0);
	lcd_num_to_str(adc_read(input_index), 4);
	lcd_gotoxy(0,1);
	lcd_string("First ch ", 9);
	lcd_gotoxy(12,1);
	sprintf(voltage, "%f", convert_to_voltage(adc_data));            
	lcd_string(voltage, 4);                    
}

uint16_t adc_read(byte adc_input)
{
	uint16_t result;

	if (adc_input<3)
	{
		adc_input*=4;
		result = ((adc_data[adc_input]+adc_data[adc_input+1]+adc_data[adc_input+2]+adc_data[adc_input+3])/4);
		return result;
	}
	else
	{
		return 0;
	};
	
}

void mcu_init()
{
	// Crystal Oscillator division factor: 1
	CLKPR=(1<<CLKPCE);
	CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);

	// Timer/Counter 0 initialization
	// Clock source: 8MHz external crystal resonator
	// Clock value: 1000,000 kHz
	// Mode: Normal top=0xFF
	// Timer Period: 0,1 ms
	TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
	TCCR0B=(0<<WGM02) | (0<<CS02) | (1<<CS01) | (0<<CS00);
	TCNT0=0x9C;
	OCR0A=0x00;
	OCR0B=0x00;

	// Timer/Counter 0 Interrupt initialization
	TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (1<<TOIE0);

	// ADC initialization
	// ADC Clock frequency: 125,000 kHz
	// ADC Voltage Reference: External ref source connected on AREF pin
	// ADC Auto Trigger Source: Timer0 Overflow
	DIDR0=(0<<ADC5D) | (0<<ADC4D) | (0<<ADC3D) | (0<<ADC2D) | (0<<ADC1D) | (0<<ADC0D);
	ADMUX=FIRST_ADC_INPUT | ADC_VREF_TYPE;
	ADCSRA=(1<<ADEN) | (0<<ADSC) | (1<<ADATE) | (0<<ADIF) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0);
	ADCSRB=(1<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);
}
