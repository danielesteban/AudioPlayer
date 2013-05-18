/*
    AudioPlayer - An arduino portable audio player
    ==============================================
    Created by Daniel Esteban, March 10, 2013.

    Edit Config.h for Port/Pin Numbers & other configurations...
*/

#include <EEPROM.h>
#include <SD.h>
#include "Config.h"

#define audioInterrupt TIMER1_COMPA_vect

byte currentBuffer;
int currentBufferIndex;
uint8_t buffers[NumBuffers][BufferSize];
bool bufferStatus[NumBuffers];

File root;
File currentFile;
File currentDir;
int currentDirIndex;
long fileSize = 0;

void setup() {
    //SDCARD
    SD.begin();
    root = SD.open(SDPath);

    currentDir = root.openNextFile();
    
    currentDirIndex = (int) EEPROM.read(0) + ((int) EEPROM.read(1) << 8);
    currentDirIndex == 32767 && (currentDirIndex = 0);

    randomSeed(currentDirIndex + micros());

    int skip = currentDirIndex + random(1, 6);

    while(skip > 0) {
        skip--;
        currentDirIndex++;
        do {
            currentDir.close();
            currentDir = root.openNextFile();
            if(!currentDir) {
                root.rewindDirectory();
                currentDir = root.openNextFile();
                currentDirIndex = 0;
            }
        } while(!currentDir.isDirectory());
    }
    EEPROM.write(0, lowByte(currentDirIndex));
    EEPROM.write(1, highByte(currentDirIndex));

    //DAC
    DacRegisterL = 0xFF;
    for(byte x=0; x<DacBitH2; x++) DacRegisterH1 |= (1 << x);
    DacRegisterH2 = 0x3F;
    
    //AUDIO TIMER
    cli(); //stop interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    TCCR1B |= (1 << WGM12); //turn on CTC mode
    TCCR1B |= (1 << CS10); //Set CS10 bit for no prescaler
    OCR1A = (F_CPU / sampleRate) - 1; //set compare match register for 32khz increments
    TIMSK1 |= (1 << OCIE1A); //enable timer compare interrupt
    sei(); //allow interrupts
}

inline void next() {
    if(currentFile) currentFile.close();
    currentFile = currentDir.openNextFile();
    if(!currentFile) {
        currentDirIndex++;
        do {
            currentDir.close();
            currentDir = root.openNextFile();
            if(!currentDir) {
                root.rewindDirectory();
                currentDir = root.openNextFile();
                currentDirIndex = 0;
            }
        } while(!currentDir.isDirectory());

        currentFile = currentDir.openNextFile();
        EEPROM.write(0, lowByte(currentDirIndex));
        EEPROM.write(1, highByte(currentDirIndex));
    }
    fileSize = currentFile.size();
    for(byte x=0; x<NumBuffers; x++) {
        bufferStatus[x] = 0;
        for(int y=0; y<BufferSize; y+=2) {
            buffers[x][y] = 255;
            buffers[x][y + 1] = 127;
        }
    }
    currentBuffer = currentBufferIndex = 0;
}

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
}

ISR(audioInterrupt) {
    //PLAYBACK
    if(!bufferStatus[currentBuffer]) return;
    DacPortL = buffers[currentBuffer][currentBufferIndex];
    const byte hb = buffers[currentBuffer][currentBufferIndex + 1];
    for(byte x=0; x<DacBitH2; x++) {
        if(hb & (1 << x)) DacPortH1 |= (1 << x);
        else DacPortH1 &= ~(1 << x);
    }
    DacPortH2 = hb >> DacBitH2;
    currentBufferIndex += 2;
    if(currentBufferIndex == BufferSize) {
        bufferStatus[currentBuffer] = currentBufferIndex = 0;
        currentBuffer++;
        currentBuffer == NumBuffers && (currentBuffer = 0);
    }
}
