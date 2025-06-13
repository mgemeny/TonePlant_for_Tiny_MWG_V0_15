
#ifndef _TONEGEN
#define _TONEGEN
//*************************************************************************************
//  Arduino Tone Generator MODIFIED by M. W. Gemeny for his TonePlant
//  MWG V2 added a tuning pot in this version, July 15 2022.
//  MWG V5 tested moving timers all around on UNO with various pwm_modes 1/11/2024
//  MWG V6 finished attiny85 support
//  MWG V7 ATtiny84 support
// Generates four simultaneous square/sine wave tones at any relative amplitude
//*************************************************************************************
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "tables.h"

#define DIFF 1
#define CHA 2
#define CHB 3
#define SPDIF 4 // For documentation on Split Differential, see notes at the end of this file.

#define SINE      0
#define SQUARE    1
#define RAMP      2

#define FS 40000.0                            //-Rate must be evenly divisable into 16,000,000. Higher rates have worse frequency error, but better resolution. Not amy issue with 32-bit phase accumulators.
                                              //-Must be at least twice the highest frequency. Frequency resolution is FS/(2^32). (2^32) = the size of the phase accumulator.

#define SET(x,y) (x |=(1<<y))		        	  	//-Bit set/clear macros
#define CLR(x,y) (x &= (~(1<<y)))       			// |
#define CHK(x,y) (x & (1<<y))           			// |
#define TOG(x,y) (x^=(1<<y))            			//-+

volatile unsigned long PCW[4] = {
  0, 0, 0, 0};			            //-Wave phase accumulators - Index into wave table (upper 8 bits of 32)
volatile unsigned long FTW[4] = {
  0, 0, 0, 0};                  //-Wave frequency tuning words - Actual frequency being produced
volatile unsigned char AMP[4] = {
  0, 0, 0, 0};                  //-Wave amplitudes [0-255]
volatile unsigned int wavs[4];  //-Wave table selector [address of wave table in memory]
volatile unsigned char output_mode; //45 bytes above
volatile unsigned char interuptworkingbyte; // This is for the interrupt handler to deposit the equation result into before deciding what register to put it into.

//*********************************************************************************************
//  Audio driver interrupt
//*********************************************************************************************

#if defined( TIM0_COMPB_vect ) && ! defined( TIMER0_COMPB_vect )
#define TIMER0_COMPB_vect TIM0_COMPB_vect//                <<<--------------------  work around for bug in the David Mellis attiny core (Ver 1.0.2) for the ATtiny84 MWG 1/14/2024
#endif

#ifndef TIMEonCTC0 // You have chosen timer 1 for your sampling interupts
//Then timer must be on CTC1 as usual
SIGNAL(TIMER1_COMPA_vect)
#endif

#ifdef TIMEonCTC0 // You have chosen timer 0 for your sampling interupts
//Then timer must be on CTC0
SIGNAL(TIMER0_COMPB_vect)
#endif

{
  //---------------------------------------------------------
  //  Audio mixer - Total must not be > 250 to avoid clipping
  //---------------------------------------------------------
  //pgm_read_byte reads a byte from the wave table in program memory. The function requires the memory address of the byte to be retrieved.
  //Here, the start address of the wave table array, wavs[X], is added to the most-significant 8 bits of the current value of the (32-bit phase accumulator + 32-bit tuning word).
  //This is the address of the retrieved wave table value.
  //The wave table signed 256-bit value is then multiplied by a 256-bit unsigned volume adjustment constant and divided by 256 to render a level-adjusted wave table sample.

//#ifdef TIMEonCTC0  
#if defined(TIMEonCTC0) && defined(PROCESSORmega328)  
                     // if timer 0 then we are not clearing the counter, and we cannot clear the counter without breaking the Ardino time functions 
                     // so we have to move the compare register up with every interrupt to schedule the next interrupt
OCR0B = OCR0B + 7;   // Schedule the next interupt in 7*64 clock ticks
#endif

#ifdef PROCESSORtiny85 // note that TIMEonCTC0 is a MUST for this processor
OCR0B = //OCR0B + 10;   // Schedule the next interupt in 10*64 clock ticks (Perhaps a more efficient core would let us get this back to 7, but 7 is it's too fast here. 9 works, but 8 does not.)
OCR0B = OCR0B + 13;   // Schedule the next interupt in 10*64 clock ticks (Perhaps a more efficient core would let us get this back to 7, but 7 is it's too fast here. 9 works, but 8 does not.)
//OCR0B = OCR0B + 20;   // Schedule the next interupt in 7*64 clock ticks <-----------Testing ATtiny fesability at internal 8MHz
#endif


// Add Tiny84 support here <--------------------------------------------------------- Place 1
#ifdef PROCESSORtiny84 // note that TIMEonCTC0 is a MUST for this processor
//OCR0B = OCR0B + 12;     // Schedule the next interupt in 12*64 clock ticks 
OCR0B = OCR0B + 14;     // Schedule the next interupt in 12*64 clock ticks 
#endif



#ifndef TONEonCTC1  // NOT CTC1, so ... It must be CTC2

//   PORTD = PORTD | B00000100;  //Turn on relay output. This is to check your CPU time spent here with a scope.

  OCR2A = OCR2B = 127 +  //Start at 127 or 2.5 volts as zero level after capacitor coupling the output. Output voltage swings 5 volts p-p.
    (
    (((signed char)pgm_read_byte(wavs[0] + ((PCW[0]+=FTW[0]) >> 24)) * AMP[0]) >> 8) +
    (((signed char)pgm_read_byte(wavs[1] + ((PCW[1]+=FTW[1]) >> 24)) * AMP[1]) >> 8) +
    (((signed char)pgm_read_byte(wavs[2] + ((PCW[2]+=FTW[2]) >> 24)) * AMP[2]) >> 8) +
    (((signed char)pgm_read_byte(wavs[3] + ((PCW[3]+=FTW[3]) >> 24)) * AMP[3]) >> 8)
    );

    

//   PORTD = PORTD & B11111011;  //Turn off relay output. This is to check your CPU time spent here with a scope.

#endif
#ifdef TONEonCTC1                               // The tone belongs on CTC1!
//digitalWrite(PB0, HIGH); //Turn on cadence LED and/or relay.  Wow this adds 134 bytes to the code. Probably need to go back to poking registers.




  interuptworkingbyte = 127 +  //Start at 127 or 2.5 volts as zero level after capacitor coupling the output. Output voltage swings 5 volts p-p.
  //OCR1A = OCR1B = 127 +  //Start at 127 or 2.5 volts as zero level after capacitor coupling the output. Output voltage swings 5 volts p-p.
    (
    (((signed char)pgm_read_byte(wavs[0] + ((PCW[0]+=FTW[0]) >> 24)) * AMP[0]) >> 8) +
    (((signed char)pgm_read_byte(wavs[1] + ((PCW[1]+=FTW[1]) >> 24)) * AMP[1]) >> 8) +
    (((signed char)pgm_read_byte(wavs[2] + ((PCW[2]+=FTW[2]) >> 24)) * AMP[2]) >> 8) +
    (((signed char)pgm_read_byte(wavs[3] + ((PCW[3]+=FTW[3]) >> 24)) * AMP[3]) >> 8)
    );
   if (output_mode == SPDIF) {
      if (interuptworkingbyte > 130) { // If we are on the upper half of the waveform
        OCR1A = ((interuptworkingbyte - 127) << 1) ; // shift it down and amplify it by 3dB
        OCR1B = 0;
      }
      if (interuptworkingbyte < 124) { // If we are on the lower half of the waveform
        OCR1A =  0;
        OCR1B = (~(interuptworkingbyte ) *2 ); // invert it and ampllify it by 3dB
      }
      if ((interuptworkingbyte >= 124) && (interuptworkingbyte <= 130)) {
        OCR1A =  0;
        OCR1B = 0;
      }
   }
   else {
   OCR1A = OCR1B = interuptworkingbyte;
   }


    
//digitalWrite(PB0, LOW); //Turn on cadence LED and/or relay. This is to check your CPU time spent here with a scope.
#endif
 
}



