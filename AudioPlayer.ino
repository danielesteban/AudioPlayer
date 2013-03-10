#define buttonPlay 40
#define buttonNext 41
#define DacRegister DDRA
#define DacPort PORTA
#define SDPath "/AUDIO/"

#include <SD.h>
#include <Buttons.h>
#include <Directory.h>

#define sampleRate 44100
#define audioInterrupt TIMER2_COMPA_vect

Buttons buttons(onPush);

bool playing = 0;

Directory * dir;
file * currentFile;
long fileSize;
File audio;

#define NumBuffers 4
#define BufferSize 1024
byte currentBuffer;
int currentBufferIndex;
uint8_t buffers[NumBuffers][BufferSize];
bool bufferStatus[NumBuffers];

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

void setup() {
	buttons.setup(buttonPlay);
	buttons.setup(buttonNext);

	DacRegister = 0xFF;
	//Serial.begin(115200);

	SD.begin();
	dir = new Directory(SDPath);
	loadFile(dir->getFiles());

	cli(); //stop interrupts

	TCCR2A = 0;
	TCCR2B = 0;
	TCCR2A |= (1 << WGM21); //turn on CTC mode
	TCCR2B |= (1 << CS21); //Set CS11 bit for 8 prescaler
	OCR2A = (F_CPU / ((long) sampleRate * 8)) - 1; //set compare match register for 16khz increments
	TIMSK2 |= (1 << OCIE2A); //enable timer compare interrupt

	sei(); //allow interrupts
}

void next() {
	loadFile(currentFile->next != NULL ? currentFile->next : dir->getFiles());
}

void loop() {
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

void onPush(byte pin) {
	switch(pin) {
		case buttonPlay:
			playing = !playing;
		break;
		case buttonNext:
			next();
	}
}

ISR(audioInterrupt) {
	if(!playing || !bufferStatus[currentBuffer]) return;
	DacPort = buffers[currentBuffer][currentBufferIndex];
	currentBufferIndex++;
	if(currentBufferIndex == BufferSize) {
		bufferStatus[currentBuffer] = currentBufferIndex = 0;
		currentBuffer++;
		currentBuffer == NumBuffers && (currentBuffer = 0);
	}
}
