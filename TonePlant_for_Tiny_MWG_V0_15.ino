      //First select a processor by uncommenting one (and only one) of the first 3 (PROCESSOR) defines
      // then chose a timer for interupts by uncomenting or comenting out the TIMEonCTC0 difinition (if comented out you are chosing CTC1, which only works on a Maga328)
      // then chose the timer for sculpting the tone waveform (and also the pin where it comes out) by uncommenting one and only one of TONEonCTC2 or TONEonCTC1.
      //     (fot tiny processors you must choose TIMEonCTC0 and TONEonCTC1)
      // then choose a user interface by uncommenting one and only one of the UIxxx options.
      // and lastly select a waveform output mode of DIFF, CHA, CHB, or SPDIF (They are Differential, Channel A, Channel B, or Split Differential, respectively.) Search for pwm_mode settings below.
      // For processor pinouts and pin definitions, see the very end of this file. 
      // For a schematic of the TinyUI user interface, see the very end of the TinyUI.h file.
      // For documentation about the Split Differential Waveform, see the very end of the tonegen.h file.

//#define PROCESSORmega328 // You can move the timers around but beware that the output pins will change if you set TONEonCTC1A or change pwm_mode = CHA | CHB | DIFF. Your timer one is 16 bit timers on this CPU
                         // Timer2 is 8 bit.
                         // With this processor enabled valid options are ((no TIMEonCTC0) (time defaults to CTC1) TONEonCTC2 and (pwm_mode = CHA | CHB | DIFF )) or (TIMEonCTC0 and (TONEonCTC2 | TONEonCTC1) and (pwm_mode = CHA | CHB | DIFF ))
                         //
//#define PROCESSORtiny84  //This 14 pin processor only has 2 timers, zero and one, and timer one is a 16 bit timer (same as the mega328 above).
                         // So, you must set TIMEonCTC0, TONEonCTC1A, and you may choose any pwm_mode, of the four.
                         // 
                         //
#define PROCESSORtiny85  //This 8 pin processor only has 2 timers, zero and one, and both timers are 8 bit.
                         // With this processor enabled the only valid options are TIMEonCTC0 and TONEonCTC1 and pwm_mode may be any of the four.
                         // Your tone is on DIP pin 6, your tuner pot goes on DIP pin 3, and your function selector pot goes on DIP pin 7
                         //
                         // To summarize some of the differences on processors above, they all have an 8 bit timer0, the 328 has two additional timers, T1 is 16bit, T2 is 8bit .
                         // Both tiny processors only have two timers 0 and 1, in the 84 timer 1 is 16 bit and in the 85 timer one is 8 bit.
                         //
#define TIMEonCTC0       // This will move the timer INT to Compare Register B of CTC0 without breaking mills or mocros of the Arduino environment
                         // This clears timer1 for PWM on processors that only have 2 timers like the ATtiny84 or ATtiny85 but should still be OK for ATmega328
                         // This will also change the waveform steps from 40,000Hz to 37,714.2957Hz with a step of 7 ticks on the compare register, or 19,230.7692Hz with a step of 13 ticks
                         //
//#define TONEonCTC2
#define TONEonCTC1      // This will move the PWM output signal to timer one
                         // I know, it's obnoxious in that it stomps on your serial port on an Uno, but it's one of the very few options that would also work on an ATtiny85 or ATtiny84
                         // this should be OK on any of the 3 processors above. But you must set TIMEonCTC0 above
                         //
//#define UIswitches     // This would be for Dons original switches on IO pins but would only work on the mega328 processor because it needs 8 IO pins. (and it is not properly put back in yet)
//#define UIpots         // This would be for Mikes original pots (potentiometers, one for "function" tone&cadense and one for tuning
#define UItinyUI       // This is for the TinyUI library which was developed specifically for this application but could be handy for other applications
                           //
                           //  Uncomment one and only one of the following 4 lines to set the default output mode 
//#define OutPWDmodeCHA    // Tone output on Channel A of the selected timer only
//#define OutPWDmodeCHB    // Tone output on Channel A of the selected timer only
#define OutPWDmodeDIFF   // Tone output differential on both A and B of the selected timer with B inverted
//#define OutPWDmodeSpDiff // Tone output as Split Differential on both A and B of the selected timer

//
//
//


/*                                                                        
 * MWG_V1 changes user interface to pots                                                                          
 * MWG_V2 adds a tuner pot Tonegen just grabs the global value and runs with it.
 * MWG_V3 Timer moved with TIMEonCTC0, gitter fixed, tuned to match the original. Now moving on to Tiny85_V4 to move the PWM to timer 1 for the Tiny84-85
 * MWG_V4 Added processor types, setOCRnloutput routines, 
 * MWG V5 tested moving timers all around on UNO with various pwm_modes (now moving on to attiny85 testing and debuging) 1/11/2024
 * MWG V6 Completed testing and tuning for ATtiny85! WOW! Dial Tone from an 8 pin chip! Moving on to the ATtiny84 where we'll have more pins! 1/12/2024
 * MWG V7 Moved tuner from A3 to A2. ATtiny 84 mostly done. discovered that TIMEonCT1 became broken for the Mega somewhere along the way. We'll look at that in V8 as we work on the user interface.
 * MWG V8 Just adds comments and cleans things up a bit.
 * MWG V0.9 changes version number convention and cleaned up comments some more. Note to self 6974 bytes flash and 74 bytes RAM for globals
 * MWG V0.10 Added tiny UI and got it working on the Mega and the tiny85 (somewhere between 10 and 11 I slowed the interrupts to fix ROH)
 * MWG V0.11 Fixed tiny UI for the ATtiny 84 and got CHB and DIFF working (although I'm not sure how to test that it's really inverted but we'll see when we split it)
 * MWG V0.12 Set some UK things back to Dons version, reworked timer setup with a table index and got SPDIF as far as the timers being unison for SPDIF (Mega328 TONEon-CTC1 may be tight on interupt time.)
 * MWG V0.13 Adding Tones for the ring frequencies, added pages to TinyUI
 * MWG V0.14 Finished adding the streight tones for ring and tested TinyUI more. 626 bytes left on the Tiny85 which is the tighest just now. moving on the finish SPDIF in interupt handler
 * MWG v0.15 Added SPDF to the interrupt handler for TONEonCTC1 only. That causes ROH (the most demanding) to start breaking up on the Mega. I'm not pressed about that. Just don't play ROH in SPDIF mode. (To fix it, one would increase the step size in the interrupt handler and reduce the tuning constant in setFrequency(), both in tonegen.h)
 * note to self on Memory utilization
 * version         Mega328   Tiny84  Tiny85
 * Flash (Program)  6974      6990    7616
 * RAM (globals)      74        74      74
 * V
 * Flash (Program)  7474      7090    7186
 * RAM (globals)      68        66      68
 * MWG V0.10 Adding TinyUI and storing config settings to EEPROM.
 *//*
  Telephone Switch Tone Plant Generator
  by: Don Froula
  7/20/2019
  Tonegen library supports up to 4 simultaneous tones
  on a single output pin. The output uses a 40KHz sampling frequency and 62.5KHz PWM frequency.
  These frequencies must be filtered from the output with a low pass filter.
  The PWM library generates tones with a sine wave/square wave lookup table, varying the pulse width of the output according
  to values in the 8-bit table. The low pass filter integrates the varying pulse width to a varying
  analog voltage. Nominal voltage at silence ("127") is 2.5 volts. Using differential output modes
  increases this to 5 volts, doubling the tone output voltage and increasing the volume.
*/
#define SET(x,y) (x |=(1<<y))                  //-Bit set/clear macros
#define CLR(x,y) (x &= (~(1<<y)))             // |
#define CHK(x,y) (x & (1<<y))                 // |
#define TOG(x,y) (x^=(1<<y))                  //-+