class tonegen
{
private:

public:

  tonegen()
  {
  }

  //*********************************************************************
  //  Startup default (  This "default" method of starting tonegen has NOT been implemented, tested, or debuged for the ATtiny84 or ATtiny85 processors. )
  //*********************************************************************

  void begin()
  {
    #ifndef TIMEonCTC0 // NOT CTC0 so it must be a Mega CPU
       output_mode=CHA;
       TCCR1A = 0x00;                                  //-Start audio interrupt
       TCCR1B = 0x09;
       TCCR1C = 0x00;
       OCR1A=16000000.0 / FS;			                    //-Auto sample rate
       SET(TIMSK1, OCIE1A);                            //-Start audio interrupt
       sei();                                          //-+
    #endif

    #if defined(TIMEonCTC0) && defined(PROCESSORmega328)  // It's a Mega and we are sharing CTC0 for this interrupt
       //Timer 0 is used for PWM audio clock
       output_mode=CHA;
       OCR0B=7;                                         //-Auto sample rate 7 times the 64 scaling 448 as opposed to 400
       SET(TIMSK0, OCIE0B);                            //-Start audio interrupt    
       CLR (TCCR0A, WGM00); 
       CLR (TCCR0A, WGM01); 
       CLR (TCCR0B, WGM02); 
       sei();                                          //-+
    #endif

   #if defined(TIMEonCTC0)  && defined (PROCESSORtiny85)                        // if it is an tiny85, then ...
      //output_mode=CHA;
      //OCR0B=7;                                         //-Auto sample rate 7 times the 64 scaling 448 as opposed to 400
      //SET(TIMSK, OCIE0B);                            //-Start audio interrupt
      //CLR (TCCR0A, WGM00); 
      //CLR (TCCR0A, WGM01); 
      //CLR (TCCR0B, WGM02); 
      //sei();                                          //-+
    #endif


// Add Tiny84 support here <--------------------------------------------------------- Place 2      This needs work. We're not using this section
    #if defined(TIMEonCTC0)  && defined (PROCESSORtiny84)
       //output_mode=CHA;
       //OCR0B=7;                                         //-Auto sample rate 7 times the 64 scaling 448 as opposed to 400
       //SET(TIMSK0, OCIE0B);                            //-Start audio interrupt    
       //CLR (TCCR0A, WGM00); 
       //CLR (TCCR0A, WGM01); 
       //CLR (TCCR0B, WGM02); 
       //sei();                                          //-+
    #endif

// 

    #if defined(TONEonCTC2) && defined(PROCESSORmega328)           //It must be CTC2  on CHA pin (11) of ATmega328
      TCCR2A = 0x83;                                  //-8 bit audio PWM
      TCCR2B = 0x01;                                  // |
      OCR2A = OCR2B = 127;                            //-+
      SET(DDRB, 3);                                   //-PWM pin
     #endif

     #if defined(TONEonCTC1) && defined(PROCESSORmega328)
       TCCR1A = 0x83;                                  //-8 bit audio PWM
       TCCR1B = 0x01;                                  // |
       OCR1A = OCR1B = 127;                            //-+
       //SET(DDRB, 10);                                   //-PWM pin
       setOCR1Boutput();
       //setOCR1Aoutput(); // This could conflict with the default serial interface Receive
     #endif
     
      #if defined(TONEonCTC1) && defined(PROCESSORtiny85)          // on CHB pin ( 9 ) of ATmega328, DIP pin 7 of Tiny84, or DIP pin 6 of Tiny85
       //TCCR1A = 0x83;                                  //-8 bit audio PWM
       ////TCCR1B = 0x01;                                  // |
       //TCCR1 = 1<<PWM1A | 1<<COM1A1 | 1<<COM1A0| 1<<CS10; // 0b01010001 0h51 FastPWM-A, CompOutMode-1 and ClkSel 1-to-1
       //OCR1A = OCR1B = 127;                            //-+
       ////SET(DDRB, 3);                                 //-PWM pin
       //setOCR1Aoutput();
      #endif


// Add Tiny84 support here <--------------------------------------------------------- Place 3
  
  }

  //*********************************************************************
  //  Startup, selecting various output modes  ( All 3 modes {DIFF, CHA, and CHB} have been tested with the new TONEonCTC1 for the Mega328. But only CHA has been done for the ATtiny processors.)
  //*********************************************************************

