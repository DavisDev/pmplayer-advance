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
 
#include "mkv_file.h"

void mkv_file_safe_constructor(struct mkv_file_struct *p) {
	p->info = 0;
	p->video_track_id = -1;
	p->audio_tracks = 0;
	p->audio_up_sample = 0;
}


void mkv_file_close(struct mkv_file_struct *p) {
	if (p->info != 0) {
		mkvinfo_close(p->info);
	}
	mkv_file_safe_constructor(p);
}
	

char *mkv_file_open(struct mkv_file_struct *p, char *s) {
	mkv_file_safe_constructor(p);
	
	p->info = mkvinfo_open(s);
	
	if (p->info == 0){
		mkv_file_close(p);
		return("mkv_file_open: can't open file");
	}
	
	if (p->info->total_indexes==0) {
		mkv_file_close(p);
		return("mkv_file_open: can't find any seek index");
	}
	
	int i;
	
	p->seek_duration = 0;
	
	for(i = 0; i < p->info->total_indexes-1; i++) {
		int64_t seek_duration = p->info->indexes[i+1].timecode - p->info->indexes[i].timecode;
		if ( (int)seek_duration > p->seek_duration )
			p->seek_duration = (int)seek_duration;
	}	
	
	for(i = 0; i < p->info->total_tracks; i++) {
		mkvinfo_track_t* track = p->info->tracks[i];
		if (track->type != MATROSKA_TRACK_VIDEO)
			continue;
		
		if ( track->width < 1 || track->height < 1 )
			continue;
		if ( track->width > 720 || track->height > 480 ) 
			continue;
		if ( track->video_type == 0x61766331 /*avc1*/) {
			if ( track->private_size < 2 )
				continue;
			uint8_t avc_profile = track->private_data[1];
			if ( avc_profile==0x42 && (track->width > 480 || track->height > 272) ) 
				continue;
		}
		else {
			if ( track->width > 480 || track->height > 272 ) 
				continue;
		}
		p->video_track_id = i;
		p->video_type = track->video_type;
		break;
	}
	if ( p->video_track_id < 0 ) {
		mkv_file_close(p);
		return("mkv_file_open: can't found video track in mkv file");
	} 
	
	for(i = 0; i < p->info->total_tracks; i++) {
		mkvinfo_track_t* track = p->info->tracks[i];
		if (track->type != MATROSKA_TRACK_AUDIO)
			continue;
		if ( p->audio_tracks == 0 ) {
			if ( track->audio_type != 0x6D703461 /*mp4a*/)
				continue;
//			if ( track->channels != 2 )
//				continue;
			if ( track->samplerate != 22050 && track->samplerate != 24000 && track->samplerate != 44100 && track->samplerate != 48000 )
				continue;
//			if ( track->samplebits != 16 )
//				continue;
			p->audio_tracks++;
			p->audio_track_ids[p->audio_tracks-1] = i;
			p->audio_type = track->audio_type;
			if ( track->samplerate == 22050 || track->samplerate == 24000 )
				p->audio_up_sample = 1;
		}
		else {
			mkvinfo_track_t* old_track = p->info->tracks[p->audio_track_ids[p->audio_tracks-1]];
			if ( old_track->audio_type != track->audio_type )
				continue;
//			if ( old_track->channels != track->channels )
//				continue;
			if ( old_track->samplerate != track->samplerate )
				continue;
//			if ( old_track->samplebits != track->samplebits )
//				continue;
			p->audio_tracks++;
			p->audio_track_ids[p->audio_tracks-1] = i;
		}
		if ( p->audio_tracks == 6 )
			break;
	}
	if ( p->audio_tracks == 0 ) {
		mkv_file_close(p);
		return("mkv_file_open: can't found audio track in mkv file");
	}
	
	for(i = 0; i < p->info->total_tracks; i++) {
		mkvinfo_track_t* track = p->info->tracks[i];
		if (track->type != MATROSKA_TRACK_SUBTITLE)
			continue;
		if ( (track->video_type != 0x74787475) && (track->video_type != 0x7478746C) 
			&& (track->video_type != 0x73736175) && (track->video_type != 0x61737375) /*txtu & txtl & ssau & assu*/)
			continue;
		p->subtitle_tracks++;
		p->subtitle_track_ids[p->subtitle_tracks-1] = i;
		p->subtitle_track_types[p->subtitle_tracks-1] = track->video_type;
		if ( p->subtitle_tracks == 4 )
			break;
	}
	
	p->video_width = p->info->tracks[p->video_track_id]->width;
	p->video_height = p->info->tracks[p->video_track_id]->height;
	p->display_width = p->info->tracks[p->video_track_id]->display_width;
	p->display_height = p->info->tracks[p->video_track_id]->display_height;
	
	p->video_rate = p->info->tracks[p->video_track_id]->time_scale;
	p->video_scale = p->info->tracks[p->video_track_id]->duration;
	//p->video_scale = p->info->tracks[p->video_track_id]->stts_sample_duration[0];
	double tmp ;
	tmp = 1.0f*p->info->duration;
	tmp *= p->info->tracks[p->video_track_id]->time_scale;
	tmp /= p->info->tracks[p->video_track_id]->duration;
	tmp /= 1000.0f;
	p->number_of_video_frames = (unsigned int)tmp;
	
	p->audio_actual_rate = p->info->tracks[p->audio_track_ids[0]]->samplerate;
	p->audio_rate = p->audio_actual_rate * (p->audio_up_sample+1) ;
	p->audio_scale = (p->audio_type == 0x6D703461 ? 1024 : 1024);
	p->audio_resample_scale = p->audio_scale * (p->audio_up_sample+1);
	
	p->audio_stereo = 1;
	
	return(0);
}