int tunerValue=0; //This is a tuning value from a pot that tonegen will use

#if defined( UItinyUI )
#include "EEPROM.h" // Adds nothing if you don't use it but we do need it before we include TinyUI
#include "TinyUI.h"     //The EEPROM User Interface Library
#endif

//     EEPROM Register Layout for TinyUI
//Register  Definition
//R0        Tone Low Nibble                                                 //4 bit 
//R1        Cadence Low Nibble
//R2        Master Tuning (10 bit value, use trimmer pot and not switches)
//R3        Master Volume (10 bit value, use trimmer pot and not switches) // It is probably a good idea not to try to reduce the volume ditially, by more than 6dB as you will lose waveform resolution. And this is not yet implemented in V0.15
//R4        Tone High Nibble                                               // The is implemented and tested in V0.15 for ring frequencies
//R5        Cadence High Nibble                                            // This is not yet implemented in V0.15
//R6        Unassigned
//...
//R14       Unassigned
//R15       Page Register (Store a 1 here to access the second page of 15 regiesters
//             or store a 2 here for a third page of registers, Etc.)
//             This register is cleared at power up (if set) and echoed on all pages
//             so that you can navigate. Above are all "Page Zero" registers. 
//             Below we will specify the page, but you can navigate back to the
//             registers above by storeing a zero in this register from any page.
//             Below we will use P for page, R for register of the page.
//P1R0      Unassigned
//...
//P1R14     Unassigned
//P1R15     Page Register (store a 0 here to access the first page of 15 regiesters) This register is cleared at power up and echoed on all pages so that you can navigate. 
//P2R0      Ring Generator Frequency selection, Time Slot 1 (See selection table below)
//P2R1      Fine frequency Tuning, Time Slot 1
//P2R2      Volume Adjust, Time Slot 1
//P2R3      Ring Generator Frequency selection, Time Slot 2
//P2R4      Fine frequency Tuning, Time Slot 2
//P2R5      Volume Adjust, Time Slot 2
//P2R6      Ring Generator Frequency selection, Time Slot 3
//P2R7      Fine frequency Tuning, Time Slot 3
//P2R8      Volume Adjust, Time Slot 3
//P2R9      Ring Generator Frequency selection, Time Slot 4
//P2R10     Fine frequency Tuning, Time Slot 4
//P2R11     Volume Adjust, Time Slot 4
//P2R12     Ring Generator Frequency selection, Time Slot 5
//P2R13     Fine frequency Tuning, Time Slot 5
//P2R14     Volume Adjust, Time Slot 5
//P2R15     Page Register (store a 0 here to access the first page of 15 regiesters) This register is cleared at power up and echoed on all pages so that you can navigate. 
//P4R0      Unassigned
//...
//Note that TinyUI stores registers as 16 bits (2 Bytes), even if the application uses the register as a 4 bit value (1 nibble).
//  The 10 bit value from the ADC is what is stored in EEPROM, and converted to a nibble when the application asks for a register as a 4 bit value. (It's not like we have a shortage of EEPROM.)
//
//Ring frequency selection (applies to registers Page2Register0, P2R3, P2R6, P2R9, P2R12) multi-phase multi-frequency ring (note: the low nibble matches here and for regular tones)
//0  = silence   =    B0000;  // Silence This is to allow you to skip one or more than one phase
//1  = 16Hz      =    B0001;  // Synchromonic series               (ITT HB5 ringers)
//2  = 16.66Hz   =    B0010;  // Harmonic series                   (ITT HA4 ringers)
//3  = 20Hz      =    B0011;  // Decimonic series                  (ITT HC1 ringers) and standard US central office ring
//4  = 25Hz      =    B0100;  // Harmonic series                   (ITT HA5 ringers)
//5  = 30Hz      =    B0101;  // Synchromonic and Decimonic series (ITT HB1 and HC1 ringers) and standard US key systems
//6  = 33.33Hz   =    B0110;  // Harmonic series                   (ITT HA1 ringers)
//7  = 40Hz      =    B0111;  // Decimonic series                  (ITT HC4 ringers)
//8  = 42Hz      =    B1000;  // Synchromonic series               (ITT HB2 ringers)
//9  = 50Hz      =    B1001;  // Decimonic and Harmonic series     (ITT HC5 and HA2 ringers)
//10 = 54Hz      =    B1010;  // Synchromonic series               (ITT HB3 ringers)
//11 = 60Hz      =    B1011;  // Decomonic series                  (ITT HC1 ringers)
//12 = 66Hz      =    B1100;  // Synchromic                        (ITT HB4 ringers)
//13 = 66.66Hz   =    B1101;  // Harmonic series                   (ITT HA3 ringers)
//
const unsigned char mLowtone =      B0000;    //Modulated Low Tone (600x120)
const unsigned char mRinging1 =     B0001;    //Modulated Ringing 1 (420x40)
const unsigned char mRinging2 =     B0010;    //Modulated Ringing 2 (500x40)
const unsigned char hz500 =         B0011;    //Old High Tone (500)
const unsigned char pDialtone =     B0100;    //Precise Dial Tone (350+440)
const unsigned char pRinging =      B0101;    //Precise Ringing (440+480)
const unsigned char pLowtone =      B0110;    //Precise Low Tone (480+620)
const unsigned char rohtones =      B0111;    //Receiver Off-Hook (ROH) tone (1400+2060+2450+2600)
const unsigned char hz480 =         B1000;    //New High Tone (480)
const unsigned char hz1004 =        B1001;    //Test Tone (1004)
const unsigned char ukOlddialtone = B1010;    //UK Old Dial Tone (400x33)
const unsigned char ukOldringing =  B1011;    //UK Old Ringing (400x133)
const unsigned char ukpDialtone =   B1100;    //UK Precise Dial Tone (350+450)
const unsigned char ukpRinging =    B1101;    //UK Ringing (400+450)
const unsigned char hz400 =         B1110;    //UK Equipment Engaged Tone/Congestion/Number Unavailable tone (400)
const unsigned char crybaby =       B1111;    //Crybaby (US Vacant Number) (200 to 400 to 200 at 1 Hz, interrupted every 6 seconds for 500ms)
//const unsigned char unas =      B010000;    //Unassigned
//       ...                        ...           ...
//const unsigned char unas =      B100000;    // Unassigned
const unsigned char hz16 =        B100001;    // Synchromonic series               (ITT HB5 ringers)    This section is to allow any party-line ring frequency to have any cadence
const unsigned char hz16p66 =     B100010;    // Harmonic series                   (ITT HA4 ringers)
const unsigned char hz20 =        B100011;    // Decimonic series                  (ITT HC1 ringers) and standard US central office ring
const unsigned char hz25 =        B100100;    // Harmonic series                   (ITT HA5 ringers)
const unsigned char hz30 =        B100101;    // Synchromonic and Decimonic series (ITT HB1 and HC1 ringers) and standard US key systems
const unsigned char hz33p33 =     B100110;    // Harmonic series                   (ITT HA1 ringers)
const unsigned char hz40 =        B100111;    // Decimonic series                  (ITT HC4 ringers)
const unsigned char hz42 =        B101000;    // Synchromonic series               (ITT HB2 ringers)
const unsigned char hz50 =        B101001;    // Decimonic and Harmonic series     (ITT HC5 and HA2 ringers)
const unsigned char hz54 =        B101010;    // Synchromonic series               (ITT HB3 ringers)
const unsigned char hz60 =        B101011;    // Decomonic series                  (ITT HC1 ringers)
const unsigned char hz66 =        B101100;    // Synchromic series                 (ITT HB4 ringers)
const unsigned char hz66p66 =     B101101;    // Harmonic series                   (ITT HA3 ringers)



