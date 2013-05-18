/*
    Config.h - Pin Declarations & Defines for AudioPlayer.
    Created by Daniel Esteban, March 10, 2013.
*/

#ifndef Config_h
#define Config_h

#define SDPath "/AUDIO/"
#define sampleRate 32000

#define DacRegisterL DDRD
#define DacPortL PORTD
#define DacRegisterH1 DDRB
#define DacPortH1 PORTB
#define DacBitH2 2
#define DacRegisterH2 DDRC
#define DacPortH2 PORTC

#define NumBuffers 2
#define BufferSize 260

#endif
