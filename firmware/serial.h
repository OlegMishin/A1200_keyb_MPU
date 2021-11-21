
// Serial interface header
#ifndef _SERIAL_H_
#define _SERIAL_H_


//#define F_CPU   16000000

//#define FIFO_ENABLED

extern void USART_Init( unsigned long baudrate);
extern void puthex(unsigned int data);
extern void putdec(long data);
extern void putdec_r(long data);
extern void putdec_u16(unsigned int data);
extern void putbin_char(unsigned char data);
extern void putstr(char *s);
extern void putfloat(float f);
extern void puthexbyte(char data);
extern void put_char(char c);
extern int  getchar(void);
extern unsigned char get_char(void);
extern unsigned char char_received( void );
// FIFO functions
extern void serial_fifo_put(char data);
extern char serial_fifo_not_empty();
extern char serial_fifo_get();

#endif