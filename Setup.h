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
#include <SD.h>
#include <Buttons.h>
#include <Directory.h>

//Pin Declarations & Audio path
#define buttonPlay 40
#define buttonPrev 41
#define buttonNext 42
#define DacRegister DDRA
#define DacPort PORTA
#define SDPath "/AUDIO/"

#define sampleRate 44100

//STOP EDITING HERE...

//Library variables
void onPush(byte pin);
Buttons buttons(onPush);
Directory * dir;
#define audioInterrupt TIMER2_COMPA_vect

void loadFile(file * f);

//Setup function
void setup() {
	//BUTTONS
	buttons.setup(buttonPlay);
	buttons.setup(buttonPrev);
	buttons.setup(buttonNext);

	//SDCARD
	SD.begin();
	dir = new Directory(SDPath);
	loadFile(dir->getFiles());

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

	//DEBUG
	//Serial.begin(115200);
}

#endif

