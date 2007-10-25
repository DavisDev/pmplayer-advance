/*
PMP Mod
Copyright (C) 2006 jonny

Homepage: http://jonny.leffe.dnsalias.com
E-mail:   jonny@leffe.dnsalias.com

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
this lib should be used to play .pmp files
*/


#include "pmp.h"


char *pmp_play(char *s, int usePos, int pspType, int tvAspectRatio, int videoMode)
	{
	struct pmp_play_struct p;


	pmp_gu_start(pspType, tvAspectRatio, videoMode);


	char *result = pmp_play_open(&p, s, usePos, pspType, tvAspectRatio, videoMode);
	if (result == 0)
		{
		result = pmp_play_start(&p);

		pmp_play_close(&p, usePos, pspType);
		}

	pmp_gu_end();

	return(result);
	}
