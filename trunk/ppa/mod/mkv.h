#ifndef __MKV_H__
#define __MKV_H__


#include "gu_draw.h"
#include "movie_file.h"
#include "mkv_play.h"

#ifdef __cplusplus
extern "C" {
#endif

char *mkv_play(struct movie_file_struct *movie, int usePos, int pspType, int tvAspectRatio, int tvOverScanLeft, int tvOverScanTop, int tvOverScanRight, int tvOverScanBottom, int videoMode);

#ifdef __cplusplus
}
#endif

#endif
