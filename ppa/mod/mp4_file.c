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
 
#include "mp4_file.h"

void mp4_file_safe_constructor(struct mp4_file_struct *p) {
	p->info = 0;
	p->video_track_id = -1;
	p->audio_tracks = 0;
	p->audio_up_sample = 0;
	
	p->subtitle_tracks = 0;
	int i;
	for(i=0; i < 4; i++) {
		p->subtitle_track_time[i] = 0;
	}
	
	p->maximum_video_sample_size = 0;
	
	p->avc_sps_size = 0;
	p->avc_sps = 0;
	p->avc_pps_size = 0;
	p->avc_pps = 0;
	
	p->sample_count = 0;
	p->samples = 0;
	p->index_count = 0;
	p->indexes = 0;
	
	p->is_not_interlace = 0;
	p->first_video_offset = 0xFFFFFFFF;
	p->first_video_sample = 0;
	p->first_audio_offset = 0xFFFFFFFF;
	p->first_audio_sample = 0;
	
	p->seek_duration = 5000;
	
}


void mp4_file_close(struct mp4_file_struct *p) {
	if (p->info != 0) {
		mp4info_close(p->info);
	}
	
	int i;
	for(i=0; i < 4; i++) {
		if ( p->subtitle_track_time[i] )
			free_64(p->subtitle_track_time[i]);
	}
	
	if ( p->avc_sps )
		free_64(p->avc_sps);
	if ( p->avc_pps )
		free_64(p->avc_pps);
	if ( p->samples )
		free_64(p->samples);
	if ( p->indexes )
		free_64(p->indexes);
	mp4_file_safe_constructor(p);
}

unsigned int mp4_get_trunk_offset(mp4info_track_t* track, unsigned int trunk) {
	if ( trunk >= track->stco_entry_count )
		return 0xFFFFFFFF;
	else
		return track->stco_chunk_offset[trunk];
}

unsigned int mp4_get_sample_size(mp4info_track_t* track, unsigned int sample) {
	return (track->stsz_sample_size ? track->stsz_sample_size : track->stsz_sample_size_table[sample]);
}

unsigned int mp4_get_sample_count(mp4info_track_t* track) {
	unsigned int i;
	unsigned int sample_count = 0;
	for( i = 0; i < track->stts_entry_count; i++)
		sample_count += track->stts_sample_count[i];
		
	return sample_count;
}

int mp4_is_keyframe(mp4info_track_t* track, unsigned int sample) {
	unsigned int i;
	for(i = 0; i < track->stss_entry_count; i++ ) {
		if ( sample+1 == track->stss_sync_sample[i] )
			return 1;
	}
	return 0;
}

