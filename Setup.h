/*
    Setup.h - Pin Declarations, library variables & setup function for AudioPlayer.
    Created by Daniel Esteban, March 10, 2013.
*/

#ifndef Setup_h
#define Setup_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <EEPROM.h>
#include <SD.h>
#include <Buttons.h>

//Pin Declarations & Audio path
#if defined(__AVR_ATmega2560__)
#define buttonPlay 40
#define buttonPrev 41
#define buttonNext 42
#define DacRegister DDRA
#define DacPort PORTA
#else if defined(__AVR_ATmega328__)
#define buttonPlay A1
#define buttonPrev A2
#define buttonNext A0
#define DacRegister DDRD
#define DacPort PORTD
#endif
#define SDPath "/AUDIO/"

#define sampleRate 44100

//STOP EDITING HERE...

//Library variables
void onPush(byte pin);
Buttons buttons(onPush);
File dir;
#define audioInterrupt TIMER2_COMPA_vect

void loadFile(File f, int index);

//Setup function
void setup() {
	//BUTTONS
	buttons.setup(buttonPlay);
	buttons.setup(buttonPrev);
	buttons.setup(buttonNext);

	//SDCARD
	SD.begin();
	dir = SD.open(SDPath);

	int lastIndex = (int) EEPROM.read(0) + ((int) EEPROM.read(1) << 8);
    lastIndex == 32767 && (lastIndex = 0);

	int c = 0;
	File f;
	while(f = dir.openNextFile()) {
		if(c == lastIndex) break;
		f.close();
		c++;
	}
	if(!f) { //sd content has change or the eeprom data was bad...
		dir.rewindDirectory();
		f = dir.openNextFile();
		c = 0;
	}
	loadFile(f, c);

	//DAC
	DacRegister = 0xFF;

	//AUDIO TIMER
	cli(); //stop interrupts
	TCCR2A = 0;
	TCCR2B = 0;
	TCCR2A |= (1 << WGM21); //turn on CTC mode
	TCCR2B |= (1 << CS21); //Set CS11 bit for 8 prescaler
	OCR2A = (F_CPU / ((long) sampleRate * 8)) - 1; //set compare match register for 16khz increments
	TIMSK2 |= (1 << OCIE2A); //enable timer compare interrupt
	sei(); //allow interrupts
}

#endif

