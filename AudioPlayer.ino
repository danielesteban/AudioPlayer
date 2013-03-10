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
#include <Directory.h>
*/
#include "Setup.h"

bool playing = 0;

file * currentFile;
long fileSize;
File audio;

#define NumBuffers 4
#define BufferSize 1024
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
		audio.read(buffers[x], BufferSize);
		fileSize -= buffSize;
		bufferStatus[x] = 1;
		break;
	}

	buttons.read();
}

void loadFile(file * f) {
	String filename = SDPath;
	filename += f->name;
	char p[filename.length() + 1];
    filename.toCharArray(p, filename.length() + 1);
	audio = SD.open(p);
	currentFile = f;
	fileSize = f->size;
	for(byte x=0; x<NumBuffers; x++) {
		bufferStatus[x] = 0;
		for(int y=0; y<BufferSize; y++) buffers[x][y] = 127;
	}
	currentBuffer = currentBufferIndex = 0;
}

void prev() {
	file * f = dir->getFiles();
	while(1) {
		if(f->next == currentFile || f->next == NULL) {
			loadFile(f);
			break;
		}
	}
}

void next() {
	loadFile(currentFile->next != NULL ? currentFile->next : dir->getFiles());
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
