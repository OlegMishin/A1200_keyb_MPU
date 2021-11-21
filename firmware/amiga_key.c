#include <string.h>
#include <ioavr.h>
#include "types.h"
#include "delay.h"
#include <intrinsics.h>
#ifdef DEBUG   
#include "serial.h"
#endif

// Own include
#include "amiga_key.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/

#define SCANCODE_FIFO_SIZE        20u  // Buffer for outgoing scancodes (max. 256)
#define TIMEOUT_US            143000u  // Timeout for the ACK from computer

#define AMIGA_RESET_WARNING     0x78u  // Reset warning, sent before reset
#define AMIGA_LAST_KEYCODE_BAD  0xF9u  // Last keycode was bad, retransmitting
#define AMIGA_KEYBUFFER_FULL    0xFAu  // Keycode buffer was full
#define AMIGA_SELFTEST_FAILED   0xFCu  // Self-test failed in keyboard controller
#define AMIGA_INIT_KEYSTREAM    0xFDu  // Sent after initialization, marks the initition of power-up key stream
#define AMIGA_TERM_KEYSTREAM    0xFEu  // Sent after initial key stream, marks the termination of key stream

// Scancode buffer -- stores scancodes waiting to be sent
struct
{
  U8 au8ScancodeBuffer[ SCANCODE_FIFO_SIZE ];  // Scancodes are stored here
  U8 u8ProduceIndex;                           // The index where the new scancode will be written to
  U8 u8ConsumeIndex;                           // The index where the scancode will be read from
} gsScancodeFIFO;

BOOL gbIsSynchronized;  // Is the communication with the Amiga computer synchronized?
BOOL gbReTransmit;      // Is getting out-of-sync happened when transmitting a character?
BOOL gbIsCapsLockOn;    // State of the Caps Lock key
BOOL gbPowerOn;         // Power on indication to skip leading pulse


//--------------------------------------------------------------------------------------------------------/
// Static functions
//--------------------------------------------------------------------------------------------------------/
// Set data line
void keyb_data_set(void)
{
     AMIGA_DAT_DIR  &= ~(1 << AMIGA_DAT_PIN );
     AMIGA_DAT_PORT |= (1 << AMIGA_DAT_PIN );
}

// Clear data line
void keyb_data_clr(void)
{
     AMIGA_DAT_PORT &= ~(1 << AMIGA_DAT_PIN );
     AMIGA_DAT_DIR  |= (1 << AMIGA_DAT_PIN );
}

// Set clock line
void keyb_clk_set(void)
{
     AMIGA_CLK_DIR  &= ~(1 << AMIGA_CLK_PIN );
     AMIGA_CLK_PORT |= (1 << AMIGA_CLK_PIN );
}
// Clear clock line
void keyb_clk_clr(void)
{
     AMIGA_CLK_DIR |= (1 << AMIGA_CLK_PIN );
     AMIGA_CLK_PORT  &= ~(1 << AMIGA_CLK_PIN );
}

// Set reset line
void keyb_reset_set(void)
{
//     AMIGA_RST_DIR  &= ~(1 << AMIGA_RST_PIN );
     AMIGA_RST_PORT |= (1 << AMIGA_RST_PIN );
}
// Clear reset line
void keyb_reset_clr(void)
{
//     AMIGA_RST_DIR |= (1 << AMIGA_RST_PIN );
     AMIGA_RST_PORT  &= ~(1 << AMIGA_RST_PIN );
}



/*! *******************************************************************
 *    Reads the oldest element in the FIFO
 * Input:  pu8ScanCode: the element will be put here
 * Return: TRUE, if success; FALSE, if FIFO is empty
 *    This function does not remove elements from the FIFO!
 *********************************************************************/