const unsigned char  continuous =      B0000;  //Always ON (Continuous Tone)
const unsigned char  precisereorder =  B0001;  //250 ON 250 OFF (Precise Reorder)
const unsigned char  tollreorder =     B0010;  //200 ON 300 OFF (Toll Reorder)
const unsigned char  localreorder =    B0011;  //300 ON 200 OFF (Local Reorder)
const unsigned char  busy =            B0100;  //500 ON 500 OFF (Busy)
const unsigned char  roh =             B0101;  //100 ON 100 OFF (Receiver off hook)
const unsigned char  ring =            B0110;  //2000 ON 4000 OFF (Standard US Ring)
const unsigned char  partyline2 =      B0111;  //1000 ON 1000 OFF 1000 ON 3000 OFF (No. 5 Xbar Coded Ring 2)
const unsigned char  partyline3 =      B1000;  //1500 ON 500 OFF 500 ON 3500 OFF (No. 5 Xbar Coded Ring 3)
const unsigned char  partyline4 =      B1001;  //1500 ON 500 OFF 500 ON 500 OFF 500 ON 2500 OFF (No. 5 Xbar Coded Ring 4)
const unsigned char  partyline5 =      B1010;  //1500 ON 500 OFF 500 ON 500 OFF 1500 ON 1500 OFF (No. 5 Xbar Coded Ring 5)
const unsigned char  ukring =          B1011;  //400 ON 200 OFF 400 ON 2000 OFF (UK Ring)
const unsigned char  ukinvertedring =  B1100;  //400 OFF 200 ON 400 OFF 2000 ON (UK Inverted Ring)
const unsigned char  ukbusy =          B1101;  //375 ON 375 OFF (UK Busy)
const unsigned char  ukreorder =       B1110;  //400 ON 350 OFF 225 ON 525 OFF (UK Reorder)
const unsigned char  ukoldbusy =       B1111;  //750 ON 750 OFF (UK Old Busy)
const unsigned char  threePhaseRing =  B10000;  //3 phase ring 2000 ON 4000 OFF (Standard US Ring) or 800 ON 400 OFF 800 ON 4000 OFF //Use page 0 register 5 (B0101) to set the upper nibble to 0001 
const unsigned char  fourPhaseRing =   B10001;  //4 phase ring 1500 ON 4500 OFF (Its always 6 sec) or 600 ON 300 OFF 600 ON 4500 OFF //Then use page 2 registers of TinyUI to assign frequencies an a per phase basis.
const unsigned char  fivePhaseRing =   B10010;  //5 phase ring 1200 ON 4800 OFF (6 sec. cam speed) or 500 ON 200 OFF 500 ON 4800 OFF //The lower nibble is the same as the streight tones above and for
                                                    //better or worse, the upper nibble deposited in R4 Ob10 (0h2 for B10xxxx) for streight tones is the same as the register bank or page (2)
                                                    //used here for register 15 (page register)


