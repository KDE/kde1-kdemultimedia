/*================================================================
 * SoundFont file extension
 *	written by Takashi Iwai <iwai@dragon.mm.t.u-tokyo.ac.jp>
 *================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "config.h"
#include "common.h"
#include "tables.h"
#include "instrum.h"
#include "playmidi.h"
#include "controls.h"
#include "sbk.h"
#include "sflayer.h"
#include "output.h"
#include "filter.h"
#include "resample.h"

/*----------------------------------------------------------------
 * compile flags
 *----------------------------------------------------------------*/

/*#define SF_CLOSE_EACH_FILE*/

/*#define SF_SUPPRESS_ENVELOPE*/
/*#define SF_SUPPRESS_TREMOLO*/
/*#define SF_SUPPRESS_VIBRATO*/
#define SF_SUPPRESS_CUTOFF

/*----------------------------------------------------------------
 * local parameters
 *----------------------------------------------------------------*/

typedef struct _Layer {
	int16 val[PARM_SIZE];
	int8 set[PARM_SIZE];
} Layer;

typedef struct _SampleList {
	Sample v;
	struct _SampleList *next;
	int32 startsample, endsample;
	int32 cutoff_freq;
	float resonance;
} SampleList;

typedef struct _InstList {
	int bank, preset, keynote;
	int samples;
	int order;
	SampleList *slist;
	struct _InstList *next;
} InstList;

typedef struct SFInsts {
	char *fname;
	FILE *fd;
	uint16 version, minorversion;
	int32 samplepos, samplesize;
	InstList *instlist;
} SFInsts;

typedef struct _SFExclude {
	int bank, preset, keynote;
	struct _SFExclude *next;
} SFExclude;

typedef struct _SFOrder {
	int bank, preset, keynote;
	int order;
	struct _SFOrder *next;
} SFOrder;


/*----------------------------------------------------------------*/

static void free_sample(InstList *ip);
static Instrument *load_from_file(SFInsts *rec, InstList *ip);
static int is_excluded(int bank, int preset, int keynote);
static void free_exclude(void);
static int is_ordered(int bank, int preset, int keynote);
static void free_order(void);
static void parse_preset(SFInsts *rec, SFInfo *sf, int preset, int order);
static void parse_gen(Layer *lay, tgenrec *gen);
static void parse_preset_layer(Layer *lay, SFInfo *sf, int idx);
/*static void merge_layer(Layer *dst, Layer *src);*/
static int search_inst(Layer *lay);
static void parse_inst(SFInsts *rec, Layer *pr_lay, SFInfo *sf, int preset, int inst, int order);
static void parse_inst_layer(Layer *lay, SFInfo *sf, int idx);
static int search_sample(Layer *lay);
static void append_layer(Layer *dst, Layer *src, SFInfo *sf);
static void make_inst(SFInsts *rec, Layer *lay, SFInfo *sf, int pr_idx, int in_idx, int order);
static int32 calc_root_pitch(Layer *lay, SFInfo *sf, SampleList *sp);
static void convert_volume_envelope(Layer *lay, SFInfo *sf, SampleList *sp);
static int32 to_offset(int offset);
static int32 calc_rate(int diff, int time);
static int32 to_msec(Layer *lay, SFInfo *sf, int index);
static float calc_volume(Layer *lay, SFInfo *sf);
static int32 calc_sustain(Layer *lay, SFInfo *sf);
static void convert_tremolo(Layer *lay, SFInfo *sf, SampleList *sp);
static void convert_vibrato(Layer *lay, SFInfo *sf, SampleList *sp);
/*static void do_lowpass(Sample *sp, int32 freq, float resonance);*/
static void calc_cutoff(Layer *lay, SFInfo *sf, SampleList *sp);
static void calc_filterQ(Layer *lay, SFInfo *sf, SampleList *sp);

/*----------------------------------------------------------------*/


static SFInsts sfrec;
static SFExclude *sfexclude;
static SFOrder *sforder;

int cutoff_allowed = 0;


