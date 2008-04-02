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

#ifndef subtitle_parse_h__
#define subtitle_parse_h__

#include <stdio.h>


#define max_subtitle_string 512


struct subtitle_frame_struct {
	unsigned int		p_start_frame;
	unsigned int		p_end_frame;
	unsigned int		p_num_lines;
	char	p_string[max_subtitle_string];
	
	struct subtitle_frame_struct *next;
	struct subtitle_frame_struct *prev;
};


struct subtitle_parse_struct {
	char				filename[1024];
	struct subtitle_frame_struct	*p_sub_frame;		// Linked list of subtitle frames
	unsigned int		p_num_sub_frames;				// Number of subtitle frames
	struct subtitle_frame_struct	*p_cur_sub_frame;		// current subtitle frame
	FILE		*p_in;
};


#define MAX_SUBTITLES 8
extern struct subtitle_parse_struct subtitle_parser[MAX_SUBTITLES];


void subtitle_frame_safe_constructor(struct subtitle_frame_struct *p);
void subtitle_frame_safe_destructor(struct subtitle_frame_struct *p);
void subtitle_parse_safe_constructor(struct subtitle_parse_struct *p);
char *subtitle_parse_search(char *folder, char *filename, unsigned int rate, unsigned int scale, unsigned int *num_subtitles);
char *subtitle_parse_open(struct subtitle_parse_struct *p, char *s, unsigned int rate, unsigned int scale);
void subtitle_parse_close(struct subtitle_parse_struct *p);
char* subtitle_parse_get_frame(struct subtitle_parse_struct *p, struct subtitle_frame_struct **f, unsigned int frame );

// Helper for parsing functions
int strconv( char* s, const char a, const char b );


#endif