  void begin(unsigned char d)
  {
  //******************* First - Set up our lower timer for the wave form interupt clock CTC0 or CTC1 considering CPU type
  // 
    #ifndef TIMEonCTC0 // NOT CTC0  so MUST be CTC1. It must be a Mega CPU
       //Timer 1 is used for PWM audio clock
       TCCR1A = 0x00;                                  //COM1cp bits (Compare output pins) 0, (Wave Gen Mode) WGM11-12 mode bits 0
       TCCR1B = 0x09;                                  // WGM13 0, WGM12 1, with WGM above 0, that sets mode "Clear TCNT1 on OCR1A match". CS12-CS10=1 Clock one-to-one
       TCCR1C = 0x00;                                  // No forcing of output compare
       OCR1A=16000000.0 / FS;			                    //-Auto sample rate 16MHz and 40kHz is 400 
       SET(TIMSK1, OCIE1A);                            //-Start audio interrupt
       sei();                                          //-+
    #endif

    #if defined(TIMEonCTC0) && defined(PROCESSORmega328)  // It's a Mega and we are sharing CTC0 for this interrupt
      //Timer 0 is used for PWM audio clock
       OCR0B=7;                                         //-Auto sample rate 7 times the 64 scaling 448 as opposed to 400
       SET(TIMSK0, OCIE0B);                            //-Start audio interrupt    
       CLR (TCCR0A, WGM00); 
       CLR (TCCR0A, WGM01); 
       CLR (TCCR0B, WGM02); 
       sei();                                          //-+
    #endif

    #if defined(TIMEonCTC0)  && defined (PROCESSORtiny85)                        // if it is an tiny85, then ...
       OCR0B=13;                                         //-Auto sample rate 13 times the 64 scaling (Perhaps a more efficient core would get is get this back to 7, but it's too fast here)
                                                        // 7 was just the starting point. It's the interupt handler that really steps it
       SET(TIMSK, OCIE0B);                            //-Start audio interrupt
       CLR (TCCR0A, WGM00); 
       CLR (TCCR0A, WGM01); 
       CLR (TCCR0B, WGM02);
       sei();                                          //-+
    #endif


// Add Tiny84 support here <--------------------------------------------------------- Place 4                  This works!
#if defined(TIMEonCTC0)  && defined (PROCESSORtiny84)                        // if it is an tiny84, then ...
       OCR0B=14;                                         //-Auto sample rate 14 times the 64 scaling 920 as opposed to 400 (We moved it to 12. Oh well)
       //SET(TIMSK0, OCIE0B);                            //-Start audio interrupt    
       CLR (TCCR0A, WGM00); 
       CLR (TCCR0A, WGM01); //                                                                      <-----------------------------------------------------TESTING in this area
       CLR (TCCR0B, WGM02); 
       //TCCR0B = 1;
       //SET (TCCR0A, WGM00);                                                                        // Finally got my interupts working with a change in the handler
       //SET (TCCR0A, WGM01);
       //SET (TCCR0B, WGM02);
       
       SET(TIMSK0, OCIE0B);                            //-Start audio interrupt   
       //SET(TIMSK0, SC02);                            //-Start audio interrupt  
        //TIMSK0 = TIMSK0 | (1<<2);             //Enable it by hand
       sei();                                          //-+
#endif
    output_mode=d;

//******************* Then - Set up our upper timer CTC1 or CTC2 for our PWM timer(s) considering A side, B side, or both, and CPU type
//                                                                             
// 
// Don had 3 options here for starting up the waveform timer.
// We are going to end up with sixteen options.
// Each of Dons options were managed with a case (or switch) statement
// at run time. We will keep that and handle the processor
// and timer choices with defined switches at compile time.
// Here is what Dons option list looked like.
// 
// Processor        Option
// and timer       DIFF      CHB      CHA
// -------------+--------+--------+--------+
// Mega328      |  Case  |  Case  |  Case  |
// Tone on      |    X   |    Y   |    Z   |
// Timer 2      |        |        |        |
// -------------+--------+--------+--------+
// 
// And here is what our option matrix is going to look like.
//
// TABLE OF 16 (serves as an index to the code snippetts below)
// 
// Processor        Option                    SPlit   See
// and timer    |  DIFF      CHB     CHA      DIF     Note 1
// -------------+--------+--------+--------+--------+
// Mega328      |  Case  |  Case  |  Case  |  Case  |
// Tone on      |    1   |    5   |    9   |   13   |
// Timer 2      | Tested | Tested | Tested | Tested | <--Status
// -------------+--------+--------+--------+--------+
// Mega328      |  Case  |  Case  |  Case  |  Case  | See
// Tone on      |    2   |    6   |   10   |   14   | Note 2
// Timer 1      | Tested | Tested | Tested | Tested | <--Status
// -------------+--------+--------+--------+--------+
// ATtiny84     |  Case  |  Case  |  Case  |  Case  |
// Tone on      |    3   |    7   |   11   |   15   |
// Timer 1      | Tested | Tested | Tested | Tested | <--Status
// -------------+--------+--------+--------+--------+
// ATtiny85     |  Case  |  Case  |  Case  |  Case  |
// Tone on      |    4   |    8   |   12   |   16   |
// Timer 1      | Tested | Tested | Tested | Tested | <--Status
// -------------+--------+--------+--------+--------+
// 
// Note 1: For documentation on Split Differential, see the end of this file.
//         For the purposes of setting up the timers, one could think of 
//         Split Differential as CHA and CHB in unison. The interrupt handler
//         will decide if we are on the upper or lower part of the composit wave
//         and deposit values into the compare registers as appropriate.
// 
// Note 2: This row was important for testing and still useful for comparison.
//         It could be useful if someone wanted to support multiple outputs,
//         such as one tone with up to 4 different cadences, or 2 tones of 2 sines
//         each cadenced two different ways. Only the Mega328 could do 4 outputs.
//         Both Tiny processors could only do 2, so it would not be so much gain.
// 
    switch(d)
    {
//                                 
//                                 
//                             ------------ Begin DIFF ----------     
    case DIFF:                                        //-Differntial signal
//                                 
//                             ------------- CASE 1 -------------      Mega328, Tone on Timer 2, DIFF      See table of 16 above
//                                 
      #ifndef TONEonCTC1                             // It must be CTC2 on CHA and CHB pins (11,3) of ATmega328
       TCCR2A = 0xB3;                                  //-8 bit audio PWM sets   Here timer 2 is an 8 bit timer and has different WGM bits from timer 0 and 1
       TCCR2B = 0x01;                                  // | Sets only 1 to 1 clock but not WGM22 which if set sould more us from Phase Corrected 8 bit to regular 8 bit fast that would be 0x09
       OCR2A = OCR2B = 127;                            //-+
       SET(DDRB, 3);                                   //-PWM pin
       SET(DDRD, 3);                                   //-PWM pin
      #endif
//                                 
//                             ------------- CASE 2 -------------      Mega328, Tone on Timer 1, DIFF      See table of 16 above
//                                 
      #if defined(TONEonCTC1) && defined(PROCESSORmega328)  // on CHA and CHB pins (9,10) of ATmega328, DIP pins 7,8 of Tiny84, or DIP pins 6,3 of Tiny85 (but we'll probably move 3 to 5 with interrupts) 
       //TCCR1A = 0xB3;                                  //-8 bit audio PWM sets                  <---- Testing
       TCCR1A = 0xB1;                                  //-8 bit audio PWM sets                  <---- That's better.  Changed from A1 to B1 to invert chan B to fix diff (MWG 2/14/25)
       //TCCR1B = 0x01;                                  // | Sets only clock one to one           <---- Testing
       TCCR1B = 0x09;                                  // | Sets only clock one to one          <---- That's better.  Caution timers 0 and 1 are different from timer 2 (16 bit vs 8 bit)
       OCR1A = OCR1B = 127;                            //-+
       //setOCR1Aoutput();                                //-PWM pin
       SET(DDRB, DDB1);                                   //-PWM pin B1 OCR1A PWM output is on PB1 (DIP pin 15) setOCR1Aoutput
       //setOCR1Boutput();                                //-PWM pin
       SET(DDRB, DDB2);                               //OCR1B PWM output is on PB2 (DIP pin 16) setOCR1Boutput() PROCESSORmega328
      #endif
//                                 
//                             ------------- CASE 3 -------------      ATtiny84, Tone on Timer 1, DIFF      See table of 16 above
//                                 
// Just now 2/12/25 this is more like Unison and more like what we will want for CASE 16 <-- Fixed MWG 2/13/25 Needs testing
// Add Tiny84 support here <--------------------------------------------------------- Place 5       This needs work. We're not using this section yet <-- Fixed MWG 2/13/25 Needs testing
#if defined(TONEonCTC1)  && defined (PROCESSORtiny84)                        // if it is an tiny84 
       //TCCR1A = 0xB3;                                  //-8 bit audio PWM sets                  <---- Testing
       TCCR1A = 0xA1;                                  //-8 bit audio PWM sets                  <---- That's better -- removed MWG 2/13/25
       TCCR1A = 0xB1;                                  //-8 bit audio PWM sets                  <---- That's better -- added MWG 2/13/25 That should invert on DIFF
       //TCCR1B = 0x01;                                  // | Sets only clock one to one           <---- Testing
       TCCR1B = 0x09;                                  // | Sets only clock one to one          <---- That's better
       OCR1A = OCR1B = 127;                            //-+
//       setOCR1Aoutput();                                //-PWM pin
       SET(DDRA, DDA6);                                   //-PWM pin A6 DIP pin 7 setOCR1Aoutput() PROCESSORtiny84
//       setOCR1Boutput();                                //-PWM pin
       SET(DDRA, DDA5);                              // OCR1B PWM output is on PA5 (DIP pin 8) PROCESSORtiny84 setOCR1Boutput()
      #endif
//                                 
//                             ------------- CASE 4 -------------      ATtiny85, Tone on Timer 1, DIFF      See table of 16 above
//                                 
      #if defined(TONEonCTC1)  && defined (PROCESSORtiny85)                        // if it is an tiny85,DIP pins 6,3 of Tiny85 
      //TCCR1 = 1<<PWM1A | 1<<COM1A1 | 1<<COM1A0| 1<<CS10; // 0b01010001 0h51 FastPWM-A, CompOutMode-1 and ClkSel 1-to-1 // That should be good for the A side <-- Removed MWG 2/13/25
      //GTCCR = 0<<TSM | 1<<PWM1B | 0<<COM1B1 | 0<<COM1B0 | 0<<FOC1B | 0<<FOC1A | 0<<PSR1 | 0<<PSR0; //That probably needs work for the B side as differential <-- Removed MWG 2/13/25
      //OCR1A = OCR1B = 127;                            //-+<-- Removed MWG 2/13/25
      //pinMode(1, OUTPUT); // make the pin on output Dip pin 6<-- Removed MWG 2/13/25
      //pinMode(4, OUTPUT); // Differential not supported on this processor and we need this as an input

      TCCR1 = (0<<CTC1) | (1<<PWM1A) | (1<<COM1A1) | (0<<COM1A0) | (0<<CS13) | (0<<CS12) | (0<<CS11) | (1<<CS10) ;  // 0b01010001 0h61 FastPWM-A, CompOutMode-1 and ClkSel 1-to-1 0xC0 now 0h61 like Case 12
      GTCCR =  (0<<TSM) | (1<<PWM1B) | (1<<COM1B1) | (1<<COM1B0) | (0<<FOC1B) | (0<<FOC1A) | (0<<PSR1) | (0<<PSR0); // 0b01110000 0h70 Sort of like Case 8 but inverted
      OCR1A = OCR1B = 127;                         //-+
      OCR1C = 0xFF;           //<--- This may not be required as CTC1 in TCCR1 is not set
      SET(DDRB, DDB4);                             //Set Data Direction Bit DDB4 like setOCR1Boutput() PROCESSORtiny85 should be chip pin 3
      SET(DDRB, DDB1);                             // set as output PWM A side on an Attiny85 like setOCR1Aoutput() PROCESSORtiny85 should be chip pin6 Added MWG 2/13/05
      
      #endif

//                                 
      break;
//                                 
//                             ------------ End DIFF ------------      
//                             ------------ Begin CHB -----------
//                                 
    case CHB:                                         // -Single ended signal 
//                                 
//                             ------------- CASE 5 -------------      Mega328, Tone on Timer 2, CHB      See table of 16 above
//                                 
      #ifndef TONEonCTC1   //It must be CTC2 on CHB pin (3) of ATmega328
      // It must be on CTC2 and it must be a Mega processor
       TCCR2A = 0x23;                                  //-8 bit audio PWM
       TCCR2B = 0x01;                                  // |
       OCR2A = OCR2B = 127;                            //-+
       SET(DDRD, 3);                                   //-PWM pin
      #endif
//                                 
//                             ------------- CASE 6 -------------      Mega328, Tone on Timer 1, CHB      See table of 16 above
//                                 
      #if defined(TONEonCTC1) && defined(PROCESSORmega328)    // on CHB pin (10) of ATmega328, 
       //TCCR1A = 0x23;                                  //-8 bit audio PWM                  <---- Testing
       TCCR1A = 0x21;                                  //-8 bit audio PWM                   <---- That's better
       //TCCR1B = 0x09;                                  // |                                <---- Testing
       TCCR1B = 0x01;                                  // |                                 <---- That's better
       OCR1A = OCR1B = 127;                            //-+
       //SET(DDRD, 3);                                   //-PWM pin
       setOCR1Boutput();                                //-PWM pin
      #endif
//                                 
//                             ------------- CASE 7 -------------      ATtiny84, Tone on Timer 1, CHB      See table of 16 above
//                                 
// Add Tiny84 support here <--------------------------------------------------------- Place 6      DIP pin 8 of Tiny84 This needs work. We're not using this section yet -- fixed MWG 2/13/25 needs testing
      #if defined(TONEonCTC1) && defined (PROCESSORtiny84)
        //TCCR1A = 0x23;                                  //-8 bit audio PWM                  <---- Testing
        TCCR1A = 0x21;                                  //-8 bit audio PWM                   <---- That's better
        //TCCR1B = 0x09;                                  // |                                <---- Testing
        //TCCR1B = 0x01;                                  // |                                 <---- That's better -- removed MWG 2/13/25
        TCCR1B = 0x09;                                  // |                                 <---- That's better -- added MWG 2/13/25
        OCR1A = OCR1B = 127;                            //-+
//        //SET(DDRD, 3);                                   //-PWM pin
//        setOCR1Boutput();                                //-PWM pin
          SET(DDRA, DDA5);              // OCR1B PWM output is on PA5 (DIP pin 8) PROCESSORtiny84 setOCR1Boutput()
      #endif
//                                 
//                             ------------- CASE 8 -------------      ATtiny85, Tone on Timer 1, CHB      See table of 16 above
//                                 
      //This option is not yet supported on an ATtiny85 processor
      #if defined(TONEonCTC1) && defined (PROCESSORtiny85)            // on CHB pin (10) of ATmega328, DIP pin 8 of Tiny84, or DIP pin 3 of Tiny85 (but we'll probably move 3 to 5 with interrupts)
       //TCCR1A = 0x23;                                  //-8 bit audio PWM <<<<<<<<<This should choke <-- Removed MWG 2/13/25
       //TCCR1B = 0x01;                                  // |<<<<<<<<<<<<<<<<<<<<<<This should choke
       //OCR1A = OCR1B = 127;                            //-+    <-- Removed MWG 2/13/25
       //SET(DDRD, 3);                                   //-PWM pin
       //setOCR1Boutput();                                //-PWM pin    <-- Removed MWG 2/13/25

      TCCR1 = (0<<CTC1) | (0<<PWM1A) | (0<<COM1A1) | (0<<COM1A0) | (0<<CS13) | (0<<CS12) | (0<<CS11) | (1<<CS10) ; // 0b00000001 0h01 Nothing on A, and ClkSel 1-to-1 
      GTCCR =  (0<<TSM) | (1<<PWM1B) | (1<<COM1B1) | (0<<COM1B0) | (0<<FOC1B) | (0<<FOC1A) | (0<<PSR1) | (0<<PSR0); //0b01100000 0h60 PWM on B not inverted
      OCR1A = OCR1B = 127;                          //-+
      OCR1C = 0xFF;           //<--- This may not be required as CTC1 in TCCR1 is not set
      SET(DDRB, DDB4);                              //Set Data Direction Bit DDB4 like setOCR1Boutput() PROCESSORtiny85 should be chip pin 3
       
      #endif
      break;
//                                 
//                             ------------ End CHB -------------      
//                             ------------ Begin CHA -----------
//                                 
    // This is where we have been testing for the ATtiny processors
    case CHA:                                          //-Single ended signal
    default:
      output_mode=CHA;                                //-Single ended signal
//                                 
//                             ------------- CASE 9 -------------      Mega328, Tone on Timer 2, CHA      See table of 16 above
//                                 
      #ifdef TONEonCTC2                              //It must be CTC2  on CHA pin (11) of ATmega328
       TCCR2A = 0x83;                                  //-8 bit audio PWM
       TCCR2B = 0x01;                                  // |
       OCR2A = OCR2B = 127;                            //-+
       SET(DDRB, 3);                                   //-PWM pin
      #endif
//                                 
//                             ------------- CASE 10 ------------      Mega328, Tone on Timer 1, CHA      See table of 16 above
//                                 
      #if defined(TONEonCTC1) && defined (PROCESSORmega328)                                // on CHB pin ( 9 ) of ATmega328, DIP pin 7 of Tiny84, or DIP pin 6 of Tiny85 
       //TCCR1A = 0x83;                                  //-8 bit audio PWM                  <---- Testing
       TCCR1A = 0x81;                                  //-8 bit audio PWM                   <---- That's better
       //TCCR1B = 0x01;                                  // |                                <---- Testing
       TCCR1B = 0x09;                                  // |                                 <---- That's better
       OCR1A = OCR1B = 127;                            //-+
       //SET(DDRB, 3);                                 //-PWM pin
       setOCR1Aoutput();
      #endif
//                                 
//                             ------------- CASE 11 ------------      ATtiny84, Tone on Timer 1, CHB      See table of 16 above
//                                 
      #if defined (TONEonCTC1) && defined (PROCESSORtiny84) 
       //TCCR1A = 0x83;                                  //-8 bit audio PWM                  
       //TCCR1A = 0x81;                                  //-8 bit audio PWM                   <---- Was on Mega
       TCCR1A = (1<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<3) | (0<<2) | (0<<WGM11) | (1<<WGM10); // Tiny84 is very much like Mega328 TCCR1A but very different from tiny85
       //                                                                                                          // COM bits are largly the same, only a slight difference for COM1x1-0 = 0,1
       //TCCR1B = 0x01;                                  // |                               
       //TCCR1B = 0x09;                                  // |                                 <---- Was on Mega
       TCCR1B = (0<<ICNC1) | (0<<ICES1) | (0<<5) | (0<<WGM13) | (1<<WGM12) | (0<<CS12) | (0<<CS11) | (1<<CS10); // This too seems layed out like the Mega328
       //                                                                                                       // WGM1(3-0) seem the same, CS are the same
       OCR1A = OCR1B = 127;                            //-+
       //SET(DDRB, 3);                                 //-PWM pin
       setOCR1Aoutput();
      #endif
//                                 
//                             ------------- CASE 12 ------------      ATtiny85, Tone on Timer 1, CHA      See table of 16 above
//                                 
      #if defined (TONEonCTC1) && defined (PROCESSORtiny85)                              // on CHB pin ( 9 ) of ATmega328, DIP pin 7 of Tiny84, or DIP pin 6 of Tiny85
       //TCCR1A = 0x83;                                  //-8 bit audio PWM <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<This should choke
       //TCCR1A =0x41                                    // My core accepts that
       //TCCR1 = 0x83;                                   // TCCR1 seems to be called TCCR1A in the Core that I'm using for the ATTINY85
       //TCCR1B = 0x01;                                  // |

/*// Set up timer/PWM items
  PLLCSR |= (1 << PLLE); //Enable PLL

  //Wait for PLL to lock
  while ((PLLCSR & (1<<PLOCK)) == 0x00)
    {
        // Do nothing until plock bit is set
    }

  // Enable clock source bit
  PLLCSR |= (1 << PCKE);*/

       // I really dont't know this timer very well and it's quite different from timers on other processors
       // This may not be the best configuration but it seems to work
       //TCCR1 = 1<<PWM1A | 0<<COM1A1 | 1<<COM1A0| 1<<CS10; // 0b01010001 0h51 FastPWM-A, CompOutMode-1 and ClkSel 1-to-1
       //TCCR1 = (0<<CTC1) | (1<<PWM1A) | (0<<COM1A1) | (1<<COM1A0) | (0<<CS13) | (0<<CS12) | (0<<CS11) | (1<<CS10) ; // This stumped me, it gave tone on both PB1 and PB0! I didn't read that it could do that.
       TCCR1 = (0<<CTC1) | (1<<PWM1A) | (1<<COM1A1) | (0<<COM1A0) | (0<<CS13) | (0<<CS12) | (0<<CS11) | (1<<CS10) ; // 0b01010001 0h51 FastPWM-A, CompOutMode-1 and ClkSel 1-to-1 0xC0 now
       OCR1A = OCR1B = 127;                            //-+
       OCR1C = 0xFF;
       GTCCR =  (0<<TSM) | (0<<PWM1B) | (0<<COM1B1) | (0<<COM1B0) | (0<<FOC1B) | (0<<FOC1A) | (0<<PSR1) | (0<<PSR0); //<------ TESTING
       //GTCCR = GTCCR | (1<<PSR1); // Kick it!
       //SET(DDRB, 3);                                 //-PWM pin
       //setOCR1Aoutput();                               // Removed MWG 2/13/25
       SET(DDRB, DDB1);                             // set as output PWM A side on an Attiny85 like setOCR1Aoutput() PROCESSORtiny85 should be chip pin6 Added MWG 2/13/05
      #endif
      break;
//                                 
//                             ------------ End CHA -------------            
//                             ------------ Begin SPDIF ---------
//                                 

    case SPDIF:                                        //-Split Differntial output
//                                 
//                             ------------- CASE 13 ------------      Mega328, Tone on Timer 2, SPDIF      See table of 16 above
//
      #ifndef TONEonCTC1                             // It must be CTC2 on CHA and CHB pins (11,3) of ATmega328  Like case 1 but dont invert B
      TCCR2A = 0xA3;                                  //-8 bit audio PWM sets  A3 vs B3 to not invert chan B. Here timer 2 is an 8 bit timer and has different WGM bits from timer 0 and 1
       TCCR2B = 0x01;                                  // | 
       OCR2A = OCR2B = 127;                            //-+
       SET(DDRB, 3);                                   //-PWM pin
       SET(DDRD, 3);                                   //-PWM pin
      #endif
//                                 
//                             ------------- CASE 14 ------------      Mega328, Tone on Timer 1, SPDIF      See table of 16 above
//
      #if defined(TONEonCTC1) && defined(PROCESSORmega328)  // on CHA and CHB pins (9,10) of ATmega328, DIP pins 7,8 of Tiny84, or DIP pins 6,3 of Tiny85 
                                                           // case 14 is like case 2 but not inverted
       //TCCR1A = 0xB3;                                  //-8 bit audio PWM sets                  <---- Testing
       TCCR1A = 0xA1;                                  //-8 bit audio PWM sets                  <---- That's better Like case 2
       //TCCR1B = 0x01;                                  // | Sets only clock one to one           <---- Testing
       TCCR1B = 0x09;                                  // | Sets only clock one to one          <---- That's better
       OCR1A = OCR1B = 127;                            //-+
       //setOCR1Aoutput();                                //-PWM pin
       SET(DDRB, DDB1);                                   //-PWM pin B1 OCR1A PWM output is on PB1 (DIP pin 15) setOCR1Aoutput
       //setOCR1Boutput();                                //-PWM pin
       SET(DDRB, DDB2);                               //OCR1B PWM output is on PB2 (DIP pin 16) setOCR1Boutput() PROCESSORmega328
      #endif
//                                 
//                             ------------- CASE 15  -----------      ATtiny84, Tone on Timer 1, SPDIF      See table of 16 above
//
      #if defined(TONEonCTC1)  && defined (PROCESSORtiny84)                        // if it is an tiny84 output is DIP pins 7 and 8
       
       TCCR1A = 0xA1;                                  //-8 bit audio PWM sets                  <---- That's better
       TCCR1B = 0x09;                                  // | Sets only clock one to one          <---- That's better
       OCR1A = OCR1B = 0;                              // Unlike some of the other modes SPDIF starts at zero
       SET(DDRA, DDA6);                                //-PWM pin PA6 DIP pin 7 setOCR1Aoutput() PROCESSORtiny84
       SET(DDRA, DDA5);                                // OCR1B PWM output is on PA5 (DIP pin 8) PROCESSORtiny84 setOCR1Boutput()
      #endif
//                                 
//                             ------------- CASE 16  -----------      ATtiny85, Tone on Timer 1, SPDIF      See table of 16 above
//
      #if defined(TONEonCTC1)  && defined (PROCESSORtiny85)                        // if it is an tiny85 output is on DIP pins 3 and 6
      TCCR1 = (0<<CTC1) | (1<<PWM1A) | (1<<COM1A1) | (0<<COM1A0) | (0<<CS13) | (0<<CS12) | (0<<CS11) | (1<<CS10) ;  // 0b01010001 0h61 FastPWM-A, CompOutMode-1 and ClkSel 1-to-1 0xC0 now 0h61 like Case 12
      GTCCR =  (0<<TSM) | (1<<PWM1B) | (1<<COM1B1) | (0<<COM1B0) | (0<<FOC1B) | (0<<FOC1A) | (0<<PSR1) | (0<<PSR0); // 0b01100000 0h60 Sort of like Case 4 but B is not inverted like Case 8
      OCR1A = OCR1B = 127;                         //-+
      OCR1C = 0xFF;           //<--- This may not be required as CTC1 in TCCR1 is not set
      SET(DDRB, DDB4);                             //Set Data Direction Bit DDB4 like setOCR1Boutput() PROCESSORtiny85 should be chip pin 3
      SET(DDRB, DDB1);                             // set as output PWM A side on an Attiny85 like setOCR1Aoutput() PROCESSORtiny85 should be chip pin6 Added MWG 2/13/05

      #endif
      
    break; 
//                             ------------ End SPDIF -----------


      
    } //                       ------------ End of switch(d) ----
  } //                         ------------ End of the begin function ----



