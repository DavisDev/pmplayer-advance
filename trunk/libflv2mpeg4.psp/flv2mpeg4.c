/*
 * flv2mpeg4 lib for psp
 *
 * Copyright (c) 2010 cooleyes
 * eyes.cooleyes@gmail.com
 *
 * This lib base on VIXY FLV Converter  (http://sourceforge.net/projects/vixynet/)
 *
 *
 * This lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This lib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
 
#include "flv2mpeg4.h"
#include<stdio.h>
#include<stdlib.h>
#include <pspkernel.h>
#include "m4v.h"
#include "bitreader.h"
#include "bitwriter.h"
#include "flv.h"

#define	PACKETBUFFER_SIZE	(256*1024)

typedef struct _CONVCTX
{
	int width;
	int height;
	
	M4V_VOL vol;
	
	uint8* out_buf;

} CONVCTX;


static CONVCTX ctx;

#define VOL_TIME_BITS		5

////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
static const uint8 ff_mpeg4_y_dc_scale_table[32]={
//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
    0, 8, 8, 8, 8,10,12,14,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,34,36,38,40,42,44,46
};

static const uint8 ff_mpeg4_c_dc_scale_table[32]={
//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
    0, 8, 8, 8, 8, 9, 9,10,10,11,11,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,20,21,22,23,24,25
};


static void copy_vol(PICTURE* flv_pic, M4V_VOL* vol)
{
	vol->width = flv_pic->width;
	vol->height = flv_pic->height;
	vol->time_bits = VOL_TIME_BITS; // 0-31
}

static void copy_vop(PICTURE* flv_pic, M4V_VOP* vop, CONVCTX* c)
{
	vop->qscale = flv_pic->qscale;
	
	//vop->time = c->frame % 30;
	//vop->icount = (c->icounter + 29) / 30;
	vop->intra_dc_threshold = 99;
	
	if (flv_pic->picture_type == FLV_I_TYPE)
	{
		vop->picture_type = M4V_I_TYPE;
	}
	else
	{
		vop->picture_type = M4V_P_TYPE;
		vop->f_code = 1;
	}
}

static void copy_microblock(MICROBLOCK* flv_mb, M4V_MICROBLOCK* m4v_mb)
{
	int i;
	
	m4v_mb->dquant = flv_mb->dquant;
	memcpy(m4v_mb->block, flv_mb->block, sizeof(m4v_mb->block)); // !!!!!!!
	m4v_mb->intra = flv_mb->intra;
	m4v_mb->skip = flv_mb->skip;
	m4v_mb->mv_type = flv_mb->mv_type;
	
	memcpy(m4v_mb->mv_x, flv_mb->mv_x, sizeof(m4v_mb->mv_x)); // !!!!!!
	memcpy(m4v_mb->mv_y, flv_mb->mv_y, sizeof(m4v_mb->mv_y)); // !!!!!!

	// dc rescale
	if (m4v_mb->intra)
	{
		for (i = 0; i < 4; i++)
		{
			m4v_mb->block[i].block[0] *= 8;
			m4v_mb->block[i].block[0] /= ff_mpeg4_y_dc_scale_table[m4v_mb->qscale];
		}

		for (i = 4; i < 6; i++)
		{
			m4v_mb->block[i].block[0] *= 8;
			m4v_mb->block[i].block[0] /= ff_mpeg4_c_dc_scale_table[m4v_mb->qscale];
		}
	}
}

int open_flv2mpeg4_convert(int w, int h) {
	memset(&ctx, 0, sizeof(ctx));
	ctx.out_buf = (uint8*)memalign(64, PACKETBUFFER_SIZE);
	if ( ! ctx.out_buf )
		return -1;
	memset(ctx.out_buf, 0, PACKETBUFFER_SIZE);
	memset(&ctx.vol, 0, sizeof(ctx.vol));
	
	CONVCTX* c = &ctx;
	
	c->width = w;
	c->height = h;
	
	alloc_dcpred(&c->vol.dcpred, (c->width+15) / 16, (c->height+15) / 16);
	
	return 0;
}

void close_flv2mpeg4_convert() {
	CONVCTX* c = &ctx;
	free_dcpred(&c->vol.dcpred);
	if ( ctx.out_buf )
		free(ctx.out_buf);
	memset(&ctx, 0, sizeof(ctx));
}

void* convert_flv_frame_to_mpeg4_frame(void* flv_frame, int in_size, int* out_size) {
	
	CONVCTX* c = &ctx;
	
	memset(c->out_buf, 0, PACKETBUFFER_SIZE);
	//memset(&c->vol, 0, sizeof(ctx.vol));
	
	BR br;
	BW bw;
	
	init_br(&br, flv_frame, in_size);
	init_bw(&bw, c->out_buf, PACKETBUFFER_SIZE);
	
	PICTURE picture;
	PICTURE* flvpic = &picture;
	
	memset(flvpic, 0, sizeof(picture));
	
	init_dcpred(&c->vol.dcpred);
	
	if (decode_picture_header(&br, flvpic) < 0) return 0;
	copy_vol(flvpic, &c->vol);
	
	sceKernelDelayThread(50);
	
	m4v_encode_m4v_header(&bw, &c->vol, 0);
	
	sceKernelDelayThread(50);
	
	if ( c->width != flvpic->width || c->height != flvpic->height )
		return 0;
		
	//alloc_dcpred(&c->vol.dcpred, (c->width+15) / 16, (c->height+15) / 16);
	
	MICROBLOCK mb;
	M4V_VOP vop;
	M4V_MICROBLOCK m4v_mb;
	int x, y;
	int mb_width = (flvpic->width + 15) / 16;
	int mb_height = (flvpic->height + 15) / 16;

	memset(&vop, 0, sizeof(vop));

	copy_vop(flvpic, &vop, c);
	m4v_encode_vop_header(&bw, &vop, VOL_TIME_BITS, 0);
	
	sceKernelDelayThread(50);
			
	// transcode flv to mpeg4
	for (y = 0; y < mb_height; y++)
	{
		for (x = 0; x < mb_width; x++)
		{
			memset(&mb, 0, sizeof(mb));
			memset(&m4v_mb, 0, sizeof(m4v_mb));
			
			if (vop.picture_type == M4V_I_TYPE)
			{
				mb.intra = 1;
				if (decode_I_mb(&br, &mb, flvpic->escape_type, flvpic->qscale) < 0) return 0;
				m4v_mb.qscale = vop.qscale;
				copy_microblock(&mb, &m4v_mb);
				m4v_encode_I_dcpred(&m4v_mb, &c->vol.dcpred, x, y);
				m4v_encode_I_mb(&bw, &m4v_mb);
			}
			else
			{
				if (decode_P_mb(&br, &mb, flvpic->escape_type, flvpic->qscale) < 0) return 0;
				m4v_mb.qscale = vop.qscale;
				copy_microblock(&mb, &m4v_mb);
				m4v_encode_I_dcpred(&m4v_mb, &c->vol.dcpred, x, y);
				m4v_encode_P_mb(&bw, &m4v_mb);
			}
		}
		sceKernelDelayThread(50);
	}

	m4v_stuffing(&bw);
	flash_bw(&bw);
	
	//free_dcpred(&c->vol.dcpred);
	
	*out_size = bw.pos;
	return bw.buf;
}