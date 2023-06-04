#include <stdio.h>
#include <math.h>
#include "oled.h"
#include <string.h>

#define ROW_HEIGHT 10
#define ACC_1G 10000

void display_init()
{
		oled_init();
		oled_setContrast(16);

		oled_clearScreen(OLED_COLOR_BLACK);
}

// Toma un entero sin signo de 32 bits y lo muestra en el tercer renglon del display
//en el cuarto renglon muestra sqrt(g/1000)
void displayMaxG(uint32_t g)
{
	char str[10];
	float gsqrt;
//mostramos el valor capturado crudo
	sprintf(str, "Max G^2 = %i", g);
	oled_putString(2, (OLED_DISPLAY_HEIGHT /4)+ROW_HEIGHT, str, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
//mostramos el valor parseado
	gsqrt = sqrt( ((float)g/ACC_1G));
	sprintf(str, "Max G = %1.2f",gsqrt);
	oled_putString(2, (OLED_DISPLAY_HEIGHT /4)+2*ROW_HEIGHT, str, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
}

//DEBUG: Para probar la comunicacion con el display
void display_test()
{
	oled_clearScreen(OLED_COLOR_BLACK);
	/* Drawing */
	oled_putString(2, 2, (uint8_t *)"Escudero, Maero", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

	//TEST SCREEN
	oled_putString(2, (OLED_DISPLAY_HEIGHT /4), (uint8_t *)"Estado acelerometro: ", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	oled_putString(2, (OLED_DISPLAY_HEIGHT /4)+9, (uint8_t *)"?", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

}

//Muestra en el primer renglon del display los apellidos de los integrantes
//Aprovechando que el primer 1/4 del display tiene otro color
void display_header()
{
	oled_clearScreen(OLED_COLOR_BLACK);
	oled_putString(2, 2, (uint8_t *)"TP3: Escudero, Maero", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
}


void display_reading(int16_t x, int16_t y, int16_t z)
{
	char str[10];
	int g;

	oled_putString(2, 2, (uint8_t *)"TP3: Escudero, Maero", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

	sprintf(str, "X = %03i", x);
	oled_putString(2, (OLED_DISPLAY_HEIGHT /4)+ROW_HEIGHT, str, OLED_COLOR_WHITE, OLED_COLOR_BLACK);

	sprintf(str, "Y = %03i", y);
	oled_putString(2, (OLED_DISPLAY_HEIGHT /4)+2*ROW_HEIGHT, str, OLED_COLOR_WHITE, OLED_COLOR_BLACK);

	sprintf(str, "Z = %03i", z);
	oled_putString(2, (OLED_DISPLAY_HEIGHT /4)+3*ROW_HEIGHT, str, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
//
//	g=( (x*x) + (y*y) + (z*z) );
//	sprintf(str, "G^2 = %i", g);
//	oled_putString(2, (OLED_DISPLAY_HEIGHT /4)+4*ROW_HEIGHT, str, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
}

//Muestra en el segundo renglon del display el string provisto (LEGACY: TP2)
void display_state(uint8_t* str)
{
	oled_putString(2, (OLED_DISPLAY_HEIGHT /4), str, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
}