void init_soundfont(char *fname, int order)
{
	static SFInfo sfinfo;
	int i;

	ctl->cmsg(CMSG_INFO, VERB_NOISY, "init soundfonts `%s'", fname);

	if ((sfrec.fd = open_file(fname, 1, OF_VERBOSE)) == NULL) {
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
			  "can't open soundfont file %s", fname);
		return;
	}
	sfrec.fname = strcpy((char*)safe_malloc(strlen(fname)+1), fname);
	load_sbk(sfrec.fd, &sfinfo);

	for (i = 0; i < sfinfo.nrpresets; i++) {
		int bank = sfinfo.presethdr[i].bank;
		int preset = sfinfo.presethdr[i].preset;
		if (is_excluded(bank, preset, -1))
			continue;
		if (bank == 128) {
			if (!drumset[preset]) {
				drumset[preset] = safe_malloc(sizeof(ToneBank));
				memset(drumset[preset], 0, sizeof(ToneBank));
			}
		} else {
			if (!tonebank[bank]) {
				tonebank[bank] = safe_malloc(sizeof(ToneBank));
				memset(tonebank[bank], 0, sizeof(ToneBank));
			}
		}
		parse_preset(&sfrec, &sfinfo, i, order);
	}

	/* copy header info */
	sfrec.version = sfinfo.version;
	sfrec.minorversion = sfinfo.minorversion;
	sfrec.samplepos = sfinfo.samplepos;
	sfrec.samplesize = sfinfo.samplesize;

	free_sbk(&sfinfo);

#ifdef SF_CLOSE_EACH_FILE
	fclose(sfrec.fd);
	sfrec.fd = NULL;
#endif
}


static void free_sample(InstList *ip)
{
	SampleList *sp, *snext;
	for (sp = ip->slist; sp; sp = snext) {
		snext = sp->next;
		free(sp);
	}
	free(ip);
}

void end_soundfont(void)
{
	InstList *ip, *next;

	if (sfrec.fd) {
		fclose(sfrec.fd);
		sfrec.fd = NULL;
	}
	free(sfrec.fname); sfrec.fname = NULL;

	for (ip = sfrec.instlist; ip; ip = next) {
		next = ip->next;
		free_sample(ip);
	}
	sfrec.instlist = NULL;

	free_exclude();
	free_order();
}


/*----------------------------------------------------------------
 * get converted instrument info and load the wave data from file
 *----------------------------------------------------------------*/

Instrument *load_soundfont(int order, int bank, int preset, int keynote)
{
	InstList *ip;
	Instrument *inst = NULL;

	if (sfrec.fd == NULL) {
		if (sfrec.fname == NULL)
			return NULL;
		if ((sfrec.fd = open_file(sfrec.fname, 1, OF_VERBOSE)) == NULL) {
			ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
				  "can't open soundfont file %s", sfrec.fname);
			return NULL;
		}
	}

	for (ip = sfrec.instlist; ip; ip = ip->next) {
		if (ip->bank == bank && ip->preset == preset &&
		    (keynote < 0 || keynote == ip->keynote) &&
		    ip->order == order)
			break;
	}
	if (ip && ip->samples)
		inst = load_from_file(&sfrec, ip);

#ifdef SF_CLOSE_EACH_FILE
	fclose(sfrec.fd);
	sfrec.fd = NULL;
#endif

	return inst;
}