char *mp4_file_build_index(struct mp4_file_struct *p) {
	
	int i,j;
	unsigned int ui;
	unsigned int trunk[MP4_MAX_TRACKS];
	unsigned int current_sample = 0;
	unsigned int current_index = 0;
	
	
	mp4info_track_t* v_track = p->info->tracks[p->video_track_id];
	mp4info_track_t* a_track = p->info->tracks[p->audio_track_ids[0]];
	
	if ( (v_track->stco_chunk_offset[v_track->stco_entry_count-1] < a_track->stco_chunk_offset[0]) 
		|| (v_track->stco_chunk_offset[0] > a_track->stco_chunk_offset[a_track->stco_entry_count-1]) ) {
		
		if ( p->info->total_tracks > 2 ) {
			mp4_file_close(p);
			return("mp4_file_open: can't support this file");
		}
		else
			p->is_not_interlace = 1;
	}
	
	
	for(i=0; i<p->info->total_tracks; i++) {
		p->sample_count += mp4_get_sample_count(p->info->tracks[i]);
	}
	
	p->index_count = p->info->tracks[p->video_track_id]->stss_entry_count;
	
	p->samples = malloc_64(p->sample_count * sizeof(struct mp4_sample_struct));
	if ( !p->samples ) {
		mp4_file_close(p);
		return("mp4_file_open: can't malloc samples buffer");
	}
	
	p->indexes = malloc_64(p->index_count * sizeof(struct mp4_index_struct));
	if ( !p->indexes ) {
		mp4_file_close(p);
		return("mp4_file_open: can't malloc indexes buffer");
	}
	
	for(i=0; i<MP4_MAX_TRACKS; i++)
		trunk[i] = 0;
	
	while(1) {
		int current_track = 0;
		unsigned int min_offset = 0xFFFFFFFF;
		unsigned int current_offset = 0xFFFFFFFF;
		unsigned int first_sample = 0;
		unsigned int last_sample = 0;
		for(i=0; i<p->info->total_tracks; i++) {
			current_offset = mp4_get_trunk_offset(p->info->tracks[i], trunk[i]);
			if ( current_offset < min_offset ) {
				min_offset = current_offset;
				current_track = i;
			}
		}
		
		if ( 0xFFFFFFFF == min_offset ) 
			break;
		
		mp4info_track_t* track = p->info->tracks[current_track];
		for( i = 0; i < track->stsc_entry_count-1; i++ ) {
			if ( (trunk[current_track]+1) >= track->stsc_first_chunk[i] && (trunk[current_track]+1) < track->stsc_first_chunk[i+1] )
				break;
		}
		for( j = 0; j < i; j++ ) {
			first_sample += ( ( track->stsc_first_chunk[j+1] - track->stsc_first_chunk[j] ) * track->stsc_samples_per_chunk[j] );
		}
		first_sample += ( ( (trunk[current_track]+1) - track->stsc_first_chunk[i] ) * track->stsc_samples_per_chunk[i] );
		last_sample = first_sample + track->stsc_samples_per_chunk[i] - 1;
		
		for(ui = first_sample; ui <= last_sample; ui++) {
			p->samples[current_sample].sample_index = (current_track << 24) | (ui & 0x00FFFFFF);
			p->samples[current_sample].sample_size = mp4_get_sample_size(track, ui);
			
			if ( current_track == p->video_track_id ) {
				if ( 0xFFFFFFFF == p->first_video_offset) {
					p->first_video_offset = min_offset;
					p->first_video_sample = current_sample;
				}
			 	if ( p->samples[current_sample].sample_size > p->maximum_video_sample_size) {
					p->maximum_video_sample_size = p->samples[current_sample].sample_size;
				}
			
				if ( mp4_is_keyframe(track, ui) ) {
					uint64_t timestamp = 1000LL;
					timestamp *= ui;
					timestamp *= p->video_scale;
					timestamp /= p->video_rate;
					p->indexes[current_index].timestamp = timestamp;
					p->indexes[current_index].sample_index = current_sample;
					p->indexes[current_index].offset = min_offset;
					current_index++;
				}
			}
			else if ( current_track == p->audio_track_ids[0] ) {
				if ( 0xFFFFFFFF == p->first_audio_offset) {
					p->first_audio_offset = min_offset;
					p->first_audio_sample = current_sample;
				}
			}
			
			min_offset += p->samples[current_sample].sample_size;
			current_sample++;
		}
		
		trunk[current_track] += 1;
		
	}
	
	if ( !p->is_not_interlace ) {
		for(i = 0; i < p->info->total_tracks; i++) {
			mp4info_track_t* track = p->info->tracks[i];
			if (track->type != MP4_TRACK_SUBTITLE)
				continue;
			p->subtitle_tracks++;
			p->subtitle_track_ids[p->subtitle_tracks-1] = i;
			
			p->subtitle_track_time_count[p->subtitle_tracks-1] = track->stts_entry_count;
			p->subtitle_track_time[p->subtitle_tracks-1] = malloc_64(track->stts_entry_count*2*sizeof(unsigned int));
			
			if ( !p->subtitle_track_time[p->subtitle_tracks-1] ) {
				p->subtitle_tracks--;
				break;
			}
			else {
				for(j=0; j<track->stts_entry_count;j++) {
					p->subtitle_track_time[p->subtitle_tracks-1][2*j] = track->stts_sample_count[j];
					p->subtitle_track_time[p->subtitle_tracks-1][2*j+1] = track->stts_sample_duration[j];
				}
			}
			
			if ( p->subtitle_tracks == 4 )
				break;
		}
	}
	
	mp4info_close(p->info);
	p->info = 0;
	return(0);
}