#ifdef UIpots            // if we are using pots for the user interface
const byte pots =3;      // The number of pots that we have on this system
byte pot =0;             // Our pointer to which pot we are working with
byte potPin [pots];      // The Analog pin number of the particular pots
int potLastRead [pots];  //Our working veriables to filter the readings
byte pot0to255 [pots];   // Our output for other routines to use
byte scratch;            //A scratch variable to let setup loop run in the values
void readonepot()
/* This code was tested with a Bourns 2399 25 Turn, 50K trimmer pot.
 * If you use a one more than 50K, your results may be less stable.
 * If you can afford the current and use one smaller than 50k, the results should be more stable.
 * If you use a pot less than 25 turns, you'll likely be dissapointed, although 10K or less and 20 turns could be workable
 * We'd recommend a 25 turn 2.2k, set pot in the middle of setting desired, and test for stable results after reset.
 * If you are off center of setting desired, it may take time to lock in or may not lock in at all to the setting desired after reset.*/
{
#endif

/*#ifndef PROCESSORtiny84 // if NOT an ATtiny84*/
#if defined( UIpots ) && ! defined( PROCESSORtiny84 )

  if (potLastRead[pot] != analogRead(potPin [pot])) {      //If it is not the same, only then try again
  potLastRead [pot] = analogRead(potPin [pot])  ;          //we would have to have read it as diffenent from before twice to have changed it
                                                           //making it a majority vote, 2 of 3 between the prior run and these two trys
  }
#endif  

/*#ifdef PROCESSORtiny84 // if it IS an ATtiny84*/
#if defined( UIpots ) && defined( PROCESSORtiny84 )

  //   This processor seems to have different behavior if the analog pins are left floating. It seems that if you read a floating pin and then try to read an adjacent connected pin,
  //   you seem to get a bazzar reading. As a result, we turned on pull up resistors on our analog input pins. 
  //   As a result of that work around, we can no longer get to a reading of zero, so well fix that here.
  //
  //   This map function adds 312 bytes to the code. If you find yourself needing that space and haven't used map anywhere else, you may consider a different approach.
  //
  if (potLastRead[pot] !=      map(    analogRead(potPin [pot])  ,12,1023, 0,1023 )       ) { //If it is not the same, only then try again
  potLastRead [pot] =          map(    analogRead(potPin [pot])  ,12,1023, 0,1023 )           ; //we would have to have read it as diffenent from before twice to have changed it
                                                                   //making it a majority vote, 2 of 3 between the prior run and these two trys
  }
#endif

#if defined( UIpots )
  pot0to255 [pot]=((potLastRead [pot]+1 + pot0to255 [pot]*4)/2)/4; // Just a tiny bit more digital filtering or averaging for gitter and divide by 4
}

void readallofthepots()
{
for ( pot = 0; pot < pots; ++pot ) {
 readonepot();
  }
  //tunerValue = (potLastRead [1] -512)*50; //produce a tuning value to be passed to a modified tonegen (A2 is [1])
  tunerValue = (potLastRead [1] -512)*15; //produce a tuning value to be passed to a modified tonegen (A2 is [1])
  //tunerValue = 0;            //<------------------ TESTING
}
#endif


unsigned char pwm_mode;                        // Storage for PWM libray output mode and thus ouput pin assignments
unsigned char selected_tone = mLowtone;        // Default
unsigned char selected_cadence = continuous;   // Default

                   // This needs to go away so stop using it!   🡻🡻🡻🡻🡻------------------------------------------------------------

// This could probably be moved to tonegen and perhaps be put in the code itself manipulating the registers directly
// These routines will let PWM outputs be set as output. The registers and bits map to the Digital Pin and not the PWM source.
// What's more is that it varies based on the processor, so we'll just do this once and reference these routines.

void setOCR1Aoutput()
{
 #ifdef PROCESSORmega328
  // OCR1A PWM output is on PB1 (DIP pin 15)
  SET(DDRB, DDB1);                                   //-PWM pin B1 OCR1A PWM output is on PB1 (DIP pin 15) setOCR1Aoutput
  //pinMode(PB1, OUTPUT);//That didn't work but the one above did.
 #endif

 #ifdef PROCESSORtiny84
  // OCR1A PWM output is on PA6 (DIP pin 7) Note that this is the only one on Data Direction Register A
  SET(DDRA, DDA6);                                   //-PWM pin A6 DIP pin 7 setOCR1Aoutput() PROCESSORtiny84
  //pinMode(PA6, OUTPUT); 
 #endif

 #ifdef PROCESSORtiny85
  // OCR1A PWM output is on PB1 (DIP pin 6)
  //SET(DDRB, DDB1);                             // set as output PWM A side on an Attiny85 like setOCR1Aoutput() PROCESSORtiny85 should be chip pin6
  pinMode(PB1, OUTPUT);  //That seems to work
 #endif
}

void setOCR1Boutput()
{
 #ifdef PROCESSORmega328
  // OCR1B PWM output is on PB2 (DIP pin 16)
  SET(DDRB, DDB2);                               //OCR1B PWM output is on PB2 (DIP pin 16) setOCR1Boutput() PROCESSORmega328
  //pinMode(PB2, OUTPUT); //That didn't work
 #endif

 #ifdef PROCESSORtiny84
  // OCR1B PWM output is on PA5 (DIP pin 8)
  SET(DDRB, DDA5);              // OCR1B PWM output is on PA5 (DIP pin 8) PROCESSORtiny84 setOCR1Boutput() <----- I think that has a typeo DDRB should be DDRA
  //pinMode(PA5, OUTPUT);// 
 #endif

 #ifdef PROCESSORtiny85
  // OCR1B PWM output is on PB4 (DIP pin 3)
  //SET(DDRB, 0x10);
  //DDRB = DDRB | B00010000;  //Set Data Direction Bit DDB4
  //DDRB = DDRB | B00010000;  //Set Data Direction Bit DDB4
  //SET(DDRB, DDB4);          //Set Data Direction Bit DDB4 like setOCR1Boutput() PROCESSORtiny85 should be chip pin 3
  //pinMode(PB4, OUTPUT); //The tuner pot goes here, so we really don't ever want to do this.
 #endif
}

void setOCR2Aoutput()
// This only exists on the mega328 
// if you have a mismatch the between the IDE and selected processor above, it just will not compile.
{
 #ifdef PROCESSORmega328
  // OCR2A PWM output is on PB3 (DIP pin 17)
  //SET(DDRB, DDB3);
  pinMode(PB3, OUTPUT);
 #endif
}

void setOCR2Boutput()
// This only exists on the mega328 
{
 #ifdef PROCESSORmega328
  // OCR2B PWM output is on PD3 (DIP pin 5) Note that this is the only one on Data Direction Register D
  //SET(DDRD, DDD3);
  pinMode(PD3, OUTPUT);
 #endif
}
                   // This needs to go away so stop using it!  🡹🡹🡹🡹🡹-----------------------------------------------------------     
                   
#include "tonegen.h"  //Tone generation library 
//Instaniate PWM tone generator
tonegen tonePlayer;

//New code to innitilize TinyUI goes here
#if defined( UItinyUI )
TinyUI mytinyUI(3); // Create an instance of the UI
#endif

void playOneSine (float freq1, unsigned long length) {
  tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        tonePlayer.setVolume(250, 0, 0, 0);
        playPWM(freq1, 0, 0, 0, length);
}




