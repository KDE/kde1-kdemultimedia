/*
 * @(#)plat_svr4.c	1.1	1/2/94
 *
 * SVR4 specific.  Much of this is similar to plat_hpux.c.
 */
static char *ident = "@(#)plat_svr4.c	1.1\t1/2/94";


#include "config.h"

#if defined(sgi)


#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dmedia/cdaudio.h>
#include <errno.h>

#include "struct.h"


void *malloc();
char *strchr();

int	min_volume = 0;
int	max_volume = 255;

extern char	*cd_device;

/*
 * Initialize the drive.  A no-op for the generic driver.
 */
int
gen_init(d)
	struct wm_drive	*d;
{
	return 0;
}

/*
 * Get the number of tracks on the CD.
 */
int
gen_get_trackcount(d, tracks)
	struct wm_drive	*d;
	int		*tracks;
{
	CDSTATUS s;
	if( CDgetstatus(d->daux, &s)==0 )
		return -1;
	*tracks = s.last;
	return 0;
}

/*
 * Get the start time and mode (data or audio) of a track.
 */
int
gen_get_trackinfo(d, track, data, startframe)
	struct wm_drive	*d;
	int		track, *data, *startframe;
{
	CDTRACKINFO i;
	int ret = CDgettrackinfo(d->daux, track, &i);
	if( ret == 0 )
		return -1;
	*data = 0;
	*startframe = CDmsftoframe(i.start_min,i.start_sec,i.start_frame);
	return 0;
}

/*
 * Get the number of frames on the CD.
 */
int
gen_get_cdlen(d, frames)
	struct wm_drive	*d;
	int		*frames;
{
	CDSTATUS s;
	if( CDgetstatus(d->daux, &s)==0 )
		return -1;
	*frames = CDmsftoframe(s.total_min,s.total_sec,s.total_frame);
	return 0;
}

/*
 * Get the current status of the drive: the current play mode, the absolute
 * position from start of disc (in frames), and the current track and index
 * numbers if the CD is playing or paused.
 */
int
gen_get_drive_status(d, oldmode, mode, pos, track, index)
	struct wm_drive	*d;
	enum cd_modes	oldmode, *mode;
	int		*pos, *track, *index;
{
	CDSTATUS s;
	if( CDgetstatus(d->daux, &s)==0 )
		return -1;
	*pos = CDmsftoframe(s.abs_min,s.abs_sec,s.abs_frame);
	*track = s.track;
	*index = 0;
	switch( s.state )
	{
		case CD_READY:	*mode = STOPPED;
						break;
		case CD_STILL:
		case CD_PAUSED: *mode = PAUSED;
						break;
		case CD_PLAYING: *mode = PLAYING;
						break;
		default:		*mode = UNKNOWN;
	}
	return 0;
}

/*
 * Set the volume level for the left and right channels.  Their values
 * range from 0 to 100.
 */
int
gen_set_volume(d, left, right)
	struct wm_drive	*d;
	int		left, right;
{
	/* We should set the volume of the audio port */
	return -1;
}

/*
 * Read the initial volume from the drive, if available.  Each channel
 * ranges from 0 to 100, with -1 indicating data not available.
 */
int
gen_get_volume(d, left, right)
	struct wm_drive	*d;
	int		*left, *right;
{
	/* We should get the volume of the audio port */
	return -1;
}

/*
 * Pause the CD.
 */
int
gen_pause(d)
	struct wm_drive	*d;
{
    CDSTATUS s;
    if( CDgetstatus(d->daux, &s)==0 )
        return -1;
    if(s.state == CD_PLAYING)
	    CDtogglepause(d->daux);
	return 0;
}

/*
 * Resume playing the CD (assuming it was paused.)
 */
int
gen_resume(d)
	struct wm_drive	*d;
{
    CDSTATUS s;
    if( CDgetstatus(d->daux, &s)==0 )
        return -1;
    if(s.state == CD_PAUSED)
	    CDtogglepause(d->daux);
	return 0;
}

/*
 * Stop the CD.
 */
int
gen_stop(d)
	struct wm_drive	*d;
{
	CDstop(d->daux);
	return 0;
}

/*
 * Play the CD from one position to another (both in frames.)
 */
int
gen_play(d, start, end)
	struct wm_drive	*d;
	int		start, end;
{
	int m, s, f;
	CDframetomsf(start, &m, &s, &f);
	CDplayabs(d->daux, m, s, f, 1);
	return (wm_scsi2_play(d, start, end));
}

/*
 * Eject the current CD, if there is one.
 */
int
gen_eject(d)
	struct wm_drive	*d;
{
	CDeject(d->daux);
	return 0;
}

/*
 * Open the CD and figure out which kind of drive is attached.
 */
int
wmcd_open(d)
	struct wm_drive	*d;
{
	int	fd;
	CDSTATUS s;

	if (d->fd < 0)		/* Device already open? */
	{
		if (cd_device == NULL)
			cd_device = DEFAULT_CD_DEVICE;

		d->fd = 1;

		/* Now fill in the relevant parts of the wm_drive structure. */
		fd = d->fd;
		*d = *(find_drive_struct("", "", ""));
		d->fd = fd;
		(d->init)(d);

		d->daux = CDopen(cd_device,"r");
		if (d->daux == 0)
		{
			perror(cd_device);
			exit(1);
		}
	}

	CDgetstatus(d->daux, &s);
	if( s.state==CD_NODISC || s.state==CD_ERROR )
		return 1;

	return (0);
}

void
keep_cd_open() { }

/*
 * Send a SCSI command out the bus.
 */
int 
wm_scsi(d, xcdb, cdblen, retbuf, retbuflen, getreply)
	struct wm_drive *d;
	unsigned char *xcdb;
	int cdblen;
	int getreply;
	char *retbuf;
	int retbuflen;
{
	return -1;
}

#endif
