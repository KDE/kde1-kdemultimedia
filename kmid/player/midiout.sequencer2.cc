/**************************************************************************

    midiout.cc   - class midiOut which handles the /dev/sequencer2 device
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
SEQ_PM_DEFINES;
#include <sys/ioctl.h>

SEQ_DEFINEBUF (2048);
#define DEBUG

midiOut::midiOut(void)
{
seqfd = -1;
device= -1;
count=0.0;
lastcount=0.0;
Map=new MidiMapper(NULL);
};

midiOut::~midiOut()
{
delete Map;
closeDev();
};

void midiOut::openDev (void)
{
seqfd = open("/dev/sequencer2", O_WRONLY, 0);
if (seqfd==-1)
    {
    printf("ERROR: Could not open the device /dev/sequencer2\n");
    exit(1);
    };
ioctl(seqfd,SNDCTL_SEQ_NRSYNTHS,&ndevs);
printf("Number of synth devices : %d\n",ndevs);
ioctl(seqfd,SNDCTL_SEQ_NRMIDIS,&nmidiports);
printf("Number of midi ports : %d\n",nmidiports);
int rate=0;
ioctl(seqfd,SNDCTL_SEQ_CTRLRATE,&rate);
printf("Rate : %d\n",rate);
device=0;
int fmdev=0;
/*PM_LOAD_PATCH(seqfd,1,1);
PM_LOAD_PATCH(seqfd,2,1);
PM_LOAD_PATCH(seqfd,3,1);
PM_LOAD_PATCH(seqfd,4,1);
PM_LOAD_PATCH(seqfd,5,1);
PM_LOAD_PATCH(seqfd,6,1);
PM_LOAD_PATCH(seqfd,7,1);
PM_LOAD_PATCH(seqfd,8,1);
PM_LOAD_PATCH(seqfd,0,1);
PM_LOAD_PATCH(seqfd,0,2);
PM_LOAD_PATCH(device,1,0);
PM_LOAD_PATCH(device,2,0);
PM_LOAD_PATCH(device,3,0);
PM_LOAD_PATCH(device,4,0);
PM_LOAD_PATCH(device,5,0);
PM_LOAD_PATCH(device,6,0);
PM_LOAD_PATCH(device,7,0);*/
//SEQ_START_TIMER();
//SEQ_DUMPBUF();
count=0.0;
lastcount=0.0;
};

void midiOut::closeDev (void)
{
if (!OK()) return;
SEQ_STOP_TIMER();
SEQ_DUMPBUF();
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
    SEQ_START_NOTE(device, Map->Channel(chn),Map->Key(chn,chn_patch[chn],note),vel);
    };
#ifdef DEBUG
//printf("Note ON >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
};

void midiOut::noteOff (uchar chn, uchar note, uchar vel)
{
SEQ_STOP_NOTE(device,Map->Channel(chn),Map->Key(chn,chn_patch[chn],note),vel);
};

void midiOut::keyPressure (uchar chn, uchar note, uchar vel)
{
SEQ_KEY_PRESSURE(device,Map->Channel(chn), Map->Key(chn,chn_patch[chn],note), vel);
};

void midiOut::chnPatchChange (uchar chn, uchar patch)
{
PM_LOAD_PATCH(device,0,Map->Patch(patch));
SEQ_SET_PATCH(device,Map->Channel(chn), Map->Patch(patch));
chn_patch[chn]=patch;
};

void midiOut::chnPressure (uchar chn, uchar vel)
{
SEQ_CHN_PRESSURE(device,Map->Channel(chn),vel);

chn_pressure[chn]=vel;
};

void midiOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
chn_bender[chn]=(msb << 8) | (lsb & 0xFF);
SEQ_BENDER(device,Map->Channel(chn),chn_bender[chn]);
};

void midiOut::chnController (uchar chn, uchar ctl, uchar v) 
{
SEQ_CONTROL(device,Map->Channel(chn),ctl,v);

chn_controller[chn][ctl]=v;
};

void midiOut::sysex(uchar *data, ulong size)
{
/*ulong i=0;
SEQ_MIDIOUT(device, MIDI_SYSTEM_PREFIX);
while (i<size)
    {
    SEQ_MIDIOUT(device, *data);
    data++;
    i++;
    };
printf("sysex\n");*/
};

void midiOut::channelSilence (uchar chn)
{
uchar i;
for ( i=0; i<127; i++)
    {
    noteOff(chn,i,0);
    };
SEQ_DUMPBUF();
};

void midiOut::channelMute(uchar chn, int a)
{
if (a==1)
    {
    chn_mute[chn]=a;
    channelSilence(chn);
    }
  else if (a==0)
    {
    chn_mute[chn]=a;
    };
/*  else ignore the call to this procedure */
};

void midiOut::seqbuf_dump (void)
{
    if (_seqbufptr)
        if (write (seqfd, _seqbuf, _seqbufptr) == -1)
        {
            perror ("write /dev/sequencer");
            exit (-1);
        }
    _seqbufptr = 0;
};


void midiOut::wait(double ticks)
{
count+=ticks;
//SEQ_DELTA_TIME((uint)(count-lastcount));
//SEQ_WAIT_TIME((int)count);
SEQ_WAIT_TIME((int)ticks);
lastcount=count;
SEQ_DUMPBUF();
};

void midiOut::tmrSetTempo(int v)
{
/*SEQ_SET_TEMPO(v);
SEQ_DUMPBUF();*/
};

void midiOut::sync(void)
{
ioctl(seqfd, SNDCTL_SEQ_SYNC);
};

void midiOut::tmrStart(void)
{
SEQ_START_TIMER();
SEQ_DUMPBUF();
};

void midiOut::tmrStop(void)
{
SEQ_STOP_TIMER();
SEQ_DUMPBUF();
};

void midiOut::tmrContinue(void)
{
SEQ_CONTINUE_TIMER();
SEQ_DUMPBUF();
};

