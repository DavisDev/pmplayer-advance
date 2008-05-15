/* 
 *	Copyright (C) 2008 cooleyes
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


#include "mp4avcdecoder.h"
#include <stdlib.h>
#include <stdio.h>

void mp4_avc_safe_constructor(struct mp4_avc_struct *p) {
	p->mpeg_init = -1;
	p->mpeg_create = -1;
	p->mpeg_buffer = 0;
	p->mpeg_au = 0;
	p->mpeg_ddrtop = 0;
	p->mpeg_sps_pps_buffer = 0;
	p->mpeg_detail2 = 0;
	p->mpeg_pic_num = 0;
}


void mp4_avc_close(struct mp4_avc_struct *p) {
	
	if (!(p->mpeg_create != 0)) 
		sceMpegDelete(&p->mpeg);

	if (!(p->mpeg_init != 0)) 
		sceMpegFinish();

	if (p->mpeg_buffer != 0) 
		free_64(p->mpeg_buffer);
	
	if (p->mpeg_au != 0) 
		free_64(p->mpeg_buffer);
	
	if (p->mpeg_ddrtop != 0) 
		free_64(p->mpeg_ddrtop);
	
	if (p->mpeg_sps_pps_buffer != 0) 
		free_64(p->mpeg_sps_pps_buffer);

	mp4_avc_safe_constructor(p);
}


char *mp4_avc_open(struct mp4_avc_struct *p, int mpeg_mode, void* sps_buffer, int sps_size, void* pps_buffer, int pps_size, int nal_prefix_size) {
	mp4_avc_safe_constructor(p);
	
	p->mpeg_sps_size = sps_size;
	p->mpeg_pps_size = pps_size;
	p->mpeg_nal_prefix_size = nal_prefix_size;
	p->mpeg_sps_pps_buffer = malloc_64(sps_size + pps_size);
	if ( p->mpeg_sps_pps_buffer == 0 ) {
		mp4_avc_close(p);
		return("mp4_avc_open: malloc_64 failed on mpeg_sps_pps_buffer");
	}
	memcpy(p->mpeg_sps_pps_buffer, sps_buffer, sps_size);
	memcpy(p->mpeg_sps_pps_buffer+sps_size, pps_buffer, pps_size);

	p->mpeg_init = sceMpegInit();
	if (p->mpeg_init != 0) {
		mp4_avc_close(p);
		return("mp4_avc_open: sceMpegInit failed");
	}
	
	p->mpeg_mode = mpeg_mode;
	p->mpeg_buffer_size = sceMpegQueryMemSize(p->mpeg_mode);
	if (p->mpeg_buffer_size < 0) {
		mp4_avc_close(p);
		return("mp4_avc_open: sceMpegQueryMemSize failed");
	}
	
	p->mpeg_buffer = malloc_64(p->mpeg_buffer_size);
	if (p->mpeg_buffer == 0) {
		mp4_avc_close(p);
		return("mp4_avc_open: malloc_64 failed on mpeg_buffer");
	}
	
	p->mpeg_ddrtop =  memalign(0x400000, 0x200000);
	if (p->mpeg_ddrtop == 0) {
		mp4_avc_close(p);
		return("mp4_avc_open: memalign(0x400000, 0x200000) failed on mpeg_ddrtop");
	}
	p->mpeg_au_buffer = p->mpeg_ddrtop + 0x10000;

	p->mpeg_create = sceMpegCreate(&p->mpeg, p->mpeg_buffer, p->mpeg_buffer_size, &p->mpeg_ringbuffer, 512, p->mpeg_mode, p->mpeg_ddrtop);
	if (p->mpeg_create != 0) {
		mp4_avc_close(p);
		return("mp4_avc_open: sceMpegCreate failed");
	}
	
	p->mpeg_au = (SceMpegAu*)malloc_64(64);
	if (p->mpeg_au == 0) {
		mp4_avc_close(p);
		return("mp4_avc_open: malloc_64 failed on mpeg_au");
	}
	memset(p->mpeg_au, 0xFF, 64);
	
	if ( sceMpegInitAu(&p->mpeg, p->mpeg_au_buffer, p->mpeg_au) != 0 ){
		mp4_avc_close(p);
		return("mp4_avc_open: sceMpegInitAu failed");
	}

	return(0);
}
	

char *mp4_avc_get(struct mp4_avc_struct *p, int mode, void *source_buffer, int size, void *destination_buffer, int* pic_num) {
	
	Mp4AvcNalStruct nal;
	nal.sps_buffer = p->mpeg_sps_pps_buffer;
	nal.sps_size = p->mpeg_sps_size;
	nal.pps_buffer = p->mpeg_sps_pps_buffer+p->mpeg_sps_size;
	nal.pps_size = p->mpeg_pps_size;
	nal.nal_prefix_size = p->mpeg_nal_prefix_size;
	nal.nal_buffer = source_buffer;
	nal.nal_size = size ;
	nal.mode = mode;
	
	if ( sceMpegGetAvcNalAu(&p->mpeg, &nal, p->mpeg_au) != 0 ) {
		return("avc_get: sceMpegGetAvcNalAu failed");
	}
	
	if ( sceMpegAvcDecode(&p->mpeg, p->mpeg_au, 512, 0, &p->mpeg_pic_num) != 0 ) {
		return("avc_get: sceMpegAvcDecode failed");
	}
	
	if ( sceMpegAvcDecodeDetail2(&p->mpeg, &p->mpeg_detail2) != 0 ) {
		return("avc_get: sceMpegAvcDecodeDetail2 failed");
	}
	if ( p->mpeg_pic_num > 0 ) {
		Mp4AvcCscStruct csc;
		csc.height = (p->mpeg_detail2->info_buffer->height+15) >> 4;
		csc.width = (p->mpeg_detail2->info_buffer->width+15) >> 4;
		csc.mode0 = 0;
		csc.mode1 = 0;
		csc.buffer0 = p->mpeg_detail2->yuv_buffer->buffer0 ;
		csc.buffer1 = p->mpeg_detail2->yuv_buffer->buffer1 ;
		csc.buffer2 = p->mpeg_detail2->yuv_buffer->buffer2 ;
		csc.buffer3 = p->mpeg_detail2->yuv_buffer->buffer3 ;
		csc.buffer4 = p->mpeg_detail2->yuv_buffer->buffer4 ;
		csc.buffer5 = p->mpeg_detail2->yuv_buffer->buffer5 ;
		csc.buffer6 = p->mpeg_detail2->yuv_buffer->buffer6 ;
		csc.buffer7 = p->mpeg_detail2->yuv_buffer->buffer7 ;
		if ( sceMpegBaseCscAvc(destination_buffer, 0, 512, &csc) != 0 ) {
			return("avc_get: sceMpegBaseCscAvc failed");
		}
	}
	*pic_num = p->mpeg_pic_num;
	
	return(0);
}

char *mp4_avc_get_cache(struct mp4_avc_struct *p, void *destination_buffer, int pic_num) {
	Mp4AvcInfoStruct* info_buffer = p->mpeg_detail2->info_buffer + (p->mpeg_pic_num-pic_num);
	Mp4AvcYuvStruct* yuv_buffer = p->mpeg_detail2->yuv_buffer + (p->mpeg_pic_num-pic_num);
	Mp4AvcCscStruct csc;
	csc.height = (info_buffer->height+15) >> 4;
	csc.width = (info_buffer->width+15) >> 4;
	csc.mode0 = 0;
	csc.mode1 = 0;
	csc.buffer0 = yuv_buffer->buffer0 ;
	csc.buffer1 = yuv_buffer->buffer1 ;
	csc.buffer2 = yuv_buffer->buffer2 ;
	csc.buffer3 = yuv_buffer->buffer3 ;
	csc.buffer4 = yuv_buffer->buffer4 ;
	csc.buffer5 = yuv_buffer->buffer5 ;
	csc.buffer6 = yuv_buffer->buffer6 ;
	csc.buffer7 = yuv_buffer->buffer7 ;
	if ( sceMpegBaseCscAvc(destination_buffer, 0, 512, &csc) != 0 ) {
		return("avc_get: sceMpegBaseCscAvc failed");
	}
	return (0);
}


