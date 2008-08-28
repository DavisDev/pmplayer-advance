#ifndef __MP4_STAT_H__
#define __MP4_STAT_H__

#include "mp4_play.h"

void mp4_stat_read( char* s, int* pos, int* vol, int* aspect, int* zoom, int* lum, int* sub, int* subfmt, int* subfcol, int* subbcol );
void mp4_stat_load( struct mp4_play_struct *p, char* s );
void mp4_stat_save( struct mp4_play_struct *p );


#endif
