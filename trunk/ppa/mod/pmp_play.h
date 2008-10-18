/*
PMP Mod
Copyright (C) 2006 jonny

Homepage: http://jonny.leffe.dnsalias.com
E-mail:   jonny@leffe.dnsalias.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
this play the file (av output and basic functions - pause, seek ... )
*/


#ifndef pmp_play_h
#define pmp_play_h


#include <pspthreadman.h>
#include <pspaudio.h>
#include "cooleyesBridge.h"
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psppower.h>
#include "pmp_decode.h"
#include "aspect_ratio.h"
#include "gu_draw.h"
#include "gu_util.h"
#include "subtitle_parse.h"
#include "movie_file.h"


struct pmp_play_struct
	{
	struct pmp_decode_struct decoder;


    int audio_reserved;

	SceUID semaphore_can_get;
	SceUID semaphore_can_put;
	SceUID semaphore_can_show;
	SceUID semaphore_show_done;

	SceUID output_thread;
	SceUID show_thread;


	int   return_request;
	char *return_result;


	int paused;
	int seek;


	unsigned int audio_stream;
	//add by cooleyes 2007/02/01
	int audio_channel;
	//add end
	unsigned int volume_boost;
	unsigned int aspect_ratio;
	unsigned int zoom;
	unsigned int luminosity_boost;
	unsigned int show_interface;
	unsigned int last_keyframe_pos;
	unsigned int resume_pos;
	char hash[16];
	unsigned int subtitle_count;
	unsigned int subtitle;
	unsigned int subtitle_format;
	unsigned int subtitle_fontcolor;
	unsigned int subtitle_bordercolor;
	unsigned int loop;
	};



#define NUMBER_OF_FONTCOLORS 6
#define NUMBER_OF_BORDERCOLORS 6

#include "movie_stat.h"
void pmp_play_safe_constructor(struct pmp_play_struct *p);
char *pmp_play_open(struct pmp_play_struct *p, struct movie_file_struct *movie, int usePos, int pspType, int tvAspectRatio, int tvWidth, int tvHeight, int videoMode);
void pmp_play_close(struct pmp_play_struct *p, int usePos, int pspType);
char *pmp_play_start(volatile struct pmp_play_struct *p);


#endif