BOOL ReadScancodeFIFO( U8* pu8ScanCode )
{
  U8   u8NewIndex;
  BOOL bRet = FALSE;
  
  if( gsScancodeFIFO.u8ProduceIndex != gsScancodeFIFO.u8ConsumeIndex )  // if there is something in the FIFO
  {
    // calculate next consume index
    u8NewIndex = gsScancodeFIFO.u8ConsumeIndex + 1u;
    if( u8NewIndex >= SCANCODE_FIFO_SIZE )
    {
      u8NewIndex = 0u;
    }
    
    // send the scancode back to caller
    *pu8ScanCode = gsScancodeFIFO.au8ScancodeBuffer[ gsScancodeFIFO.u8ConsumeIndex ];
    bRet = TRUE;
  }
  return bRet;
}

/*! *******************************************************************
 *    Reads the oldest element in the FIFO
 * Return: TRUE, if success; FALSE, if FIFO is empty
 *********************************************************************/
BOOL RemoveElementFromScancodeFIFO( void )
{
  U8   u8NewIndex;
  BOOL bRet = FALSE;
  
  if( gsScancodeFIFO.u8ProduceIndex != gsScancodeFIFO.u8ConsumeIndex )  // if there is something in the FIFO
  {
    // calculate next consume index
    u8NewIndex = gsScancodeFIFO.u8ConsumeIndex + 1u;
    if( u8NewIndex >= SCANCODE_FIFO_SIZE )
    {
      u8NewIndex = 0u;
    }
    
    gsScancodeFIFO.u8ConsumeIndex = u8NewIndex;
    bRet = TRUE;
  }
  return bRet;
}

/*! *******************************************************************
 *    Flush the scancode FIFO
 *    -
 * Return: -
 *********************************************************************/
void FlushScancodeFIFO( void )
{
  gsScancodeFIFO.u8ProduceIndex = 0u;
  gsScancodeFIFO.u8ConsumeIndex = 0u;
}

/********************************************************************
 *    Synchronizes the communication with the Amiga computer
 *    Blocking function!
 *********************************************************************/
void SynchronizeCommunication( void )
{
  U32 u32Wait;

  while( gbIsSynchronized  == FALSE)
  {
//     putstr(" pulse ");
    keyb_data_clr();
    delay_us( 20u );
    keyb_clk_clr();
    delay_us( 20u );
    keyb_clk_set();
    delay_us( 20u );
    keyb_data_set();
    
    // Wait for High to LOW transition
    for( u32Wait = 0u; u32Wait < TIMEOUT_US/10; u32Wait++ )
    {
      if( (AMIGA_DAT_INPORT & ( 1<< AMIGA_DAT_PIN )) == 0 )
      {
        gbIsSynchronized = TRUE;
        u32Wait = TIMEOUT_US;
      }
      delay_us(8);
    }
  }
//  putstr(" H->L ");
  // If HIGH-> LOW happened, wait for LOW - > HIGH
  gbIsSynchronized = FALSE;
  for( u32Wait = 0u; u32Wait < TIMEOUT_US/10; u32Wait++ )
    {
      if( (AMIGA_DAT_INPORT & ( 1<< AMIGA_DAT_PIN )))
      {
        gbIsSynchronized = TRUE;
        u32Wait = TIMEOUT_US;
      }
      delay_us(8);
    }

//  putstr(" L-H  ");
  
  
  
  // Update the state of the Caps lock LED
  
  if(gbIsCapsLockOn )
  {
    AMIGA_CAPSLED_PORT &= ~(1 << AMIGA_CAPSLED_PIN );
  }
  else
  {
    AMIGA_CAPSLED_PORT |= (1 << AMIGA_CAPSLED_PIN );
  }

}

/********************************************************************
 *    Sends the scancode to the Amiga computer
 * Input:   u8Scancode: the scancode itself
 * Return: TRUE, if success; FALSE, if timeout occured
 *    Blocking function!
 *********************************************************************/