//Setup                             Setup                    Setup                      Setup
void setup() {

//  EEPROM.get(0, scratch);// Thesting to see how much it adds to the code size. This only adds 30 BYTES! Yeay!
/*   Serial.begin(9600); // Initialize serial port for debugging
   Serial.print("\n  Reset - \J \m");
  Serial.print("\n  TCCR0A - \t");
  Serial.println(TCCR0A, HEX);
  Serial.print("\n  TCCR0B - \t");
  Serial.println(TCCR0B, HEX);

  CLR (TCCR0A, WGM00); 
    CLR (TCCR0A, WGM01); 
    CLR (TCCR0B, WGM02); 

      Serial.print("\n  TCCR0A - \t");
  Serial.println(TCCR0A, HEX);
  Serial.print("\n  TCCR0B - \t");
  Serial.println(TCCR0B, HEX);*/

  // Configure the tone select pins for input - Upper nibble of Port D, Atmega328p pins 13,12,11,6 (from MSB to LSB)
//  DDRD = DDRD &
//         ~ (
//           (1 << DDD4) |  // pinMode( 4, INPUT ); // Set to input - Pin 6
//           (1 << DDD5) |  // pinMode( 5, INPUT ); // Set to input - Pin 11
//           (1 << DDD6) |  // pinMode( 6, INPUT ); // Set to input - Pin 12
//           (1 << DDD7)    // pinMode( 7, INPUT ); // Set to input - Pin 13
//         );

  // Enable the pullups
//  PORTD = PORTD |
//          (
//            (1 << PORTD4) |  // digitalWrite( 4, HIGH ); // Enable the pullup
//            (1 << PORTD5) |  // digitalWrite( 5, HIGH ); // Enable the pullup
//            (1 << PORTD6) |  // digitalWrite( 6, HIGH ); // Enable the pullup
//            (1 << PORTD7)    // digitalWrite( 7, HIGH ); // Enable the pullup
//          );

//  // Configure the cadence select pins for input - Lower nibble of Port B, Atmega328p pins 17,16,15,14 (from MSB to LSB)
//  DDRB = DDRB &
//         ~ (
//           (1 << DDB0) |  // pinMode( 8, INPUT ); // Set to input - Pin 14
//           (1 << DDB1) |  // pinMode( 9, INPUT ); // Set to input - Pin 15
//           (1 << DDB2) |  // pinMode( 10, INPUT ); // Set to input - Pin 16
//           (1 << DDB3)    // pinMode( 11, INPUT ); // Set to input - Pin 17
//         );

  // Enable the pullups
//  PORTB = PORTB |
//          (
//            (1 << PORTB0) |  // digitalWrite( 8, HIGH ); // Enable the pullup
//            (1 << PORTB1) |  // digitalWrite( 9, HIGH ); // Enable the pullup
//            (1 << PORTB2) |  // digitalWrite( 10, HIGH ); // Enable the pullup
//            (1 << PORTB3)    // digitalWrite( 11, HIGH ); // Enable the pullup

//          );

#ifdef PROCESSORmega328
  //Set audio output physical pin 5 as output
  //DDRD |= (1 << DDD3);
  //setOCR2Boutput();

  //Set relay driver physical pin 4 as output
  DDRD |= (1 << DDD2);

  //Set cadence LED pin as output
  DDRB |= (1 << DDB5);

  //Turn off relay output
  PORTD = PORTD & B11111011;

  //Turn off cadence LED
  PORTB = PORTB & B11011111;
#endif

#ifdef PROCESSORtiny85
  //setOCR1Aoutput(); // This gets done by tonegen so we really don,t need to do it here.
  pinMode(PB0, OUTPUT); // Set DIP pin 5 as output for the cadence
#endif







//Uncomment ONE (and only one) of the following four lines to set audio output mode and pin(s) (I got tired of hunting for this so I added defines for it at the top MWG 2/15/25)
  #ifdef OutPWDmodeCHA
  pwm_mode = CHA;  //Single ended signal in CHA pin (11) - Physical Pin 17 on chip
  #endif
  #ifdef OutPWDmodeCHB
  //pwm_mode = CHB;  //Single ended signal on CHB pin (3) - Physical Pin 5 on chip (or if define TONEonCTC1 Arduino pin 10 of Uno)
  #endif
  #ifdef OutPWDmodeDIFF
  pwm_mode = DIFF; //Differntial signal on CHA and CHB pins (11,3) - Physical Pin 17 and Physical Pin 5 on chip
  #endif
  #ifdef OutPWDmodeSpDiff
  pwm_mode = SPDIF; //Split Differntial signal on CHA and CHB (See the end of tonegen.h for documentation on this.)
  #endif

  tonePlayer.begin(pwm_mode); //Sets up output pins and single-ended or differential mode, according to the uncommented mode above.

  //Set default wave table for each voice
  tonePlayer.setWave(SINE, SINE, SINE, SQUARE);

  tonePlayer.setVolume(60, 60, 60, 60);

//New code to innitilize TinyUI goes here
#if defined( UItinyUI )

mytinyUI.begin();
mytinyUI.service();
#endif


// set up the user interface pots

/*#ifdef PROCESSORtiny84*/
#if defined( UIpots ) && defined( PROCESSORtiny84 )
potPin [0]=A1; // set the pot pin for this pot to A1 (Function)
potPin [1]=A2; // set the pot pin this pot to A2 (Tuner)
potPin [2]=A3; // set the pot pin this pot to A3
#endif

/*#ifdef PROCESSORtiny85*/
#if defined( UIpots ) && defined( PROCESSORtiny85 )
potPin [0]=A1; // set the pot pin for this pot to A1 (Function)
potPin [1]=A3; // set the pot pin this pot to A2 (Tuner)
potPin [2]=A3; // set the pot pin this pot to A3
#endif

/*#ifdef PROCESSORmega328*/
#if defined( UIpots ) && defined( PROCESSORmega328 )

potPin [0]=A1; // set the pot pin for this pot to A1 (Function)
potPin [1]=A2; // set the pot pin this pot to A2 (Tuner)
potPin [2]=A3; // set the pot pin this pot to A3
#endif



 #ifdef PROCESSORtiny84 // <----------------- I really dont remember why this is here
//pinMode(1, INPUT_PULLUP);
//pinMode(2, INPUT_PULLUP); // Wow taking out this section we went from 7156 bytes of flash to 7030 bytes MWG 2/11/25
//pinMode(3, INPUT_PULLUP);
 #endif

#if defined( UIpots )
// and run in the values before we start making tones
for ( scratch = 0; scratch < 20; ++scratch ) {
 readallofthepots();
  }
  #endif

}




