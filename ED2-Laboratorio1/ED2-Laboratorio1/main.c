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
#define F_CPU 16000000		//Frecuencia es 16Mhz
#include <avr/io.h>			
#include <avr/interrupt.h>	// Librería de interrupciones
#include <stdint.h>
#include "display7seg/display7seg.h" //SIEMPRE usar con el PORTD

/****************************************/
// Prototipos de función
void setup();
void cuenta_regresiva();
void multiplexado();
void reinicio_variables();

// Variables globales
volatile uint8_t contador1 = 0;		// Contador jugador 1
volatile uint8_t contador2 = 0;		// Contador jugador 2
volatile uint8_t out_LEDS = 0;

volatile uint8_t bandera_tm1 = 0;		// Bandera ciclos TIMER1
volatile uint8_t bandera_reinicio = 0;	// Bandera reinicio de valores del juego
volatile uint8_t reloj = 5;				// Cuenta regresiva
volatile uint8_t start = 0;				// Bandera de comenzar juego
volatile uint8_t winner = 0;			// Bandera para reconocer al ganador
uint8_t estado_actual = 0b0001;			// Estado multiplexado

#define DISPLAY (1 << PORTB0)		// Pin del PORTB que enciende el display
#define LEDS (1 << PORTB1)			// Pin del PORTB que enciende los leds

/****************************************/
// Función principal

int main(void)
{
    setup();	// Se llama a la configuracion del arduino
	
    while (1) 
    {
		// Reinicio de variables
		if (bandera_reinicio)
		{
			reinicio_variables();
		}
		// Cuenta regresiva para comenzar
		else if (bandera_tm1)
		{
			cuenta_regresiva();
		}
		else if (start) // Si el juego ya a comenzado
		{	 
			if ((winner == 1) || (winner == 2)) // Si gano jugador 1 o 2
			{
				start = 0x00;		// Reiniciar variable
				reloj = winner;		// Mostrar ganador en el display
				contador1 = 0;		// Reiniciar contadores
				contador2 = 0;
				if (winner == 1)	// Encender LEDs del ganador
				{
					contador1 = 0x0F;
				}
				else{
					contador2 = 0x0F;
				}
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
	reloj--;			// Disminuye contador de reloj
	bandera_tm1 = 0;	// Desactiva bandera del TIMER1
	if (reloj == 0)		// Si la cuenta llego a 0
	{
		start = 0x01;			// Comenzar el juego
		TIMSK1 = (0 << TOIE1);	// Desactivar interrupciones del TIMER1
	}
}

void reinicio_variables()
{
	bandera_reinicio = 0;
	winner = 0;
	contador1 = 0;
	contador2 = 0;
	reloj = 6;
}
/****************************************/
// Subrutinas de Interrupcion
ISR(PCINT1_vect)
{
	if (start) // Si el juego ya ha comenzado
	{
		if(!(PINC & 0b00000010)) // Si es PC1 aumenta Jugador 1
		{
			if (contador1 == 0)		// Si el contador es 0
			{
				contador1 = 0x01;	// Inicializarlo con 1
			}
			else{					// De lo contrario
				contador1 = ((contador1 << 1) & 0x0F);	// Correr los bits 1 posición
				if (contador1 == 0x08)	// Si es 0b1000
				{
					winner = 1;			// Declararlo como ganador
				}
			}
		}
		if(!(PINC & 0b00000100)) // Si es PC2 aumenta Jugador 2
		{
			if (contador2 == 0)		// Si el contador es 0
			{
				contador2 = 0x01;	// Inicializarlo con 1
			}
			else{					// De lo contrario
				contador2 = ((contador2 << 1) & 0x0F);	// Correr los bits 1 posición
				if (contador2 == 0x08)	// Si es 0b1000
				{
					winner = 2;			// Declararlo como ganador
				}
			}
		}
	}
	else if(!(PINC & 0b00000001)) //Si el juego NO ha comenzado y es PC0
	{
		TCNT1 = 34286;	// Interrupción cada 0.5s
		TIMSK1 = (1 << TOIE1); // Activar interrupciones
		bandera_reinicio = 1; // Encender bandera para reinicio de variables
	}
	
}

ISR(TIMER0_OVF_vect)	// Verificar estado actual y cambiarlo al siguiente
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
	bandera_tm1 = 1;	// Activa bandera para decremento del reloj
}