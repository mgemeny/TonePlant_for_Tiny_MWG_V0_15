/*
   TinyUI.c - Library for a detachable tiny user interface for tiny ATtiny processors
            - handy for setting configurations in-the-field and removing the "Keyboard" when done
            - One 4 position DIP switch of Address with a Select button
            - One 4 position DIP switch of Data
            - Optional Tuner Potentiometer for full 10 bit resolution data
            - Set address, short press to audition, adjust data, long press to save
            - See schematic and notes at end of this file
            -  For more than 15 registers, we now use the 16th register (register 15) as a "Page" regiser to "Turn the Page" of the other registers.
            - M. W. Gemeny (AKA .MWG., Mike, That old assembly language programmer pretending to write C)
            -
            -Memory utilization
            - Globals 7 bytes
            - STACK (and heap)
            -checkFORkeyPress 3 bytes
            -convert2nibble 3 bytes
            -processService 4 bytes plus checkFORkeyPress
            -processEvent convert2nibble 3 or EEPROM.update (probably 2 bytes)
            -TinyUI probably 1 byte
            -TinyUI::serviceDelay 6 bytes
            -readRegAsSwitches 2
            -readRegAsPot 3
            - It looks to me as though 8 bytes of stack would be worst case
            - So, that, with the globals makes about 15 bytes of RAM (I think) 
            - 
            - Version V0.01 2/4/2024
            - Version V0.0.4 1/20/2025 Added EEPROM support and tested 
            - Version C0.0.5 1/20/2025 Starting to optimize
*/

/*#ifndef TinyUI_h
#define TinyUI_h*/
#define SET(x,y) (x |=(1<<y))                  //-Bit set/clear macros
#define CLR(x,y) (x &= (~(1<<y)))             // |
#define CHK(x,y) (x & (1<<y))                 // |
#define TOG(x,y) (x^=(1<<y))                  //-+

#include "Arduino.h"

#include <avr/pgmspace.h>
//***********************
unsigned int presstime;
unsigned int addressSwitches; // This is the raw AD value
unsigned int dataSwitches;    // This is the raw AD value
byte whosInAudition = 255;    // This is the effective register number //start with nobody in audition mode

//byte registerBytes[8];// These are your byte registers accessable by the nibble
//int registerWords[1];// These are your 16 bit registers for analog values from the pot
//***********************

bool checkFORkeyPress()  {
  //    Check to see of we have a keyPress
  
//Address side of our keyboard was Function Selector.
//A1 is PC1 on a Mega
//A1 is PA1 on a Tiny84 (14 pin CPU)
//A1 is PB2 on a Tiny85 (8 pin CPU)

#ifdef PROCESSORmega328
//int arWOpullup = analogRead(A1); // Rememnber where we started
SET(PORTC, PORTC1); // Turn the pullup ON  to charge things up
int ARafterpullup = analogRead(A1); // Read the pin again. If it's floating it should now be way up from what it was.
CLR(PORTC, PORTC1); // Turn the pullup OFF
#endif

#ifdef PROCESSORtiny84
//int arWOpullup = analogRead(A1); // Rememnber where we started
SET(PORTA, PORTA1); // Turn the pullup ON  to charge things up
int ARafterpullup = analogRead(A1); // Read the pin again. If it's floating it should now be way up from what it was.
CLR(PORTA, PORTA1); // Turn the pullup OFF
#endif

#ifdef PROCESSORtiny85
//int arWOpullup = analogRead(A1); // Rememnber where we started
SET(PORTB, PORTB2); // Turn the pullup ON  to charge things up
int ARafterpullup = analogRead(A1); // Read the pin again. If it's floating it should now be way up from what it was.
CLR(PORTB, PORTB2); // Turn the pullup OFF
#endif

if    (ARafterpullup >=950   )     { // If turning on the pullup moved the value up by much, we probably don't have at keyPress (or perhaps not even a Key Board).
  return false; // We don't seem to have a keyPress
  }
else {
  return true;  // Yes we have a keyPress
  }
}

