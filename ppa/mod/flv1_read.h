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

#ifndef __FLV1_READ_H__
#define __FLV1_READ_H__


#include <string.h>
#include <pspiofilemgr.h>
#include "flv1_file.h"
#include "common/mem64.h"
#include "common/buffered_reader.h"

#define FLV1_VIDEO_QUEUE_MAX 64
#define FLV1_AUDIO_QUEUE_MAX 64

struct flv1_read_output_struct {
	unsigned int  size;
	void* data;
	int timestamp;
};

struct flv1_read_struct {
	struct flv1_file_struct file;
	
	buffered_reader_t* reader;
	
	uint64_t current_timecode;
	
	struct flv1_read_output_struct video_queue[FLV1_VIDEO_QUEUE_MAX];
	struct flv1_read_output_struct audio_queue[FLV1_AUDIO_QUEUE_MAX];
	unsigned int video_queue_front, video_queue_rear, video_queue_size;
	unsigned int audio_queue_front, audio_queue_rear, audio_queue_size;
};


void flv1_read_safe_constructor(struct flv1_read_struct *p);
void flv1_read_close(struct flv1_read_struct *p);
char *flv1_read_open(struct flv1_read_struct *p, char *s);
char *flv1_read_seek(struct flv1_read_struct *p, int timestamp, int last_timestamp);
char *flv1_read_get_video(struct flv1_read_struct *p, struct flv1_read_output_struct *output);
char *flv1_read_get_audio(struct flv1_read_struct *p, unsigned int audio_stream, struct flv1_read_output_struct *output);
char *flv1_read_keyframe_forward(struct flv1_read_struct *p, int keyframes);
char *flv1_read_keyframe_backward(struct flv1_read_struct *p, int keyframes); 

#endif
