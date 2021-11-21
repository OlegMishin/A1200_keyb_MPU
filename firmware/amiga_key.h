#ifndef AMIGA_KEY_H_INCLUDED
#define AMIGA_KEY_H_INCLUDED

#define AMIGA_CLK_PORT     (PORTC)
#define AMIGA_CLK_DIR      (DDRC)
#define AMIGA_CLK_PIN      (PORTC1)

#define AMIGA_DAT_PORT     (PORTC)
#define AMIGA_DAT_DIR      (DDRC)
#define AMIGA_DAT_INPORT   (PINC)
#define AMIGA_DAT_PIN      (PORTC0)

#define AMIGA_RST_PORT     (PORTB)
#define AMIGA_RST_DIR      (DDRB)
#define AMIGA_RST_PIN      (PORTB4)

#define AMIGA_CAPSLED_PORT (PORTA)
#define AMIGA_CAPSLED_DIR  (DDRA)
#define AMIGA_CAPSLED_INPORT (PINA)
#define AMIGA_CAPSLED_PIN  (PORTA7)

// Special keys

#define AMIGA_RSHIFT_PORT     (PORTE)
#define AMIGA_RSHIFT_INPORT   (PINE)
#define AMIGA_RSHIFT_DIR      (DDRE)
#define AMIGA_RSHIFT_PIN      (PORTE5)

#define AMIGA_RALT_PORT       (PORTE)
#define AMIGA_RALT_INPORT     (PINE)
#define AMIGA_RALT_DIR        (DDRE)
#define AMIGA_RALT_PIN        (PORTE6)

#define AMIGA_RAMI_PORT       (PORTB)
#define AMIGA_RAMI_INPORT     (PINB)
#define AMIGA_RAMI_DIR        (DDRB)
#define AMIGA_RAMI_PIN        (PORTB0)

#define AMIGA_LCTRL_PORT      (PORTB)
#define AMIGA_LCTRL_INPORT    (PINB)
#define AMIGA_LCTRL_DIR       (DDRB)
#define AMIGA_LCTRL_PIN       (PORTB1)

#define AMIGA_LSHIFT_PORT     (PORTB)
#define AMIGA_LSHIFT_INPORT   (PINB)
#define AMIGA_LSHIFT_DIR      (DDRB)
#define AMIGA_LSHIFT_PIN      (PORTB2)

#define AMIGA_LALT_PORT       (PORTB)
#define AMIGA_LALT_INPORT     (PINB)
#define AMIGA_LALT_DIR        (DDRB)
#define AMIGA_LALT_PIN        (PORTB3)

#define AMIGA_LAMI_PORT       (PORTE)
#define AMIGA_LAMI_INPORT     (PINE)
#define AMIGA_LAMI_DIR        (DDRE)
#define AMIGA_LAMI_PIN        (PORTE4)


void AmigaKey_Init( void );
void AmigaKey_Cycle( void );
BOOL AmigaKey_RegisterScanCode( U8 u8Code, BOOL bIsPressed );
void AmigaKey_Reset( void );

#endif // AMIGA_KEY_H_INCLUDED
