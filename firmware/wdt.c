
#include <ioavr.h>
#include <intrinsics.h>
#include "wdt.h"


void WatchdogInit(void)
{
//   __disable_interrupt(); // Interrupt management should be done outside of the function
  __watchdog_reset();
  /* Start timed sequence */
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  /* Set new prescaler(time-out) value = 128K cycles (~1 s) */
  WDTCSR = (1<<WDE) | (1<<WDP2) | (1<<WDP1);
}

void WatchdogStop(void)
{
    __watchdog_reset();
    //Write logical one to WDCE and WDE
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    // Turn off WDT
    WDTCSR = 0x00;
}

void WatchdogStart(void)
{
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  /* Set new prescaler(time-out) value = 128K cycles (~1 s) */
  WDTCSR = (1<<WDE) | (1<<WDP2) | (1<<WDP1);
}