static Instrument *load_from_file(SFInsts *rec, InstList *ip)
{
	SampleList *sp;
	Instrument *inst;
	int i;

	ctl->cmsg(CMSG_INFO, VERB_NOISY, "Loading SF bank%d prg%d note%d",
		  ip->bank, ip->preset, ip->keynote);

	inst = safe_malloc(sizeof(Instrument));
	inst->type = INST_SF2;
	inst->samples = ip->samples;
	inst->sample = safe_malloc(sizeof(Sample)*ip->samples);
	for (i = 0, sp = ip->slist; i < ip->samples && sp; i++, sp = sp->next) {
		Sample *sample = inst->sample + i;
#ifndef LITTLE_ENDIAN
		int32 j;
		int16 *tmp, s;
#endif
		memcpy(sample, &sp->v, sizeof(Sample));
		sample->data = safe_malloc(sp->endsample);
		fseek(rec->fd, sp->startsample, SEEK_SET);
		fread(sample->data, sp->endsample, 1, rec->fd);
#ifndef LITTLE_ENDIAN
		tmp = (int16*)sample->data;
		for (j = 0; j < sp->endsample/2; j++) {
			s = LE_SHORT(*tmp);
			*tmp++ = s;
		}
#endif

		/* do some filtering if necessary */
#ifndef SF_SUPPRESS_CUTOFF
		if (sp->cutoff_freq > 0 && cutoff_allowed) {
			/* restore the normal value */
			sample->data_length >>= FRACTION_BITS;
			fprintf(stderr, "bank=%d, preset=%d, keynote=%d / cutoff = %d / resonance = %g\n",
				ip->bank, ip->preset, ip->keynote, sp->cutoff_freq, sp->resonance);
			do_lowpass(sample, sp->cutoff_freq, sp->resonance);
			/* convert again to the fractional value */
			sample->data_length <<= FRACTION_BITS;
		}
#endif

		if (antialiasing_allowed) {
			/* restore the normal value */
			sample->data_length >>= FRACTION_BITS;
			antialiasing(sample, play_mode->rate);
			/* convert again to the fractional value */
			sample->data_length <<= FRACTION_BITS;
		}

		/* resample it if possible */
		if (sample->note_to_use && !(sample->modes & MODES_LOOPING))
			pre_resample(sample);

#ifdef LOOKUP_HACK
		/* squash the 16-bit data into 8 bits. */
		{
			uint8 *gulp,*ulp;
			int16 *swp;
			int l = sample->data_length >> FRACTION_BITS;
			gulp = ulp = safe_malloc(l + 1);
			swp = (int16 *)sample->data;
			while (l--)
				*ulp++ = (*swp++ >> 8) & 0xFF;
			free(sample->data);
			sample->data=(sample_t *)gulp;
		}
#endif
	}
	return inst;
}


/*----------------------------------------------------------------
 * excluded samples
 *----------------------------------------------------------------*/

void exclude_soundfont(int bank, int preset, int keynote)
{
	SFExclude *rec;
	rec = (SFExclude*)safe_malloc(sizeof(SFExclude));
	rec->bank = bank;
	rec->preset = preset;
	rec->keynote = keynote;
	rec->next = sfexclude;
	sfexclude = rec;
}

/* check the instrument is specified to be excluded */
static int is_excluded(int bank, int preset, int keynote)
{
	SFExclude *p;
	for (p = sfexclude; p; p = p->next) {
		if (p->bank == bank &&
		    (p->preset < 0 || p->preset == preset) &&
		    (p->keynote < 0 || p->keynote == keynote))
			return 1;
	}
	return 0;
}

/* free exclude list */
static void free_exclude(void)
{
	SFExclude *p, *next;
	for (p = sfexclude; p; p = next) {
		next = p->next;
		free(p);
	}
	sfexclude = NULL;
}


/*----------------------------------------------------------------
 * ordered samples
 *----------------------------------------------------------------*/

void order_soundfont(int bank, int preset, int keynote, int order)
{
	SFOrder *rec;
	rec = (SFOrder*)safe_malloc(sizeof(SFOrder));
	rec->bank = bank;
	rec->preset = preset;
	rec->keynote = keynote;
	rec->order = order;
	rec->next = sforder;
	sforder = rec;
}

/* check the instrument is specified to be ordered */
static int is_ordered(int bank, int preset, int keynote)
{
	SFOrder *p;
	for (p = sforder; p; p = p->next) {
		if (p->bank == bank &&
		    (p->preset < 0 || p->preset == preset) &&
		    (p->keynote < 0 || p->keynote == keynote))
			return p->order;
	}
	return -1;
}

/* free order list */
static void free_order(void)
{
	SFOrder *p, *next;
	for (p = sforder; p; p = next) {
		next = p->next;
		free(p);
	}
	sforder = NULL;
}


/*----------------------------------------------------------------
 * parse a preset
 *----------------------------------------------------------------*/

static void parse_preset(SFInsts *rec, SFInfo *sf, int preset, int order)
{
	int from_ndx, to_ndx;
	Layer lay, glay;
	int i, inst;

	from_ndx = sf->presethdr[preset].bagNdx;
	to_ndx = sf->presethdr[preset+1].bagNdx;

	memset(&glay, 0, sizeof(glay));
	for (i = from_ndx; i < to_ndx; i++) {
		memset(&lay, 0, sizeof(Layer));
		parse_preset_layer(&lay, sf, i);
		inst = search_inst(&lay);
		if (inst < 0) /* global layer */
			memcpy(&glay, &lay, sizeof(Layer));
		else {
			append_layer(&lay, &glay, sf);
			parse_inst(rec, &lay, sf, preset, inst, order);
		}
	}
}

