#include <ioavr.h>
#include <intrinsics.h>
#include "types.h"
//#include "stdio.h"
#include "delay.h"
#include "matrix.h"
#include "amiga_key.h"
#include "wdt.h"

#ifdef DEBUG   
#include "serial.h"
#endif
#define F_CPU 8000000




U8 int_in_progress = 0;

#pragma vector=TIMER1_COMPB_vect
__interrupt void TIMER1_COMPB_isr(void)
{
  Matrix_Sample();
  __watchdog_reset();
}

  extern  const S_MATRIX_GPIO_DESC gcsKeyMatrixRows[ 6 ];
 extern volatile unsigned char debug_byte;

int main( void )
{
#ifdef DEBUG   
  USART_Init(19200);
#endif

  WatchdogInit();

#ifdef DEBUG  
  // Determine restart source
  if(MCUSR & 1) putstr("PORF: Power-on Reset Flag\r\n");
  if(MCUSR & 2) putstr("EXTRF: External Reset Flag\r\n");
  if(MCUSR & 4) putstr("BORF: Brown-out Reset Flag\r\n");
  if(MCUSR & 8) putstr("WDRF: Watchdog System Reset Flag\r\n");
  if(MCUSR & 16) putstr("JTRF: JTAG Reset Flag\r\n");
#endif

  
  Matrix_Init();
  AmigaKey_Init();
  
  
  // Timer init for 5ms interval interrupt
  	/* Enable TC1 */
	PRR0 &= ~(1 << PRTIM1);

	// TCCR1A = (0 << COM1A1) | (0 << COM1A0) /* Normal port operation, OCA disconnected */
	//		 | (0 << COM1B1) | (0 << COM1B0) /* Normal port operation, OCB disconnected */
	//		 | (0 << WGM11) | (0 << WGM10); /* TC16 Mode 12 CTC */

	TCCR1B = (0 << WGM13) | (1 << WGM12)                /* TC16 Mode 4 CTC */
	         | 0 << ICNC1                               /* Input Capture Noise Canceler: disabled */
	         | 0 << ICES1                               /* Input Capture Edge Select: disabled */
	         | (0 << CS12) | (0 << CS11) | (1 << CS10); /* No prescaling */

	// ICR1 = 0x0; /* Top counter value: 0x0 */

	OCR1A = 0x3e80; //(500hz).  0x9c40; // 200Hz scan frequency

	// OCR1B = 0x0; /* Output compare B: 0x0 */

	// GTCCR = 0 << TSM /* Timer/Counter Synchronization Mode: disabled */
	//		 | 0 << PSRASY /* Prescaler Reset Timer/Counter2: disabled */
	//		 | 0 << PSRSYNC; /* Prescaler Reset: disabled */

	TIMSK1 = 1 << OCIE1B   /* Output Compare B Match Interrupt Enable: enabled */
	         | 0 << OCIE1A /* Output Compare A Match Interrupt Enable: disabled */
	         | 0 << ICIE1  /* Input Capture Interrupt Enable: disabled */
	         | 0 << TOIE1; /* Overflow Interrupt Enable: disabled */
  
  
  

  asm("sei"); //enable interrupts
  
#ifdef DEBUG  
  putstr("Init done\r\n");
#endif
  
  while(1)
  {// Main while loop
    Matrix_Cycle();
    AmigaKey_Cycle();
  } // Main while loop
  
}
