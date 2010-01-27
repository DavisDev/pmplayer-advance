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
 
#include "flv1_file.h"

void flv1_file_safe_constructor(struct flv1_file_struct *p) {
	p->info = 0;
	p->audio_up_sample = 0;
}


void flv1_file_close(struct flv1_file_struct *p) {
	if (p->info != 0) {
		flv1info_close(p->info);
	}
	flv1_file_safe_constructor(p);
}
	

char *flv1_file_open(struct flv1_file_struct *p, char *s) {
	flv1_file_safe_constructor(p);
	
	p->info = flv1info_open(s, 1);
	
	if (p->info == 0){
		flv1_file_close(p);
		return("flv1_file_open: can't open file");
	}
	
	if (p->info->total_indexes==0) {
		flv1_file_close(p);
		return("flv1_file_open: can't find any seek index");
	}
	
	int video_ok = 0;
	if ( p->info->video_type == 0x666C7631 /*flv1*/) {
		if ( p->info->width >= 1 && p->info->width <= 480 && p->info->height >= 1 && p->info->height <= 480 ) {
			p->video_type = p->info->video_type;
			video_ok = 1;
		}
	}
	else if ( p->info->video_type == 0x61766331 /*avc1*/ ) {
		if ( p->info->avc_profile==0x42 ) {
			if ( p->info->width >= 1 && p->info->width <= 480 && p->info->height >= 1 && p->info->height <= 272 ) {
				p->video_type = p->info->video_type;
				video_ok = 1;
			} 
		}
		else {
			if ( p->info->width >= 1 && p->info->width <= 720 && p->info->height >= 1 && p->info->height <= 480 ) {
				p->video_type = p->info->video_type;
				video_ok = 1;
			} 
		}
	}
	if ( !video_ok ) {
		flv1_file_close(p);
		return("flv1_file_open: can't found video track in flv1 file");
	}
	
	int audio_ok = 0;
	if ( p->info->audio_type == 0x6D703320 || p->info->audio_type == 0x6D703461) {
		if ( p->info->samplebits == 0x10 ) {
			if ( p->info->samplerate == 22050 || p->info->samplerate == 24000 || p->info->samplerate == 44100 || p->info->samplerate == 48000 ) {
				p->audio_type = p->info->audio_type;
				if ( p->info->samplerate == 22050 || p->info->samplerate == 24000 )
					p->audio_up_sample = 1;
				audio_ok = 1;
			}
		}
	}
	if ( !audio_ok ) {
		flv1_file_close(p);
		return("flv1_file_open: can't found audio track in flv1 file");
	}
	
	p->video_width = p->info->width;
	p->video_height = p->info->height;
		
	p->video_rate = p->info->video_scale;
	p->video_scale = p->info->video_frame_duration;
	
	double tmp ;
	tmp = 1000.0f*p->info->duration;
	tmp /= p->info->video_frame_duration;
	p->number_of_video_frames = (unsigned int)tmp;
	
	p->audio_actual_rate = p->info->samplerate;
	p->audio_rate = p->audio_actual_rate * (p->audio_up_sample+1) ;
	p->audio_scale = (p->info->samples_per_frame != 0 ? p->info->samples_per_frame : (p->audio_type == 0x6D703320 ? 1152 : 1024));
	p->audio_resample_scale = p->audio_scale * (p->audio_up_sample+1);
	
	p->audio_stereo = 1;
	
	int64_t v0 = p->video_rate * p->audio_resample_scale;
	int64_t v1 = p->audio_rate * p->video_scale;
	
	if (v0 >= v1)
		p->video_audio_interval = (int)(v0 / v1);
	else
		p->video_audio_interval = (int)(-(v1/v0));
	
	return(0);
}