  //*********************************************************************
  //  Setup waves
  //*********************************************************************

  void setWave(unsigned char wave1, unsigned char wave2, unsigned char wave3, unsigned char wave4)
  {

    if(wave1 == SQUARE) {wavs[0] = SquareTable;} 
      else if (wave1 == SINE){wavs[0] = SinTable;} 
      else if (wave1 == RAMP){wavs[0] = RampTable;}
      else {wavs[0] = SinTable;}
    if(wave2 == SQUARE) {wavs[1] = SquareTable;} 
      else if (wave2 == SINE){wavs[1] = SinTable;} 
      else if (wave2 == RAMP){wavs[1] = RampTable;}
      else {wavs[1] = SinTable;}
    if(wave3 == SQUARE) {wavs[2] = SquareTable;} 
      else if (wave3 == SINE){wavs[2] = SinTable;} 
      else if (wave3 == RAMP){wavs[2] = RampTable;}
      else {wavs[2] = SinTable;}   
    if(wave4 == SQUARE) {wavs[3] = SquareTable;} 
      else if (wave4 == SINE){wavs[3] = SinTable;} 
      else if (wave4 == RAMP){wavs[3] = RampTable;}
      else {wavs[3] = SinTable;}

  }


  //*********************************************************************
  //  Set frequency direct
  //*********************************************************************

