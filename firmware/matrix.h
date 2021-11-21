
#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED


typedef struct
{
  unsigned char volatile __io * psGPIOPort;
  unsigned char volatile __io * psGPIOPIN;
  unsigned char volatile __io * psGPIODIR;
  uint8_t  ePin;
} S_MATRIX_GPIO_DESC;

void Matrix_Init( void );
void Matrix_Cycle( void );
void Matrix_Sample( void );
void SetColumn( U8 u8Column );


#endif // MATRIX_H_INCLUDED

