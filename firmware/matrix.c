#include <string.h>
#include <ioavr.h>
#include "types.h"
#include "amiga_key.h"

#ifdef DEBUG   
#include "serial.h"
#endif
// Own include
#include "matrix.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
// Matrix definitions
#define MATRIX_ROW      7    //  Number of rows in the keyboard matrix (max. 8)
#define MATRIX_COL      15   //  Number of columns in the keyboard matrix (note, that there are max. 8 rows)
#define MATRIX_SAMPLE   2    //  Number of samples per key -- used for debouncing


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
//  Element of logic bit -- GPIO pin assignment tables


volatile unsigned char debug_byte = 0;


// Amiga 1200 keyboard connector pinout
// PIN:    1    2    3    4    5    6    7    8     9   10    11    12   13    14    15    16    17    18    19    20   21   22   23   24   25   26   27   28   29   30   31
// SIGNAL: ROW2 ROW3 ROW4 ROW5 ROW1 R.SH ROW0 R.ALT VCC R.Ami LCTRL L.SH L.ALT L.Ami COL14 COL13 COL12 COL11 COL10 COL9 COL8 COL7 COL6 COL5 COL4 COL3 COL2 COL1 COL0 GND  CAPSLED
// PORT:   PC4  PC5  PC6  PC7  PC3  PE5  PC2  PE6       PB0   PB1   PB2  PB3   PE4   PE3   PE2   PD5   PD4   PD3   PD2  PD1  PD0  PA0  PA1  PA2  PA3  PA4  PA5  PA6       PA7

   
//--------------------------------------------------------------------------------------------------------/
// Constants
//--------------------------------------------------------------------------------------------------------/
//!  Matrix rows -- there are max. 8 rows!
 const S_MATRIX_GPIO_DESC gcsKeyMatrixRows[ MATRIX_ROW ] =
{
  { &PORTC, &PINC, &DDRC, (1 << PORTC2) },   //  ROW0
  { &PORTC, &PINC, &DDRC, (1 << PORTC3) },   //  ROW1
  { &PORTC, &PINC, &DDRC, (1 << PORTC4) },   //  ROW2
  { &PORTC, &PINC, &DDRC, (1 << PORTC5) },   //  ROW3
  { &PORTC, &PINC, &DDRC, (1 << PORTB6) },   //  ROW4
  { &PORTC, &PINC, &DDRC, (1 << PORTC7) },   //  ROW5
  { &PORTB, &PINB, &DDRB, (1 << PORTB6) }    //  ROW6 (for dedicated buttons)
};

//!  Matrix columns
 const S_MATRIX_GPIO_DESC gcsKeyMatrixColumns[ MATRIX_COL ] =
{
  { &PORTA, &PINA, &DDRA, (1 << PORTA6) },   //  COL0
  { &PORTA, &PINA, &DDRA, (1 << PORTA5) },   //  COL1
  { &PORTA, &PINA, &DDRA, (1 << PORTA4) },   //  COL2
  { &PORTA, &PINA, &DDRA, (1 << PORTA3) },   //  COL3
  { &PORTA, &PINA, &DDRA, (1 << PORTA2) },   //  COL4
  { &PORTA, &PINA, &DDRA, (1 << PORTA1) },   //  COL5
  { &PORTA, &PINA, &DDRA, (1 << PORTA0) },   //  COL6
  { &PORTD, &PIND, &DDRD, (1 << PORTD0) },   //  COL7
#ifdef DEBUG      
  { &PORTE, &PINE, &DDRE, (1 << PORTE0) },   //  COL8  // ***** Debug!
#else
  { &PORTD, &PIND, &DDRD, (1 << PORTD1) },   //  COL8  // *****
#endif  
  { &PORTD, &PIND, &DDRD, (1 << PORTD2) },   //  COL9
  { &PORTD, &PIND, &DDRD, (1 << PORTD3) },   //  COL10
  { &PORTD, &PIND, &DDRD, (1 << PORTD4) },   //  COL11
  { &PORTD, &PIND, &DDRD, (1 << PORTD5) },   //  COL12
  { &PORTE, &PINE, &DDRE, (1 << PORTE2) },   //  COL13
  { &PORTE, &PINE, &DDRE, (1 << PORTE3) }    //  COL14
};

