#ifndef _MIDIMAPPER_H
#define _MIDIMAPPER_H

#include <stdio.h>
#include "dattypes.h"

struct Keymap
{
	char name[30];
	uchar key[128];
	struct Keymap *next;
};

class MidiMapper
{
private:
	uchar	channel[16]; 
	Keymap *channelKeymap[16]; // pointer to the keymap to use for a channel
				// this is to make it faster
				// The index is with the real channel (already mapped)

	uchar	patchmap[128];
	Keymap *patchKeymap[128]; // Same as channelKeymap
   
	Keymap *keymaps; // Real linked list of keymaps used around the program

        void getValue(char *s,char *v);
        void removeSpaces(char *s);
        int  countWords(char *s);
        void getWord(char *t,char *s,int w); // get from s the word in position
                                             //  w and put it in t


        void DeallocateMaps(void);
        Keymap *createKeymap(char *name,uchar use_same_note=0,uchar note=0);
	void readPatchmap(FILE *fh);
	void readKeymap(FILE *fh,char *first_line);
	void readChannelmap(FILE *fh);

        void AddKeymap(Keymap *newkm);
	Keymap *GiveMeKeymap(char *n);

public:
	MidiMapper(const char *name);
	~MidiMapper();


	void LoadFile(const char *name);	

	uchar Channel(uchar chn) { return channel[chn];};
	uchar Patch(uchar pgm) { return patchmap[pgm];};
	uchar Key(uchar chn,uchar pgm, uchar note);

};
#endif