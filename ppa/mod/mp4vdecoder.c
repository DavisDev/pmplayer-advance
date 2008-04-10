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
 
#include "mp4vdecoder.h"

void mp4v_safe_constructor(struct mp4v_struct *p){
	p->mpeg_init = -1;
	p->codec_buffer = 0;
	p->src_buffer = 0;
	p->dest_buffer = 0;
};

void mp4v_close(struct mp4v_struct *p) {
	
	if (p->src_buffer != 0) free_64(p->src_buffer);
	if (p->dest_buffer != 0) free_64(p->dest_buffer);
	if (p->codec_buffer != 0) {
		sceVideocodecStop(p->codec_buffer, 0x1);
		sceVideocodecDelete(p->codec_buffer, 0x1);
		sceVideocodecReleaseEDRAM(p->codec_buffer);
		free_64(p->codec_buffer);
	}
	
	if (!(p->mpeg_init != 0)) sceMpegFinish();
	
	mp4v_safe_constructor(p);
};

char *mp4v_open(struct mp4v_struct *p){
	mp4v_safe_constructor(p);
	
	p->mpeg_init = sceMpegInit();
	if (p->mpeg_init != 0) {
		mp4v_close(p);
		return("mp4v_open: sceMpegInit failed");
	}
	
	sceMpegBaseCscInit(512);
	
	p->codec_buffer = malloc_64(384);
	if (p->codec_buffer == 0){
		mp4v_close(p);
		return("mp4v_open: malloc_64 failed on codec_buffer");
	}
	
	p->src_buffer = malloc_64(96);
	if (p->src_buffer == 0){
		mp4v_close(p);
		return("mp4v_open: malloc_64 failed on src_buffer");
	}
	
	p->dest_buffer = malloc_64(96);
	if (p->dest_buffer == 0){
		mp4v_close(p);
		return("mp4v_open: malloc_64 failed on dest_buffer");
	}
	
	memset(p->codec_buffer, 0, 384);
	
	int res ;
	
	p->codec_buffer[4] = (unsigned long)(((void*)(p->codec_buffer)) + 128 );
	p->codec_buffer[11] = 512;
	p->codec_buffer[12] = 512;
	p->codec_buffer[13] = 512*512;
	
	if ( (res = sceVideocodecOpen(p->codec_buffer, 0x1)) < 0 ) {
		mp4v_close(p);
		return("mp4v_open: sceVideocodecOpen failed");
	}
	
	p->codec_buffer[7] = 16384;
	
	if ( (res = sceVideocodecGetEDRAM(p->codec_buffer, 0x1)) < 0 ) {
		mp4v_close(p);
		return("mp4v_open: sceVideocodecGetEDRAM failed");
	}
	
	if ( (res = sceVideocodecInit(p->codec_buffer, 0x1)) < 0 ) {
		mp4v_close(p);
		return("mp4v_open: sceVideocodecInit failed");
	}
	
	p->codec_buffer[34] = 7;
	p->codec_buffer[36] = 0;
	
	if ( (res = sceVideocodecStop(p->codec_buffer, 0x1)) < 0 ) {
		mp4v_close(p);
		return("mp4v_open: sceVideocodecStop failed");
	}
	
	return(0);
};

