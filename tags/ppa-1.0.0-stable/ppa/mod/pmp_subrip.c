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

#include<psptypes.h>
#include<string.h>

#include "pmp_subrip.h"
#include "common/mem64.h"

#ifdef PSPFW3XX
#include "common/libminiconv.h"
#else
#include "common/miniconv.h"

static utf8_convert_function utf8_convertor = NULL;

void set_pmp_subrip_charset(const char* charset) {
	if ( stricmp(charset, "GBK") == 0)
		utf8_convertor = gbk_to_utf8;
	else if (stricmp(charset, "BIG5") == 0)
		utf8_convertor = big5_to_utf8;
	else if (stricmp(charset, "SHIFT-JIS") == 0)
		utf8_convertor = sjis_to_utf8;
	else if (stricmp(charset, "EUC-KR") == 0)
		utf8_convertor = euc_kr_to_utf8;
	else if (stricmp(charset, "MS-EE") == 0)
		utf8_convertor = ms_ee_to_utf8;
	else
		utf8_convertor = NULL;
}
#endif

struct pmp_sub_frame_struct* pmp_sub_parse_subrip( FILE *f, unsigned int rate, unsigned int scale )
	{
	if (!f) return(0);
	
	struct pmp_sub_frame_struct *p = (struct pmp_sub_frame_struct*)malloc_64( sizeof(struct pmp_sub_frame_struct) );
	if (p==0) return(0);
	p->p_string[0] = '\0';
	
	
    char line[1024];
    char c;
    u64 temp;
    int a1,a2,a3,a4,b1,b2,b3,b4;
    int i, j = 0;
    
    while (1)
		{
		if (!fgets(line, 1024, f)) return(0);
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
	    if (!fgets (line, 1023, f)) break;
	    
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
#ifdef PSPFW3XX
	if ( miniConvHaveSubtitleConv() ){
		char* temp_str = miniConvSubtitleConv(p->p_string);
		if( temp_str != NULL ) {
			strncpy(p->p_string, temp_str, max_subtitle_string-1);
		}
	}
#else
	//add by cooleyes 2006/12/15
	if ( utf8_convertor != NULL){
		char* temp_str = utf8_convertor(p->p_string);
		if( temp_str != NULL ) {
			strncpy(p->p_string, temp_str, max_subtitle_string-1);
			free(temp_str);
		}
	}
	//add end
#endif	
	return(p);
	}

