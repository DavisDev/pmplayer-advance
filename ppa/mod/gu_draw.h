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
gu routines
*/


#ifndef gu_draw_h
#define gu_draw_h


#define number_of_luminosity_boosts 16


#include <string.h>
#include <pspkernel.h>
#include <pspgu.h>
#include "aspect_ratio.h"
#include "common/texture_subdivision.h"
#include "common/m33sdk.h"
#include "movie_interface.h"
#include "subtitle_parse.h"
#include "gu_font.h"


struct vertex_struct
	{
	short texture_x;
	short texture_y;

	float vertex_x;
	float vertex_y;
	float vertex_z;
	};


extern unsigned int __attribute__((aligned(16))) pmp_gu_list[262144];

char info_string[512];
int info_count;

extern void *pmp_gu_draw_buffer;
extern void *pmp_gu_rgb_buffer;


void pmp_gu_init_previous_values();
void pmp_gu_start(int psp_type, int tv_aspectratio, int tv_overscan_left, int tv_overscan_top, int tv_overscan_right, int tv_overscan_bottom, int video_mode);
void pmp_gu_end();
void pmp_gu_wait();
void pmp_gu_draw(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer);

#endif
