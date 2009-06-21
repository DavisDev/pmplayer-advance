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

#ifndef __MP4_READ_H__
#define __MP4_READ_H__


#include <string.h>
#include <pspiofilemgr.h>
#include "mp4_file.h"
#include "common/mem64.h"
#include "common/buffered_reader.h"

#define MP4_VIDEO_QUEUE_MAX 64
#define MP4_AUDIO_QUEUE_MAX 64

struct mp4_read_output_struct {
	unsigned int  size;
	void* data;
	int timestamp;
};

struct mp4_read_struct {
	struct mp4_file_struct file;
	
	buffered_reader_t* reader;
	
	int current_audio_track;
	
	unsigned int current_sample;
	
	struct mp4_read_output_struct video_queue[MP4_VIDEO_QUEUE_MAX];
	struct mp4_read_output_struct audio_queue[MP4_AUDIO_QUEUE_MAX];
	unsigned int video_queue_front, video_queue_rear, video_queue_size;
	unsigned int audio_queue_front, audio_queue_rear, audio_queue_size;
};


void mp4_read_safe_constructor(struct mp4_read_struct *p);
void mp4_read_close(struct mp4_read_struct *p);
char *mp4_read_open(struct mp4_read_struct *p, char *s);
char *mp4_read_seek(struct mp4_read_struct *p, int timestamp, int last_timestamp);
char *mp4_read_get_video(struct mp4_read_struct *p, struct mp4_read_output_struct *output);
char *mp4_read_get_audio(struct mp4_read_struct *p, unsigned int audio_stream, struct mp4_read_output_struct *output);

#endif
