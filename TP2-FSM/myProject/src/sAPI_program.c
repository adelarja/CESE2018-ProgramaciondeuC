/*============================================================================
 * Autor:
 * Licencia:
 * Fecha:
 *===========================================================================*/

/*==================[inlcusiones]============================================*/

#include "sapi.h"        // <= Biblioteca sAPI
#include <stdio.h>
/*==================[definiciones y macros]==================================*/
#define LONGDELAY 1000 //Mayor delay
#define SHORTDELAY 20 //Menor delay
#define NUMLEDS 3 //Número de leds que participan del blink
#define NUMPER 2 //Número de períodos de blink

//Delay de estados: Non delay para cuando no hay delay y DELAY# para # ms.
#define NONDELAY 0
#define DELAY40 40

//Definición de estados
#define BOTTOM 0
#define RISING 1
#define UP 2
#define FALLING 3

//Definición de salida
#define OUTOFF 0
#define OUTON 1
#define UNDIFINED 2
/*==================[definiciones de datos internos]=========================*/

DEBUG_PRINT_ENABLE
CONSOLE_PRINT_ENABLE

/*==================[definiciones de datos externos]=========================*/
/*Definimos un Struct que va a representar a un estado
 *El estado tiene en cuenta los siguientes parámetros:
 *OUT: Salida única del estado, tipo char.
 *NEXT[]: Siguiente estado dependiente de la entrada y del estado actual.
 *delay: Tiempo de delay que tiene el estado.
 */
struct state{

	uint16_t delay;
	char out;
	char next[2];

};
typedef struct state estado;

estado FSM[] = {{NONDELAY, OUTOFF, {BOTTOM, RISING}}, //Estado 0
		{DELAY40, OUTOFF, {BOTTOM, UP}}, //Estado 1
		{NONDELAY, OUTON, {FALLING, UP}}, //Estado 2
		{DELAY40, OUTON, {BOTTOM, UP}} //Estado 3
};

char cState1 = BOTTOM;
char cState2 = BOTTOM;

delay_t delayTec1;
delay_t delayTec2;
delay_t toggleDelay;

/*==================[declaraciones de funciones internas]====================*/
char getOut(estado);/*Función para obtener el valor de la salida correspondiente al estado*/
void getWork(bool_t, estado[], char *st);/*Función para hacer funcionar la máquina de estados de acuerdo a la entrada*/

bool_t delayStart(delay_t *, uint16_t);
void toggleDelayed(gpioMap_t *, uint16_t);
/*==================[declaraciones de funciones externas]====================*/

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void ){

   // ---------- CONFIGURACIONES ------------------------------

   // Inicializar y configurar la plataforma
   boardConfig();

   // Inicializar UART_USB como salida Serial de debug
   debugPrintConfigUart( UART_USB, 115200 );
   debugPrintlnString( "DEBUG: UART_USB configurada." );
   
   // Inicializar UART_232 como salida Serial de consola
   consolePrintConfigUart( UART_232, 115200 );
   consolePrintlnString( "UART_232 configurada." );

   // Crear varias variables del tipo booleano
   bool_t tec1Value = OFF; //Variable que indica cambio de frecuencia
   bool_t tec2Value = OFF; //Variable que indica  cambio de led que hace blink

    //Variables para delay
   uint16_t delayTime = SHORTDELAY; //Variable de tiempo de delay
   uint16_t delayOptions[] = {SHORTDELAY, LONGDELAY}; //Array de posibles delays
   uint8_t delayIndex = 0; //Indice del delayOptions[]

   //Variables para cambio de leds
   gpioMap_t leds[] = {LEDR, LED3, LEDB}; //Nos indica los leds que participan del blink
   gpioMap_t led = LEDR; //Nos indica que led está blinkeando actualmente
   uint8_t ledIndex = 0; //Indice del array de leds que participan del blink
   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
	  getWork(gpioRead( TEC1 ), FSM, &cState1);

	  // Si el switch 1 se presiona, cambio frecuencia de delay
	  if(getOut(FSM[cState1]) == OUTON){
		  if(!tec1Value){ //Si el pulsador no estaba presionado desde antes, ingresa.
			  delayTime = delayOptions[delayIndex++];
			  tec1Value = ON;
			  if(delayIndex == NUMPER){
				  delayIndex = 0;
			  }
		  }
	  }
	  else if(getOut(FSM[cState1]) == OUTOFF){
		  tec1Value = OFF;
	  }
      /* Si se presiona TEC2, cambio el color del led que titila dentro de una array */

	  getWork(gpioRead( TEC2 ), FSM, &cState2);

	  if(getOut(FSM[cState2]) == OUTON){ //Si se presiona el switch 2, cambio el led de blink
		  if(!tec2Value){
			  gpioWrite(led, OFF);
			  led = leds[ledIndex++];
			  tec2Value = ON;
			  if(ledIndex == NUMLEDS){
				  ledIndex = 0;
			  }
		  }
	  }
	  else if(getOut(FSM[cState2]) == OUTOFF){
		  tec2Value = OFF;
	  }

      toggleDelayed( &led, delayTime ); //Función que cambia el estado del led entre apagado y encendido


      /* Retardo bloqueante durante 250ms */
   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones internas]=====================*/
char getOut(estado st){
	return st.out;
}

void getWork(bool_t input, estado FSM[], char *cSt){

	//delay(FSM[cState].delay);
	*cSt = FSM[*cSt].next[input];

}

bool_t delayStart(delay_t *time, uint16_t mS){
	if(!(*time).running){
		delayConfig(time, mS);
		delayRead(time);
	}

	if(delayRead(time)){
	    return TRUE;
	}

	return FALSE;
}

void toggleDelayed(gpioMap_t *led, uint16_t time){

	if(delayStart(&toggleDelay, time))
		gpioToggle( *led );

}
/*==================[definiciones de funciones externas]=====================*/

/*==================[fin del archivo]========================================*/
