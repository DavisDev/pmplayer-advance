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
 
#include "flv1decoder.h"

void flv1_safe_constructor(struct flv1_struct *p){
	p->mpeg_init = -1;
	p->jpeg_init = -1;
	p->dest_buffer = 0;
	p->avcodec = 0;
	p->avcodec_context = 0;
	p->avframe = 0;
};

void flv1_close(struct flv1_struct *p) {
	
	if (p->dest_buffer != 0) free_64(p->dest_buffer);
	
	if (!(p->jpeg_init != 0)) sceJpegFinishMJpeg();
	if (!(p->mpeg_init != 0)) sceMpegFinish();
	
	if (p->avcodec_context != 0) {
		avcodec_close(p->avcodec_context);
		av_free(p->avcodec_context);
	}

	if (p->avframe != 0) av_free(p->avframe);
	
	flv1_safe_constructor(p);
};

char *flv1_open(struct flv1_struct *p, unsigned int width, unsigned int height){
	flv1_safe_constructor(p);
	
	p->mpeg_init = sceMpegInit();
	if (p->mpeg_init != 0) {
		flv1_close(p);
		return("flv1_open: sceMpegInit failed");
	}
	
	sceMpegBaseCscInit(512);
	
	p->jpeg_init = sceJpegInitMJpeg();
	if (p->jpeg_init != 0) {
		flv1_close(p);
		return("flv1_open: sceJpegInitMJpeg failed");
	}
	
	p->dest_buffer = malloc_64(96);
	if (p->dest_buffer == 0){
		flv1_close(p);
		return("flv1_open: malloc_64 failed on dest_buffer");
	}
	
	avcodec_register_all();
	
	p->avcodec = avcodec_find_decoder(CODEC_ID_FLV1);
	if( p->avcodec == 0 ) {
		flv1_close(p);
		return("flv1_open: avcodec_find_decoder fail");
	}
	
	p->avcodec_context = avcodec_alloc_context();
	if (p->avcodec_context == 0) {
		flv1_close(p);
		return("flv1_open: avcodec_alloc_context fail");
	}
	
	p->avcodec_context->width = width;
	p->avcodec_context->height = height;
	
	if (avcodec_open(p->avcodec_context, p->avcodec) < 0) {
		av_free(p->avcodec_context);
    	p->avcodec_context = 0;
    	flv1_close(p);
		return("flv1_open: avcodec_open fail");
    }
    
    p->avframe = avcodec_alloc_frame();
    if (p->avframe == 0) {
    	flv1_close(p);
		return("flv1_open: avcodec_alloc_frame fail");
    }
	
	
	return(0);
};

char *flv1_get_rgb(struct flv1_struct *p, void *source_buffer, int size, void* rgbp) {
	
	int got_frame;
	if (avcodec_decode_video(p->avcodec_context, p->avframe, &got_frame, source_buffer, size) != size){
		return("flv1_get: avcodec_decode_video failed");
	}
	
	if ( got_frame == 0 ) {
		return("flv1_get: avcodec_decode_video failed");
	}
	
	int res;
	unsigned long height;
	height = (p->avcodec_context->height+15) & 0xFFFFFFF0;

	p->dest_buffer[0] = (p->avcodec_context->height+15) >> 4;
	p->dest_buffer[1] = (p->avframe->linesize[0]+15) >> 4;
	p->dest_buffer[2] = 0;
	p->dest_buffer[3] = 1;
	p->dest_buffer[4] = p->avframe->data[0];
	p->dest_buffer[5] = p->dest_buffer[4] + (p->avframe->linesize[0] * (height >> 1)); 
	p->dest_buffer[6] = p->avframe->data[1];
	p->dest_buffer[7] = p->avframe->data[2];
	p->dest_buffer[8] = p->dest_buffer[6] + (p->avframe->linesize[1] * (height >> 2));
	p->dest_buffer[9] = p->dest_buffer[7] + (p->avframe->linesize[2] * (height >> 2));
	p->dest_buffer[10] = p->avcodec_context->height;
	p->dest_buffer[11] = p->avcodec_context->width;
	p->dest_buffer[12] = 512;
	
	if ( (res = sceMpegBaseCscVme(rgbp, rgbp+ (((p->dest_buffer[0]<<4)>>1)<<11), 512, p->dest_buffer)) < 0 ) {
		return("flv1_get: sceMpegBaseCscVme failed");
	}
	
	return(0);
};