//  Matrix to scancode translation tables
//   Invalid keys are marked with 0xff
static const U8 gcau8ScanCodeTable[ MATRIX_ROW ][ MATRIX_COL ] =
{
//  COL0  COL1  COL2  COL3  COL4  COL5  COL6  COL7  COL8  COL9  COL10 COL11 COL12 COL13 COL14 
  { 0x5F, 0x59, 0x58, 0x57, 0x56, 0x5C, 0x55, 0x5B, 0x54, 0x53, 0x52, 0x51, 0x50, 0x5A, 0x45 }, // ROW0
  { 0x4C, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 }, // ROW1
  { 0x4F, 0x44, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10, 0x42 }, // ROW2
  { 0x4E, 0x46, 0x2B, 0x2A, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20, 0x62 }, // ROW3
  { 0x4D, 0x41, 0x40, 0x61u,0x3A, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30 }, // ROW4
  { 0x4A, 0x0F, 0x1D, 0x2D, 0x3D, 0x43, 0x1E, 0x2E, 0x3E, 0x3C, 0x1F, 0x2F, 0x3F, 0x5E, 0x5D }, // ROW5
  { 0x61, 0x65, 0x67, 0x63, 0x60, 0x64, 0x66, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }  // ROW6
};



//--------------------------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------------------------
U8 gau8KeyMatrixSample[ MATRIX_COL ][ MATRIX_SAMPLE ];  //  Used for sampling and debouncing (bitfield, 0 means pressed, 1 means not pressed)
U8 gau8KeyMatrixState[ MATRIX_COL ];                    //  Current state of the keys (bitfield, 0 means pressed, 1 means not pressed)
U8 gau8KeyEventPressed[ MATRIX_COL ];                   //  Press events of the keys (bitfield, 1 means press, 0 means no event)
U8 gau8KeyEventReleased[ MATRIX_COL ];                  //  Release events of the keys (bitfield, 1 means release, 0 means no event)

U8 u8Column = 0;
U8 u8Sample = 0;

//--------------------------------------------------------------------------------------------------------/
//  functions
//--------------------------------------------------------------------------------------------------------/

//    Sets the column of the matrix
//   Input: u8Column: value to set
 void SetColumn( U8 u8Column )
{
  U8 u8Index;
  
//   putstr("Col 0x"); puthexbyte(u8Column);putstr("\r"); // DEbug
          
  for( u8Index = 0; u8Index < MATRIX_COL; u8Index++ )  // Columns
  {
    if( u8Column == u8Index )
    {
       *(gcsKeyMatrixColumns[ u8Index ].psGPIOPort) &= ~(gcsKeyMatrixColumns[ u8Index ].ePin);
       *(gcsKeyMatrixColumns[ u8Index ].psGPIODIR) |= (gcsKeyMatrixColumns[ u8Index ].ePin);

    }
    else
    {
     *(gcsKeyMatrixColumns[ u8Index ].psGPIOPort) |= (gcsKeyMatrixColumns[ u8Index ].ePin);
     *(gcsKeyMatrixColumns[ u8Index ].psGPIODIR) &= ~(gcsKeyMatrixColumns[ u8Index ].ePin);
    }
  }
}