/* map a generator operation to the layer structure */
static void parse_gen(Layer *lay, tgenrec *gen)
{
	lay->set[gen->oper] = 1;
	lay->val[gen->oper] = gen->amount;
}

/* parse preset generator layers */
static void parse_preset_layer(Layer *lay, SFInfo *sf, int idx)
{
	int i;
	for (i = sf->presetbag[idx]; i < sf->presetbag[idx+1]; i++)
		parse_gen(lay, sf->presetgen + i);
}

/* merge two layers; never overrides on the destination */
/*
static void merge_layer(Layer *dst, Layer *src)
{
	int i;
	for (i = 0; i < PARM_SIZE; i++) {
		if (src->set[i] && !dst->set[i]) {
			dst->val[i] = src->val[i];
			dst->set[i] = 1;
		}
	}
}
*/

/* search instrument id from the layer */
static int search_inst(Layer *lay)
{
	if (lay->set[SF_instrument])
		return lay->val[SF_instrument];
	else
		return -1;
}

/* parse an instrument */
static void parse_inst(SFInsts *rec, Layer *pr_lay, SFInfo *sf, int preset, int inst, int order)
{
	int from_ndx, to_ndx;
	int i, sample;
	Layer lay, glay;

	from_ndx = sf->insthdr[inst].bagNdx;
	to_ndx = sf->insthdr[inst+1].bagNdx;

	memcpy(&glay, pr_lay, sizeof(Layer));
	for (i = from_ndx; i < to_ndx; i++) {
		memset(&lay, 0, sizeof(Layer));
		parse_inst_layer(&lay, sf, i);
		sample = search_sample(&lay);
		if (sample < 0) /* global layer */
			append_layer(&glay, &lay, sf);
		else {
			append_layer(&lay, &glay, sf);
			make_inst(rec, &lay, sf, preset, inst, order);
		}
	}
}

/* parse instrument generator layers */
static void parse_inst_layer(Layer *lay, SFInfo *sf, int idx)
{
	int i;
	for (i = sf->instbag[idx]; i < sf->instbag[idx+1]; i++)
		parse_gen(lay, sf->instgen + i);
}

/* search a sample id from instrument layers */
static int search_sample(Layer *lay)
{
	if (lay->set[SF_sampleId])
		return lay->val[SF_sampleId];
	else
		return -1;
}


/* two (high/low) 8 bit values in 16 bit parameter */
#define LO_VAL(val)	((val) & 0xff)
#define HI_VAL(val)	(((val) >> 8) & 0xff)
#define SET_LO(vp,val)	((vp) = ((vp) & 0xff00) | (val))
#define SET_HI(vp,val)	((vp) = ((vp) & 0xff) | ((val) << 8))

/* append two layers; parameters are added to the original value */
static void append_layer(Layer *dst, Layer *src, SFInfo *sf)
{
	int i;
	for (i = 0; i < PARM_SIZE; i++) {
		if (src->set[i]) {
			if (sf->version == 1 && i == SF_instVol)
				dst->val[i] = (src->val[i] * 127) / 127;
			else if (i == SF_keyRange || i == SF_velRange) {
				/* high limit */
				if (HI_VAL(dst->val[i]) > HI_VAL(src->val[i]))
					SET_HI(dst->val[i], HI_VAL(src->val[i]));
				/* low limit */
				if (LO_VAL(dst->val[i]) < LO_VAL(src->val[i]))
					SET_LO(dst->val[i], LO_VAL(src->val[i]));
			} else
				dst->val[i] += src->val[i];
			dst->set[i] = 1;
		}
	}
}

