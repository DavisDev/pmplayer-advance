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
 
#ifndef __FLV1_FILE_H__
#define __FLV1_FILE_H__

#include <stdio.h>
#include "flv1info.h"

struct flv1_file_struct {
	flv1info_t *info;
	
	unsigned int video_type;
	unsigned int video_width;
	unsigned int video_height;
	unsigned int video_rate;
	unsigned int video_scale;
	unsigned int number_of_video_frames;
	
	unsigned int audio_type;
	unsigned int audio_actual_rate;
	unsigned int audio_rate;
	unsigned int audio_resample_scale;
	unsigned int audio_scale;
	unsigned int audio_stereo;
	
	int audio_up_sample;
	
	int video_audio_interval;
	
};

void flv1_file_safe_constructor(struct flv1_file_struct *p);
void flv1_file_close(struct flv1_file_struct *p);
char *flv1_file_open(struct flv1_file_struct *p, char *s);

#endif