//#ifndef PROCESSORtiny84 // if it is NOT an attiny84
 byte  convert2nibble(int dataIn)  {
    // Non linear table for reading switches
    //
    // based on 2.7k, 5.6k, 12k, 24K on each of 4 switches to Vcc
    // with a common 510ohm switch-buss to ground (all 1% resistors)
    // switch-buss is out to CPU with a button on Address switches
    // data switches pulls about 2mA with all switches on, so turn switches off when not in use.
    //
    byte dataout =  0;
    #ifdef PROCESSORtiny84
    //dataIn = dataIn -14;
    #endif
    if (dataIn < 7) dataout =  0;                             //0 is typical
    if (dataIn < 29 && dataIn >= 7) dataout =  1;       //19 or 20 is typical
    if (dataIn < 49 && dataIn >= 29) dataout =  2;      //39 or 40 is typical
    if (dataIn < 72 && dataIn >= 49) dataout =  3;      //59 or 60 is typical
    if (dataIn < 93 && dataIn >= 72) dataout =  4;      //84 or 85 is typical
    if (dataIn < 111 && dataIn >= 93) dataout =  5;     //102 or 103 is typical
    if (dataIn < 128 && dataIn >= 111) dataout =  6;    //120 is typical  
    if (dataIn < 150 && dataIn >= 128) dataout =  7;    //137 is typical
    if (dataIn < 171 && dataIn >= 150) dataout =  8;    //163 is typical
    if (dataIn < 186 && dataIn >= 171) dataout =  9;    //178 or 179 is typical
    if (dataIn < 200 && dataIn >= 186) dataout =  10;   //193 is typical
    if (dataIn < 216 && dataIn >= 200) dataout =  11;   //207 or 208 is typical
    if (dataIn < 231 && dataIn >= 216) dataout =  12;   //225 is typical
    if (dataIn < 244 && dataIn >= 231) dataout =  13;   //238 is typical
    if (dataIn < 257 && dataIn >= 244) dataout =  14;   //251 is typical
    if (dataIn >= 257) dataout =  15;                         //263 is typical
    return dataout; 
 }
    
//#endif

void processService (){
      if (checkFORkeyPress()){
    unsigned long timemark = millis();
    delay(10); // wait for key bounce
        //
        //Address side of our keyboard was Function Selector.
        //A1 is PC1 on a Mega
        //A1 is PA1 on a Tiny84 (14 pin CPU)
        //A1 is PB2 on a Tiny85 (8 pin CPU)
        //
        //Data side of our keyboard was Tuner
        //A3 is PC3 on a Mega
        //A3 is PA3 on a Tiny84 (14 pin CPU)
        //A3 is PB3 on a Tiny85 (8 pin CPU)
        //
    addressSwitches = analogRead(A1);
    //dataSwitches = analogRead(A3);
    while (checkFORkeyPress()) {// Wait for the key to be released
    delay(10); //*/
      }
     presstime = millis()-timemark;
    }

#ifdef PROCESSORtiny84      
      CLR(PORTA, PORTA3); // Turn the pullup OFF  for data switches
#endif 
  dataSwitches = analogRead(A3);
}

void processEvent (){
      if (presstime >0) { //We have a pending event to service
        // Check if the key press was too short to be real, then, we're out of here
        if (presstime < 15) {
          //whosInAudition = 255; // Make no one in audition
          presstime = 0; //clear the event
          return;
        }
        if (presstime >= 750) { // over 3/4 second would be a long press

          if  (convert2nibble(addressSwitches) == 15 ) { // special case for the page register -- if it is the page register we dont consider the page
             if ((convert2nibble(addressSwitches)) != whosInAudition) { // and it has been moved
                whosInAudition = 255; // Make no one in audition
                presstime = 0; //clear the event
                return;
             }
             else { // it is 15 and it has not been moved
              EEPROM.update((whosInAudition * 2 ), dataSwitches >> 8); //First write the high byte (Big Indian)
              EEPROM.update( (whosInAudition * 2 )+1 , (dataSwitches & 0xff)); //Then write the low byte
              whosInAudition = 255; // nobody is in audition
              presstime = 0; //clear the event
              return;
              
             }
             
          }
          else {      // It's not any register 15 we do consider the page
            
            // Check if he moved the address switches on us,  then, we're out of here
            //if ((convert2nibble(addressSwitches)) != whosInAudition) {                          // <--------------- Need attention for pages
            if ( (convert2nibble(addressSwitches) +    (     convert2nibble(   (((EEPROM.read(15 * 2))<<8)+(EEPROM.read((15 * 2)+1)))  ) << 4    )        ) != whosInAudition) {   // <--------------- I think that is it
              whosInAudition = 255; // Make no one in audition
              presstime = 0; //clear the event
              return;
            }

          }

          
          // Handle storing data and exiting audition mode
          EEPROM.update((whosInAudition * 2 ), dataSwitches >> 8); //First write the high byte (Big Indian)    
          EEPROM.update( (whosInAudition * 2 )+1 , (dataSwitches & 0xff)); //Then write the low byte          
          
          
          whosInAudition = 255; // nobody is in audition
          presstime = 0; //clear the event
          return;
        }
        else {  // if we got here we know that we have a good short press
          // Handle entering audition mode
           
           // Fist check of we are already in audition and back it out if we are
           if  ((convert2nibble(addressSwitches) == 15) && (whosInAudition == 15)) { // special case for the page register -- if it is the page register we dont consider the page
              whosInAudition = 255; // nobody is in audition
              presstime = 0; //clear the event
              return;
           }
           else {
              if ((convert2nibble(addressSwitches) == 15)){ // We are entering audition on 15 and we dont care about the page
                whosInAudition = 15;
                presstime = 0; //clear the event
                return;
              }

            
           
            //if (whosInAudition == convert2nibble(addressSwitches)) { // if we were already in audition on this register then back it out                             // <--------------- Need attention for pages
              if ( (convert2nibble(addressSwitches) +    (     convert2nibble(   (((EEPROM.read(15 * 2))<<8)+(EEPROM.read((15 * 2)+1)))  ) << 4    )        ) == whosInAudition) {    // <--------------- I think that is it
                whosInAudition = 255; // nobody is in audition
                presstime = 0; //clear the event
                return;
            }

        }


            // ok here we go into audition mode

             if  (convert2nibble(addressSwitches) == 15 ) { // special case for the page register -- if it is the page register we dont consider the page
               whosInAudition = convert2nibble(addressSwitches);  
             }
             else {
              
             
          
              //whosInAudition = convert2nibble(addressSwitches);                                                              // <--------------- Need attention for pages
              whosInAudition = (convert2nibble(addressSwitches) +    (     convert2nibble(   (((EEPROM.read(15 * 2))<<8)+(EEPROM.read((15 * 2)+1)))  ) << 4    )            );  // <--------------- I think that is it
             }


          
          presstime = 0; //clear the event
          //return;
        }
      }
    //return;
}