  void setFrequency(unsigned char voice,float f)
  {
    //32-bit phase accumulator provides 0.000009313 Hz. resolution at 40KHz sampling rate (FS/(2^32))
    //FTW[voice]=f/(FS/4294967295.0); //frequency/(sample_rate/max_32bit_count)
    //f * 1/(40,000/4294967295.0) = f * 107,374.182375
    // Moving to timer 0 with about a 20kHz (20,833.333Hz) sampling rate will change this to about 206158
    // 16,000,000 clock / Arduino 64 scaling / 12 tick steps on the compare interupts = 20,833.333Hz steps
    // 37,714.2957Hz with a step of 7 ticks on the compare register  
    // or 19,230.7692Hz with a step of 13 ticks
    #ifndef TIMEonCTC0
      //Time is on CTC1
      FTW[voice]=f * (107374.182375 + tunerValue);
      //FTW[voice]=f * (214748.36475 + tunerValue);
    #endif
    
    //#ifdef TIMEonCTC0
    #if defined(TIMEonCTC0) && defined(PROCESSORmega328)  // It's a Mega and we are sharing CTC0 for this interrupt
      FTW[voice]=f * (120100.8957 + tunerValue); // gives 1004.3 when above timer was 1004.3 with whatever tuner value Step of 7
     #endif

     #ifdef PROCESSORtiny85
      //FTW[voice]=f * (217346.1272 + tunerValue);//<--------TESTING step of 13
      //FTW[voice]=f * (216128.1272 + tunerValue);//<--------TESTING step of 13 MG 2/9/25
      //FTW[voice]=f * (211128.1272 + tunerValue);//<--------TESTING step of 13 MG 4/25/25 Processors running to fast (too high freq)
      FTW[voice]=f * (211128 + tunerValue);//<--------TESTING step of 13 MG 4/26/25 Processors running to fast (too high freq) (Wow! removing the fraction reduced the flash size by 204 Bytes! 2.659%)
      //FTW[voice]=f * (167253 + tunerValue);//<-------- step of 10 Attiny85 works will at internal 16MHz
      //FTW[voice]=f * (166253.3368 + tunerValue);//<-------- step of 10 Attiny85 works will at internal 16MHz Found a CPU that was too high 2/6/2025 MWG 2/9/25
      //FTW[voice]=f * (334506.6736 + tunerValue);//<--------TESTING for 8MHz internal for Attiny84. First tested steps of 20 and then slowed the clock steps of 10 ATtiny84 Just testing here on the 85 for feasibility
     #endif






// Add Tiny84 support here <--------------------------------------------------------- Place 8
     #ifdef PROCESSORtiny84
      //FTW[voice]=f * (217346.1272 + tunerValue);//<--------TESTING step of 13 Attiny84 only if you have an external 16MHz clock
      //FTW[voice]=f * (167253.3368 + tunerValue);//<-------- step of 10 Attiny84 only if you have an external 16MHz clock
      //FTW[voice]=f * (334506.6736 + tunerValue);//<-------- 8MHz internal max for Attiny84, re're stuck with steps of 10 Too Fast
      //FTW[voice]=f * (402209.0000 + tunerValue);//Gave 983 for 1004
      //FTW[voice]=f * (410800.0000 + tunerValue);//Gave 1025 for 1004
      //FTW[voice]=f * (406500.0000 + tunerValue);//Gave 1015
      //FTW[voice]=f * (402100.0000 + tunerValue); //Gave 1005.2 for 1004
      //FTW[voice]=f * (401620.0000 + tunerValue);//Gave 1002.5
      //FTW[voice]=f * (401860.0000 );// gives 1004.5
      //FTW[voice]=f * (401850.0000 );// gives 1004.4
      //FTW[voice]=f * (401700.0000 );// gives a touch high
      //FTW[voice]=f * (401675.0000 + tunerValue);// gives right close but lets shoot for 1002.5 without the tooner and rounded (steps of 12)
      FTW[voice]=f * (468621.0000 + tunerValue);// gives right close but lets shoot for 1002.5 without the tooner and rounded (steps of 14)
      //FTW[voice]=f * (401675.0000);//That's roght at 1004 but with some buzz
      //FTW[voice]=f * (401625.0000);// that's 1003.7 or 8
      //FTW[voice]=f * (401500.0000);//1003.5
      //FTW[voice]=f * (401400.0000);//1003.2
      //FTW[voice]=f * (401250.0000);//1003.0
      //FTW[voice]=f * (401000.0000);//1002.2
      //FTW[voice]=f * (401125.0000);//
     #endif

  }

  
  //*********************************************************************
  //  Set volume
  //*********************************************************************

