#include "mp4.h"


char *mp4_play(char *s, int usePos, int pspType, int tvAspectRatio, int tvOverScanLeft, int tvOverScanTop, int tvOverScanRight, int tvOverScanBottom, int videoMode) {
	struct mp4_play_struct p;


	pmp_gu_start(pspType, tvAspectRatio, tvOverScanLeft, tvOverScanTop, tvOverScanRight, tvOverScanBottom, videoMode);


	char *result = mp4_play_open(&p, s, usePos, pspType, tvAspectRatio, (720-tvOverScanLeft-tvOverScanRight), (480-tvOverScanTop-tvOverScanBottom), videoMode);
	if (result == 0) {
		result = mp4_play_start(&p);

		mp4_play_close(&p, usePos, pspType);
	}

	pmp_gu_end();

	return(result);
}
