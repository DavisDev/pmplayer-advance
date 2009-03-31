#include "mkv.h"


char *mkv_play(struct movie_file_struct *movie, int usePos, int pspType, int tvAspectRatio, int tvOverScanLeft, int tvOverScanTop, int tvOverScanRight, int tvOverScanBottom, int videoMode) {
	struct mkv_play_struct p;


	pmp_gu_start(pspType, tvAspectRatio, tvOverScanLeft, tvOverScanTop, tvOverScanRight, tvOverScanBottom, videoMode);


	char *result = mkv_play_open(&p, movie, usePos, pspType, tvAspectRatio, (720-tvOverScanLeft-tvOverScanRight), (480-tvOverScanTop-tvOverScanBottom), videoMode);
	if (result == 0) {
		result = mkv_play_start(&p);

		mkv_play_close(&p, usePos, pspType);
	}

	pmp_gu_end();

	return(result);
}
