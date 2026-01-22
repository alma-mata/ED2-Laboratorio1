/*
 * Universidad del Valle de Guatemala
 * IE3054: Electrónica Digital 2
 * Laboratorio1.c
 * Autor: Alma Lisbeth Mata Ixcayau
 * Proyecto: Laboratorio 1 - Digital 2
 * Descripcion: Juego de carreras con LEDs
 * Creado: 20/01/2026 16:41:46
*/
/****************************************/
// Encabezado
#define F_CPU 16000000	//Frecuencia es 16Mhz
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "display7seg/display7seg.h" //SIEMPRE usar con el PORTD

/****************************************/
// Prototipos de función
void setup();
void cuenta_regresiva();
void multiplexado();

// Variables globales
volatile uint8_t contador1 = 0;
volatile uint8_t contador2 = 0;
volatile uint8_t out_LEDS = 0;

volatile uint8_t bandera_tm1 = 0;	// Bandera ciclos TIMER2
volatile uint8_t reloj = 5;			// Cuenta regresiva
volatile uint8_t start = 0;			// Bandera de comenzar juego
volatile uint8_t winner = 0;		// Bandera para reconocer al ganador
uint8_t estado_actual = 0b0001;		// Estado multiplexado
//uint8_t Tabla7seg[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10, 0x08, 0x03, 0x46, 0x21, 0x06, 0x0E};

#define DISPLAY (1 << PORTB0)
#define LEDS (1 << PORTB1)

/****************************************/
// Función principal

int main(void)
{
    setup();	// Se llama a la configuracion del arduino
	
    while (1) 
    {
		// Cuenta regresiva para comenzar
		if (bandera_tm1)
		{
			cuenta_regresiva();
		}
		else if (start)
		{
			reloj = winner;	 
			if ((winner == 1) || (winner == 2))
			{
				start = 0x00;
			}
		}
		// Multiplexado
		multiplexado();
    }
}

/****************************************/
// Subrutinas sin Interrupcion
void setup()
{
	cli();
	SPCR = 0x00; // Asegura que el SPI esté desactivado
	// Configuración de puertos
	DDRB = 0xFF;		// PORT B como salida
	DDRD = 0xFF;		// PORT D como salida
	DDRC &= ~((1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2));	// entradas del PORTC
	PORTC |= (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2);		// pull-ups
	
	// Habilitar interrupciones en el puerto C
	PCMSK1 |= (1 << PCINT8) | (1 << PCINT9) | (1 << PCINT10);
	PCICR |= (1 << PCIE1);
	
	// Habilitar interrupciones del TIMER0
	TCCR0A = 0x00;		// Modo Normal
	TCCR0B = (1 << CS01) ;   //Prescaler 8
	TCNT0 = 131;
	TIMSK0 = (1 << TOIE0);
	
	// Habilitar interrupciones del TIMER1
	TCCR1A = 0x00;		// Modo Normal
	TCCR1B = (1 << CS12);	// Prescaler 256
	
	// Interrupciones del TIMER2
	TCCR2A = 0x00;			// Modo Normal
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);		// Prescaler 1024
	sei();
}

void multiplexado()
{
	PORTB = 0x00;
	if (estado_actual == 0b0001) // mostrar LEDS en PORTD
	{
		out_LEDS = ((contador1 << 4) | contador2);
		PORTD = out_LEDS;
		PORTB |= LEDS;
	}
	else // mostrar DISPLAY en PORTD
	{
		mostrar_display(reloj);  // Librería Display 7 Segmentos
		PORTB |= DISPLAY;
	}
}

void cuenta_regresiva()
{
	reloj--;
	bandera_tm1 = 0;
	if (reloj == 0)
	{
		start = 0x01;
		TIMSK1 = (0 << TOIE1);
	}
}

/****************************************/
// Subrutinas de Interrupcion
ISR(PCINT1_vect)
{
	if (start) // Si el juego ya ha comenzado
	{
		if(!(PINC & 0b00000010)) // Si es PC1 aumenta Jugador 1
		{
			if (contador1 == 0)
			{
				contador1 = 0x01;
			}
			else{
				contador1 = ((contador1 << 1) & 0x0F);
				if (contador1 == 0x08)
				{
					winner = 1;
				}
			}
		}
		if(!(PINC & 0b00000100)) // Si es PC2 aumenta Jugador 2
		{
			if (contador2 == 0)
			{
				contador2 = 0x01;
			}
			else{
				contador2 = ((contador2 << 1) & 0x0F);
				if (contador2 == 0x08)
				{
					winner = 2;
				}
			}
		}
	}
	else if(!(PINC & 0b00000001)) //Si el juego NO ha comenzado y es PC0
	{
		TCNT1 = 34286;	// Interrupción cada 0.5s
		TIMSK1 = (1 << TOIE1);
		//TCNT2 = 100;		// Interrupcion cada 0.05s
		//TIMSK2 = (1 << TOIE2);
	}
	
}

ISR(TIMER0_OVF_vect)
{
	if (estado_actual == 0b0001)
	{
		estado_actual = 0b0010; // LEDS
	}
	else
	{
		estado_actual = 0b0001;	// DISPLAY
	}
}

ISR(TIMER1_OVF_vect)
{
	bandera_tm1 = 1;
}

//ISR(TIMER2_OVF_vect)
//{
	//if (contador_tm2 == 50)
	//{
		//bandera_tm2 = 1;
		//contador_tm2 = 0;
	//}
	//contador_tm2++;
//}