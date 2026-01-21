/*
 * Universidad del Valle de Guatemala
 * IE3054: Electrónica Digital 2
 * display7seg.c
 * Autor: Alma Lisbeth Mata Ixcayau
 * Proyecto: Laboratorio 1 - Digital 2
 * Descripcion: Juego de carreras con LEDs
 * Creado: 20/01/2026
*/

#include "display7seg.h"
uint8_t Tabla7seg[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10, 0x08, 0x03, 0x46, 0x21, 0x06, 0x0E};

void mostrar_display(int numero)
{
	PORTD = Tabla7seg[numero & 0x0F];
}