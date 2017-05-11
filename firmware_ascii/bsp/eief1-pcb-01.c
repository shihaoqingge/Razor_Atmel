/*!**********************************************************************************************************************
@file eief1-pcb-01.c                                                                
@brief This file provides core and GPIO functions for the eief1-pcb-01 board.

Basic board setup functions are here which are not part of the API for the system since they
are one-time configurations for the processor.  

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- NONE

PUBLIC FUNCTIONS
- 

PROTECTED FUNCTIONS
- void ClockSetup(void)
- void SystemSleep(void)
- void WatchDogSetup(void)
- void GpioSetup(void)

***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxBsp"
***********************************************************************************************************************/
/* New variables */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /*!< @brief From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Bsp_" and be declared as static.
***********************************************************************************************************************/


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!---------------------------------------------------------------------------------------------------------------------
@fn void ClockSetup(void)

@brief Loads all registers required to set up the processor clocks.

Requires:
- NONE

Promises:
- EFC is set up with proper flash access wait states based on 48MHz system clock
- PMC is set up with proper oscillators and clock sources

*/
void ClockSetup(void)
{
  /* Set flash wait states to allow 48 MHz system clock (2 wait states required) */
  AT91C_BASE_EFC0->EFC_FMR = AT91C_EFC_FWS_2WS;

  /* Activate the peripheral clocks needed for the system */
  AT91C_BASE_PMC->PMC_PCER = PMC_PCER_INIT;

  /* Enable the master clock on the PKC0 clock out pin (PA_27_CLOCK_OUT) */
  AT91C_BASE_PMC->PMC_PCKR[0] = AT91C_PMC_CSS_SYS_CLK | AT91C_PMC_PRES_CLK;
  AT91C_BASE_PMC->PMC_SCER = AT91C_PMC_PCK0;

  /* Turn on the main oscillator and wait for it to start up */
  AT91C_BASE_PMC->PMC_MOR = PMC_MOR_INIT;
  while ( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MOSCXTS) );

  /* Assign main clock as crystal */
  AT91C_BASE_PMC->PMC_MOR |= (AT91C_CKGR_MOSCSEL | MOR_KEY);
  
  /* Initialize PLLA and wait for lock */
  AT91C_BASE_PMC->PMC_PLLAR = PMC_PLAAR_INIT;
  while ( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCKA) );
  
  /* Assign the PLLA as the main system clock with prescaler active using the sequence suggested on pg. 472 */
  AT91C_BASE_PMC->PMC_MCKR = PMC_MCKR_INIT;
  while ( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );
  AT91C_BASE_PMC->PMC_MCKR = PMC_MCKR_PLLA;
  while ( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );

  /* Initialize UTMI for USB usage */
  AT91C_BASE_CKGR->CKGR_UCKR |= (AT91C_CKGR_UPLLCOUNT & (3 << 20)) | AT91C_CKGR_UPLLEN;
  while ( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCKU) );
  
} /* end ClockSetup */


/*!---------------------------------------------------------------------------------------------------------------------
@fn void SystemSleep(void)

@brief Puts the system into sleep mode.

Right now, sleep mode is just a for loop that does nothing for 1ms.
So it's more like "lazy" mode...

Requires:
- Main clock is 48MHz

Promises:
- Roughly 1ms of time will be wasted

*/
void SystemSleep(void)
{    
  /* Set the sleep flag (which doesn't do anything yet) */
  G_u32SystemFlags |= _SYSTEM_SLEEPING;

  /* Kill 1ms worth of instructions.  At a clock rate of 48MHz, there
  are 48000000 clock ticks per second or 48,000 clock ticks per ms.
  The tighest loop we can write takes 4 clock ticks, so use 12,000 */
  for(u32 i = 0; i < 12000; i++);

  /* Clear the sleep flag */
  G_u32SystemFlags |= _SYSTEM_SLEEPING;
  
} /* end SystemSleep(void) */


/*!---------------------------------------------------------------------------------------------------------------------
@fn void WatchDogSetup(void)

@brief Configures the watchdog timer.  

The dog runs at 32kHz from the slow built-in RC clock source which varies 
over operating conditions from 20kHz to 44kHz.

Since the main loop time / sleep time should be 1 ms most of the time, choosing a value
of 5 seconds should be plenty to avoid watchdog resets.  

Note: the processor allows the WDMR register to be written just once.

Requires:
- SLCK is active at about 32kHz

Promises:
- Watchdog is set for 5 second timeout but not yet enabled

*/
void WatchDogSetup(void)
{
  AT91C_BASE_WDTC->WDTC_WDMR = WDT_MR_INIT;
  WATCHDOG_BONE();
 
} /* end WatchDogSetup() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn void GpioSetup(void)

@brief Loads registers required to set up GPIO on the processor.

Requires:
- All configurations must match connected hardware.

Promises:
- Output pin for PA31_HEARTBEAT is configured

*/
void GpioSetup(void)
{
  /* Set all of the pin function registers in port A */
  AT91C_BASE_PIOA->PIO_PER  = 0x00000001;
  AT91C_BASE_PIOA->PIO_PDR  = ~0x00000001;  
  
} /* end GpioSetup() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/


