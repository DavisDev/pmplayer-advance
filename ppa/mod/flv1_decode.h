/* 
 *	Copyright (C) 2010 cooleyes
 *	eyes.cooleyes@gmail.com 
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#ifndef __FLV1_DECODE_H__
#define __FLV1_DECODE_H__

#include <pspkernel.h>
#include <pspdisplay.h>
#include "flv1_read.h"
#include "audiodecoder.h"
#include "common/mem64.h"
#include "common/m33sdk.h"
#include "aspect_ratio.h"
#include "gu_draw.h"
#include "movie_interface.h"
#include "mp4avcdecoder.h"

#ifdef USE_FFMPEG_FLV1_DECODER
#include "flv1decoder.h"
#endif
#ifdef USE_FLV2MPEG4_CONV
#include "mp4vdecoder.h"
#include "flv2mpeg4.h"
#endif

#include "cpu_clock.h"

#include "gu_font.h"

#define flv1_maximum_frame_buffers 64
#define flv1_number_of_free_video_frame_buffers 6
#define flv1_timestamp_queue_max 16


struct flv1_decode_buffer_struct {
	void *data;
	int timestamp;
};


struct flv1_decode_struct {
	struct flv1_read_struct reader;

	struct mp4_avc_struct avc;
#ifdef USE_FFMPEG_FLV1_DECODER
	struct flv1_struct flv1;
#else
#ifdef USE_FLV2MPEG4_CONV
	int flv_mpeg4_conv_opened;
	struct mp4v_struct mp4v;
#endif
#endif
	
	unsigned int video_format;

	int audio_decoder;


	void *video_frame_buffers[flv1_maximum_frame_buffers];
	void *audio_frame_buffers[flv1_maximum_frame_buffers];
	
	unsigned int video_frame_size;
	unsigned int audio_frame_size;
	unsigned int number_of_frame_buffers;


	struct flv1_decode_buffer_struct output_audio_frame_buffers[flv1_maximum_frame_buffers];
	struct flv1_decode_buffer_struct output_video_frame_buffers[flv1_maximum_frame_buffers];

	unsigned int current_audio_buffer_number;
	unsigned int current_video_buffer_number;
	
	int output_texture_width;
	int video_frame_duration;
	int audio_frame_duration;
	
	int timestamp_queue[flv1_timestamp_queue_max];
	unsigned int timestamp_queue_size;
	
	int last_audio_timestamp;
	int last_video_timestamp;
	
	int is_eof;
	
};


void  flv1_decode_safe_constructor(struct flv1_decode_struct *p);
char *flv1_decode_open(struct flv1_decode_struct *p, char *s, int pspType, int tvAspectRatio, int tvWidth, int tvHeight, int videoMode);
void  flv1_decode_close(struct flv1_decode_struct *p, int pspType);
int   flv1_decode_is_eof(struct flv1_decode_struct *p);
void  flv1_decode_reset(struct flv1_decode_struct *p);
char *flv1_decode_seek(struct flv1_decode_struct *p, int timestamp, int last_timestamp);
char *flv1_decode_get_video(struct flv1_decode_struct *p, unsigned int audio_stream, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop, int* pic_num );
char *flv1_decode_get_cached_video(struct flv1_decode_struct *p, unsigned int pic_num, unsigned int audio_stream, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop);
char *flv1_decode_get_audio(struct flv1_decode_struct *p, unsigned int audio_stream, int audio_channel, int decode_audio, unsigned int volume_boost);
char *flv1_decode_keyframe_forward(struct flv1_decode_struct *p, int keyframes);
char *flv1_decode_keyframe_backward(struct flv1_decode_struct *p, int keyframes);

#endif
