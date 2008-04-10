#ifndef __MP4_PLAY_H__
#define __MP4_PLAY_H__


#include <pspthreadman.h>
#include <pspaudio.h>
#ifdef DEVHOOK
#include "cooleyesAudio.h"
#else
#include <pspaudio_kernel.h>
#endif
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psppower.h>
#include "mp4_decode.h"
#include "aspect_ratio.h"
#include "gu_draw.h"
#include "subtitle_parse.h"


struct mp4_play_struct {
	struct mp4_decode_struct decoder;

	int audio_reserved;

	SceUID semaphore_can_get;
	SceUID semaphore_can_put;
	SceUID semaphore_can_show;
	SceUID semaphore_show_done;

	SceUID output_thread;
	SceUID show_thread;

	int   return_request;
	char *return_result;

	int paused;
	int seek;

	unsigned int audio_stream;
	int audio_channel;
	unsigned int volume_boost;
	unsigned int aspect_ratio;
	unsigned int zoom;
	unsigned int luminosity_boost;
	unsigned int show_interface;
	unsigned int last_keyframe_pos;
	unsigned int resume_pos;
	char resume_filename[256];
	unsigned int subtitle_count;
	unsigned int subtitle;
	unsigned int subtitle_format;
	unsigned int subtitle_fontcolor;
	unsigned int subtitle_bordercolor;
	unsigned int loop;
};


#ifndef NUMBER_OF_FONTCOLORS
#define NUMBER_OF_FONTCOLORS 6
#endif
#ifndef NUMBER_OF_BORDERCOLORS
#define NUMBER_OF_BORDERCOLORS 6
#endif

#include "mp4_stat.h"
void mp4_play_safe_constructor(struct mp4_play_struct *p);
char *mp4_play_open(struct mp4_play_struct *p, char *s, int usePos, int pspType, int tvAspectRatio, int tvWidth, int tvHeight, int videoMode);
void mp4_play_close(struct mp4_play_struct *p, int usePos, int pspType);
char *mp4_play_start(volatile struct mp4_play_struct *p);


#endif
