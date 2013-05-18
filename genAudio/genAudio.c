#include <stdlib.h>
#include <stdio.h>

typedef struct {
	int sampleRate;
	int numChannels;
	int bitsPerSample;

	int dataLength;
} wavSound;

wavSound * loadWaveHeader(FILE * fp) {
	char c[5];
	int nbRead;
	int chunkSize;
	int subChunk1Size;
	int subChunk2Size;
	short int audFormat;
	short int nbChannels;
	int sampleRate;
	int byteRate;
	short int blockAlign;
	short int bitsPerSample;
	wavSound *w;

	c[4] = 0;

	nbRead=fread(c, sizeof(char), 4, fp);

	// EOF ?
	if (nbRead < 4) return NULL;

	// Not a RIFF ?
	if (strcmp(c, "RIFF") != 0) {
		printf("Not a RIFF: %s\n", c);
		return NULL;
	}

	nbRead=fread(&chunkSize, sizeof(int), 1, fp);

	// EOF ?
	if (nbRead < 1) return NULL;

	nbRead=fread(c, sizeof(char), 4, fp);

	// EOF ?
	if (nbRead < 4) return NULL;

	// Not a WAVE riff ?
	if (strcmp(c, "WAVE") != 0) {
		printf("Not a WAVE: %s\n", c);
		return NULL;
	}

	nbRead=fread(c, sizeof(char), 4, fp);

	// EOF ?
	if (nbRead < 4) return NULL;

	// Not a "fmt " subchunk ?
	if (strcmp(c, "fmt ") != 0) {
		printf("No fmt subchunk: %s\n", c);
		return NULL;
	}

	// read size of chunk.
	nbRead=fread(&subChunk1Size, sizeof(int), 1, fp);
	if (nbRead < 1) return NULL;

	// is it a PCM ?
	if (subChunk1Size != 16) {
		printf("Not PCM fmt chunk size: %x\n", subChunk1Size);
		return NULL;
	}

	nbRead=fread(&audFormat, sizeof(short int), 1, fp);
	if (nbRead < 1) return NULL;

	// is it PCM ?
	if (audFormat != 1) {
		printf("No PCM format (1): %x\n", audFormat);
		return NULL;
	}

	nbRead=fread(&nbChannels, sizeof(short int), 1, fp);
	if (nbRead < 1) return NULL;

	// is it mono or stereo ?
	if (nbChannels > 2 || nbChannels < 1) {
		printf("Number of channels invalid: %x\n", nbChannels);
		return NULL;
	}

	nbRead=fread(&sampleRate, sizeof(int), 1, fp);
	if (nbRead < 1) return NULL;

	nbRead=fread(&byteRate, sizeof(int), 1, fp);
	if (nbRead < 1) return NULL;

	nbRead=fread(&blockAlign, sizeof(short int), 1, fp);
	if (nbRead < 1) return NULL;

	nbRead=fread(&bitsPerSample, sizeof(short int), 1, fp);
	if (nbRead < 1) return NULL;

	nbRead=fread(c, sizeof(char), 4, fp);

	// EOF ?
	if (nbRead < 4) return NULL;

	// Not a data section ?
	if (strcmp(c, "data") != 0) {
		printf("Not a data subchunk: %s\n", c);
		return NULL;
	}

	nbRead=fread(&subChunk2Size, sizeof(int), 1, fp);
	if (nbRead < 1) return NULL;

	// Now we can generate the structure...

	w = (wavSound *) malloc(sizeof(wavSound));
	// out of memory ?
	if (w == NULL) {
		printf("Out of memory, sorry\n");
		return w;
	}

	w->sampleRate = sampleRate;
	w->numChannels = nbChannels;
	w->bitsPerSample = bitsPerSample;
	w->dataLength = subChunk2Size;

	return w;
}

void error(char * str) {
	printf("Error: %s!!!\n", str);
	exit(1);
}

int main(int argc, char **argv) {
	if(!argv[1]) error("Must specify an input file");
	if(!argv[2]) error("Must specify an output file");

	FILE *fin = fopen(argv[1], "r");
	FILE *fout = fopen(argv[2], "w");
	
	wavSound*s = loadWaveHeader(fin);

	if(s == NULL) error("Invalid wav file");

	if(s->sampleRate != 32000) error("Must be at 32000hz");

	if(s->numChannels != 1) error("Must be monoaural");
	
	if(s->bitsPerSample != 16) error("Must be 16bits");
	
	int length = (s->dataLength / s->numChannels / s->bitsPerSample * 8),
		i;
	
	short int signedData;
	unsigned short int data;

	for(i=0; i<length; i++) {
		fread(&signedData, sizeof(short int), 1, fin);
		data = signedData + 32767;
		fwrite(&data, 1, sizeof(unsigned short int), fout);
	}

	fclose(fout);
	fclose(fin);
	return 0;
}
