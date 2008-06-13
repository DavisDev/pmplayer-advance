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
 
#ifndef __mp4v_decoder_h_cooleyes__
#define __mp4v_decoder_h_cooleyes__

#include <stdio.h>
#include <string.h>
#include <pspsdk.h>
#include "pspvideocodec.h"
#include "pspmpeg.h"
#include "common/mem64.h"
#include "mpegbase.h"

struct mp4v_struct
	{
	int      mpeg_init;
	unsigned long* codec_buffer;
	unsigned long* src_buffer;
	unsigned long* dest_buffer;
	void* mp4v_frame_buffer;
	int mp4v_decinfo_size;
	int mp4v_max_frame_size;
	};


#ifdef __cplusplus
extern "C" {
#endif

void mp4v_safe_constructor(struct mp4v_struct *p);
void mp4v_close(struct mp4v_struct *p);
char *mp4v_open(struct mp4v_struct *p);
char *mp4v_open_ex(struct mp4v_struct *p, void* mp4v_decinfo_buffer, int mp4v_decinfo_size, int mp4v_max_frame_size);
char *mp4v_get_rgb(struct mp4v_struct *p, void *source_buffer, int size, void* rgbp);

#ifdef __cplusplus
}
#endif

#endif
