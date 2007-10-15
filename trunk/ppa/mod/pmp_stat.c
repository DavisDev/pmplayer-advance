#include <pspiofilemgr.h>
#include "pmp_stat.h"

#undef printf


void pmp_stat_read( char* s, int* pos, int* vol, int* aspect, int* zoom, int* lum, int* sub, int* subfmt, int* subfcol, int* subbcol )
	{
	char filename[256];
	snprintf( filename, 256, "%s.pos", s);
	
	SceUID	fd;
	
	
	if (pos) *pos = 0;
	if (vol) *vol = 0;
	if (aspect) *aspect = 0;
	if (zoom) *zoom = 100;
	if (lum) *lum = 0;
	if (sub) *sub = 0;
	if (subfmt) *subfmt = 0;
	if (subfcol) *subfcol = 0;
	if (subbcol) *subbcol = 0;

	#define clamp(v,x,y) ((v<x)?(v=x):(v>y)?(v=y):v)
	
	if((fd = sceIoOpen( filename, PSP_O_RDONLY, 0777))>=0)
		{
		int val;
		sceIoRead( fd, &val, sizeof(int) );
		if (pos) *pos = val;
		sceIoRead( fd, &val, sizeof(int) );
		if (vol) *vol = clamp(val,0,3);
		sceIoRead( fd, &val, sizeof(int) );
		if (aspect) *aspect = clamp(val,0,number_of_aspect_ratios-1);
		sceIoRead( fd, &val, sizeof(int) );
		if (zoom) *zoom = clamp(val,100,200);
		sceIoRead( fd, &val, sizeof(int) );
		if (lum) *lum = clamp(val,0,number_of_luminosity_boosts-1);
		sceIoRead( fd, &val, sizeof(int) );
		if (sub) *sub = clamp(val,0,MAX_SUBTITLES);
		sceIoRead( fd, &val, sizeof(int) );
		if (subfmt) *subfmt = clamp(val,0,1);
		sceIoRead( fd, &val, sizeof(int) );
		if (subfcol) *subfcol = clamp(val,0,NUMBER_OF_FONTCOLORS-1);
		sceIoRead( fd, &val, sizeof(int) );
		if (subbcol) *subbcol = clamp(val,0,NUMBER_OF_BORDERCOLORS-1);
		sceIoClose(fd);
		}
	}


void pmp_stat_load( struct pmp_play_struct *p, char* s )
	{
	if (p==0) return;
	
	snprintf( p->resume_filename, 256, "%s.pos", s);
	
	SceUID	fd;

	// device:path
	if((fd = sceIoOpen( p->resume_filename, PSP_O_RDONLY, 0777))>=0)
		{
		sceIoRead( fd, &p->resume_pos, sizeof(int) );
		sceIoRead( fd, &p->audio_stream, sizeof(int) );
		if (p->audio_stream>p->decoder.reader.file.header.audio.number_of_streams)
			p->audio_stream = 0;
			
		sceIoRead( fd, &p->volume_boost, sizeof(int) );
		if (p->volume_boost>3)
			p->volume_boost = 3;
			
		sceIoRead( fd, &p->aspect_ratio, sizeof(int) );
		if (p->aspect_ratio>=number_of_aspect_ratios)
			p->aspect_ratio = number_of_aspect_ratios-1;
		
		sceIoRead( fd, &p->zoom, sizeof(int) );
		if (p->zoom>200)
			p->zoom = 200;
		if (p->zoom<100)
			p->zoom = 100;
		
		sceIoRead( fd, &p->luminosity_boost, sizeof(int) );
		if (p->luminosity_boost>=number_of_luminosity_boosts)
			p->luminosity_boost = number_of_luminosity_boosts-1;
			
		sceIoRead( fd, &p->subtitle, sizeof(int) );
		if (p->subtitle>p->subtitle_count)
			p->subtitle = p->subtitle_count;
		
		sceIoRead( fd, &p->subtitle_format, sizeof(int) );
		if (p->subtitle_format>1)
			p->subtitle_format = 1;

		sceIoRead( fd, &p->subtitle_fontcolor, sizeof(int) );
		if (p->subtitle_fontcolor>=NUMBER_OF_FONTCOLORS)
			p->subtitle_fontcolor = NUMBER_OF_FONTCOLORS-1;

		sceIoRead( fd, &p->subtitle_bordercolor, sizeof(int) );
		if (p->subtitle_bordercolor>=NUMBER_OF_BORDERCOLORS)
			p->subtitle_bordercolor = NUMBER_OF_BORDERCOLORS-1;

		sceIoClose( fd );
		}
	}


void pmp_stat_save( struct pmp_play_struct *p )
	{
	if (p==0) return;
	
	SceUID	fd;
	
	// device:path
	if((fd = sceIoOpen( p->resume_filename, PSP_O_WRONLY|PSP_O_CREAT, 0777))>=0)
		{
		sceIoWrite( fd, &p->last_keyframe_pos, sizeof(int) );
		//printf("pos.\n");
		sceIoWrite( fd, &p->audio_stream, sizeof(int) );
		//printf("audio.\n");
		sceIoWrite( fd, &p->volume_boost, sizeof(int) );
		//printf("volume.\n");
		sceIoWrite( fd, &p->aspect_ratio, sizeof(int) );
		//printf("aspect.\n");
		sceIoWrite( fd, &p->zoom, sizeof(int) );
		//printf("zoom.\n");
		sceIoWrite( fd, &p->luminosity_boost, sizeof(int) );
		//printf("lumi.\n");
		sceIoWrite( fd, &p->subtitle, sizeof(int) );
		//printf("subtitle.\n");
		sceIoWrite( fd, &p->subtitle_format, sizeof(int) );
		//printf("sub format.\n");
		sceIoWrite( fd, &p->subtitle_fontcolor, sizeof(int) );
		//printf("sub forecolor.\n");
		sceIoWrite( fd, &p->subtitle_bordercolor, sizeof(int) );
		//printf("sub bordercolor.\n");
		sceIoClose( fd );
		}
	else
		{
		sceIoRemove( p->resume_filename );	// Delete the file if something got wrong, or else there's a 0kb file
		}

	}
