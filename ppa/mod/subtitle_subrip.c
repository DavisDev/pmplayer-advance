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
subrip subtitle format parser
*/

//a little modify by cooleyes

#include<psptypes.h>
#include<string.h>

#include "subtitle_subrip.h"
#include "common/mem64.h"

#include "common/fgets_mod.h"
#include "common/libminiconv.h"

struct subtitle_frame_struct* subtitle_parse_subrip( FILE *f, char* charset, unsigned int rate, unsigned int scale )
	{
	if (!f) return(0);
	
	struct subtitle_frame_struct *p = (struct subtitle_frame_struct*)malloc_64( sizeof(struct subtitle_frame_struct) );
	if (p==0) return(0);
	subtitle_frame_safe_constructor(p);
	
	fgets_function fgets_f;
	if ( stricmp(charset, "UTF-16LE") == 0 )
		fgets_f = fgets_utf16_le;
	else if ( stricmp(charset, "UTF-16BE") == 0 )
		fgets_f = fgets_utf16_be;
	else
		fgets_f = fgets;
		
    char line[3072];
    char c;
    u64 temp;
    int a1,a2,a3,a4,b1,b2,b3,b4;
    int i, j = 0;
    
    while (1)
		{
		memset(line, 0, 3072);
		if (!fgets_f(line, 1024, f)) 
			{
			free_64(p);
			return(0);
			}
		if (sscanf(line, "%d:%d:%d%[,.:]%d --> %d:%d:%d%[,.:]%d",&a1,&a2,&a3,(char *)&i,&a4,&b1,&b2,&b3,(char *)&i,&b4) >= 10)
			break;
		}
		
	//modify by cooleyes 2006/12/11	
	temp = 1;
	temp = temp * (a1*360000+a2*6000+a3*100+a4/10) * rate;
	//p->p_start_frame = (a1*360000+a2*6000+a3*100+a4/10) * rate / (scale*100);
	p->p_start_frame = temp/scale/100;
	temp = 1;
	temp = temp * (b1*360000+b2*6000+b3*100+b4/10) * rate;
	//p->p_end_frame   = (b1*360000+b2*6000+b3*100+b4/10) * rate / (scale*100);
	p->p_end_frame   = temp/scale/100;
	//modify end
	
	p->p_num_lines = 0;
	j = 0;
	while (j<max_subtitle_string)
		{
		memset(line, 0, 3072);
	    if (!fgets_f(line, 1024, f)) break;
	    if (line[0]=='\n' || line[0]=='\r') break;
	    p->p_num_lines++;
	    i = 0;
		while (i<1024)
			{
			c = line[i++];
			if (c=='<')
				{
				while (line[i]!='>') i++;
				i++;
				}
			else if (c=='\n' || c=='\r' || c==EOF) break;
			else
				{
				p->p_string[j++]=c;
				}
			}
		p->p_string[j++]='\n';
		}
	p->p_string[j-1] = '\0';

	if ( miniConvHaveSubtitleConv(charset) ) {
		char* temp_str = miniConvSubtitleConv(p->p_string, charset);
		if( temp_str != NULL ) {
			memset(p->p_string, 0, max_subtitle_string);
			strncpy(p->p_string, temp_str, max_subtitle_string-1);
		}
	}
	else if ( stricmp(charset, "DEFAULT") == 0 && miniConvHaveDefaultSubtitleConv() ){
		char* temp_str = miniConvDefaultSubtitleConv(p->p_string);
		if( temp_str != NULL ) {
			memset(p->p_string, 0, max_subtitle_string);
			strncpy(p->p_string, temp_str, max_subtitle_string-1);
		}
	}

	return(p);
	}