/* convert layer info to timidity instrument strucutre */
static void make_inst(SFInsts *rec, Layer *lay, SFInfo *sf, int pr_idx, int in_idx, int order)
{
	int bank = sf->presethdr[pr_idx].bank;
	int preset = sf->presethdr[pr_idx].preset;
	int keynote, n_order;
	char **namep;
	InstList *ip;
	tsampleinfo *sample;
	SampleList *sp;

	sample = &sf->sampleinfo[lay->val[SF_sampleId]];
	if (sample->sampletype & 0x8000) /* is ROM sample? */
		return;

	/* set bank/preset name */
	if (bank == 128) {
		keynote = LO_VAL(lay->val[SF_keyRange]);
		namep = &drumset[preset]->tone[keynote].name;
	} else {
		keynote = -1;
		namep = &tonebank[bank]->tone[preset].name;
	}
	if (is_excluded(bank, preset, keynote))
		return;
	if ((n_order = is_ordered(bank, preset, keynote)) >= 0)
		order = n_order;

	if (*namep == NULL) {
		*namep = safe_malloc(21);
		memcpy(*namep, sf->insthdr[in_idx].name, 20);
		(*namep)[20] = 0;
	}

	/* search current instrument list */
	for (ip = rec->instlist; ip; ip = ip->next) {
		if (ip->bank == bank && ip->preset == preset &&
		    (keynote < 0 || keynote == ip->keynote))
			break;
	}
	if (ip == NULL) {
		ip = (InstList*)safe_malloc(sizeof(InstList));
		ip->bank = bank;
		ip->preset = preset;
		ip->keynote = keynote;
		ip->order = order;
		ip->samples = 0;
		ip->slist = NULL;
		ip->next = rec->instlist;
		rec->instlist = ip;
	}

	/* add a sample */
	sp = (SampleList*)safe_malloc(sizeof(SampleList));
	sp->next = ip->slist;
	ip->slist = sp;
	ip->samples++;

	/* set sample position */
	sp->startsample = (lay->val[SF_startAddrsHi] << 16)
		+ lay->val[SF_startAddrs]
		+ sample->startsample;
	sp->endsample = (lay->val[SF_endAddrsHi] << 16)
		+ lay->val[SF_endAddrs]
		+ sample->endsample - sp->startsample;

	/* set loop position */
	sp->v.loop_start = (lay->val[SF_startloopAddrsHi] << 16)
		+ lay->val[SF_startloopAddrsHi]
		+ sample->startloop - sp->startsample;
	sp->v.loop_end = (lay->val[SF_endloopAddrsHi] << 16)
		+ lay->val[SF_endloopAddrsHi]
		+ sample->endloop - sp->startsample;
	sp->v.data_length = sp->endsample;

#if 0
	if (sp->v.loop_start < 0)
		fprintf(stderr, "negative loop pointer\n");
	if (sp->v.loop_start > sp->v.loop_end)
		fprintf(stderr, "illegal loop position\n");
	if (sp->v.loop_end > sp->v.data_length)
		fprintf(stderr, "illegal loop end or data size\n");
#endif

	sp->v.sample_rate = sample->samplerate;
	if (lay->set[SF_keyRange]) {
		sp->v.low_freq = freq_table[LO_VAL(lay->val[SF_keyRange])];
		sp->v.high_freq = freq_table[HI_VAL(lay->val[SF_keyRange])];
	} else {
		sp->v.low_freq = freq_table[0];
		sp->v.high_freq = freq_table[127];
	}

	/* scale tuning: 0  - 100 */
	sp->v.scale_tuning = 100;
	if (lay->set[SF_scaleTuning]) {
		if (sf->version == 1)
			sp->v.scale_tuning = lay->val[SF_scaleTuning] ? 50 : 100;
		else
			sp->v.scale_tuning = lay->val[SF_scaleTuning];
	}

	/* root pitch */
	sp->v.root_freq = calc_root_pitch(lay, sf, sp);

	sp->v.modes = MODES_16BIT;

	/* volume envelope & total volume */
	sp->v.volume = calc_volume(lay,sf);
	if (lay->val[SF_sampleFlags] == 1 || lay->val[SF_sampleFlags] == 3) {
		sp->v.modes |= MODES_LOOPING|MODES_SUSTAIN;
#ifndef SF_SUPPRESS_ENVELOPE
		convert_volume_envelope(lay, sf, sp);
#endif
		if (lay->val[SF_sampleFlags] == 3)
			/* strip the tail */
			sp->v.data_length = sp->v.loop_end + 1;
	}

	/* panning position: 0 to 127 */
	sp->v.panning = 64;
	if (lay->set[SF_panEffectsSend]) {
		if (sf->version == 1)
			sp->v.panning = (int8)lay->val[SF_panEffectsSend];
		else
			sp->v.panning = (int8)(((int)lay->val[SF_panEffectsSend] + 500) * 127 / 1000);
	}

	/* tremolo & vibrato */
	sp->v.tremolo_sweep_increment = 0;
	sp->v.tremolo_phase_increment = 0;
	sp->v.tremolo_depth = 0;
#ifndef SF_SUPPRESS_TREMOLO
	convert_tremolo(lay, sf, sp);
#endif
	sp->v.vibrato_sweep_increment = 0;
	sp->v.vibrato_control_ratio = 0;
	sp->v.vibrato_depth = 0;
#ifndef SF_SUPPRESS_VIBRATO
	convert_vibrato(lay, sf, sp);
#endif

	/* set note to use for drum voices */
	if (bank == 128)
		sp->v.note_to_use = keynote;
	else
		sp->v.note_to_use = 0;

	/* convert to fractional samples */
	sp->v.data_length <<= FRACTION_BITS;
	sp->v.loop_start <<= FRACTION_BITS;
	sp->v.loop_end <<= FRACTION_BITS;

	/* point to the file position */
	sp->startsample = sp->startsample * 2 + sf->samplepos;
	sp->endsample *= 2;

	/* set cutoff frequency */
	sp->cutoff_freq = 0;
	if (lay->set[SF_initialFilterFc] || lay->set[SF_env1ToFilterFc])
		calc_cutoff(lay, sf, sp);
	if (lay->set[SF_initialFilterQ])
		calc_filterQ(lay, sf, sp);
}

