/**************************************************************************

    midiout.cc   - class midiOut which handles the /dev/sequencer device
    Copyright (C) 1997  Antonio Larrosa Jimenez

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Send comments and bug fixes to antlarr@arrakis.es
    or to Antonio Larrosa, Rio Arnoya, 10 5B, 29006 Malaga, Spain

***************************************************************************/
#include "midiout.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "sndcard.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/time.h>

//SEQ_DEFINEBUF (1024); 
//#define MIDIOUTDEBUG

midiOut::midiOut(void)
{
seqfd = -1;
device= -1;
count=0.0;
lastcount=0.0;
lasttime=0;
begintime=0;
Map=new MidiMapper("yamaha790.map");
rate=100;
};

midiOut::~midiOut()
{
delete Map;
closeDev();
};

void midiOut::openDev (void)
{
seqfd = open("/dev/midi", O_WRONLY, 0);
if (seqfd==-1)
    {
    printf("ERROR: Could not open /dev/midi\n");
    return;
    };
//ioctl(seqfd,SNDCTL_SEQ_NRSYNTHS,&ndevs);
//printf("Number of synth devices : %d\n",ndevs);
//ioctl(seqfd,SNDCTL_SEQ_NRMIDIS,&nmidiports);
//printf("Number of midi ports : %d\n",nmidiports);
//*ioctl(seqfd,SNDCTL_SEQ_CTRLRATE,&rate);
//printf("Rate : %d\n",rate);
//convertrate=1000/rate;
//device=0;
//count=0.0;
//lastcount=0.0;
};

void midiOut::closeDev (void)
{
if (!OK()) return;
//SEQ_STOP_TIMER();
//SEQ_DUMPBUF();
if (seqfd>=0)
    close(seqfd);
seqfd=-1;
device=-1;
};

void midiOut::initDev (void)
{
int chn;
if (!OK()) return;
count=0.0;
lastcount=0.0;
uchar gm_reset[5]={0x7e, 0x7f, 0x09, 0x01, 0xf7};
sysex(gm_reset, sizeof(gm_reset));
for (chn=0;chn<16;chn++)
    {
    chn_mute[chn]=0;
    chnPatchChange(chn,0);
    chnPressure(chn,127);
    chnPitchBender(chn, 0x00, 0x40);
    chnController(chn, CTL_MAIN_VOLUME,127);
    chnController(chn, CTL_EXT_EFF_DEPTH, 0);
    chnController(chn, CTL_CHORUS_DEPTH, 0);
    chnController(chn, 0x4a, 127);

    };
};

void midiOut::useMapper(MidiMapper *map)
{
delete Map;
Map=map;
};

