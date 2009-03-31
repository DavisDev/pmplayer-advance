/* 
 *	Copyright (C) 2009 cooleyes
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
#ifndef __MKV_DECODE_H__
#define __MKV_DECODE_H__

#include <pspkernel.h>
#include <pspdisplay.h>
#include "mkv_read.h"
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

#define mkv_maximum_frame_buffers 64
#define mkv_number_of_free_video_frame_buffers 6
#define mkv_timestamp_queue_max 16


struct mkv_decode_buffer_struct {
	void *data;
	int timestamp;
};


struct mkv_decode_struct {
	struct mkv_read_struct reader;

	struct mp4_avc_struct avc;
	struct mp4v_struct mp4v;
	
	unsigned int video_format;

	int audio_decoder;


	void *video_frame_buffers[mkv_maximum_frame_buffers];
	void *audio_frame_buffers[mkv_maximum_frame_buffers];
	
	unsigned int video_frame_size;
	unsigned int audio_frame_size;
	unsigned int number_of_frame_buffers;


	struct mkv_decode_buffer_struct output_audio_frame_buffers[mkv_maximum_frame_buffers];
	struct mkv_decode_buffer_struct output_video_frame_buffers[mkv_maximum_frame_buffers];

	unsigned int current_audio_buffer_number;
	unsigned int current_video_buffer_number;
	
	int output_texture_width;
	int video_frame_duration;
	int audio_frame_duration;
	
	int timestamp_queue[mkv_timestamp_queue_max];
	unsigned int timestamp_queue_size;
	
	int last_audio_timestamp;
	int last_video_timestamp;
	
	int is_eof;
	
};


void  mkv_decode_safe_constructor(struct mkv_decode_struct *p);
char *mkv_decode_open(struct mkv_decode_struct *p, char *s, int pspType, int tvAspectRatio, int tvWidth, int tvHeight, int videoMode);
void  mkv_decode_close(struct mkv_decode_struct *p, int pspType);
int   mkv_decode_is_eof(struct mkv_decode_struct *p);
void  mkv_decode_reset(struct mkv_decode_struct *p);
char *mkv_decode_seek(struct mkv_decode_struct *p, int timestamp);
char *mkv_decode_get_video(struct mkv_decode_struct *p, unsigned int audio_stream, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop, int* pic_num );
char *mkv_decode_get_cached_video(struct mkv_decode_struct *p, unsigned int pic_num, unsigned int audio_stream, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop);
char *mkv_decode_get_audio(struct mkv_decode_struct *p, unsigned int audio_stream, int audio_channel, int decode_audio, unsigned int volume_boost);

#endif
