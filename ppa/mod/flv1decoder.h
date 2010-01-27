/* 
 *	Copyright (C) 2010 cooleyes
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
 
#ifndef __flv1_decoder_h_cooleyes__
#define __flv1_decoder_h_cooleyes__

#include <stdio.h>
#include <string.h>
#include <pspsdk.h>
#include <pspjpeg.h>
#include <libavcodec/avcodec.h>
#include "pspvideocodec.h"
#include "pspmpeg.h"
#include "common/mem64.h"
#include "mpegbase.h"

struct flv1_struct
	{
	int      mpeg_init;
	int      jpeg_init;
	unsigned long* dest_buffer;
	AVCodec* avcodec;
	AVCodecContext* avcodec_context;
	AVFrame* avframe;
	};


#ifdef __cplusplus
extern "C" {
#endif

void flv1_safe_constructor(struct flv1_struct *p);
void flv1_close(struct flv1_struct *p);
char *flv1_open(struct flv1_struct *p, unsigned int width, unsigned int height);
char *flv1_get_rgb(struct flv1_struct *p, void *source_buffer, int size, void* rgbp);

#ifdef __cplusplus
}
#endif

#endif
