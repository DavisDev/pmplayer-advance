/*
PMP Mod
Copyright (C) 2006 Raphael

E-mail:   raphael@fx-world.org

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
subtitle parsing layer
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "subtitle_parse.h"
#include "subtitle_subrip.h"
#include "subtitle_microdvd.h"
#include "common/mem64.h"



struct subtitle_frame_struct* (*subtitle_parse_line)( FILE *f, char* charset, unsigned int rate, unsigned int scale ) = 0;	// function pointer to line parsing

struct subtitle_parse_struct subtitle_parser[MAX_SUBTITLES];


int strconv( char* s, const char a, const char b )
	{
	int i = 1;
	char* t = s;
	while (*t)
		{
		if (*t==a)
			{
			*t = b;
			i++;
			}
		t++;
		}

	return(i);
	}


void subtitle_frame_safe_constructor(struct subtitle_frame_struct *p)
	{
		p->p_start_frame = 0;
		p->p_end_frame = 0;
		p->p_num_lines = 0;
		//p->p_string[0] = '\0';
		memset(p->p_string, 0, max_subtitle_string);
		p->next = 0;
		p->prev = 0;
	}
	

void subtitle_frame_safe_destructor(struct subtitle_frame_struct *p)
	{
		if (p->next != 0) subtitle_frame_safe_destructor(p->next);
		p->next = 0;
		p->prev = 0;
		free_64( p );
		p = 0;
	}


void subtitle_parse_safe_constructor(struct subtitle_parse_struct *p)
	{
		//if (p->p_sub_frame != 0) subtitle_frame_safe_destructor(p->p_sub_frame);
		p->p_sub_frame = 0;
		p->p_num_sub_frames = 0;
		p->p_cur_sub_frame = 0;
		p->p_in = 0;
		//p->filename[0] = '\0';
		memset(p->filename, 0, 1024);
	}
	

void subtitle_parse_close(struct subtitle_parse_struct *p)
	{
		if (p==0) return;
	    if (p->p_in != 0)        fclose(p->p_in);
		if (p->p_sub_frame != 0) subtitle_frame_safe_destructor(p->p_sub_frame);
		
		subtitle_parse_safe_constructor( p );
	}


char *subtitle_parse_search(struct movie_file_struct* movie, unsigned int rate, unsigned int scale, unsigned int *num_subtitles)
	{
		
		int i = 0;
		while (i < movie->movie_subtitle_num)
			{
			if (subtitle_parse_open( &subtitle_parser[*num_subtitles], movie->movie_subtitles[i].subtitle_file, movie->movie_subtitles[i].subtitle_charset, rate, scale )==0)
				{
				(*num_subtitles)++;
				if (*num_subtitles>=MAX_SUBTITLES) break;
				}
			i++;
			}
		if (*num_subtitles)
			return(0);
		else
			return("subtitle_parse_search: no subtitles found.");
	}


char *subtitle_parse_open(struct subtitle_parse_struct *p, char *s, char* charset, unsigned int rate, unsigned int scale)
	{
		if (p==0) return("subtitle_parse_open: p not initialized");
		subtitle_parse_safe_constructor(p);
		
		p->p_in = fopen(s, "rb");
		if (p->p_in == 0)
			{
			subtitle_parse_close(p);
			return("subtitle_parse_open: can't open file");
			}
	
		strncpy(p->filename, s, 1024);
		p->filename[1023]='\0';
		
		struct subtitle_frame_struct *new_frame = 0;
		p->p_sub_frame = (struct subtitle_frame_struct*)malloc_64( sizeof(struct subtitle_frame_struct) );
		if (p->p_sub_frame==0)
			{
			subtitle_parse_close(p);
			return("subtitle_parse_open: malloc_64 failed on p_sub_frame");
			}
		subtitle_frame_safe_constructor(p->p_sub_frame);
		p->p_cur_sub_frame = p->p_sub_frame;
		
		struct subtitle_frame_struct *cur_frame = p->p_sub_frame;
		
		/* subtitle format selection here */
		char *ext = p->filename+strlen(p->filename)-3;
		if (strncmp(strupr(ext),"SUB",3)==0)
			subtitle_parse_line = &subtitle_parse_microdvd;
		else if (strncmp(strupr(ext),"SRT",3)==0)
			subtitle_parse_line = &subtitle_parse_subrip;
		else
			{
			subtitle_parse_close(p);
			return("subtitle_parse_open: unknown subtitle format");
			}

		while ((new_frame=subtitle_parse_line( p->p_in, charset, rate, scale ))!=0)
			{
			cur_frame = subtitle_parse_add_frame(p, cur_frame, new_frame);
			/*/
			if (new_frame->p_start_frame<=cur_frame->p_end_frame) new_frame->p_start_frame=cur_frame->p_end_frame+1;
			cur_frame->next = new_frame;
			new_frame->prev = cur_frame;
			cur_frame = new_frame;
			p->p_num_sub_frames++;
			//*/
			}

		//cur_frame->next = 0;
		
		fclose(p->p_in);
		p->p_in = 0;
		
		if (p->p_num_sub_frames==0) {
			subtitle_parse_close(p);
			return("subtitle_parse_open: no subtitle frames parsed");
		}
		return(0);
	}
	
