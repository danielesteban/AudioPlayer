/*
	AudioPlayer - An arduino portable audio player
	==============================================
	Created by Daniel Esteban, March 10, 2013.

	Edit Setup.h for Port/Pin Numbers & other configurations...
*/

/*
//Uncomment this ones to compile with the Arduino IDE
#include <SD.h>
#include <Buttons.h>
*/
#include "Setup.h"

bool playing = 0;

File currentFile;
int currentIndex = 0;
long fileSize;

#if defined(__AVR_ATmega2560__)
#define NumBuffers 4
#define BufferSize 1024
#else if defined(__AVR_ATmega328__)
#define NumBuffers 2
#define BufferSize 256
#endif
byte currentBuffer;
int currentBufferIndex;
uint8_t buffers[NumBuffers][BufferSize];
bool bufferStatus[NumBuffers];

void next();

void loop() {
	//BUFFERING
	for(byte x=0; x<NumBuffers; x++) {
		if(fileSize == 0) next();
		if(bufferStatus[x]) continue;
		unsigned int buffSize = fileSize > BufferSize ? BufferSize : fileSize;
		currentFile.read(buffers[x], BufferSize);
		fileSize -= buffSize;
		bufferStatus[x] = 1;
		break;
	}

	buttons.read();
}

void loadFile(File f) {
	currentFile = f;
	fileSize = f.size();
	for(byte x=0; x<NumBuffers; x++) {
		bufferStatus[x] = 0;
		for(int y=0; y<BufferSize; y++) buffers[x][y] = 127;
	}
	currentBuffer = currentBufferIndex = 0;
}

void prev() {
	int c = 0;
	File f,
		prev = currentFile;

	dir.rewindDirectory();
	while(f = dir.openNextFile()) {
		prev.close();
		prev = f;
		c++;
		if(c == currentIndex) break;
	}
	currentIndex = c - 1;
	loadFile(prev);
}

void next() {
	if(currentFile) currentFile.close();
	File next = dir.openNextFile();
	currentIndex++;
	if(!next) {
		dir.rewindDirectory();
		next = dir.openNextFile();
		currentIndex = 0;
	}
	loadFile(next);
}

void onPush(byte pin) {
	switch(pin) {
		case buttonPlay:
			playing = !playing;
		break;
		case buttonPrev:
			prev();
		break;
		case buttonNext:
			next();
	}
}

ISR(audioInterrupt) {
	//PLAYBACK
	if(!playing || !bufferStatus[currentBuffer]) return;
	DacPort = buffers[currentBuffer][currentBufferIndex];
	currentBufferIndex++;
	if(currentBufferIndex == BufferSize) {
		bufferStatus[currentBuffer] = currentBufferIndex = 0;
		currentBuffer++;
		currentBuffer == NumBuffers && (currentBuffer = 0);
	}
}