/* calculate root pitch */
static int32 calc_root_pitch(Layer *lay, SFInfo *sf, SampleList *sp)
{
	int32 root, tune;
	tsampleinfo *sample;

	sample = &sf->sampleinfo[lay->val[SF_sampleId]];

	root = sample->originalPitch;
	tune = sample->pitchCorrection;
	if (sf->version == 1) {
		if (lay->set[SF_samplePitch]) {
			root = lay->val[SF_samplePitch] / 100;
			tune = -lay->val[SF_samplePitch] % 100;
			if (tune <= -50) {
				root++;
				tune = 100 + tune;
			}
			if (sp->v.scale_tuning == 50)
				tune /= 2;
		}
		/* orverride root key */
		if (lay->set[SF_rootKey])
			root += lay->val[SF_rootKey] - 60;
		/* tuning */
		tune += lay->val[SF_coarseTune] * sp->v.scale_tuning +
			lay->val[SF_fineTune] * sp->v.scale_tuning / 100;
	} else {
		/* orverride root key */
		if (lay->set[SF_rootKey])
			root = lay->val[SF_rootKey];
		/* tuning */
		tune += lay->val[SF_coarseTune] * 100
			+ lay->val[SF_fineTune];
	}
	/* it's too high.. */
	if (lay->set[SF_keyRange] &&
	    root >= HI_VAL(lay->val[SF_keyRange]) + 60)
		root -= 60;

	while (tune <= -100) {
		root++;
		tune += 100;
	}
	while (tune > 0) {
		root--;
		tune -= 100;
	}
	return (int32)((double)freq_table[root] * bend_fine[(-tune*255)/100]);
}


/*----------------------------------------------------------------
 * convert volume envelope
 *----------------------------------------------------------------*/

static void convert_volume_envelope(Layer *lay, SFInfo *sf, SampleList *sp)
{
	int32 sustain = calc_sustain(lay, sf);
	/*int delay = to_msec(lay, sf, SF_delayEnv2);*/
	int32 attack = to_msec(lay, sf, SF_attackEnv2);
	int32 hold = to_msec(lay, sf, SF_holdEnv2);
	int32 decay = to_msec(lay, sf, SF_decayEnv2);
	int32 release = to_msec(lay, sf, SF_releaseEnv2);

	sp->v.envelope_offset[0] = to_offset(255);
	sp->v.envelope_rate[0] = calc_rate(255, attack) * 2;

	sp->v.envelope_offset[1] = to_offset(250);
	sp->v.envelope_rate[1] = calc_rate(5, hold);
	sp->v.envelope_offset[2] = to_offset(sustain);
	sp->v.envelope_rate[2] = calc_rate(250 - sustain, decay);
	sp->v.envelope_offset[3] = to_offset(5);
	sp->v.envelope_rate[3] = calc_rate(255, release);
	sp->v.envelope_offset[4] = to_offset(4);
	sp->v.envelope_rate[4] = to_offset(200);
	sp->v.envelope_offset[5] = to_offset(4);
	sp->v.envelope_rate[5] = to_offset(200);

	sp->v.modes |= MODES_ENVELOPE;
}

