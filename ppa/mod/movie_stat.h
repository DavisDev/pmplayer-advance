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
 
#ifndef __MOVIE_STAT_H__
#define __MOVIE_STAT_H__

#include "pmp_play.h"
#include "mp4_play.h"
#include "mkv_play.h"
#include "flv1_play.h"

#define MAX_MOVIE_STAT 20

struct movie_stat_struct {
	char hash[16];
	int resume_pos;
	int audio_stream;
	int volume_boost;
	int aspect_ratio;
	int zoom;
	int luminosity_boost;
	int subtitle;
	int subtitle_format;
	int subtitle_fontcolor;
	int subtitle_bordercolor;
};

#ifdef __cplusplus
extern "C" {
#endif

void init_movie_stat(const char* s);

void pmp_stat_load( struct pmp_play_struct *p);
void pmp_stat_save( struct pmp_play_struct *p);

void mp4_stat_load( struct mp4_play_struct *p);
void mp4_stat_save( struct mp4_play_struct *p);

void mkv_stat_load( struct mkv_play_struct *p);
void mkv_stat_save( struct mkv_play_struct *p);

void flv1_stat_load( struct flv1_play_struct *p);
void flv1_stat_save( struct flv1_play_struct *p);

#ifdef __cplusplus
}
#endif

#endif