int subtitle_compare_frames(struct subtitle_frame_struct *src, struct subtitle_frame_struct *dest) {
	if ( dest->p_end_frame < src->p_start_frame )
		return -1;
	else if ( dest->p_start_frame > src->p_end_frame )
		return 1;
	else {
		if ( dest->p_start_frame < src->p_start_frame && dest->p_end_frame <= src->p_end_frame ) {
			dest->p_end_frame = src->p_start_frame-1;
			return -1;
		}
		else if ( dest->p_end_frame > src->p_end_frame && dest->p_start_frame >= src->p_start_frame ) {
			dest->p_start_frame = src->p_end_frame+1;
			return 1;
		}
		else
			return 0;
	} 
}

struct subtitle_frame_struct* subtitle_parse_add_frame(struct subtitle_parse_struct *p, struct subtitle_frame_struct *cur, struct subtitle_frame_struct *f ) {
	if (f == 0)
		return cur;	
	if ( f->p_start_frame < 0 || f->p_end_frame < 0 || f->p_start_frame > f->p_end_frame ) {
		free_64(f);
		return cur;
	}
	int comp = subtitle_compare_frames( cur, f );
	if (  comp == 0 ) {
		free_64(f);
		return cur;
	}
	else if ( comp > 0 ) {
		while( cur->next != 0 ) {
			cur = cur->next;
			comp = subtitle_compare_frames( cur, f );
			if ( comp == 0 ) {
				free_64(f);
				return cur;
			}
			else if ( comp < 0 ) {
				cur->prev->next = f;
				f->prev = cur->prev;
				f->next = cur;
				cur->prev = f;
				p->p_num_sub_frames++;
				return f;
			}
		}
		cur->next = f;
		f->prev = cur;
		p->p_num_sub_frames++;
		return f;
	}
 	else {
 		while( cur->prev != 0 ) {
 			cur = cur->prev;
 			comp = subtitle_compare_frames( cur, f );
 			if ( comp == 0 ) {
				free_64(f);
				return cur;
			}
			else if ( comp > 0 ) {
				cur->next->prev = f;
				f->next = cur->next;
				f->prev = cur;
				cur->next = f;
				p->p_num_sub_frames++;
				return f;
			}
 		}
 		f->next = cur;
 		p->p_sub_frame = f;
 		p->p_num_sub_frames++;
 		return f;
 	}
}

char* subtitle_parse_get_frame(struct subtitle_parse_struct *p, struct subtitle_frame_struct **f, unsigned int frame )
	{
		*f = 0;
		if (p==0) return("subtitle_parse_get_frame: p not initialized");
		if (p->p_sub_frame==0) return("subtitle_parse_get_frame: p_sub_frame not initialized");
	
		if (p->p_cur_sub_frame==0) p->p_cur_sub_frame = p->p_sub_frame;
		
		*f = p->p_cur_sub_frame;
		if (p->p_cur_sub_frame->p_start_frame<=frame && p->p_cur_sub_frame->p_end_frame>=frame) return(0);
		
		if (p->p_cur_sub_frame->next!=0 && p->p_cur_sub_frame->next->p_start_frame<=frame)
			{
			// Forward search
			while (p->p_cur_sub_frame->next!=0)
				{
				if (p->p_cur_sub_frame->next->p_start_frame>frame) break;
				p->p_cur_sub_frame = p->p_cur_sub_frame->next;
				*f = p->p_cur_sub_frame;
				if (p->p_cur_sub_frame->p_start_frame<=frame && p->p_cur_sub_frame->p_end_frame>=frame) return(0);
				}
			}
		else if (p->p_cur_sub_frame->prev!=0 && p->p_cur_sub_frame->prev->p_end_frame>=frame)
			{
			// Backward search
			while (p->p_cur_sub_frame->prev!=0)
				{
				if (p->p_cur_sub_frame->prev->p_end_frame<frame) break;
				p->p_cur_sub_frame = p->p_cur_sub_frame->prev;
				*f = p->p_cur_sub_frame;
				if (p->p_cur_sub_frame->p_start_frame<=frame && p->p_cur_sub_frame->p_end_frame>=frame) return(0);
				}
			}
		
		*f = 0;
		return(0);
	}
	
