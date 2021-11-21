

// Serial module 

#include <ioavr.h>
#include "serial.h"

#define F_CPU   8000000

#ifdef FIFO_ENABLED

#define SERIAL_FIFO_SIZE        128
char serial_fifo[SERIAL_FIFO_SIZE];
static char serial_fifo_head = 0;
static char serial_fifo_tail = 0;

void serial_fifo_put(char data)
{
  serial_fifo[serial_fifo_head] = data;
  if(serial_fifo_head + 1 >= SERIAL_FIFO_SIZE)
			serial_fifo_head = 0;
		else
			serial_fifo_head++;
		if(serial_fifo_tail == serial_fifo_head)
		{  // ошибка - FIFO переполнился
		}
}

char serial_fifo_not_empty()
{
  //возвращает нулевое значение когда пуст FIFO
 return (serial_fifo_tail != serial_fifo_head);
}

char serial_fifo_get()
{
  char r = serial_fifo[serial_fifo_tail];
  if(serial_fifo_tail + 1>= SERIAL_FIFO_SIZE)
    {
	serial_fifo_tail = 0;
    }
   else serial_fifo_tail++;
  return r;
}
#endif

void USART_Init( unsigned long baudrate)
{
unsigned long baud =  F_CPU / (16*baudrate) - 1;// baud rate 500000 baud FOSC / (16*baud_rate) - 1;

DDRD |= (1 << PORTD1); // Configure TX pin as output
 
UBRR0H = (unsigned char)(baud>>8);
UBRR0L = (unsigned char)baud;

//UCSRA |= (1 << U2X);
// Enable  transmitter 
UCSR0B = (1<<TXEN);
// Set frame format: 8data, 1stop bit 
UCSR0C = (3<<UCSZ0);

}




int putchar(int c)
{
 #ifdef __ATmega328P__
   // Wait for empty transmit buffer 
   while ( !( UCSR0A & (1<<UDRE0)) );
   // Put data into buffer, sends the data 
   UDR0 = c;
#else
   // Wait for empty transmit buffer 
   while ( !( UCSR0A & (1<<UDRE)) );
   // Put data into buffer, sends the data 
   UDR0 = c;

#endif   
   return c;
}

void put_char(char c)
{
 #ifdef __ATmega328P__
   // Wait for empty transmit buffer 
   while ( !( UCSR0A & (1<<UDRE0)) ) asm("nop");
   // Put data into buffer, sends the data 
   UDR0 = c;
#else
   // Wait for empty transmit buffer 
   while ( !( UCSR0A & (1<<UDRE)) );
   // Put data into buffer, sends the data 
   UDR0 = c;

#endif   
}


int getchar(void)
{
#ifdef __ATmega328P__
 if (!(UCSR0A & (1<<RXC0))) return (-1);
  else return (UDR0);
 
#else
 if (!(UCSR0A & (1<<RXC))) return (-1);
  else return (UDR0);
#endif  
}

unsigned char get_char( void )
{
#ifdef __ATmega328P__
while ( !(UCSR0A & (1<<RXC0)) );
/* Get and return received data from buffer */
return UDR0;
#else
/* Wait for data to be received */
while ( !(UCSR0A & (1<<RXC)) );
/* Get and return received data from buffer */
return UDR0;
#endif
}

unsigned char char_received( void ) // return 1 - receiver, 0 - NOT received
{
#ifdef __ATmega328P__
/* Wait for data to be received */
 unsigned char r = (UCSR0A & (1<<RXC0));
#else
/* Wait for data to be received */
 unsigned char r = (UCSR0A & (1<<RXC));
#endif 
 return r;
}

void puthex(unsigned int data)
{
 char ch;
  ch = (data >> 12) & 0x0f;
  if (ch > 9){ch += 7;};
  put_char(ch + '0');
  
  ch = (data >> 8) & 0x0f;
  if (ch > 9){ch += 7;};
  put_char(ch + '0');
  
  ch = (data >> 4) & 0x0f;
  if (ch > 9){ch += 7;};
  put_char(ch + '0');
  
  ch = data & 0x0f;
  if (ch > 9){ch += 7;};
  put_char(ch + '0');
}

void puthexbyte(char data)
{
 char ch;
  ch = (data >> 4) & 0x0f;
  if (ch > 9){ch += 7;};
  put_char(ch + '0');
  
  ch = data & 0x0f;
  if (ch > 9){ch += 7;};
  put_char(ch + '0');
}


void putdec(long data)
{          
  char c,flag;
  
  if (data < 0)
  {
    put_char('-');  
    data = - data;
  } else put_char(' ');  
  flag = 0;
  c = (data/10000) % 10;
  flag |=c; 
  if (flag) put_char(c + '0');  
  c = (data/1000) % 10;            
  flag |=c; 
  if (flag) put_char(c + '0');  
  c = (data/100) % 10;            
  flag |=c; 
  if (flag) put_char(c + '0');  
  c = (data/10) % 10;            
  flag |=c; 
  if (flag) put_char(c + '0');  
  c = data % 10;            
  put_char(c + '0');  
}

void putdec_r(long data)
{          
  char c,flag;
  
  if (data < 0)
  {
    put_char('-');  
    data = - data;
  }
  flag = 0; 
  c = (data/10000) % 10;
  flag |=c; 
  if (flag) put_char(c + '0'); else put_char(' ');  
  c = (data/1000) % 10;            
  flag |=c; 
  if (flag) put_char(c + '0'); else put_char(' ');  
  c = (data/100) % 10;            
  flag |=c; 
  if (flag) put_char(c + '0'); else put_char(' ');
  c = (data/10) % 10;            
  flag |=c; 
  if (flag) put_char(c + '0'); else put_char(' ');  
  c = data % 10;            
  put_char(c + '0');  
}

void putbin_char(unsigned char data)
{
 char i;
 
 for(i=8; i>0; i--){
   if(data&(1<<(i-1))){put_char('1');} else put_char('0');
 }
 put_char('b');
 
}

void putstr(char *s)
	{
	while (*s)
		{
		put_char(*s);
		s++;
		}
	}

void putfloat(float f)
{                      
 float ftmp;
// unsigned int i;
 unsigned long data;
               
  if (f < 0)
   {
    put_char('-');
    f = - f;
   } 
  putdec((long)f);
  put_char('.');
 
  ftmp = (f - (long)f) * 1000;
  data = (unsigned long)ftmp;
  put_char((data/100) % 10 + '0');
  put_char((data/10) % 10 + '0');
  put_char(data % 10 + '0');

}

void putdec_u16(unsigned int data)
{          
  char c,flag;
  
  flag = 0;
  c = (char)((data/10000) % 10);
  flag |=c; 
  if (flag) put_char((char)(c + '0'));// else put_char(' ');  
  c =(char)( (data/1000) % 10);            
  flag |=c; 
  if (flag) put_char((char)(c + '0'));// else put_char(' ');   
  c = (char)((data/100) % 10);            
  flag |=c; 
  if (flag) put_char((char)(c + '0'));// else put_char(' ');    
  c =  (char)((data/10) % 10);            
  flag |=c; 
  if (flag) put_char((char)(c + '0'));// else put_char(' ');    
  c =  (char)(data % 10);            
 put_char((char)(c + '0'));   
}


