/*
TV-B-Gone Firmware version 1.2
for use with ATtiny85v and v1.2 hardware
(c) Mitch Altman + Limor Fried 2009
Last edits, August 16 2009

With some code from:
Kevin Timmerman & Damien Good 7-Dec-07

Distributed under Creative Commons 2.5 -- Attib & Share Alike

This is the 'universal' code designed for v1.2 - it will select EU or NA
depending on a pulldown resistor on pin B1 !
*/

#include <avr/io.h>             // this contains all the IO port definitions
#include "main.h"


/*
This project transmits a bunch of TV POWER codes, one right after the other, 
with a pause in between each.  (To have a visible indication that it is 
transmitting, it also pulses a visible LED once each time a POWER code is 
transmitted.)  That is all TV-B-Gone does.  The tricky part of TV-B-Gone 
was collecting all of the POWER codes, and getting rid of the duplicates and 
near-duplicates (because if there is a duplicate, then one POWER code will 
turn a TV off, and the duplicate will turn it on again (which we certainly 
do not want).  I have compiled the most popular codes with the 
duplicates eliminated, both for North America (which is the same as Asia, as 
far as POWER codes are concerned -- even though much of Asia USES PAL video) 
and for Europe (which works for Australia, New Zealand, the Middle East, and 
other parts of the world that use PAL video).

Before creating a TV-B-Gone Kit, I originally started this project by hacking 
the MiniPOV kit.  This presents a limitation, based on the size of
the Atmel ATtiny2313 internal flash memory, which is 2KB.  With 2KB we can only 
fit about 7 POWER codes into the firmware's database of POWER codes.  However,
the more codes the better! Which is why we chose the ATtiny85 for the 
TV-B-Gone Kit.

This version of the firmware has the most popular 100+ POWER codes for 
North America and 100+ POWER codes for Europe. You can select which region 
to use by soldering a 10K pulldown resistor.
*/


/*
This project is a good example of how to use the AVR chip timers.
*/


/*
The hardware for this project is very simple:
     ATtiny85 has 8 pins:
       pin 1   RST + Button
       pin 2   one pin of ceramic resonator MUST be 8.0 mhz
       pin 3   other pin of ceramic resonator
       pin 4   ground
       pin 5   OC1A - IR emitters, through a '2907 PNP driver that connects 
               to 4 (or more!) PN2222A drivers, with 1000 ohm base resistor 
               and also connects to programming circuitry
       pin 6   Region selector. Float for US, 10K pulldown for EU,
               also connects to programming circuitry
       pin 7   PB0 - visible LED, and also connects to programming circuitry
       pin 8   +3-5v DC (such as 2-4 AA batteries!)
    See the schematic for more details.

    This firmware requires using an 8.0MHz ceramic resonator 
       (since the internal oscillator may not be accurate enough).

    IMPORTANT:  to use the ceramic resonator, you must perform the following:
                    make burn-fuse_cr
*/

/* This function is the 'workhorse' of transmitting IR codes.
   Given the on and off times, it turns on the PWM output on and off
   to generate one 'pair' from a long code. */
void xmitCodeElement(uint16_t ontime, uint16_t offtime )
{
  // start Timer0 outputting the carrier frequency to IR emitters on and OC0A 
  // (PB0, pin 5)
  TCNT0 = 0; // reset the timers so they are aligned
  TIFR = 0;  // clean out the timer flags

  // 99% of codes are PWM codes, they are pulses of a carrier frequecy
  // Usually the carrier is around 38KHz, and we generate that with PWM
  // timer 0
  TCCR0A =_BV(COM0A0) | _BV(WGM01);          // set up timer 0
  TCCR0B = _BV(CS00);

  // Now we wait, allowing the PWM hardware to pulse out the carrier 
  // frequency for the specified 'on' time
  delay_us(ontime);
  
  // Now we have to turn it off so disable the PWM output
  TCCR0A = 0;
  TCCR0B = 0;
  // And make sure that the IR LED is off too (since the PWM may have 
  // been stopped while the LED is on!)
  PORTB |= _BV(IRLED);           // turn off IR LED

  // Now we wait for the specified 'off' time
  delay_us(offtime);
}

int main(void) {
    uint16_t flash;
 
    DDRB = _BV(LED) | _BV(IRLED);    //set the visible and IR LED pins to outputs
    PORTB = _BV(LED) | _BV(IRLED);   //LEDs are off when pins are high
    OCR0A = (uint8_t)freq_to_timerval(38000); //value for 38kHz
    TCCR0A = 0;   //stop timer0
    TCCR0B = 0;
 
    flash = 0;
    while(1) {
        flash++;
        if (flash == 2000)
          PORTB &= ~_BV(LED);      //turn on visible LED at PB0 by pulling pin to ground
        //measured AIM beacon pattern [inverted by PNA4602M]:
        //high 6ms/low 624us/high 1.2ms/low 624us/high 1.2ms/low 624us [repeat]
        //Alternative Private Beacon Code:
        //300us ON / 1200us OFF / 300us ON / 1200us OFF / 300us ON / 6000us OFF
        //900us ON/9300us = 9.7% duty time
        xmitCodeElement(622, 1195);  //timing tweaked via AVR Studio Stopwatch
        xmitCodeElement(622, 1195);
        xmitCodeElement(622, 5994);  //IRLED on 1.872ms/off 8.4ms = 18.2% on time
        if (flash == 2005) {
          PORTB |= _BV(LED);       //turn off visible LED
          flash = 0;
        }
    }
}

/****************************** DELAY FUNCTIONS ********/


//for 8MHz we want to delay 8 cycles per microsecond
//this code is tweaked to give about that amount
void delay_us(uint16_t us) {
  while (us != 0) {
    NOP;      
    NOP;      
    us--;      
  }
}