class TinyUI
{
private:
public:

//TinyUI::TinyUI(int addrPin, int dataPin)
//TinyUI::TinyUI(int addrPin)
TinyUI::TinyUI(byte addrPin)
  {
//int    _addrPin = addrPin;
//int    _dataPin = dataPin;
//byte whosInAudition = 255; //start with nobody in audition mode
  }

void TinyUI::begin()
  {
    whosInAudition = 255; // Make no one in audition
    //whosInAudition = 0; // Make Reg 0 in audition at startup for testing
    presstime = 0; //clear any event

#ifdef PROCESSORtiny84
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // This was a test to make sure that the ADC was given enough time to complete
#endif   
    //
    // And when we get around to implementing the page register, check for UI in page mode and clear that if it is.
    // 
    if ((((EEPROM.read(15 * 2))<<8)+(EEPROM.read((15 * 2)+1))) != 0) {  
      // if we have a page register set

      EEPROM.update((15 * 2 ), 0); //First write the high byte (Big Indian) //     Then clear
      EEPROM.update( (15 * 2 )+1 , 0); //Then write the low byte            //        it!
    }
  }
  
void TinyUI::service()
  {
processService (); 
  }

void TinyUI::serviceDelay(int timedelay)
  {
long int millscheck = millis();
while (millis() < millscheck + timedelay) {
  processService ();
    }
  }

void TinyUI::serviceEvent()
  {
    processEvent();
  }

byte TinyUI::readRegAsSwitches(byte reg) // Read a register as if it was the switches (a Nibble) respecting Audition 
  {
    if (reg == whosInAudition) { // if he's asking for the register in audition mode then return the real switches   // <--------------- Need attention for pages   These may actually be OK
      return convert2nibble(dataSwitches);
    }
    
    //EEPROM.read(addreess);
    return ( convert2nibble(((EEPROM.read(reg * 2)) << 8 ) + (EEPROM.read((reg * 2)+1)))); //Else - Give him the switches from the reconstructed value in the EEPROM    // <--------------- Need attention for pages   These may actually be OK
  }

int TinyUI::readRegAsPot(byte reg)       // Read a register as if it was a trimmer pot (IE a 10 bit raw value) respecting Audition Mode
  {
    if (reg == whosInAudition) { // if he's asking for the register in audition mode then return the real analog value 
      return dataSwitches;
    }
    return (((EEPROM.read(reg * 2)) << 8 ) + (EEPROM.read((reg * 2)+1))); //Else - Give him the switches (Pot value) from the reconstructed value in the EEPROM    // <--------------- Need attention for pages    These may actually be OK
  }

