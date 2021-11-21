// Delay module

void delay_us(unsigned int value)
{
  for(unsigned int i=0; i < value; i++)
  {
    asm("nop");
  }
}

void delay_ms(unsigned int value)
{
  for(unsigned int i=0; i < value; i++)
  {
    delay_us(1000);
  }
}