BOOL SendScancode( U8 u8Scancode, BOOL SkipStart )
{
  U32 u32Wait;
  BOOL bRet = FALSE;
  U8 u8Index;
  
  for( u32Wait = 0; u32Wait < TIMEOUT_US/10; u32Wait++ )
  {
    delay_us(8);
    if( AMIGA_DAT_INPORT & (1<< AMIGA_DAT_PIN ))  // waiting for the data pin to get high
    {
      u32Wait = TIMEOUT_US;
      delay_us(80);
      bRet = TRUE;
    }
  }
  
  // if the data pin got released
  if( bRet )
  {
    asm("cli");
    // pulse the data line before sending
    if(SkipStart == 0)
    {
    keyb_data_clr();
    delay_us( 17 );
    keyb_data_set();
    }
    delay_us( 103 );  
    
    for( u8Index = 0; u8Index < 8; u8Index++ )
    {
      if( (u8Scancode & (128u>>u8Index)) ==0 )  // NOTE: data line is inverted!
        keyb_data_set();
           else keyb_data_clr();
      delay_us( 17 );
      keyb_clk_clr();
      delay_us( 17 );
      keyb_clk_set();
      delay_us( 17 );
    }
    keyb_data_set();
    asm("nop"); asm("nop");

    bRet = FALSE;
    for( u32Wait = 0u; u32Wait < TIMEOUT_US/10; u32Wait++ )
    {
      delay_us(8); 
      if( (AMIGA_DAT_INPORT & (1<< AMIGA_DAT_PIN )) == 0)
      {
        u32Wait = TIMEOUT_US;
        bRet = TRUE;
      }
    }
#ifdef DEBUG      
    if(bRet == 0) 
    {
      PORTE |= (1<< PORTE1);
      putstr("*TO2\r");
      
      delay_us(100);
      PORTE &= ~(1<< PORTE1);
    }
#endif
    
    asm("sei");
  } 
#ifdef DEBUG      
  else 
  {
     PORTE |= (1<< PORTE1);
     putstr("*TO1\r");
      delay_us(100);
      PORTE &= ~(1<< PORTE1);
  }
#endif
  return bRet;
}


//--------------------------------------------------------------------------------------------------------/
// Public functions
//--------------------------------------------------------------------------------------------------------/
//  Module init
//  Must be called before AmigaKey_Cycle(), or the IT routine!
//
void AmigaKey_Init( void )
{
  // GPIO init
  
  // Debug
  PORTE &= ~(1<< PORTE1);
  DDRE |=  (1<< PORTE1);

  AMIGA_CLK_DIR |=  (1 << AMIGA_CLK_PIN);
  AMIGA_CLK_PORT |= (1 << AMIGA_CLK_PIN);
  

  AMIGA_DAT_DIR &= ~(1 << AMIGA_DAT_PIN);  
  AMIGA_DAT_PORT |= (1 << AMIGA_DAT_PIN);
  
  // Start with RESET active (low)
  AMIGA_RST_DIR |=  (1 << AMIGA_RST_PIN); 
  AMIGA_RST_PORT &=~(1 << AMIGA_RST_PIN);
  
  AMIGA_CAPSLED_DIR   |= (1 << AMIGA_CAPSLED_PIN); 
  // Switching on the Caps lock LED
  AMIGA_CAPSLED_PORT &= ~(1<< AMIGA_CAPSLED_PIN );
  

  // Global variables init
  memset( (void*)&gsScancodeFIFO, 0x00u, sizeof( gsScancodeFIFO ) );
  gbIsSynchronized = FALSE;  // this way the controller will start communication by synchronizing first
  gbReTransmit = FALSE;
  gbIsCapsLockOn = FALSE;
  gbPowerOn = TRUE;
  
  // Standard initialization sequence -- 0xFD, 0xFE --> note: synchronization will be performed before sending any of these
  AmigaKey_RegisterScanCode( AMIGA_INIT_KEYSTREAM, FALSE );
  AmigaKey_RegisterScanCode( AMIGA_TERM_KEYSTREAM, FALSE );
  delay_us(500);
  // release RESET
  keyb_reset_set();
}

/********************************************************************
 *    Main cycle
 *    Must be called from main cycle!
 *********************************************************************/
