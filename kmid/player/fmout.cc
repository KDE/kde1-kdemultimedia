/**************************************************************************

    fmout.cc   - class fmOut which handles the /dev/sequencer device
			for fm synths
    Copyright (C) 1998  Antonio Larrosa Jimenez

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
#include "fmout.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "sndcard.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include "../version.h"


//SEQ_DEFINEBUF (1024); 
SEQ_USE_EXTBUF();

//#define O3MELODIC    "/etc/std.o3"
//#define O3DRUMS      "/etc/drums.o3"
//#define SBMELODIC    "/etc/std.sb"
//#define SBDRUMS      "/etc/drums.sb"

#define PERCUSSION_CHANNEL 9

fmOut::fmOut(int d,int total)
{
seqfd = -1;
device= d;
count=0.0;
lastcount=0.0;
rate=100;
ok=1;
// Put opl=3 for opl/3 (better quality/ 6 voices)
//  or opl=2 for fm output (less quality/ 18 voices, which is better imho) : 
opl=2;
// But be aware that opl3 is not intended to be fully supported by now

nvoices=total;
vm=new voiceManager(nvoices);
};

fmOut::~fmOut()
{
delete Map;
closeDev();
if (delete_FM_patches_directory) 
    {
    delete FM_patches_directory;
    delete_FM_patches_directory = 0;
    FM_patches_directory="/etc";
    };
};

void fmOut::openDev (int sqfd)
{
ok=1;
seqfd = sqfd;
//vm->cleanLists();
if (seqfd==-1)
    {
    printf("ERROR: Could not open /dev/sequencer\n");
    return;
    };
ioctl(seqfd,SNDCTL_SEQ_NRSYNTHS,&ndevs);
ioctl(seqfd,SNDCTL_SEQ_NRMIDIS,&nmidiports);

rate=0;
int r=ioctl(seqfd,SNDCTL_SEQ_CTRLRATE,&rate);
if ((r==-1)||(rate<=0)) rate=HZ;
convertrate=1000/rate;

//seqbuf_clean();
//ioctl(seqfd,SNDCTL_SEQ_RESET);
//ioctl(seqfd,SNDCTL_SEQ_PANIC);

loadFMPatches();

printf("FM Device %d opened\n",device);
#ifdef FMOUTDEBUG
printf("Number of synth devices : %d\n",ndevs);
printf("Number of midi ports : %d\n",nmidiports);
printf("Rate : %d\n",rate);
#endif

count=0.0;
lastcount=0.0;

};

void fmOut::closeDev (void)
{
if (!OK()) return;
SEQ_STOP_TIMER();
SEQ_DUMPBUF();
vm->cleanLists();
//if (seqfd>=0)
//    close(seqfd);
seqfd=-1;
#ifdef FMOUTDEBUG
printf("FM Device %d closed\n",device);
#endif
};

void fmOut::initDev (void)
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


if (opl==3) ioctl(seqfd, SNDCTL_FM_4OP_ENABLE, &device);
for (int i = 0; i < nvoices; i++)
     {
     SEQ_CONTROL(device, i, SEQ_VOLMODE, VOL_METHOD_LINEAR);
     SEQ_STOP_NOTE(device, i, vm->Note(i), 64);
     };

printf("FM Device %d initialized\n",device);
};

void fmOut::loadFMPatches(void)
{
char patchesfile[120];
char drumsfile[120];
int size;
struct sbi_instrument instr;
char tmp[60];
printf("Loading FM patches ...\n");
int i,j;
for (i=0;i<256;i++)
    patchloaded[i] = 0;

FILE *fh;
int datasize;

if (opl==3)
   {
   sprintf(patchesfile,"%s/std.o3",FM_patches_directory);
   size=60;
   }
  else
   {
   sprintf(patchesfile,"%s/std.sb",FM_patches_directory);
   size=52;
   };
fh=fopen(patchesfile,"rb");
if (fh==NULL) return;

for (i=0;i<128;i++)
   {
   fread(tmp,size,1,fh);
   patchloaded[i]=1;
   instr.key = ((strncmp(tmp, "4OP", 3) == 0))? OPL3_PATCH : FM_PATCH;
   datasize = (strncmp(tmp, "4OP", 3) == 0)? 22 : 11;
   instr.device=device;
   instr.channel = i;
   for (j=0; j<22; j++)
       instr.operators[j] = tmp[j+36];
   SEQ_WRPATCH(&instr,sizeof(instr));
   };
fclose(fh);

if (opl==3)
   {
   sprintf(drumsfile,"%s/drums.o3",FM_patches_directory);
   }
  else
   {
   sprintf(drumsfile,"%s/drums.sb",FM_patches_directory);
   };

fh=fopen(drumsfile,"rb");
if (fh==NULL) return;

for (i=128;i<256;i++)
   {
   fread(tmp,size,1,fh);
   patchloaded[i]=1;
   instr.key = (strncmp(tmp, "4OP", 3) == 0)? OPL3_PATCH : FM_PATCH;
   datasize = (strncmp(tmp, "4OP", 3) == 0)? 22 : 11;
   instr.device=device;
   instr.channel = i;
   for (j=0; j<22; j++)
       instr.operators[j] = tmp[j+36];
   SEQ_WRPATCH(&instr,sizeof(instr));
   };
fclose(fh);

#ifdef FMOUTDEBUG
printf("Patches loaded\n");
#endif
};

int fmOut::Patch(int p)
{
if (patchloaded[p]==1) return p;
printf("Not loaded %d!\n",p);
p=0;
while ((p<256)&&(patchloaded[p]==0)) p++;
return p;
};

void fmOut::noteOn  (uchar chn, uchar note, uchar vel)
{
if (vel==0)
    {
    noteOff(chn,note,vel);
    }
   else
    {
    if (chn==PERCUSSION_CHANNEL)
        {
	if (patchloaded[note+128]==0) return;
       else
	if (patchloaded[chn_patch[chn]]==0) return;
        };
    int v=vm->allocateVoice(chn,note);
    int p;
    if (chn==PERCUSSION_CHANNEL)
        SEQ_SET_PATCH(device,v ,p=Patch(note+128))
       else
        SEQ_SET_PATCH(device,v ,p=Map->Patch(chn,chn_patch[chn])); 
    SEQ_BENDER(device, v, chn_bender[chn]);

    SEQ_START_NOTE(device, v, note, vel);
    SEQ_CONTROL(device, v, CTL_MAIN_VOLUME, chn_controller[chn][CTL_MAIN_VOLUME]);
    SEQ_CHN_PRESSURE(device, v , chn_pressure[chn]);
    };

#ifdef FMOUTDEBUG
printf("Note ON >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
};

void fmOut::noteOff (uchar chn, uchar note, uchar vel)
{
int i;
vm->initSearch();
while ((i=vm->Search(chn,note))!=-1)
   {
   SEQ_STOP_NOTE(device, i, note, vel);
   vm->deallocateVoice(i);
   };

#ifdef FMOUTDEBUG
printf("Note OFF >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
};

void fmOut::keyPressure (uchar chn, uchar note, uchar vel)
{
int i;
vm->initSearch();
while ((i=vm->Search(chn,note))!=-1)
   SEQ_KEY_PRESSURE(device, i, note,vel);
};

void fmOut::chnPatchChange (uchar chn, uchar patch)
{
if (chn==PERCUSSION_CHANNEL) return;
int i;
vm->initSearch();
while ((i=vm->Search(chn))!=-1)
   SEQ_SET_PATCH(device,i,Map->Patch(chn,patch)); 
chn_patch[chn]=patch;
   
};

void fmOut::chnPressure (uchar chn, uchar vel)
{
int i;
vm->initSearch();
while ((i=vm->Search(chn))!=-1)
   SEQ_CHN_PRESSURE(device, i , vel);
chn_pressure[chn]=vel;
};

void fmOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
//chn_bender[chn]=(msb << 8) | (lsb & 0xFF);
//chn_bender[chn]=(msb << 7)+ (lsb);
chn_bender[chn]=((int)msb<<7) | (lsb & 0x7F);

int i;
vm->initSearch();
while ((i=vm->Search(chn))!=-1)
//   SEQ_PITCHBEND(device, i, chn_bender[chn]);
   SEQ_BENDER(device, i, chn_bender[chn]);
/*
int i=0;
while (i<nvoices)
    {
    if ((vm->Used(i)==1)&&(vm->Channel(i)==chn))
	SEQ_BENDER(device, i, chn_bender[chn]);
    i++;
    };
*/
};

void fmOut::chnController (uchar chn, uchar ctl, uchar v) 
{
int i;
vm->initSearch();
while ((i=vm->Search(chn))!=-1)
   SEQ_CONTROL(device, i, ctl, v);

/*
int i=0;
while (i<nvoices)
    {
    if ((vm->Used(i)==1)&&(vm->Channel(i)==chn))
	SEQ_CONTROL(device, i, ctl, v);
    i++;
    };
*/
chn_controller[chn][ctl]=v;
};

void fmOut::sysex(uchar *, ulong )
{

};

void fmOut::setFMPatchesDirectory(const char *dir)
{
if ((dir==NULL)||(dir[0]==0)) return;
if (delete_FM_patches_directory) delete FM_patches_directory;
FM_patches_directory=new char[strlen(dir)+1];
strcpy(FM_patches_directory,dir);
delete_FM_patches_directory=1;
};

char *fmOut::FM_patches_directory="/etc";
int fmOut::delete_FM_patches_directory = 0;