  void setVolume(unsigned char vol0, unsigned char vol1, unsigned char vol2,unsigned char vol3)
  {
    AMP[0] = vol0;
    AMP[1] = vol1;
    AMP[2] = vol2;
    AMP[3] = vol3;
  }
  //*********************************************************************
  //  Suspend/resume tonegen
  //*********************************************************************

  void suspend()
  {
   //  CLR(TIMSK1, OCIE1A);                            //-Stop audio interrupt 
   
   #ifndef TONEonCTC1
   //Must be CTC2
    while(OCR2A != 127){                            //-Ramp down voltage to zero for click reduction
      if(OCR2A > 127){OCR2A = OCR2B -= 1;}
      if(OCR2A < 127){OCR2A = OCR2B += 1;}
      delayMicroseconds(8);                         //Sets ramp down speed, 1.008 mS worst case (126*0.008)
    }
    #endif
    
    #ifdef TONEonCTC1
     while(OCR1A != 127){                            //-Ramp down voltage to zero for click reduction
      if(OCR1A > 127){OCR1A = OCR1B -= 1;}
      if(OCR1A < 127){OCR1A = OCR1B += 1;}
      delayMicroseconds(8);                         //Sets ramp down speed, 1.008 mS worst case (126*0.008)
    }
    #endif  
    
    #ifndef TIMEonCTC0                                //The timier must be on Timer 1
      //Must be on timer 1
      CLR(TIMSK1, OCIE1A);                            //-Stop audio interrupt 
    #endif
    
    #if defined(TIMEonCTC0) && defined(PROCESSORmega328)
      CLR(TIMSK0, OCIE0B);                            //-Stop audio interrupt 
    #endif
    
    #if defined(PROCESSORtiny85) && defined(TIMEonCTC0)    // TIMEonCTC0 && PROCESSORtiny85 if it is an tiny85, then ...
      CLR(TIMSK, OCIE0B);                            //-Stop audio interrupt 
    #endif





// Add Tiny84 support here <--------------------------------------------------------- Place 9
    #if defined(PROCESSORtiny84) && defined(TIMEonCTC0)    // TIMEonCTC0 && PROCESSORtiny84 if it is an tiny84, then ...
      CLR(TIMSK0, OCIE0B);                            //-Stop audio interrupt 
    #endif





    //Resetting phase accumulators to zero assures voltage always starts at zero for consistent "on" click.
    PCW[0] = PCW[1] = PCW[2] = PCW[3] = 0;          //-Reset phase accumulators to avoid weird on/off click phasing patterns
  }
  
