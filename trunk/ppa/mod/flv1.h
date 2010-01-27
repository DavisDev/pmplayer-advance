#ifndef __FLV1_H__
#define __FLV1_H__


#include "gu_draw.h"
#include "movie_file.h"
#include "flv1_play.h"

#ifdef __cplusplus
extern "C" {
#endif

char *flv1_play(struct movie_file_struct *movie, int usePos, int pspType, int tvAspectRatio, int tvOverScanLeft, int tvOverScanTop, int tvOverScanRight, int tvOverScanBottom, int videoMode);

#ifdef __cplusplus
}
#endif

#endif