char *mp4_file_open(struct mp4_file_struct *p, char *s) {
	mp4_file_safe_constructor(p);
	
	p->info = mp4info_open(s);
	
	if (p->info == 0){
		mp4_file_close(p);
		return("mp4_file_open: can't open file");
	}
	
	int i;
	
	for(i = 0; i < p->info->total_tracks; i++) {
		mp4info_track_t* track = p->info->tracks[i];
		if (track->type != MP4_TRACK_VIDEO)
			continue;
		
		if ( track->width < 1 || track->height < 1 )
			continue;
		if ( track->width > 720 || track->height > 480 ) 
			continue;
		if ( track->video_type == 0x61766331 /*avc1*/) {
			if ( track->avc_profile==0x42 && (track->width > 480 || track->height > 272) ) 
				continue;
		}
		else {
			if ( track->width > 480 || track->height > 272 ) 
				continue;
		}
		p->video_track_id = i;
		p->video_type = track->video_type;
		if ( p->video_type == 0x61766331 ) {
			p->avc_profile = track->avc_profile;
			p->avc_sps_size = track->avc_sps_size;
			p->avc_pps_size = track->avc_pps_size;
			p->avc_nal_prefix_size = track->avc_nal_prefix_size;
			p->avc_sps = malloc_64(p->avc_sps_size);
			if ( ! p->avc_sps ) {
				mp4_file_close(p);
				return("mp4_file_open: can't malloc avc_sps buffer");
			}
			memcpy(p->avc_sps, track->avc_sps, p->avc_sps_size);
			p->avc_pps = malloc_64(p->avc_pps_size);
			if ( ! p->avc_pps ) {
				mp4_file_close(p);
				return("mp4_file_open: can't malloc avc_pps buffer");
			}
			memcpy(p->avc_pps, track->avc_pps, p->avc_pps_size);
		}
		else {
			p->mp4v_decinfo_size = track->mp4v_decinfo_size;
			p->mp4v_decinfo = malloc_64(p->mp4v_decinfo_size);
			if ( ! p->mp4v_decinfo ) {
				mp4_file_close(p);
				return("mp4_file_open: can't malloc mp4v_decinfo buffer");
			}
			memcpy(p->mp4v_decinfo, track->mp4v_decinfo, p->mp4v_decinfo_size);
		}
		break;
	}
	if ( p->video_track_id < 0 ) {
		mp4_file_close(p);
		return("mp4_file_open: can't found video track in mp4 file");
	}
	
	for(i = 0; i < p->info->total_tracks; i++) {
		mp4info_track_t* track = p->info->tracks[i];
		if (track->type != MP4_TRACK_AUDIO)
			continue;
		if ( p->audio_tracks == 0 ) {
			if ( track->audio_type != 0x6D703461 && track->audio_type != 0x73616D72 )
				continue;
//			if ( track->channels != 2 )
//				continue;
			if ( track->samplerate != 8000 && track->samplerate != 22050 && track->samplerate != 24000 && track->samplerate != 44100 && track->samplerate != 48000 )
				continue;
//			if ( track->samplebits != 16 )
//				continue;
			p->audio_tracks++;
			p->audio_track_ids[p->audio_tracks-1] = i;
			p->audio_type = track->audio_type;
			if ( track->samplerate == 22050 || track->samplerate == 24000 )
				p->audio_up_sample = 1;
			else if ( track->samplerate == 8000 )
				p->audio_up_sample = 5;
		}
		else {
			mp4info_track_t* old_track = p->info->tracks[p->audio_track_ids[p->audio_tracks-1]];
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
		mp4_file_close(p);
		return("mp4_file_open: can't found audio track in mp4 file");
	}
	
//	int sample_id = 0;
//	unsigned int trunk_size = 0;
//	int j, k;
//	
//	mp4info_track_t* video_track = p->info->tracks[p->video_track_id];
//	for( i = 0; i < video_track->stsc_entry_count-1; i++ ) {
//		int trunk_num = video_track->stsc_first_chunk[i+1] - video_track->stsc_first_chunk[i];
//		for( j = 0; j < trunk_num; j++ ) {
//			trunk_size = 0;
//			for( k = 0; k < video_track->stsc_samples_per_chunk[i]; k++, sample_id++) {
//				unsigned int sample_size = (video_track->stsz_sample_size ? video_track->stsz_sample_size : video_track->stsz_sample_size_table[sample_id]);
//				if ( sample_size > p->maximum_video_sample_size )
//					p->maximum_video_sample_size = sample_size;
//				trunk_size += sample_size;
//			}
//			if ( trunk_size > p->maximum_video_trunk_size )
//				p->maximum_video_trunk_size = trunk_size;
//		}
//	}
//	trunk_size = 0;
//	for( k = 0; k < video_track->stsc_samples_per_chunk[i]; k++, sample_id++)
//		trunk_size += (video_track->stsz_sample_size ? video_track->stsz_sample_size : video_track->stsz_sample_size_table[sample_id]);	
//	if ( trunk_size > p->maximum_video_trunk_size )
//		p->maximum_video_trunk_size = trunk_size;
//		
//	int l;
//	for( l = 0; l < p->audio_tracks; l++ ) {
//		sample_id = 0;
//		mp4info_track_t* audio_track = p->info->tracks[p->audio_track_ids[l]];
//		for( i = 0; i < audio_track->stsc_entry_count-1; i++ ) {
//			int trunk_num = audio_track->stsc_first_chunk[i+1] - audio_track->stsc_first_chunk[i];
//			for( j = 0; j < trunk_num; j++ ) {
//				trunk_size = 0;
//				for( k = 0; k < audio_track->stsc_samples_per_chunk[i]; k++, sample_id++) {
//					unsigned int sample_size = (audio_track->stsz_sample_size ? audio_track->stsz_sample_size : audio_track->stsz_sample_size_table[sample_id]);
//					if ( sample_size > p->maximum_audio_sample_size )
//						p->maximum_audio_sample_size = sample_size;
//					trunk_size += sample_size;
//				}
//				if ( trunk_size > p->maximum_audio_trunk_size )
//					p->maximum_audio_trunk_size = trunk_size;
//			}
//		}
//		trunk_size = 0;
//		for( k = 0; k < audio_track->stsc_samples_per_chunk[i]; k++, sample_id++) {
//			unsigned int sample_size = (audio_track->stsz_sample_size ? audio_track->stsz_sample_size : audio_track->stsz_sample_size_table[sample_id]);
//			if ( sample_size > p->maximum_audio_sample_size )
//				p->maximum_audio_sample_size = sample_size;
//			trunk_size += sample_size;
//		}
//		if ( trunk_size > p->maximum_audio_trunk_size )
//			p->maximum_audio_trunk_size = trunk_size;
//	}
	
	p->video_width = p->info->tracks[p->video_track_id]->width;
	p->video_height = p->info->tracks[p->video_track_id]->height;
	p->number_of_video_frames = 0;
	for( i = 0; i < p->info->tracks[p->video_track_id]->stts_entry_count; i++)
		p->number_of_video_frames += p->info->tracks[p->video_track_id]->stts_sample_count[i];
	//p->number_of_video_frames = p->info->tracks[p->video_track_id]->stts_sample_count[0];
	p->video_rate = p->info->tracks[p->video_track_id]->time_scale;
	p->video_scale = p->info->tracks[p->video_track_id]->duration / p->number_of_video_frames;
	//p->video_scale = p->info->tracks[p->video_track_id]->stts_sample_duration[0];
	
	p->audio_actual_rate = p->info->tracks[p->audio_track_ids[0]]->samplerate;
	p->audio_rate = p->audio_actual_rate * (p->audio_up_sample+1) ;
	p->audio_scale = (p->audio_type == 0x6D703461 ? 1024 : 160);
	p->audio_resample_scale = p->audio_scale * (p->audio_up_sample+1);
	
	p->audio_stereo = 1;
	
	int64_t v0 = p->video_rate * p->audio_resample_scale;
	int64_t v1 = p->audio_rate * p->video_scale;
	
	if (v0 >= v1)
		p->video_audio_interval = (int)(v0 / v1);
	else
		p->video_audio_interval = (int)(-(v1/v0));
	
	
	return(mp4_file_build_index(p));
}

