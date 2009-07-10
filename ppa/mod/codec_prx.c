/* 
 *	Copyright (C) 2006 cooleyes
 *	eyes.cooleyes@gmail.com 
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
 
#include "codec_prx.h"
#include "common/m33sdk.h"
 
char *load_codec_prx(const char* ppa_path, int devkitVersion)
	{
	int result;

	result = sceUtilityLoadAvModule(0); 
	if (result < 0)
		{
		return("prx_static_init: pspSdkLoadStartModule failed on avcodec.prx");
		}

	char prx_path[512];
	memset(prx_path, 0, 512);
	if ( devkitVersion < 0x03050000)
		sprintf(prx_path, "%s%s", ppa_path, "mpeg_vsh330.prx");
	else if ( devkitVersion < 0x03070000)
		sprintf(prx_path, "%s%s", ppa_path, "mpeg_vsh350.prx");
	else
		sprintf(prx_path, "%s%s", ppa_path, "mpeg_vsh370.prx");
	int status;
	result = m33KernelLoadModule(prx_path, 0, NULL);
	if(result >= 0) {
		result = sceKernelStartModule(result, 0, 0, &status, NULL);
	}

	if (result < 0)
		{
		return("prx_static_init: pspSdkLoadStartModule failed on mpeg_vsh.prx");
		}

	
	return(0);
	}