//Main loop                             Main loop                    Main loop                      Main loop
void loop()
{

  // Read all input pins and set tone and cadence variables
  
  //Read Port D, shift to the right four places, take the complement, and mask the upper 4 bits
//  selected_tone = (~((PIND & ( (1 << PIND4) | (1 << PIND5) | (1 << PIND6) | (1 << PIND7) )) >> 4) & B00001111);

  //Read Port B, take the complement, and mask the upper 4 bits
//  selected_cadence = (~(PINB & ( (1 << PINB0) | (1 << PINB1) | (1 << PINB2) | (1 << PINB3) )) & B00001111);

#if defined( UIpots )
readallofthepots(); // 
selected_tone = (pot0to255[0] & B00001111);
selected_cadence = (pot0to255[0] & B11110000)/16;
#endif


// new code here to use TinyUI in the run loop
#if defined( UItinyUI )
  mytinyUI.service();
  mytinyUI.serviceEvent();


  
  //selected_tone = (mytinyUI.readRegAsSwitches(0)); // Register zero is the selected tone (If you implement an upper nibble you will need to slide it in here)
  
    selected_tone = (((mytinyUI.readRegAsSwitches(4) << 4 ) + (mytinyUI.readRegAsSwitches(0)))); // Register 4 is the high nibble and register 0 
    if (selected_tone > 239)  selected_tone = (selected_tone - 240); // unitialized EEPROM locations read as FF, so if the upper nibble is all ones then make them zeros
    
  
  //selected_cadence = (mytinyUI.readRegAsSwitches(1)); // Register one is the selected cadence (And here)
  
  selected_cadence = (mytinyUI.readRegAsSwitches(5) << 4 ) + (mytinyUI.readRegAsSwitches(1));
  if (selected_cadence > 239)  selected_cadence = (selected_cadence -240 ); // unitialized EEPROM locations read as FF, so if the upper nibble is all ones then make them zeros
  //if (selected_cadence > 239)  selected_cadence = (selected_cadence & 0x0f ); // unitialized EEPROM locations read as FF, so if the upper nibble is all ones then make them zeros
  
  tunerValue = ((mytinyUI.readRegAsPot(2)) *20); //Read Register two as the tuner value (this basically did the same thing as below)
  //tunerValue = ((mytinyUI.readRegAsPot(2)) << 3 ); //Read Register two as the tuner value times 16 (this added 8 bytes
//

#endif

//selected_tone = 0b00001001; // <-------------TESTING 1004
//selected_cadence = 0b00000000; Steady
//    Dial Tone
//selected_tone = 0b00000100; // pDialtone
//selected_cadence = 0b00000000; //Steady 
//    Busy
//selected_tone = 0b00000110; // Busy
//selected_cadence = 0b00000100;
//    Ring
//selected_tone = 0b00000101; // pRing
//selected_cadence = 0b00000110;
  
  playcadence();

}
//End of loop


void playcadence()
{
  switch (selected_cadence) {
    case continuous:
      playtone(0);
      break;
    case precisereorder:
      playtone(250);
      #ifndef UItinyUI
      delay(250);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(250);
      #endif
      break;
    case tollreorder:
      playtone(200);
      #ifndef UItinyUI
      delay(300);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(300);
      #endif
      break;
    case localreorder:
      playtone(300);
      #ifndef UItinyUI
      delay(200);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(200);
      #endif  
      break;
    case busy:
      playtone(500);
      #ifndef UItinyUI
      delay(500);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(500);
      #endif 
      break;
    case roh:
      playtone(100);
      #ifndef UItinyUI
      delay(100);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(100);
      #endif
      break;
    case ring:
      playtone(2000);
      #ifndef UItinyUI
      delay(4000);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(4000);
      #endif
      break;
    case partyline2:
      playtone(1000);
      #ifndef UItinyUI
      delay(1000);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(1000);
      #endif
      playtone(1000);
      #ifndef UItinyUI
      delay(3000);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(3000);
      #endif 
      break;
    case partyline3:
      playtone(1500);
      #ifndef UItinyUI
      delay(500);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(500);
      #endif   
      playtone(500);
      #ifndef UItinyUI
      delay(3500);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(3500);
      #endif
      break;
    case partyline4:
      playtone(1500);
      #ifndef UItinyUI
      delay(500);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(500);
      #endif
      playtone(500);
      #ifndef UItinyUI
      delay(500);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(500);
      #endif
      playtone(500);
      #ifndef UItinyUI
      delay(2500);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(2500);
      #endif
      break;
    case partyline5:
      playtone(1500);
      #ifndef UItinyUI
      delay(500);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(500);
      #endif
      playtone(500);
      #ifndef UItinyUI
      delay(500);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(500);
      #endif
      playtone(1500);
      #ifndef UItinyUI
      delay(1500);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(1500);
      #endif
      break;
    case ukring:
      playtone(400);
      #ifndef UItinyUI
      delay(200);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(200);
      #endif 
      playtone(400);
      #ifndef UItinyUI
      delay(2000);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(2000);
      #endif
      break;
    case ukinvertedring:
      playtone(200);
      #ifndef UItinyUI
      delay(400);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(400);
      #endif
      playtone(2000);
      #ifndef UItinyUI
      delay(400);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(400);
      #endif
      break;
    case ukbusy:
      playtone(375);
      #ifndef UItinyUI
      delay(375);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(375);
      #endif
      break;
    case ukreorder:
      playtone(400);
      #ifndef UItinyUI
      delay(350);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(350);
      #endif
      playtone(225);
      #ifndef UItinyUI
      delay(525);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(252);
      #endif
      break;
    case ukoldbusy:
      playtone(750);
      #ifndef UItinyUI
      delay(750);
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(750);
      #endif
      break;
    default:
      playtone(0);
      break;
  }
}