void midiOut::noteOn  (uchar chn, uchar note, uchar vel)
{
if (vel==0)
    {
    noteOff(chn,note,vel);
    }
   else
    {
    char temp[3];
    temp[0]=MIDI_NOTEON + Map->Channel(chn);
    temp[1]=Map->Key(chn,chn_patch[chn],note);
    temp[2]=vel;
    write(seqfd,temp,3);
//    SEQ_MIDIOUT(device, MIDI_NOTEON + Map->Channel(chn));
//    SEQ_MIDIOUT(device, Map->Key(chn,chn_patch[chn],note));
//    SEQ_MIDIOUT(device, vel);
    };
#ifdef MIDIOUTDEBUG
printf("Note ON >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
};

void midiOut::noteOff (uchar chn, uchar note, uchar vel)
{
char temp[3];
temp[0]=MIDI_NOTEOFF + Map->Channel(chn);
temp[1]=Map->Key(chn,chn_patch[chn],note);
temp[2]=vel;
write(seqfd,temp,3);
//SEQ_MIDIOUT(device, MIDI_NOTEOFF + Map->Channel(chn));
//SEQ_MIDIOUT(device, Map->Key(chn,chn_patch[chn],note));
//SEQ_MIDIOUT(device, vel);
#ifdef MIDIOUTDEBUG
printf("Note OFF >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
};

void midiOut::keyPressure (uchar chn, uchar note, uchar vel)
{
//SEQ_MIDIOUT(device, MIDI_KEY_PRESSURE + Map->Channel(chn));
//SEQ_MIDIOUT(device, Map->Key(chn,chn_patch[chn],note));
//SEQ_MIDIOUT(device, vel);
char temp[3];
temp[0]=MIDI_KEY_PRESSURE + Map->Channel(chn);
temp[1]=Map->Key(chn,chn_patch[chn],note);
temp[2]=vel;
write(seqfd,temp,3);

};

void midiOut::chnPatchChange (uchar chn, uchar patch)
{
//printf("PATCHCHANGE [%d] %d -> %d",chn,patch,Map->Patch(patch));
//SEQ_MIDIOUT(device, MIDI_PGM_CHANGE + Map->Channel(chn));
//SEQ_MIDIOUT(device, Map->Patch(patch));
char temp[2];
temp[0]=MIDI_PGM_CHANGE + Map->Channel(chn);
temp[1]=Map->Patch(patch);
write(seqfd,temp,2);
chn_patch[chn]=patch;
};

void midiOut::chnPressure (uchar chn, uchar vel)
{
//SEQ_MIDIOUT(device, MIDI_CHN_PRESSURE + Map->Channel(chn));
//SEQ_MIDIOUT(device, vel);

chn_pressure[chn]=vel;
};

void midiOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
//SEQ_MIDIOUT(device, MIDI_PITCH_BEND + Map->Channel(chn));
//SEQ_MIDIOUT(device, lsb);
//SEQ_MIDIOUT(device, msb);
//chn_bender[chn]=(msb << 8) | (lsb & 0xFF);
};

void midiOut::chnController (uchar chn, uchar ctl, uchar v) 
{
char temp[3];
temp[0]=MIDI_CTL_CHANGE + Map->Channel(chn);
temp[1]=ctl;
temp[2]=v;
write(seqfd,temp,3);

//SEQ_MIDIOUT(device, MIDI_CTL_CHANGE + Map->Channel(chn));
//SEQ_MIDIOUT(device, ctl);
//SEQ_MIDIOUT(device, v);

chn_controller[chn][ctl]=v;
};

void midiOut::sysex(uchar *data, ulong size)
{
/*
ulong i=0;
SEQ_MIDIOUT(device, MIDI_SYSTEM_PREFIX);
while (i<size)
    {
    SEQ_MIDIOUT(device, *data);
    data++;
    i++;
    };
printf("sysex\n");
*/
};

void midiOut::channelSilence (uchar chn)
{
uchar i;
for ( i=0; i<127; i++)
    {
    noteOff(chn,i,0);
    };
//SEQ_DUMPBUF();
};

void midiOut::channelMute(uchar chn, int a)
{
/*
if (a==1)
    {
    chn_mute[chn]=a;
    channelSilence(chn);
    }
  else if (a==0)
    {
    chn_mute[chn]=a;
    };
//  else ignore the call to this procedure 
*/
};


void midiOut::seqbuf_dump (void)
{
/*    if (_seqbufptr)
        if (write (seqfd, _seqbuf, _seqbufptr) == -1)
        {
            perror ("write /dev/sequencer");
            exit (-1);
        }
    _seqbufptr = 0;*/
};

void midiOut::seqbuf_clean(void)
{
//_seqbufptr=0;
};

void midiOut::wait(double ticks)
{
timeval tv;
gettimeofday(&tv, NULL);
double currenttime=tv.tv_sec*1000+tv.tv_usec/1000;
//SEQ_START_TIMER();

//count+=ticks;
//printf("tick : %ld\n",cuenta);
//SEQ_DELTA_TIME((uint)(count-lastcount));
//SEQ_WAIT_TIME((int)count);
lasttime=currenttime-begintime;
if (ticks>lasttime) usleep((ticks-lasttime)*1000);
//SEQ_WAIT_TIME(((int)(ticks/convertrate)));
//lastcount=count;
//SEQ_DUMPBUF();
#ifdef MIDIOUTDEBUG
printf("Wait  >\t ticks: %g\n",ticks);
#endif
};

void midiOut::tmrSetTempo(int v)
{
#ifdef MIDIOUTDEBUG
printf("SETTEMPO  >\t tempo: %d\n",v);
#endif

//SEQ_SET_TEMPO(v);
//SEQ_DUMPBUF();
};

void midiOut::sync(int i)
{
#ifdef MIDIOUTDEBUG
printf("Sync %d\n",i);
#endif
//if (i==1) seqbuf_clean();
//ioctl(seqfd, SNDCTL_SEQ_SYNC);
};

void midiOut::tmrStart(void)
{
lasttime=0;
timeval tv;
gettimeofday(&tv, NULL);
begintime=tv.tv_sec*1000+tv.tv_usec/1000;
//SEQ_START_TIMER();
//SEQ_DUMPBUF();
};

void midiOut::tmrStop(void)
{
//SEQ_STOP_TIMER();
//SEQ_DUMPBUF();
};

void midiOut::tmrContinue(void)
{
//SEQ_CONTINUE_TIMER();
//SEQ_DUMPBUF();
};