  void resume()
  {
    #ifndef TIMEonCTC0
      SET(TIMSK1, OCIE1A);                            //-Start audio interrupt
    #endif
    
    #if defined(TIMEonCTC0) && defined(PROCESSORmega328)  // if it's NOT a tiny85, then ...
      SET(TIMSK0, OCIE0B);                            //-Start audio interrupt
    #endif
    
    #if defined(TIMEonCTC0)  && defined (PROCESSORtiny85)                        // if it is an tiny85, then ...
      SET(TIMSK, OCIE0B);                            //-Start audio interrupt
    #endif
    


// Add Tiny84 support here <--------------------------------------------------------- Place 10
    #if defined(TIMEonCTC0)  && defined (PROCESSORtiny84)                        // if it is an tiny84, then ...
      SET(TIMSK0, OCIE0B);                            //-Start audio interrupt
    #endif


 
/*    CLR(TIMSK0, TOIE0);                            // - for testing, disable the overflow which breaks Arduino time functions (I'm just rracking jitter)*/
    
  }

};

#endif

// 
// 
// SPLIT DIFFERENTIAL WAVEFORM
// 
// This is intended for driving a center-tapped transformer where
// the center-tap is attached to a power rail and the outer sides
// of the winding are alternatively driven to the other rail.
// Such a transformer when coupled with a cap can serve as an LR filter
// and at the same time provide an isolated output, while also changing
// the voltage/current ratio of the output. Additionally a second cap can
// be placed on the secondary side of such a transformer to provide a second 
// stage LR filter which could be tuned to widen or narrow the bandpass.
// The output winding of such a transformer can also be biased to
// an external DC power rail, conducting lateral DC bias, allowing
// the signal produced here to be impressed onto an external
// power feed. The waveforms below are output by the processor
// as PWM (pulse width modulated) which would be suitable for driving 
// a power device with varying length short pulses to fully saturate the
// device. This would work well with power MOSFETs such as IRF510 or IRF610.
// You may want opto-isolators between the CPU and the MOSFETs.
// Many examples of such circuits can be found online as DIY Power Inverters.
// It is added here to support a telephone ring generator but may have
// other applications.
// 
// Output A
// 5V --                 ******                                                    
//    |               ***      ***                                                  
//    |             **            **                                                
//    |           **                **                                              The upper half of the sine
//    |          *                    *                                             wave is represented here 2x
//    |         *                      *                                           
//    |        *                        *                                          
//    |       *                          *                                        
//    |      *                            *                                       
//    |     **                            **                                      
//    |                                                                            <--- Gaurd band around 0V
// 0V ---***                                ***************************************
//                                         |< >|  <--- Guard Band around 0V                   
//                  
// Output B                                                                      
// 5V --                                                    ******                 
//    |                                                  ***      ***               
//    |                                                **            **              
//    |                                              **                **            The bottom half of the
//    |                                             *                    *           sine wave above is
//    |                                            *                      *          reflected here inverted
//    |                                           *                        *        
//    |                                          *                          *      
//    |                                         *                            *     
//    |                                        **                            **    
//    |                                                                           <--- Gard band around 0V
// 0V ---**************************************                                 ***
// 
// 
