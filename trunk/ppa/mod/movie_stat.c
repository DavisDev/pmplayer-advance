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
 
#include "movie_stat.h"
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

static char stat_filename[1024];

void init_movie_stat(const char* s) {
	memset(stat_filename, 0, 1024);
	strcpy(stat_filename, s);
}

void pmp_stat_load( struct pmp_play_struct *p) {
	
	if (p==0) return;
	
	SceUID	fd;

	// device:path
	if((fd = sceIoOpen( stat_filename, PSP_O_RDONLY, 0777))>=0) {
		
		struct movie_stat_struct stats[MAX_MOVIE_STAT];
		memset(stats, 0, MAX_MOVIE_STAT*sizeof(struct movie_stat_struct));
		sceIoRead( fd, stats, MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		
		int i;
		
		for(i=0; i<MAX_MOVIE_STAT; i++) {
			if ( memcmp(p->hash, stats[i].hash, 16) == 0 ) {
				
				p->resume_pos = stats[i].resume_pos;
				
				p->audio_stream = stats[i].audio_stream;
				if (p->audio_stream>=p->decoder.reader.file.header.audio.number_of_streams)
					p->audio_stream = 0;
			
				p->volume_boost = stats[i].volume_boost;
				if (p->volume_boost>6)
					p->volume_boost = 6;
					
				p->aspect_ratio = stats[i].aspect_ratio;
				if (p->aspect_ratio>=number_of_aspect_ratios)
					p->aspect_ratio = number_of_aspect_ratios-1;
				
				p->zoom = stats[i].zoom;
				if (p->zoom>200)
					p->zoom = 200;
				if (p->zoom<100)
					p->zoom = 100;
				
				p->luminosity_boost = stats[i].luminosity_boost;
				if (p->luminosity_boost>=number_of_luminosity_boosts)
					p->luminosity_boost = number_of_luminosity_boosts-1;
					
				p->subtitle = stats[i].subtitle;
				if (p->subtitle>p->subtitle_count)
					p->subtitle = p->subtitle_count;
				
				p->subtitle_format = stats[i].subtitle_format;
				if (p->subtitle_format>1)
					p->subtitle_format = 1;
		
				p->subtitle_fontcolor = stats[i].subtitle_fontcolor;
				if (p->subtitle_fontcolor>=NUMBER_OF_FONTCOLORS)
					p->subtitle_fontcolor = NUMBER_OF_FONTCOLORS-1;
		
				p->subtitle_bordercolor = stats[i].subtitle_bordercolor;
				if (p->subtitle_bordercolor>=NUMBER_OF_BORDERCOLORS)
					p->subtitle_bordercolor = NUMBER_OF_BORDERCOLORS-1;
			}
		}
		sceIoClose( fd );
	}
}

void pmp_stat_save( struct pmp_play_struct *p) {
	
	if (p==0) return;
	
	SceUID	fd;
	
	struct movie_stat_struct stats[MAX_MOVIE_STAT+1];
	memset(stats, 0, (MAX_MOVIE_STAT+1)*sizeof(struct movie_stat_struct));
	
	if((fd = sceIoOpen( stat_filename, PSP_O_RDONLY, 0777))>=0) {	
		sceIoRead( fd, stats, MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		sceIoClose( fd );
	}
	
	if((fd = sceIoOpen( stat_filename, PSP_O_WRONLY|PSP_O_CREAT, 0777))>=0) {
		
		int i;
		
		for(i=0; i<MAX_MOVIE_STAT; i++) {
			if ( memcmp(p->hash, stats[i].hash, 16) == 0 ) {
				break;
			}
		}
		
		memcpy(stats[i].hash, p->hash, 16);
		stats[i].resume_pos = p->last_keyframe_pos;
		stats[i].audio_stream = p->audio_stream;
		stats[i].volume_boost = p->volume_boost;
		stats[i].aspect_ratio = p->aspect_ratio;
		stats[i].zoom = p->zoom;
		stats[i].luminosity_boost = p->luminosity_boost;
		stats[i].subtitle = p->subtitle;
		stats[i].subtitle_format = p->subtitle_format;
		stats[i].subtitle_fontcolor = p->subtitle_fontcolor;
		stats[i].subtitle_bordercolor = p->subtitle_bordercolor;
				
		if ( i == MAX_MOVIE_STAT ) {
			sceIoWrite(fd, &stats[1], MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		}
		else {
			sceIoWrite(fd, &stats[0], MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		}
		sceIoClose( fd );
	}
}

void mp4_stat_load( struct mp4_play_struct *p) {
	
	if (p==0) return;
	
	SceUID	fd;

	// device:path
	if((fd = sceIoOpen( stat_filename, PSP_O_RDONLY, 0777))>=0) {
		
		struct movie_stat_struct stats[MAX_MOVIE_STAT];
		memset(stats, 0, MAX_MOVIE_STAT*sizeof(struct movie_stat_struct));
		sceIoRead( fd, stats, MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		
		int i;
		
		for(i=0; i<MAX_MOVIE_STAT; i++) {
			if ( memcmp(p->hash, stats[i].hash, 16) == 0 ) {
				
				p->resume_pos = stats[i].resume_pos;
				
				p->audio_stream = stats[i].audio_stream;
				if (p->audio_stream>=p->decoder.reader.file.audio_tracks)
					p->audio_stream = 0;
			
				p->volume_boost = stats[i].volume_boost;
				if (p->volume_boost>6)
					p->volume_boost = 6;
					
				p->aspect_ratio = stats[i].aspect_ratio;
				if (p->aspect_ratio>=number_of_aspect_ratios)
					p->aspect_ratio = number_of_aspect_ratios-1;
				
				p->zoom = stats[i].zoom;
				if (p->zoom>200)
					p->zoom = 200;
				if (p->zoom<100)
					p->zoom = 100;
				
				p->luminosity_boost = stats[i].luminosity_boost;
				if (p->luminosity_boost>=number_of_luminosity_boosts)
					p->luminosity_boost = number_of_luminosity_boosts-1;
					
				p->subtitle = stats[i].subtitle;
				if (p->subtitle>p->subtitle_count)
					p->subtitle = p->subtitle_count;
				
				p->subtitle_format = stats[i].subtitle_format;
				if (p->subtitle_format>1)
					p->subtitle_format = 1;
		
				p->subtitle_fontcolor = stats[i].subtitle_fontcolor;
				if (p->subtitle_fontcolor>=NUMBER_OF_FONTCOLORS)
					p->subtitle_fontcolor = NUMBER_OF_FONTCOLORS-1;
		
				p->subtitle_bordercolor = stats[i].subtitle_bordercolor;
				if (p->subtitle_bordercolor>=NUMBER_OF_BORDERCOLORS)
					p->subtitle_bordercolor = NUMBER_OF_BORDERCOLORS-1;
			}
		}
		sceIoClose( fd );
	}
}

void mp4_stat_save( struct mp4_play_struct *p) {
	
	if (p==0) return;
	
	SceUID	fd;
	
	struct movie_stat_struct stats[MAX_MOVIE_STAT+1];
	memset(stats, 0, (MAX_MOVIE_STAT+1)*sizeof(struct movie_stat_struct));
	
	if((fd = sceIoOpen( stat_filename, PSP_O_RDONLY, 0777))>=0) {	
		sceIoRead( fd, stats, MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		sceIoClose( fd );
	}
	
	if((fd = sceIoOpen( stat_filename, PSP_O_WRONLY|PSP_O_CREAT, 0777))>=0) {
		
		int i;
		
		for(i=0; i<MAX_MOVIE_STAT; i++) {
			if ( memcmp(p->hash, stats[i].hash, 16) == 0 ) {
				break;
			}
		}
		
		memcpy(stats[i].hash, p->hash, 16);
		stats[i].resume_pos = p->last_keyframe_pos;
		stats[i].audio_stream = p->audio_stream;
		stats[i].volume_boost = p->volume_boost;
		stats[i].aspect_ratio = p->aspect_ratio;
		stats[i].zoom = p->zoom;
		stats[i].luminosity_boost = p->luminosity_boost;
		stats[i].subtitle = p->subtitle;
		stats[i].subtitle_format = p->subtitle_format;
		stats[i].subtitle_fontcolor = p->subtitle_fontcolor;
		stats[i].subtitle_bordercolor = p->subtitle_bordercolor;
				
		if ( i == MAX_MOVIE_STAT ) {
			sceIoWrite(fd, &stats[1], MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		}
		else {
			sceIoWrite(fd, &stats[0], MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		}
		sceIoClose( fd );
	}
}

void mkv_stat_load( struct mkv_play_struct *p) {
	
	if (p==0) return;
	
	SceUID	fd;

	// device:path
	if((fd = sceIoOpen( stat_filename, PSP_O_RDONLY, 0777))>=0) {
		
		struct movie_stat_struct stats[MAX_MOVIE_STAT];
		memset(stats, 0, MAX_MOVIE_STAT*sizeof(struct movie_stat_struct));
		sceIoRead( fd, stats, MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		
		int i;
		
		for(i=0; i<MAX_MOVIE_STAT; i++) {
			if ( memcmp(p->hash, stats[i].hash, 16) == 0 ) {
				
				p->resume_pos = stats[i].resume_pos;
				
				p->audio_stream = stats[i].audio_stream;
				if (p->audio_stream>=p->decoder.reader.file.audio_tracks)
					p->audio_stream = 0;
			
				p->volume_boost = stats[i].volume_boost;
				if (p->volume_boost>6)
					p->volume_boost = 6;
					
				p->aspect_ratio = stats[i].aspect_ratio;
				if (p->aspect_ratio>=number_of_aspect_ratios)
					p->aspect_ratio = number_of_aspect_ratios-1;
				
				p->zoom = stats[i].zoom;
				if (p->zoom>200)
					p->zoom = 200;
				if (p->zoom<100)
					p->zoom = 100;
				
				p->luminosity_boost = stats[i].luminosity_boost;
				if (p->luminosity_boost>=number_of_luminosity_boosts)
					p->luminosity_boost = number_of_luminosity_boosts-1;
					
				p->subtitle = stats[i].subtitle;
				if (p->subtitle>p->subtitle_count)
					p->subtitle = p->subtitle_count;
				
				p->subtitle_format = stats[i].subtitle_format;
				if (p->subtitle_format>1)
					p->subtitle_format = 1;
		
				p->subtitle_fontcolor = stats[i].subtitle_fontcolor;
				if (p->subtitle_fontcolor>=NUMBER_OF_FONTCOLORS)
					p->subtitle_fontcolor = NUMBER_OF_FONTCOLORS-1;
		
				p->subtitle_bordercolor = stats[i].subtitle_bordercolor;
				if (p->subtitle_bordercolor>=NUMBER_OF_BORDERCOLORS)
					p->subtitle_bordercolor = NUMBER_OF_BORDERCOLORS-1;
			}
		}
		sceIoClose( fd );
	}
}

void mkv_stat_save( struct mkv_play_struct *p) {
	
	if (p==0) return;
	
	SceUID	fd;
	
	struct movie_stat_struct stats[MAX_MOVIE_STAT+1];
	memset(stats, 0, (MAX_MOVIE_STAT+1)*sizeof(struct movie_stat_struct));
	
	if((fd = sceIoOpen( stat_filename, PSP_O_RDONLY, 0777))>=0) {	
		sceIoRead( fd, stats, MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		sceIoClose( fd );
	}
	
	if((fd = sceIoOpen( stat_filename, PSP_O_WRONLY|PSP_O_CREAT, 0777))>=0) {
		
		int i;
		
		for(i=0; i<MAX_MOVIE_STAT; i++) {
			if ( memcmp(p->hash, stats[i].hash, 16) == 0 ) {
				break;
			}
		}
		
		memcpy(stats[i].hash, p->hash, 16);
		stats[i].resume_pos = p->last_keyframe_pos;
		stats[i].audio_stream = p->audio_stream;
		stats[i].volume_boost = p->volume_boost;
		stats[i].aspect_ratio = p->aspect_ratio;
		stats[i].zoom = p->zoom;
		stats[i].luminosity_boost = p->luminosity_boost;
		stats[i].subtitle = p->subtitle;
		stats[i].subtitle_format = p->subtitle_format;
		stats[i].subtitle_fontcolor = p->subtitle_fontcolor;
		stats[i].subtitle_bordercolor = p->subtitle_bordercolor;
				
		if ( i == MAX_MOVIE_STAT ) {
			sceIoWrite(fd, &stats[1], MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		}
		else {
			sceIoWrite(fd, &stats[0], MAX_MOVIE_STAT*sizeof(struct movie_stat_struct) );
		}
		sceIoClose( fd );
	}
}