//   Module init
//   Must be called before Matrix_Cycle(), or the IT routine!
void Matrix_Init( void )
{
  U8 u8Index;
  
  // GPIO init
  for( u8Index = 0u; u8Index < MATRIX_ROW; u8Index++ )  // Rows
  {
    // GPIO_Init as input
              *(gcsKeyMatrixRows[ u8Index ].psGPIODIR) &= ~gcsKeyMatrixRows[ u8Index ].ePin;
// Enable pull-ups for debug              
              *(gcsKeyMatrixRows[ u8Index ].psGPIOPort) |= gcsKeyMatrixRows[ u8Index ].ePin;
  }
  for( u8Index = 0u; u8Index < MATRIX_COL; u8Index++ )  // Columns
  {
    // Init as output, high level.
    // GPIO_Init( gcsKeyMatrixColumns[ u8Index ].psGPIOPort, gcsKeyMatrixColumns[ u8Index ].ePin, GPIO_MODE_OUT_OD_LOW_FAST );
    *(gcsKeyMatrixColumns[ u8Index ].psGPIOPort) |= (gcsKeyMatrixColumns[ u8Index ].ePin);
    *(gcsKeyMatrixColumns[ u8Index ].psGPIODIR) &= ~(gcsKeyMatrixColumns[ u8Index ].ePin);
  }
  
  // Special keys init
  
  AMIGA_RSHIFT_DIR &= ~(1 << AMIGA_RSHIFT_PIN);
  AMIGA_RSHIFT_PORT |= (1 << AMIGA_RSHIFT_PIN);

  AMIGA_RALT_DIR &= ~(1 << AMIGA_RALT_PIN);
  AMIGA_RALT_PORT |= (1 << AMIGA_RALT_PIN);

  AMIGA_RAMI_DIR &= ~(1 << AMIGA_RAMI_PIN);
  AMIGA_RAMI_PORT |= (1 << AMIGA_RAMI_PIN);

  AMIGA_LCTRL_DIR &= ~(1 << AMIGA_LCTRL_PIN);
  AMIGA_LCTRL_PORT |= (1 << AMIGA_LCTRL_PIN);

  AMIGA_LSHIFT_DIR &= ~(1 << AMIGA_LSHIFT_PIN);
  AMIGA_LSHIFT_PORT |= (1 << AMIGA_LSHIFT_PIN);

  AMIGA_LALT_DIR &= ~(1 << AMIGA_LALT_PIN);
  AMIGA_LALT_PORT |= (1 << AMIGA_LALT_PIN);

  AMIGA_LAMI_DIR &= ~(1 << AMIGA_LAMI_PIN);
  AMIGA_LAMI_PORT |= (1 << AMIGA_LAMI_PIN);

// Globals init
  memset( (void*)gau8KeyMatrixSample,  0xFFu, sizeof( gau8KeyMatrixSample ) );
  memset( (void*)gau8KeyMatrixState,   0xFFu, sizeof( gau8KeyMatrixState ) );
  memset( (void*)gau8KeyEventPressed,  0x00u, sizeof( gau8KeyEventPressed ) );
  memset( (void*)gau8KeyEventReleased, 0x00u, sizeof( gau8KeyEventReleased ) );
}

//  Main cycle
//  Must be called from main cycle!
void Matrix_Cycle( void )
{
  U8 u8Row, u8Column;
  U8 u8ScanCode;

  // note: this cycle can be blocked by AmigaKey_Cycle()
  
  // search for new events
  for( u8Column = 0u; u8Column < MATRIX_COL; u8Column++ )
  {
    for( u8Row = 0u; u8Row < MATRIX_ROW; u8Row++ )
    {
      if(  (1u<<u8Row) & gau8KeyEventPressed[ u8Column ] )  // if there is a press event
      {
        u8ScanCode = gcau8ScanCodeTable[ u8Row ][ u8Column ];  // translating the matrix code to scancode
        
//        putstr("PR[R/C]="); puthexbyte(u8Row);putstr("/");puthexbyte(u8Column);putstr("\r");
//        putstr("[Press] Scan code :  0x"); puthexbyte(u8ScanCode);putstr("\r"); // DEbug
        
        if( TRUE == AmigaKey_RegisterScanCode( u8ScanCode, TRUE ) )
        {
          gau8KeyEventPressed[ u8Column ] &= ~(1<<u8Row);
        }
        else  // scancode buffer full, terminate cycle
        {
          u8Row = MATRIX_ROW;
          u8Column = MATRIX_COL;
        }
      }
      else if( 0u != ( (1u<<u8Row) & gau8KeyEventReleased[ u8Column ] ) )  // if there is a release event
      {
        u8ScanCode = gcau8ScanCodeTable[ u8Row ][ u8Column ];  // translating the matrix code to scancode
        
//        putstr("[Release] Matrix [ROW/COL] =  0x"); puthexbyte(u8Row);putstr("/0x");puthexbyte(u8Column);putstr("\r");
//        putstr("[Release] Scan code :  0x"); puthexbyte(u8ScanCode);putstr("\r"); // DEbug
        
        if( AmigaKey_RegisterScanCode( u8ScanCode, FALSE ) )
        {
          gau8KeyEventReleased[ u8Column ] &= ~(1<<u8Row);
        }
        else  // scancode buffer full, terminate cycle
        {
          u8Row = MATRIX_ROW;
          u8Column = MATRIX_COL;
        }

      }
    }
  }
}