/* convert from 8bit value to fractional offset (15.15) */
static int32 to_offset(int offset)
{
	return (int32)offset << (7+15);
}

/* calculate ramp rate in fractional unit;
 * diff = 8bit, time = msec
 */
static int32 calc_rate(int diff, int time)
{
	int32 rate;

	if (time < 6) time = 6;
	if (diff == 0) diff = 255;
	diff <<= (7+15);
	rate = (diff / play_mode->rate) * control_ratio;
	rate = rate * 1000 / time;
	if (fast_decay) rate *= 2;

	return rate;
}


#define TO_MSEC(tcents) (int32)(1000 * pow(2.0, (double)(tcents) / 1200.0))
#define TO_MHZ(abscents) (int32)(8176.0 * pow(2.0,(double)(abscents)/1200.0))
#define TO_HZ(abscents) (int32)(8.176 * pow(2.0,(double)(abscents)/1200.0))
#define TO_LINEAR(centibel) pow(10.0, -(double)(centibel)/200.0)
#define TO_VOLUME(centibel) (uint8)(255 * (1.0 - (centibel) / (1200.0 * log10(2.0))));

/* convert the value to milisecs */
static int32 to_msec(Layer *lay, SFInfo *sf, int index)
{
	int16 value;
	if (! lay->set[index])
		return 6;  /* 6msec minimum */
	value = lay->val[index];
	if (sf->version == 1)
		return value;
	else
		return TO_MSEC(value);
}

/* convert peak volume to linear volume (0-255) */
static float calc_volume(Layer *lay, SFInfo *sf)
{
	if (sf->version == 1)
		return (float)(lay->val[SF_instVol] * 2) / 255.0;
	else
		return TO_LINEAR((double)lay->val[SF_instVol] / 10.0);
}

/* convert sustain volume to linear volume */
static int32 calc_sustain(Layer *lay, SFInfo *sf)
{
	int32 level;
	if (!lay->set[SF_sustainEnv2])
		return 250;
	level = lay->val[SF_sustainEnv2];
	if (sf->version == 1) {
		if (level < 96)
			level = 1000 * (96 - level) / 96;
		else
			return 0;
	}
	return TO_VOLUME(level);
}


#ifndef SF_SUPPRESS_TREMOLO
/*----------------------------------------------------------------
 * tremolo (LFO1) conversion
 *----------------------------------------------------------------*/

static void convert_tremolo(Layer *lay, SFInfo *sf, SampleList *sp)
{
	int32 level, freq;

	if (!lay->set[SF_lfo1ToVolume])
		return;

	level = lay->val[SF_lfo1ToVolume];
	if (sf->version == 1)
		level = (120 * level) / 64;  /* to centibel */
	/* centibel to linear */
	sp->v.tremolo_depth = TO_LINEAR(level);

	/* frequency in mHz */
	if (lay->set[SF_freqLfo1]) {
		if (sf->version == 1)
			freq = TO_MHZ(-725);
		else
			freq = 0;
	} else {
		freq = lay->val[SF_freqLfo1];
		if (freq > 0 && sf->version == 1)
			freq = (int)(3986.0 * log10((double)freq) - 7925.0);
		freq = TO_MHZ(freq);
	}
	/* convert mHz to sine table increment; 1024<<rate_shift=1wave */
	sp->v.tremolo_phase_increment = (freq * 1024) << RATE_SHIFT;

	sp->v.tremolo_sweep_increment = 0;
}
#endif


#ifndef SF_SUPPRESS_VIBRATO
/*----------------------------------------------------------------
 * vibrato (LFO2) conversion
 *----------------------------------------------------------------*/

