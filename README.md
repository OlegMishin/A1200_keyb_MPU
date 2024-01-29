# A1200_keyb_MPU
Amiga 1200 keyboard MPU drop-in replacement pcb


As the 68HC05 (p/n 391508-01) used in the Amiga 1200 is getting to be very expensive, I designed a drop-in replacement pcb using Atmel (Microchip) MPU - ATMEGA324PB-20MU. 

Advantages:
- No crystal required
- No Voltage monitor IC required
- SW can be updated to support an external keyboard with other protocol. Connected e.g. to the unsoldered crystal's pins.

How it looks:

![изображение](https://user-images.githubusercontent.com/81614352/142762803-39e37363-bc13-4729-8000-2b5181df1634.png)

 
# Firmware:
 
 Designed with "IAR EW for Microchip AVR 7.30". 

 To download the hex code an STK500 can be used with avrdude tool. A batch file for avrdude is included in "firmware\release\exe" folder as well as in "\debug\exe"). 
 STK500 to the pcb connection is shown below:
 ![изображение](https://user-images.githubusercontent.com/81614352/142762608-4e24c24a-dde0-4b1f-9b20-feb42675fe86.png)

 
# PCB:
 
 To fit the MPU's footprint it shall be made with "castellated holes" technology, which is expensive. So I made a simple board with normal holes. This board needs to be cutted to fit the footprint as shown below.
 
 ![изображение](https://user-images.githubusercontent.com/81614352/142762503-699979e4-981a-40ae-a84a-f004c9e2f14c.png)

 Both gerber files included in the "PCB" folder:
 1. A1200_keyb_mpu_olm_normal_Gerber.zip - Normal pcb, to be trimmed.
 2. A1200_keyb_mpu_olm_cast_holes_Gerber.zip - Ready to be produced with "castellated holes" technology.

 Passive components are 0603(imperial) size.
 
 The pcb is not compatible with a PLCC socket. Must be soldered to Amiga's pcb.
 
 
 
 