// Total volume on a tone for all four voices MAY NOT EXCEED 250!!!!!!!!
void playtone(unsigned long length)
{
  switch (selected_tone) {
    case mLowtone:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      //tonePlayer.setVolume(80, 80, 58, 22);
      tonePlayer.setVolume(80, 80, 80, 0);
      playPWM(480, 720, 600, 0, length);
      break;
    case mRinging1:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(80, 80, 80, 0);
      playPWM(380, 460, 420, 0, length);
      break;
    case mRinging2:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      //tonePlayer.setVolume(80, 80, 68, 12);
      tonePlayer.setVolume(80, 80, 80, 0);
      playPWM(460, 540, 500, 0, length);
      break;
    case hz500:
      playOneSine (500, length);
      //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      //tonePlayer.setVolume(240, 0, 0, 0);
      //playPWM(500, 0, 0, 0, length); // <----Was 500
      break;
    case pDialtone:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(120, 120, 0, 0);
      playPWM(350, 440, 0, 0, length);
      break;
    case pRinging:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(120, 120, 0, 0);
      playPWM(440, 480, 0, 0, length);
      break;
    case pLowtone:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(120, 120, 0, 0);
      playPWM(480, 620, 0, 0, length);
      break;
    case rohtones:
      tonePlayer.setWave(SINE, SINE, SINE, SINE);
      tonePlayer.setVolume(60, 60, 60, 60);
      playPWM(1400, 2060, 2450, 2600, length);
      break;  
    case hz480:
      playOneSine (480, length);
      //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      //tonePlayer.setVolume(240, 0, 0, 0);
      //playPWM(480, 0, 0, 0, length);
      break;
    case hz1004:
      playOneSine (1004, length);
     // tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      //tonePlayer.setVolume(240, 0, 0, 0);
      //playPWM(1004, 0, 0, 0, length);
      break;
    case ukOlddialtone:
      tonePlayer.setWave(SQUARE, SQUARE, SQUARE, SQUARE);
      //tonePlayer.setWave(SINE, SINE, SINE, SINE); //<====== Testing Yea, square is a problem and gets stuck cadensing and won't let the function selector pot sweep past it.
      tonePlayer.setVolume(20, 20, 20, 180);
      playPWM(434, 366, 400, 33, length);
      break;
    case ukOldringing:
      tonePlayer.setWave(SQUARE, SQUARE, SQUARE, SQUARE);
      //tonePlayer.setWave(SINE, SINE, SINE, SINE); //<====== Testing Yea, square is a problem and gets stuck cadensing and won't let the function selector pot sweep past it.
      tonePlayer.setVolume(20, 20, 20, 180);
      //playPWM(433,367,400,133,length);
      playPWM(434, 366, 400, 133, length);
      break;
    case ukpDialtone:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(120, 120, 0, 0);
      playPWM(350, 450, 0, 0, length);
      break;
      
    case ukpRinging:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(120, 120, 0, 0);
      playPWM(400, 450, 0, 0, length);
      break;
      
    case hz400:
      playOneSine (400, length);
      //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      //tonePlayer.setVolume(240, 0, 0, 0);
      //playPWM(400, 0, 0, 0, length);
      break;
      
    case crybaby:
    if (selected_cadence == continuous) {  //Only play crybaby if set to continuous cadence, otherwise silence
      tonePlayer.setWave(RAMP, SINE, SQUARE, SQUARE);
      tonePlayer.setVolume(250, 0, 0, 0);
      starttones();
      for (int j = 1; j <= 6; j++) { //This was making things hard with pots and getting stuck
        for (int i = 0; i < 100; i++) {
           //Read exponential table to mimic capacitor discharge of the real Crybaby circuit (frequency rises)
           changetones(pgm_read_float(discharge_sequence + i), 0, 0, 0);
           #ifndef UItinyUI
           delay(5);
           #endif
           #ifdef UItinyUI
           mytinyUI.serviceDelay(5);
           #endif   
           } // end of i loop
         for (int i = 0; i < 100; i++) {
          //Read exponential table to mimic capacitor charge of the real Crybaby circuit (frequency falls)
           changetones(pgm_read_float(charge_sequence + i), 0, 0, 0);
           #ifndef UItinyUI
           delay(5);
           #endif
           #ifdef UItinyUI
           mytinyUI.serviceDelay(5);
           #endif     
         } //end of I loop
        } //This was making things hard with pots and getting stuck
      stoptones();
      //delay(500);
              }
      else {  //Play silence
        tonePlayer.setWave(SINE, SINE, SINE, SINE);
        tonePlayer.setVolume(120, 0, 0, 0);
        playPWM(400, 0, 0, 0, length);
        }
      break;


      case hz16:
        playOneSine (16, length);
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        //tonePlayer.setVolume(250, 0, 0, 0);
        //playPWM(16, 0, 0, 0, length);
        break;

      case hz16p66:
        playOneSine (16.66, length);
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        //tonePlayer.setVolume(250, 0, 0, 0);
        //playPWM(16.66, 0, 0, 0, length);
        break;

      case hz20:
        playOneSine (20, length);
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        //tonePlayer.setVolume(250, 0, 0, 0);
        //playPWM(20, 0, 0, 0, length);
        break;

      case hz25:
        playOneSine (25, length);
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        //tonePlayer.setVolume(250, 0, 0, 0);
        //playPWM(25, 0, 0, 0, length);
        break;

      case hz30:
        playOneSine (30, length);
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        //tonePlayer.setVolume(250, 0, 0, 0);
        //playPWM(30, 0, 0, 0, length);
        break;
        
      case hz33p33:
        playOneSine (33.33, length);
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        //tonePlayer.setVolume(250, 0, 0, 0);
        //playPWM(33.33, 0, 0, 0, length);
        break;

      case hz40:
        playOneSine (40, length);
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        //tonePlayer.setVolume(250, 0, 0, 0);
        //playPWM(40, 0, 0, 0, length);
        break;

      case hz42:
        playOneSine (42, length);
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        //tonePlayer.setVolume(250, 0, 0, 0);
        //playPWM(42, 0, 0, 0, length);
        break;

      case hz54:
        playOneSine (54, length);
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        //tonePlayer.setVolume(250, 0, 0, 0);
        //playPWM(54, 0, 0, 0, length);
        break;

      case hz60:
        playOneSine (60, length);                        // 12 bytes
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);  // 40 bytes
        //tonePlayer.setVolume(250, 0, 0, 0);            // 18 bytes
        //playPWM(60, 0, 0, 0, length);                  // 30 bytes
        break;

      case hz66:
        playOneSine (66, length);
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        //tonePlayer.setVolume(250, 0, 0, 0);
        //playPWM(66, 0, 0, 0, length);
        break;
     
      case hz66p66:
        playOneSine (60.66, length);
        //tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
        //tonePlayer.setVolume(250, 0, 0, 0);
        //playPWM(60.66, 0, 0, 0, length);
        break;
      
      
  }
}