void AmigaKey_Cycle( void )
{
  U8 u8Scancode;

  // If the keyboard is out-of-sync, then resynchronize
  if(gbIsSynchronized != TRUE)
  {
//    putstr("Sync..."); // Debug
    while( AMIGA_DAT_INPORT & (1<< AMIGA_DAT_PIN ) == 0) asm("nop");  // wait for the data pin to reach 1
    SynchronizeCommunication();  // blocking call!
//    putstr("OK\r"); // Debug
    
    if(gbReTransmit)
    {
 //     putstr("Send LAST_CODE_IS_BAD\r"); 

      gbIsSynchronized = SendScancode( (U8)((AMIGA_LAST_KEYCODE_BAD<<1u) | 0x01), gbPowerOn );  // so the computer knows, that the last scancode was bad
    }
  }

  // Sending scancodes
  if( ReadScancodeFIFO( &u8Scancode ) )
  {
//    putstr("Send 0x"); puthexbyte(u8Scancode); putstr("\r"); // Debug
    
    if(SendScancode( u8Scancode, gbPowerOn ))  // if the send succeeded
    {
      RemoveElementFromScancodeFIFO();  //TODO: if this function returns with FALSE, then there is a huge error in somewhere...
      if(u8Scancode == AMIGA_TERM_KEYSTREAM) gbPowerOn = FALSE;
    }
    else
    {
      gbIsSynchronized = FALSE;
      gbReTransmit = TRUE;
    }
  }
}

/********************************************************************
 *    Put scancode in out FIFO
 * Input: u8Code: scancode to send
 *        bIsPressed: TRUE, if button is pressed; FALSE, if it's released
 * Return: TRUE, if success; FALSE, if output FIFO is full
 *********************************************************************/
BOOL AmigaKey_RegisterScanCode( U8 u8Code, BOOL bIsPressed )
{
  U8   u8NewIndex;
  BOOL bRet = FALSE;
  
  // Caps lock key is special: only pressed events will be sent
  if( u8Code == 0x62)
  {
    if( TRUE == bIsPressed )
    {
      gbIsCapsLockOn = (TRUE == gbIsCapsLockOn) ? FALSE : TRUE;
      bIsPressed = gbIsCapsLockOn;
      
      // Switch the LED on or off
      if(gbIsCapsLockOn )
           AMIGA_CAPSLED_PORT &= ~(1<<AMIGA_CAPSLED_PIN);
      else AMIGA_CAPSLED_PORT |=  (1<<AMIGA_CAPSLED_PIN);
    }
    else
      return TRUE;  // discard the release event
  }
  
  // calculate next produce index
  u8NewIndex = gsScancodeFIFO.u8ProduceIndex + 1u;
  if( u8NewIndex >= SCANCODE_FIFO_SIZE )
  {
    u8NewIndex = 0u;
  }
  
  if( u8NewIndex != gsScancodeFIFO.u8ConsumeIndex )  // if the FIFO is not full yet
  {
    // add the new element to the FIFO
    gsScancodeFIFO.au8ScancodeBuffer[ gsScancodeFIFO.u8ProduceIndex ] = ( u8Code << 1u ) | ( TRUE == bIsPressed ? 0u : 1u );  // Amiga communication format
    gsScancodeFIFO.u8ProduceIndex = u8NewIndex;
    bRet = TRUE;
  }
  
  return bRet;
}

/********************************************************************
 *    Send reset signal to the computer
 *    There can be a delay between this function call (--> reset warning) and the actual reset. This function is blocking.
 *********************************************************************/
void AmigaKey_Reset( void )
{
   
  FlushScancodeFIFO();
  //TODO: send reset warning, wait and pull the reset line
#ifdef DEBUG      
  putstr("RESET\r");
#endif
  // Pull the reset line
  keyb_reset_clr();
  
  // Wait for at least 500 ms
    delay_ms(500);

  //TODO: wait for releasing Ctrl+LAmiga+RAmiga
  
  // Release the reset line
//  keyb_reset_set(); - will be released after restart

  // Reset self -- there is no soft reset instruction on STM8, so unconditional jump will be used
/*  disableInterrupts();
  __asm( "JPF $008000" );  //TODO: is the reset vector a jump instruction??
  */
  
  __disable_interrupt(); // Disable interrupt to stop WD reset
  while(1) asm("nop");  // Wait until WD timeout
}

