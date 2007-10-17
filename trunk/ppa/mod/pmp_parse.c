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
#include "pmp_parse.h"
#include "pmp_subrip.h"
#include "pmp_microdvd.h"
#include "../common/mem64.h"
#include "opendir.h"



struct pmp_sub_frame_struct* (*pmp_sub_parse_line)( FILE *f, unsigned int rate, unsigned int scale ) = 0;	// function pointer to line parsing

struct pmp_sub_parse_struct subtitle_parser[MAX_SUBTITLES];


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


void pmp_sub_frame_safe_constructor(struct pmp_sub_frame_struct *p)
	{
		p->p_start_frame = 0;
		p->p_end_frame = 0;
		p->p_num_lines = 0;
		p->p_string[0] = '\0';
		p->next = 0;
		p->prev = 0;
	}
	

void pmp_sub_frame_safe_destructor(struct pmp_sub_frame_struct *p)
	{
		if (p->next != 0) pmp_sub_frame_safe_destructor(p->next);
		p->next = 0;
		p->prev = 0;
		free_64( p );
		p = 0;
	}


void pmp_sub_parse_safe_constructor(struct pmp_sub_parse_struct *p)
	{
		if (p->p_sub_frame != 0) pmp_sub_frame_safe_destructor(p->p_sub_frame);
		p->p_sub_frame = 0;
		p->p_num_sub_frames = 0;
		p->p_cur_sub_frame = 0;
		p->p_in = 0;
		p->filename[0] = '\0';
	}
	

void pmp_sub_parse_close(struct pmp_sub_parse_struct *p)
	{
		if (p==0) return;
	    if (p->p_in != 0)        fclose(p->p_in);
		if (p->p_sub_frame != 0) pmp_sub_frame_safe_destructor(p->p_sub_frame);
		
		pmp_sub_parse_safe_constructor( p );
	}


char *pmp_sub_parse_search(char *folder, char *filename, unsigned int rate, unsigned int scale, unsigned int *num_subtitles)
	{
		struct opendir_struct directory;
		
		char *fname, fbuffer[1024];
		strncpy( fbuffer, filename, 1024 );
		fname = strupr(fbuffer);
		char format[1024];
		char* ext = strrchr(fname,'.');
		int format_sz = (ext-fname<1024?ext-fname:1023);
		strncpy( format, fname, format_sz );
		format[format_sz] = '\0';
		
		*num_subtitles = 0;
		
		char* filter[] = 
		{	".sub",
			".srt",
			0
		};
		
		char *result = opendir_open(&directory, folder, filter, SORT_NAME);
		if (result != 0)
			{
			opendir_close(&directory);
			return("pmp_sub_parse_search: directory empty or doesn't exist.");
			}
		
		int i = 0;
		while (i < directory.number_of_directory_entries)
			{
			char name[512];
			strncpy(name,directory.directory_entry[i].d_name,512);
			if (strncmp(strupr(name),format,format_sz)==0 && strcmp(strupr(name),filename)!=0)
				{
				//modify by cooleyes 2006/12/11
				char sub_name[1024];
				memset(sub_name,0,1024);
				sprintf(sub_name,"%s%s", folder, directory.directory_entry[i].d_name);
				if (pmp_sub_parse_open( &subtitle_parser[*num_subtitles], sub_name, rate, scale )==0)
				//if (pmp_sub_parse_open( &subtitle_parser[*num_subtitles], directory.directory_entry[i].d_name, rate, scale )==0)
				//modify end
					{
					(*num_subtitles)++;
					if (*num_subtitles>=MAX_SUBTITLES) break;
					}
				}
			i++;
			}
		opendir_close(&directory);
		if (*num_subtitles)
			return(0);
		else
			return("pmp_sub_parse_search: no subtitles found.");
	}


char *pmp_sub_parse_open(struct pmp_sub_parse_struct *p, char *s, unsigned int rate, unsigned int scale)
	{
		if (p==0) return("pmp_sub_parse_open: p not initialized");
		pmp_sub_parse_safe_constructor(p);
		
		p->p_in = fopen(s, "rb");
		if (p->p_in == 0)
			{
			pmp_sub_parse_close(p);
			return("pmp_sub_parse_open: can't open file");
			}
	
		strncpy(p->filename, s, 1024);
		p->filename[1023]='\0';
		
		struct pmp_sub_frame_struct *new_frame = 0;
		p->p_sub_frame = (struct pmp_sub_frame_struct*)malloc_64( sizeof(struct pmp_sub_frame_struct) );
		if (p->p_sub_frame==0)
			{
			pmp_sub_parse_close(p);
			return("pmp_sub_parse_open: malloc_64 failed on p_sub_frame");
			}
		pmp_sub_frame_safe_constructor(p->p_sub_frame);
		p->p_cur_sub_frame = p->p_sub_frame;
		
		struct pmp_sub_frame_struct *cur_frame = p->p_sub_frame;
		
		/* subtitle format selection here */
		char *ext = p->filename+strlen(p->filename)-3;
		if (strncmp(strupr(ext),"SUB",3)==0)
			pmp_sub_parse_line = &pmp_sub_parse_microdvd;
		else if (strncmp(strupr(ext),"SRT",3)==0)
			pmp_sub_parse_line = &pmp_sub_parse_subrip;
		else
			{
			pmp_sub_parse_close(p);
			return("pmp_sub_parse_open: unknown subtitle format");
			}

		while ((new_frame=pmp_sub_parse_line( p->p_in, rate, scale ))!=0)
			{
			if (new_frame->p_start_frame<=cur_frame->p_end_frame) new_frame->p_start_frame=cur_frame->p_end_frame+1;
			cur_frame->next = new_frame;
			new_frame->prev = cur_frame;
			cur_frame = new_frame;
			p->p_num_sub_frames++;
			}

		cur_frame->next = 0;
		
		fclose(p->p_in);
		p->p_in = 0;
		
		if (p->p_num_sub_frames==0)
			return("pmp_sub_parse_open: no subtitle frames parsed");

		return(0);
	}


char* pmp_sub_parse_get_frame(struct pmp_sub_parse_struct *p, struct pmp_sub_frame_struct **f, unsigned int frame )
	{
		*f = 0;
		if (p==0) return("pmp_sub_parse_get_frame: p not initialized");
		if (p->p_sub_frame==0) return("pmp_sub_parse_get_frame: p_sub_frame not initialized");
	
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
	
