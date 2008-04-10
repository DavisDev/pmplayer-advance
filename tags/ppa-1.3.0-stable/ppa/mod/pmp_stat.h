#ifndef pmp_stat_h__
#define pmp_stat_h__

#include "pmp_play.h"

void pmp_stat_read( char* s, int* pos, int* vol, int* aspect, int* zoom, int* lum, int* sub, int* subfmt, int* subfcol, int* subbcol );
void pmp_stat_load( struct pmp_play_struct *p, char* s );
void pmp_stat_save( struct pmp_play_struct *p );


#endif
