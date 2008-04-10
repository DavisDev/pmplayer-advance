#ifndef __MP4_DECODE_H__
#define __MP4_DECODE_H__

#include <pspkernel.h>
#include <pspdisplay.h>
#include "mp4_read.h"
#include "audiodecoder.h"
#include "common/mem64.h"
#include "common/m33sdk.h"
#include "aspect_ratio.h"
#include "gu_draw.h"
#include "movie_interface.h"
#include "mp4avcdecoder.h"
#include "mp4vdecoder.h"
#include "cpu_clock.h"

#include "subtitle_parse.h"
#include "gu_font.h"

#define maximum_frame_buffers 64
#define number_of_free_video_frame_buffers 8


struct mp4_decode_buffer_struct {
	void *video_frame;
	void *audio_frame;

	unsigned int number_of_audio_frames;

	int first_delay;
	int last_delay;
};


struct mp4_decode_struct {
	struct mp4_read_struct reader;

	struct mp4_avc_struct avc;
	struct mp4v_struct mp4v;
	
	unsigned int video_format;

	int audio_decoder;


	void *video_frame_buffers[maximum_frame_buffers];
	void *audio_frame_buffers[maximum_frame_buffers];

	unsigned int audio_frame_size;
	unsigned int number_of_frame_buffers;


	struct mp4_decode_buffer_struct output_frame_buffers[maximum_frame_buffers];

	unsigned int current_buffer_number;
	
	int output_texture_width;
};


void mp4_decode_safe_constructor(struct mp4_decode_struct *p);
char *mp4_decode_open(struct mp4_decode_struct *p, char *s, int pspType, int tvAspectRatio, int tvWidth, int tvHeight, int videoMode);
void mp4_decode_close(struct mp4_decode_struct *p, int pspType);
char *mp4_decode_get(struct mp4_decode_struct *p, unsigned int frame_number, unsigned int audio_stream, int audio_channel, int decode_audio, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop);

#endif
