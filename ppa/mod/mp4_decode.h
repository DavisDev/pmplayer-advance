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

#define mp4_maximum_frame_buffers 64
#define mp4_number_of_free_video_frame_buffers 6


struct mp4_decode_buffer_struct {
	void *data;
	int timestamp;
};


struct mp4_decode_struct {
	struct mp4_read_struct reader;

	struct mp4_avc_struct avc;
	struct mp4v_struct mp4v;
	
	unsigned int video_format;

	int audio_decoder;


	void *video_frame_buffers[mp4_maximum_frame_buffers];
	void *audio_frame_buffers[mp4_maximum_frame_buffers];
	
	unsigned int video_frame_size;
	unsigned int audio_frame_size;
	unsigned int number_of_frame_buffers;


	struct mp4_decode_buffer_struct output_audio_frame_buffers[mp4_maximum_frame_buffers];
	struct mp4_decode_buffer_struct output_video_frame_buffers[mp4_maximum_frame_buffers];

	unsigned int current_audio_buffer_number;
	unsigned int current_video_buffer_number;
	
	int output_texture_width;
	int video_frame_duration;
	int audio_frame_duration;
	
};


void mp4_decode_safe_constructor(struct mp4_decode_struct *p);
char *mp4_decode_open(struct mp4_decode_struct *p, char *s, int pspType, int tvAspectRatio, int tvWidth, int tvHeight, int videoMode);
void mp4_decode_close(struct mp4_decode_struct *p, int pspType);
char *mp4_decode_get_video(struct mp4_decode_struct *p, unsigned int read_num, unsigned int frame_number, unsigned int audio_stream, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop, int* pic_num );
char *mp4_decode_get_cached_video(struct mp4_decode_struct *p, unsigned int pic_num, unsigned int frame_number, unsigned int audio_stream, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop);
char *mp4_decode_get_audio(struct mp4_decode_struct *p, unsigned int frame_number, unsigned int audio_stream, int audio_channel, int decode_audio, unsigned int volume_boost);

#endif