//  Sample the keys and generate events
//  Must be called from timed IT routine!
void Matrix_Sample( void )
{
  U8 u8Index;
  U8 u8Row;
  
  // read row pins
  u8Row = 0xff;
    for( u8Index = 0u; u8Index < MATRIX_ROW; u8Index++ )
    {
      if(u8Index <  (MATRIX_ROW-1))
      {
        if ((*gcsKeyMatrixRows[ u8Index ].psGPIOPIN & gcsKeyMatrixRows[ u8Index ].ePin) == 0) u8Row &= ~(1 <<u8Index);
          else u8Row |= (1 <<u8Index);
          
      }
       else
         { // Special (separated) keys are mapped to ROW6
           switch (u8Column)
           {
           case 0: if((AMIGA_RSHIFT_INPORT & (1 << AMIGA_RSHIFT_PIN)) == 0 ) u8Row &= ~(1 <<u8Index);
            break;
           case 1: if((AMIGA_RALT_INPORT & (1 << AMIGA_RALT_PIN)) == 0 ) u8Row &= ~(1 <<u8Index);
            break;
           case 2: if((AMIGA_RAMI_INPORT & (1 << AMIGA_RAMI_PIN)) == 0 ) u8Row &= ~(1 <<u8Index);
            break;
           case 3: if((AMIGA_LCTRL_INPORT & (1 << AMIGA_LCTRL_PIN)) == 0 ) u8Row &= ~(1 <<u8Index);
            break;
           case 4: if((AMIGA_LSHIFT_INPORT & (1 << AMIGA_LSHIFT_PIN)) == 0 ) u8Row &= ~(1 <<u8Index);
            break;
           case 5: if((AMIGA_LALT_INPORT & (1 << AMIGA_LALT_PIN)) == 0 ) u8Row &= ~(1 <<u8Index);
            break;
           case 6: if((AMIGA_LAMI_INPORT & (1 << AMIGA_LAMI_PIN)) == 0 ) u8Row &= ~(1 <<u8Index);
            break;
           default:
             break;
           }
         }
  }

  debug_byte =  u8Row; 

  // store sampled value of rows
  gau8KeyMatrixSample[ u8Column ][ u8Sample ] = u8Row;

  // generate actual state of keys based on samples
  u8Row = 0u;
  for( u8Index = 0u; u8Index < MATRIX_SAMPLE; u8Index++ )
  {
    u8Row |= gau8KeyMatrixSample[ u8Column ][ u8Index ];  // key presses are registered immediately, releases will be registered after MATRIX_SAMPLE times of the sampling period
  }
  
  // search for events
  gau8KeyEventPressed[ u8Column ]  |= ( gau8KeyMatrixState[ u8Column ] ^ u8Row ) & ~u8Row;
  gau8KeyEventReleased[ u8Column ] |= ( gau8KeyMatrixState[ u8Column ] ^ u8Row ) & u8Row;
  
  // the new state will be the one after the events
  gau8KeyMatrixState[ u8Column ] &= ~gau8KeyEventPressed[ u8Column ];
  gau8KeyMatrixState[ u8Column ] |=  gau8KeyEventReleased[ u8Column ];
  
  // Next column, and next sample
  u8Column++;
  if(u8Column == MATRIX_COL)
  {
    u8Column = 0;
    u8Sample++;
    if( MATRIX_SAMPLE == u8Sample )
    {
      u8Sample = 0;
    }
  }
  
//    putstr("Send 0x"); puthexbyte(u8Scancode); putstr("\r"); // Debug
  
  // look for special key combinations, eg. CTRL + LAmiga + RAmiga
  
  if( ( 0u == ( gau8KeyMatrixState[ 6 ] & (1<<6) ) )    // ROW6 + COL6 = LAmiga
   && ( 0u == ( gau8KeyMatrixState[  2 ] & (1<<6) ) )    // ROW6 + COL2  = RAmiga
   && ( 0u == ( gau8KeyMatrixState[ 3 ] & (1<<6) ) ) )  // ROW6 + COL3 = Ctrl
  {
    AmigaKey_Reset();
  }
  
  // Increment MUX state
  SetColumn( u8Column );
}
 
