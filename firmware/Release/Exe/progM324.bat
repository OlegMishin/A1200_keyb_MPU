@echo off
cls
rem set fuses to 8mhz internal RC generator
"e:\Program Files\avrdude\avrdude.exe" -P COM4 -p m324pb -c stk500v2 -e -U lfuse:w:0xe2:m
"e:\Program Files\avrdude\avrdude.exe" -P COM4 -p m324pb -c stk500v2 -e -U hfuse:w:0xd9:m
"e:\Program Files\avrdude\avrdude.exe" -P COM4 -p m324pb -c stk500v2 -e -U flash:w:".\A1200_keyb_mpu.hex"
rem pause
