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
microdvd subtitle format parser
*/


#include "subtitle_microdvd.h"
#include "common/mem64.h"


struct subtitle_frame_struct* subtitle_parse_microdvd( FILE *f, unsigned int rate, unsigned int scale )
	{
	if (!f) return(0);
	
	struct subtitle_frame_struct *p = (struct subtitle_frame_struct*)malloc_64( sizeof(struct subtitle_frame_struct) );
	if (p==0) return(0);
	p->p_string[0] = '\0';
	
	int result = fscanf( f, "{%i}{%i}", &p->p_start_frame, &p->p_end_frame );
	if (result==EOF)
		{
		free_64(p);
		return(0);
		}
	
	char c;
	int i = 0;
	p->p_num_lines = 1;
	while (i<max_subtitle_string-1)
		{
		c = fgetc(f);
		if (c=='\n' || c==EOF) break;
		if (c=='|')
			{
			p->p_string[i++]='\n';
			p->p_num_lines++;
			}
		else
			{
			p->p_string[i++]=c;
			}
		}
	p->p_string[i] = '\0';
	
	return(p);
	}
	