                              //
                              // Most of the routines below you probably do not need
                              // they are handy for testing, debuging, and perhaps if you really need to know what is going on
                              // inside of the UI for a complex application
                              //

byte TinyUI::WhoIsInAudition() //Check who is in audition mode
  {    return whosInAudition;  }
void TinyUI::readData()  {     }
bool TinyUI::checkForEvent()  {    if (presstime >0) {      return true;    }    else {      return false;    }  }
//bool TinyUI::checkIfShortPress()  {    if (presstime < 750 && presstime > 0) {// under 3/4 second would be a short press      return true;    }    else {      return false;    }  }
//bool TinyUI::checkIfLongPress()  {    if (presstime >= 750) {// over 3/4 second would be a long press      return true;    }    else {      return false;    }  }
byte TinyUI::getAddress()  {    return convert2nibble(addressSwitches);  }
byte TinyUI::getData()  {    return convert2nibble(dataSwitches);  }
int TinyUI::getRawData()  {    return dataSwitches;  }
int TinyUI::getPressTime()
  {
  return presstime;
  }


//void TinyUI::clearLastEvent()  {    presstime = 0;  }

};

/*class TinyUI
{
  public:
    TinyUI(int addrPin, int dataPin);
    void begin();
    void service();           // Check on the UI
    void serviceDelay();      // Replacement for delay servicing UI (Caution - a key press will pause your delay)
    void serviceEvent();      // Enter or exit audition mode and update EEPROM
    byte readRegAsSwitches(); // Read a register as if it was the switches (a Nibble) respecting Audition Mode
    int readRegAsPot();       // Read a register as if it was a trimmer pot (IE a 10 bit raw value) respecting Audition Mode
                              //
                              // Most of the routines below you probably do not need
                              // they are handy for testing, debuging, and perhaps if you really need to know what is going on
                              // inside of the UI for a complex application
                              //
    void readData();          // Just read the current Data switch nibble                          
    bool checkForEvent();     // See if a keypress and release has occured since last Clear
    bool checkIfShortPress(); // Was the last (uncleared) press and release a short one?
    bool checkIfLongPress();  // Was the last (uncleared) press and release a long one?
    byte getAddress();        // Get the Address nibble during the last (uncleared) press
    byte getData();           // Get the last Data (durring the last press or from the last readData()
    int  getRawData();        // Get the whole 10 bits from the ADC just in case it was actually a Pot value
    int  getPressTime();      // How many Mills was the last press and release event
    void clearLastEvent();    // When you are done processing this event, just clear it, and we'll wait for the next event

  private:
    int _pin;
    int _addrPin;
    int _dataPin;
};

#endif
*/

/*
Schematic

     Vcc >---------------------+-------------------------------------------------------+-------------------------------------------------------+
                               |                                                       |                                                       |
                               |                                                       |                                                       |
                 +--------+----+----+--------+                           +--------+----+----+--------+                                         |
                 |        |         |        |                           |        |         |        |                                         |
                 |        |         |        |                           |        |         |        |                                         |
                .-.      .-.       .-.      .-.                         .-.      .-.       .-.      .-.                                      .---.
                | |      | |       | |      | |                         | |      | |       | |      | |                                      |   | 10K-50K
                | |      | |       | |      | |                         | |      | |       | |      | |      All resistors                   |   | Optionsl
               2.7kΩ    5.6kΩ     12kΩ     24kΩ                        2.7kΩ    5.6kΩ     12kΩ     24kΩ      should be 1%              +---->|   | POT to
                | |      | |       | |      | |                         | |      | |       | |      | |      otherwise, YMMV           |     |   | fine tune
                | |      | |       | |      | |                         | |      | |       | |      | |                                |     |   | values
                '-'      '-'       '-'      '-'                         '-'      '-'       '-'      '-'                                |     '---'
                 |        |         |        |                           |        |         |        |                                 |       |
                 |        |         |        |                           |        |         |        |                                 |       |
                 |  /     |  /      |  /     |  /     Address            |  /     |  /      |  /     |  /     Data                     |       |
                   /        /         /        /      Switches             /        /         /        /      Switches                 |       |
                  / S4     / S3      / S2     / S1                        / S8     / S7      / S6     / S5                             |       |
                 / MSB    /         /        / LSB                       /        /         /        /  LSB   Optional                 |       |
                 |        |         |        |                           |        |         |        |        POT Selector             |       |
                 |        |         |        |                           |        |         |        |      .---.                      |       |
                 +--------+----+----+--------+-------> To Analog         +--------+----+----+--------+------o   o   o------------------+       |
                               |                       input pin                       |                        |                              |    
                               |  /                    (Address)                       |                        |                              |
                                 /                                                     |                        v                              |
                                / Select                                               |                       To Analog                       |
                               /  Button                                               |                       input pin                       |
                               |                                                       |                       (Data)                          |
                              .-.                                                     .-.                                                      |
                              | |                                                     | |                                                      |
                              | |                                                     | |                                                      |
                             510 Ω                                                   510 Ω                                                     |
                              | |                                                     | |                                                      |
                              | |                                                     | |                                                      |
                              '-'                                                     '-'                                                      |
                               |                                                       |                                                       |
     Vdd (Ground) >------------+-------------------------------------------------------+-------------------------------------------------------+

*/