char *mp4v_get_rgb(struct mp4v_struct *p, void *source_buffer, int size, void* rgbp) {
	int res;
	
	p->codec_buffer[9] = source_buffer;//MEMP4VBUF;
	p->codec_buffer[10] = size;
	p->codec_buffer[14] = 7;
	if ( (res = sceVideocodecDecode(p->codec_buffer, 0x1)) < 0 ) {
		return("mp4v_get: sceVideocodecDecode failed");
	}
	
	unsigned long height;
	height = (p->codec_buffer[45]+15) & 0xFFFFFFF0;
	
	
	p->dest_buffer[0] = (p->codec_buffer[45]+15) >> 4;
	p->dest_buffer[1] = (p->codec_buffer[56]+15) >> 4;
	p->dest_buffer[2] = 0;
	p->dest_buffer[3] = 1;
	p->dest_buffer[4] = p->codec_buffer[53];
	p->dest_buffer[5] = p->dest_buffer[4] + (p->codec_buffer[56] * (height >> 1)); 
	p->dest_buffer[6] = p->codec_buffer[54];
	p->dest_buffer[7] = p->codec_buffer[55];
	p->dest_buffer[8] = p->dest_buffer[6] + (p->codec_buffer[57] * (height >> 2));
	p->dest_buffer[9] = p->dest_buffer[7] + (p->codec_buffer[58] * (height >> 2));
	p->dest_buffer[10] = p->codec_buffer[45];
	p->dest_buffer[11] = p->codec_buffer[44];
	p->dest_buffer[12] = 512;
	
	if ( (res = sceMpegBaseCscVme(rgbp, rgbp+ (((p->dest_buffer[0]<<4)>>1)<<11), 512, p->dest_buffer)) < 0 ) {
		return("mp4v_get: sceMpegBaseCscVme failed");
	}
	
	return(0);
};

char *mp4v_get_yuv(struct mp4v_struct *p, void *source_buffer, int size, void* y, void* u, void* v, int* y_width, int* u_width, int* v_width, int* height) {
	int res;
	p->codec_buffer[9] = source_buffer;
	p->codec_buffer[10] = size;
	p->codec_buffer[14] = 7;
	if ( (res = sceVideocodecDecode(p->codec_buffer, 0x1)) < 0 ) {
		return("mp4v_get: sceVideocodecDecode failed");
	}
	
	p->src_buffer[0] = (p->codec_buffer[45]+15) & 0xFFFFFFF0;
	p->src_buffer[1] = (p->codec_buffer[44]+15) & 0xFFFFFFF0;
	p->src_buffer[2] = 0;
	p->src_buffer[3] = 1;
	p->src_buffer[4] = p->codec_buffer[53];
	p->src_buffer[5] = p->src_buffer[4] + (p->codec_buffer[56] * (p->src_buffer[0] >> 1)); 
	p->src_buffer[6] = p->codec_buffer[54];
	p->src_buffer[7] = p->codec_buffer[55];
	p->src_buffer[8] = p->src_buffer[6] + (p->codec_buffer[57] * (p->src_buffer[0] >> 2));
	p->src_buffer[9] = p->src_buffer[7] + (p->codec_buffer[58] * (p->src_buffer[0] >> 2));
	p->src_buffer[10] = 0;
	p->src_buffer[11] = 0;
	
	p->dest_buffer[0] = (p->codec_buffer[45]+15) >> 4;
	p->dest_buffer[1] = (p->codec_buffer[56]+15) >> 4;
	p->dest_buffer[4] = y;
	p->dest_buffer[5] = (unsigned long)(((void*)y) + (p->codec_buffer[56] * (p->src_buffer[0] >> 1)) );
	p->dest_buffer[6] = u;
	p->dest_buffer[7] = v;
	p->dest_buffer[8] = (unsigned long)(((void*)u) + (p->codec_buffer[57] * (p->src_buffer[0] >> 2)) );
	p->dest_buffer[9] = (unsigned long)(((void*)v) + (p->codec_buffer[58] * (p->src_buffer[0] >> 2)) );
	p->dest_buffer[10] = p->codec_buffer[45];
	p->dest_buffer[11] = p->codec_buffer[44];
	p->dest_buffer[12] = 512;
	
	*y_width = p->codec_buffer[56];
	*u_width = p->codec_buffer[57];
	*v_width = p->codec_buffer[58];
	*height = p->src_buffer[0];
	
	if ( (res = sceMpegBaseYCrCbCopyVme(p->dest_buffer, p->src_buffer, 3)) < 0 ) {
		return("mp4v_get: sceMpegBaseYCrCbCopyVme failed");
	}
	
	return(0);
}