static void convert_vibrato(Layer *lay, SFInfo *sf, SampleList *sp)
{
	int32 shift, freq;

	if (!lay->set[SF_lfo2ToPitch])
		return;

	/* pitch shift in cents (= 1/100 semitone) */
	shift = lay->val[SF_lfo2ToPitch];
	if (sf->version == 1)
		shift = (1200 * shift / 64 + 1) / 2;

	/* cents to linear; 400cents = 256 */
	sp->v.vibrato_depth = (int8)((int32)shift * 256 / 400);

	/* frequency in mHz */
	if (lay->set[SF_freqLfo2]) {
		if (sf->version == 1)
			freq = TO_MHZ(-725);
		else
			freq = 0;
	} else {
		freq = lay->val[SF_freqLfo2];
		if (freq > 0 && sf->version == 1)
			freq = (int)(3986.0 * log10((double)freq) - 7925.0);
		freq = TO_MHZ(freq);
	}
	/* convert mHz to control ratio */
	sp->v.vibrato_control_ratio = freq *
		(VIBRATO_RATE_TUNING * play_mode->rate) /
		(2 * VIBRATO_SAMPLE_INCREMENTS);

	sp->v.vibrato_sweep_increment = 0;
}
#endif


/* calculate cutoff/resonance frequency */
static void calc_cutoff(Layer *lay, SFInfo *sf, SampleList *sp)
{
	int16 val;
	if (! lay->set[SF_initialFilterFc]) {
		val = 13500;
	} else {
		val = lay->val[SF_initialFilterFc];
		if (sf->version == 1) {
			if (val == 127)
				val = 14400;
			else if (val > 0)
				val = 50 * val + 4366;
		}
	}
	if (lay->set[SF_env1ToFilterFc]) {
		val += lay->val[SF_env1ToFilterFc];
	}
	if (val >= 13500)
		sp->cutoff_freq = 0;
	else
		sp->cutoff_freq = TO_HZ(val);
}

static void calc_filterQ(Layer *lay, SFInfo *sf, SampleList *sp)
{
	int16 val = lay->val[SF_initialFilterQ];
	if (sf->version == 1)
		val = val * 3 / 2; /* to centibels */
	sp->resonance = pow(10.0, (double)val / 2.0 / 200.0) - 1;
	if (sp->resonance < 0)
		sp->resonance = 0;
}


#ifndef SF_SUPPRESS_CUTOFF
/*----------------------------------------------------------------
 * low-pass filter:
 * 	y(n) = A * x(n) + B * y(n-1)
 * 	A = 2.0 * pi * center
 * 	B = exp(-A / frequency)
 *----------------------------------------------------------------
 * resonance filter:
 *	y(n) = a * x(n) - b * y(n-1) - c * y(n-2)
 *	c = exp(-2 * pi * width / rate)
 *	b = -4 * c / (1+c) * cos(2 * pi * center / rate)
 *	a = sqt(1-b*b/(4 * c)) * (1-c)
 *----------------------------------------------------------------*/

#ifdef LOOKUP_HACK
#define MAX_DATAVAL 127
#define MIN_DATAVAL -128
#else
#define MAX_DATAVAL 32767
#define MIN_DATAVAL -32768
#endif

static void do_lowpass(Sample *sp, int32 freq, float resonance)
{
	double A, B, C;
	sample_t *buf, pv1, pv2;
	int32 i;

	if (freq > sp->sample_rate * 2) {
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
			  "Lowpass: center must be < data rate*2");
		return;
	}
	A = 2.0 * M_PI * freq * 2.5 / sp->sample_rate;
	B = exp(-A / sp->sample_rate);
	A *= 0.8;
	B *= 0.8;
	C = 0;

	/*
	if (resonance) {
		double a, b, c;
		int32 width;
		width = freq / 5;
		c = exp(-2.0 * M_PI * width / sp->sample_rate);
		b = -4.0 * c / (1+c) * cos(2.0 * M_PI * freq / sp->sample_rate);
		a = sqrt(1 - b * b / (4 * c)) * (1 - c);
		b = -b; c = -c;

		A += a * resonance;
		B += b;
		C = c;
	}
	*/

	pv1 = 0;
	pv2 = 0;
	buf = sp->data;
	for (i = 0; i < sp->data_length; i++) {
		sample_t l = *buf;
		double d = A * l + B * pv1 + C * pv2;
		if (d > MAX_DATAVAL)
			d = MAX_DATAVAL;
		else if (d < MIN_DATAVAL)
			d = MIN_DATAVAL;
		pv2 = pv1;
		pv1 = *buf++ = (sample_t)d;
	}
}
#endif
