/* 
 *	Copyright (C) 2008 cooleyes
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
 
#ifndef __MP4_FILE_H__
#define __MP4_FILE_H__

#include <stdio.h>
#include <string.h>
#include "mp4info.h"
#include "common/mem64.h"

struct mp4_sample_struct {
	unsigned int sample_index;
	unsigned int sample_size;
};

struct mp4_index_struct {
	unsigned int timestamp;
	unsigned int sample_index;
	unsigned int offset;
};

struct mp4_file_struct {
	mp4info_t *info;
	int video_track_id;
	int audio_tracks;
	int audio_track_ids[6];
	int subtitle_tracks;
	int subtitle_track_ids[4];
	
	int subtitle_track_time_count[4];
	unsigned int *subtitle_track_time[4];
	
	unsigned int video_type;
	unsigned int video_width;
	unsigned int video_height;
	unsigned int number_of_video_frames;
	unsigned int video_rate;
	unsigned int video_scale;
	
	unsigned int maximum_video_sample_size;
	
	unsigned int avc_profile;
	unsigned int avc_sps_size;
	unsigned char* avc_sps;
	unsigned int avc_pps_size;
	unsigned char* avc_pps;
	unsigned int avc_nal_prefix_size;
	
	unsigned int mp4v_decinfo_size;
	unsigned char* mp4v_decinfo;
	
	unsigned int audio_type;
	unsigned int audio_actual_rate;
	unsigned int audio_rate;
	unsigned int audio_resample_scale;
	unsigned int audio_scale;
	unsigned int audio_stereo;
	
	int audio_up_sample;
	
	int video_audio_interval;
	
	int seek_duration;
	
	unsigned int sample_count;
	struct mp4_sample_struct* samples;
	unsigned int index_count;
	struct mp4_index_struct* indexes;
	
	int is_not_interlace;
	unsigned int first_video_offset;
	unsigned int first_video_sample;
	unsigned int first_audio_offset;
	unsigned int first_audio_sample;
	
};

void mp4_file_safe_constructor(struct mp4_file_struct *p);
void mp4_file_close(struct mp4_file_struct *p);
char *mp4_file_open(struct mp4_file_struct *p, char *s);

#endif