//Play PWM-generated mixed tones
void playPWM(float freq1, float freq2, float freq3, float freq4, unsigned long length)
{
  tonePlayer.setFrequency(0, freq1); //Set the new Tone 1 frequency
  tonePlayer.setFrequency(1, freq2); //Set the new Tone 2 frequency
  tonePlayer.setFrequency(2, freq3); //Set the new Tone 3 frequency
  tonePlayer.setFrequency(3, freq4); //Set the new Tone 4 frequency
  tonePlayer.resume(); //Turn on PWM interrupts
  
  #ifdef PROCESSORmega328
    PORTD = PORTD | B00000100;  //Turn on relay output
    PORTB = PORTB | B00100000;  //Turn on cadence LED
  #endif

  #ifdef PROCESSORtiny85
    digitalWrite(PB0, HIGH); //Turn on cadence LED and/or relay
  #endif
   
  if (length > 0) {
      #ifndef UItinyUI
      delay(length); //Wait for the tone duration
      #endif
      #ifdef UItinyUI
      mytinyUI.serviceDelay(length); //Wait for the tone duration
      #endif
    tonePlayer.suspend(); //Turn off PWM interrupts
    
    #ifdef PROCESSORmega328
      PORTD = PORTD & B11111011;  //Turn off relay output
      PORTB = PORTB & B11011111;  //Turn off cadence LED
    #endif

    #ifdef PROCESSORtiny85
      digitalWrite(PB0, LOW); //Turn off cadence LED and/or relay
    #endif
    
    tonePlayer.setFrequency(0, 0); //Set for silence
    tonePlayer.setFrequency(1, 0); //Set for silence
    tonePlayer.setFrequency(2, 0); //Set for silence
    tonePlayer.setFrequency(3, 0); //Set for silence
  }
}

//Change frequencies
void changetones(float freq1, float freq2, float freq3, float freq4)
{
  tonePlayer.setFrequency(0, freq1); //Set the new Tone 1 frequency
  tonePlayer.setFrequency(1, freq2); //Set the new Tone 2 frequency
  tonePlayer.setFrequency(2, freq3); //Set the new Tone 3 frequency
  tonePlayer.setFrequency(3, freq4); //Set the new Tone 4 frequency
}

void starttones()
{
  tonePlayer.setFrequency(0, 0); //Set for silence
  tonePlayer.setFrequency(1, 0); //Set for silence
  tonePlayer.setFrequency(2, 0); //Set for silence
  tonePlayer.setFrequency(3, 0); //Set for silence
  tonePlayer.resume(); //Turn on PWM interrupts
  
  #ifdef PROCESSORmega328
    PORTD = PORTD | B00000100;  //Turn on relay output
    PORTB = PORTB | B00100000;  //Turn on cadence LED
  #endif
  
  #ifdef PROCESSORtiny85
    digitalWrite(PB0, HIGH); //Turn on cadence LED and/or relay
  #endif
  
}

void stoptones()
{
  tonePlayer.suspend(); //Turn off PWM interrupts
  
  #ifdef PROCESSORmega328
    PORTD = PORTD & B11111011;  //Turn off relay output
    PORTB = PORTB & B11011111;  //Turn off cadence LED
  #endif
  
  #ifdef PROCESSORtiny85
    digitalWrite(PB0, LOW); //Turn off cadence LED and/or relay
  #endif
  
  tonePlayer.setFrequency(0, 0); //Set for silence
  tonePlayer.setFrequency(1, 0); //Set for silence
  tonePlayer.setFrequency(2, 0); //Set for silence
  tonePlayer.setFrequency(3, 0); //Set for silence
}

/*
I dont have a Mega328 in a DIP package to test with, so Im a bit hesitant to put the DIP pins for the outputs of a DIP processor here. 


                                                        ATmega328
                                                       +---\/---+
                                         (RESET)  PC6  |1     28|  PC5  (ADC5 / SCL)
                                           (RXD)  PD0  |2     27|  PC4  (ADC4 / SDA)
                                           (TXD)  PD1  |3     26|  PC3  (ADC3) <--------------------- Tuner Pot IN
                                          (INT0)  PD2  |4     25|  PC2  (ADC2) 
                         -----> (PWM)     (INT1)  PD3  |5     24|  PC1  (ADC1) <--------------------- Function Selector Pot IN (Address for TinyUI)
                                      (XCK / T0)  PD4  |6     23|  PC0  (ADC0)
                                                  VCC  |7     22|  GND
                                                  GND  |8     21|  AREF
                                 (XTAL1 / TOSC1)  PB6  |9     20|  AVCC
                                 (XTAL2 / TOSC2)  PB7  |10    19|  PB5  (SCK)
                                (PWM)       (T1)  PD5  |11    18|  PB4  (MISO)
                                (PWM)     (AIN0)  PD6  |12    17|  PB3  (MOSI / OC2) (PWM) <-----
                                          (AIN1)  PD7  |13    16|  PB2  (SS / OC1B)  (PWM) <-----
                                          (ICP1)  PB0  |14    15|  PB1  (OC1A)       (PWM) <-----
                                                       +--------+


This Processor has enough pins for additional cadence outputs. It only has 2 timers and timer0 is tied up with millis() and too slow for us, so we're limited to 2 tones out.
It also will not run 16MHz without an external OSC so we're running it at 8MHz. At this speed, it really can't do the Receiver-Off-Hook tone.
An external OSC will take 2 pins, so the net gain by steppin up to this chip is 4 additional IO pins over the ATtiny85

                                                        ATtiny84
                                                       +---\/---+
Vcc 5V IN   --------------------->                VCC  |1     14|  GND         <--------------------- Ground IN
Cadence OUT (or diag serial out)->       (XTAL1)  PB0  |2     13|  PA0  (ADC0)
                                         (XTAL2)  PB1  |3     12|  PA1  (ADC1) <--------------------- Function Selector Pot IN (Address for TinyUI)
                                         (RESET)  PB3  |4     11|  PA2  (ADC2) <--------------------- Tuner Pot IN
                                   (INT0 / OC0A)  PB2  |5     10|  PA3  (ADC3) <--------------------- Tuner Pot IN (may actually be here, please check this)
                                   (ADC7 / OC0B)  PA7  |6      9|  PA4  (ADC4 / SCK / SCL)
PWM Audio OUT -------------------> (ADC6 / OC1A)  PA6  |7      8|  PA5  (ADC5 / OC1B / MISO)) <------ Future PWM Audio2 out
                                                       +--------+



                                                        ATtiny85
                                                       +---\/---+
                                  (RESET / ADC0)  PB5  |1*     8|  VCC         <--------------------- Vcc 5V IN
Tuner Pot IN (Data for TinyUI) -->        (ADC3)  PB3  |2      7|  PB2  (ADC1) <--------------------- Function Selector Pot IN (Address for TinyUI)
Future PWM Audio2 OUT | Cadence2 > (OC1B / ADC2)  PB4  |3      6|  PB1  (OC0B / OC1A) <-------------- PWM Audio OUT
Ground IN  --------------------->                 GND  |4      5|  PB0  (OC0A) <--------------------- Cadence OUT 
                                                       +--------+


FUTURE CONSIDERATIONS

"TIMER_TO_USE_FOR_MILLIS" could be worth exploring for some ATtiny cores.

The Bourns resister array 4610X-R2R-103LF (1R 2R resister ladder array aka parallel D to A converter) could be handy for the user interface.


*/
