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


#include "gu_draw.h"
#include <psprtc.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int __attribute__((aligned(16))) pmp_gu_list[262144];


static unsigned char __attribute__((aligned(64))) luminosity_textures[64 * number_of_luminosity_boosts] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0};

typedef void (*f_pmp_gu_draw)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, void *);

void *pmp_gu_draw_buffer;
void *pmp_gu_rgb_buffer;

static unsigned int previous_aspect_ratio;
static unsigned int previous_zoom;
static unsigned int previous_subtitle;
static unsigned int previous_info;
static unsigned int previous_interface;
static unsigned int output_left;
static unsigned int output_top;
static unsigned int output_width;
static unsigned int output_height;
static f_pmp_gu_draw p_pmp_gu_draw;

void pmp_gu_draw_without_tvout_supported(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer);
void pmp_gu_draw_psplcd(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer);
void pmp_gu_draw_tvout_interlace(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer);
void pmp_gu_draw_tvout_progressive(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer);

void pmp_gu_init_previous_values()
	{
	previous_aspect_ratio = 0xffffffff;
	previous_zoom         = 0xffffffff;
	previous_subtitle     = 1;
	previous_info         = 1;
	previous_interface    = 0;
	}


void pmp_gu_end()
	{
	sceGuStart(GU_DIRECT, pmp_gu_list);
	sceGuClearColor(0);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);
	}


void pmp_gu_start_without_tvout_supported()
	{
	p_pmp_gu_draw = pmp_gu_draw_without_tvout_supported;

	pmp_gu_draw_buffer  = (void *) 0x04000000;
	pmp_gu_rgb_buffer  = pmp_gu_draw_buffer + (4 * 512 * 272);

	memset(pmp_gu_rgb_buffer, 0, 4 * 768 * 480);
	sceKernelDcacheWritebackInvalidateAll();
	}

void pmp_gu_start_psplcd()
	{
	p_pmp_gu_draw = pmp_gu_draw_psplcd;

	pmp_gu_draw_buffer  = (void *) (0x04000000);
	pmp_gu_rgb_buffer  = (void*)0x0a000000;

	memset(pmp_gu_rgb_buffer, 0, 4 * 768 * 480);
	sceKernelDcacheWritebackInvalidateAll();
	}

void pmp_gu_start_tvout_interlace()
	{
	p_pmp_gu_draw = pmp_gu_draw_tvout_interlace;

	pmp_gu_draw_buffer  = (void *) (0x04000000);
	pmp_gu_rgb_buffer  = (void*)0x0a000000;

	memset(pmp_gu_rgb_buffer, 0, 4 * 768 * 480);
	sceKernelDcacheWritebackInvalidateAll();
	}

void pmp_gu_start_tvout_progressive()
	{
	p_pmp_gu_draw = pmp_gu_draw_tvout_progressive;

	pmp_gu_draw_buffer  = (void *) (0x04000000);
	pmp_gu_rgb_buffer  = (void*)0x0a000000;

	memset(pmp_gu_rgb_buffer, 0, 4 * 512 * 512);
	sceKernelDcacheWritebackInvalidateAll();
	}

void pmp_gu_start(int psp_type, int tv_aspectratio, int tv_overscan_left, int tv_overscan_top, int tv_overscan_right, int tv_overscan_bottom, int video_mode)
	{
	sceGuStart(GU_DIRECT, pmp_gu_list);
	sceGuClearColor(0);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);
	if (!m33IsTVOutSupported(psp_type))
		{
		output_left = 0;
		output_top = 0;
		output_width = 480;
		output_height = 272;
		gu_font_output_set(output_left, output_top, output_width, output_height);
		pmp_gu_start_without_tvout_supported();
		}
	else
		{
			if (video_mode == 0 )
				{				
				output_left = 0;
				output_top = 0;
				output_width = 480;
				output_height = 272;
				gu_font_output_set(output_left, output_top, output_width, output_height);
				pmp_gu_start_psplcd();
				}
			else
				{
				output_left = tv_overscan_left;
				output_top = tv_overscan_top;
				output_width = 720 - tv_overscan_left - tv_overscan_right;
				output_height = 480 - tv_overscan_top - tv_overscan_bottom;
				gu_font_output_set(output_left, output_top, output_width, output_height);
				if (video_mode == 1 || video_mode == 2)
					{
					pmp_gu_start_tvout_interlace();
					}
				else
					{
					pmp_gu_start_tvout_progressive();
					}
				}
		}
	}

void pmp_gu_wait()
	{
	sceGuSync(0, 0);
	}


static void pmp_gu_load(void *image, int filter)
	{
	sceGuDisable(GU_BLEND);
	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexImage(0, 512, 512, 512, image);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
	sceGuTexFilter(filter, filter);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	}


static void pmp_gu_load_luminosity_texture(void *image)
	{
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0xffffff, 0xffffff);
	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexImage(0, 4, 4, 4, image);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	}

static void pmp_gu_load_interface_texture(void *image)
	{
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexImage(0, 512, 512, 512, image);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	}

static void pmp_gu_draw_sprite(struct texture_subdivision_struct *t)
	{
	struct vertex_struct *v = sceGuGetMemory(2 * sizeof(struct vertex_struct));

	v[0].texture_x = t->output_texture_x_start;
	v[0].texture_y = t->output_texture_y_start;
	v[0].vertex_x  = (int) t->output_vertex_x_start;
	v[0].vertex_y  = t->output_vertex_y_start;
	v[0].vertex_z  = 0.0;

	v[1].texture_x = t->output_texture_x_end;
	v[1].texture_y = t->output_texture_y_end;
	v[1].vertex_x  = (int) t->output_vertex_x_end;
	v[1].vertex_y  = t->output_vertex_y_end;
	v[1].vertex_z  = 0.0;

	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, v);
	}


void pmp_gu_draw(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer)
	{
	p_pmp_gu_draw(aspect_ratio, zoom, luminosity_boost, show_interface, show_subtitle, subtitle_format, frame_number, video_frame_buffer);
	}

void pmp_gu_draw_without_tvout_supported(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer)
	{
	short texture_width  = aspect_ratios[0].width;
	short texture_height = aspect_ratios[0].height;

	int vertex_width  = aspect_ratios[aspect_ratio].psp_width;
	int vertex_height = aspect_ratios[aspect_ratio].psp_height;

	vertex_width  = zoom * vertex_width  / 100;
	vertex_height = zoom * vertex_height / 100;

	int vertex_x = output_left + (output_width >> 1) - (vertex_width  >> 1);
	int vertex_y = output_top + (output_height >> 1) - (vertex_height >> 1);

	int filter;
	if ((texture_width == vertex_width) && (texture_height == vertex_height))
		{
		filter = GU_NEAREST;
		}
	else
		{
		filter = GU_LINEAR;
		}
		
		
	sceGuStart(GU_DIRECT, pmp_gu_list);
	if ((previous_aspect_ratio != aspect_ratio) || (previous_zoom != zoom) || (previous_interface != show_interface) ||
	    ((vertex_width < output_width || vertex_height < output_height) && (previous_subtitle || previous_info)))
		{
		sceGuClearColor(0);
		sceGuClear(GU_COLOR_BUFFER_BIT);

		previous_aspect_ratio = aspect_ratio;
		previous_zoom         = zoom;
		previous_subtitle     = 0;
		previous_info         = 0;
		previous_interface = show_interface;
		}



	struct texture_subdivision_struct texture_subdivision;


	pmp_gu_load(pmp_gu_rgb_buffer, filter);
	texture_subdivision_constructor(&texture_subdivision, texture_width, texture_height, 16, vertex_width, vertex_height, vertex_x, vertex_y);
	do
		{
		texture_subdivision_get(&texture_subdivision);
		pmp_gu_draw_sprite(&texture_subdivision);
		}
	while (texture_subdivision.output_last == 0);
	



	pmp_gu_load_luminosity_texture(luminosity_textures + 64 * luminosity_boost);
	texture_subdivision_constructor(&texture_subdivision, 4, 4, 16, vertex_width, vertex_height, vertex_x, vertex_y);
	do
		{
		texture_subdivision_get(&texture_subdivision);
		pmp_gu_draw_sprite(&texture_subdivision);
		}
	while (texture_subdivision.output_last == 0);
	
	if (info_count)
		{
		info_count--;
		gu_font_print( (476-gu_font_width_get(info_string,0)), 2, 0, info_string );
		previous_info = 1;
		}
	
	if (show_subtitle )
		{
		int flags = 0;
		int pwidth = 0;

		struct subtitle_frame_struct *frame = 0;
		if (subtitle_parse_get_frame(&subtitle_parser[show_subtitle-1], &frame, frame_number)==0)
			{
				if (frame!=0)
					{
					if ((subtitle_format==1) && (gufont_haveflags&GU_FONT_HAS_UNICODE_CHARMAP))
						{
						flags = FLAG_UTF8;
						pwidth = gu_font_utf8_width_get(frame->p_string,0);
						}
					else
						pwidth = gu_font_width_get(frame->p_string,0);
					unsigned int sub_distance = gu_font_distance_get();
					if ( gu_font_align_get() > 0 ) 
						gu_font_print( (480-pwidth)/2, 272-(frame->p_num_lines)*gu_font_height()-sub_distance, flags | FLAG_ALIGN_CENTER, frame->p_string);
					else
						gu_font_print( (480-pwidth)/2, sub_distance, flags | FLAG_ALIGN_CENTER, frame->p_string);
					//gu_font_print( (480-pwidth)/2, gu_font_height(), flags | FLAG_ALIGN_CENTER, frame->p_string);
					
					previous_subtitle = 1;//(pwidth<=vertex_width?(vertex_height<272?1:0):1);
					}
			}
		}
	
	if (show_interface)
		{
		pmp_gu_load_interface_texture(background_8888);
		texture_subdivision_constructor(&texture_subdivision, 480, interface_height, 16, output_width, interface_height*output_height/272, output_left, output_top);
		do
			{
			texture_subdivision_get(&texture_subdivision);
			pmp_gu_draw_sprite(&texture_subdivision);
			}
		while (texture_subdivision.output_last == 0);
		}
	
	sceGuCopyImage(GU_PSM_8888, 0, 0, 480, 272, 512, pmp_gu_draw_buffer, 0, 0, 512, video_frame_buffer);
	sceGuTexSync();
	
	sceGuFinish();
	}

void pmp_gu_draw_psplcd(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer)
	{
	short texture_width  = aspect_ratios[0].width;
	short texture_height = aspect_ratios[0].height;

	int vertex_width  = aspect_ratios[aspect_ratio].psp_width;
	int vertex_height = aspect_ratios[aspect_ratio].psp_height;

	vertex_width  = zoom * vertex_width  / 100;
	vertex_height = zoom * vertex_height / 100;

	int vertex_x = output_left + (output_width >> 1) - (vertex_width  >> 1);
	int vertex_y = output_top + (output_height >> 1) - (vertex_height >> 1);

	int filter;
	if ((texture_width == vertex_width) && (texture_height == vertex_height))
		{
		filter = GU_NEAREST;
		}
	else
		{
		filter = GU_LINEAR;
		}
		
		
	sceGuStart(GU_DIRECT, pmp_gu_list);
	if ((previous_aspect_ratio != aspect_ratio) || (previous_zoom != zoom) || (previous_interface != show_interface) ||
	    ((vertex_width < output_width || vertex_height < output_height) && (previous_subtitle || previous_info)))
		{
		sceGuClearColor(0);
		sceGuClear(GU_COLOR_BUFFER_BIT);

		previous_aspect_ratio = aspect_ratio;
		previous_zoom         = zoom;
		previous_subtitle     = 0;
		previous_info         = 0;
		previous_interface = show_interface;
		}



	struct texture_subdivision_struct texture_subdivision;


	pmp_gu_load(pmp_gu_rgb_buffer, filter);
	texture_subdivision_constructor(&texture_subdivision, texture_width, texture_height, 16, vertex_width, vertex_height, vertex_x, vertex_y);
	do
		{
		texture_subdivision_get(&texture_subdivision);
		pmp_gu_draw_sprite(&texture_subdivision);
		}
	while (texture_subdivision.output_last == 0);
	



	pmp_gu_load_luminosity_texture(luminosity_textures + 64 * luminosity_boost);
	texture_subdivision_constructor(&texture_subdivision, 4, 4, 16, vertex_width, vertex_height, vertex_x, vertex_y);
	do
		{
		texture_subdivision_get(&texture_subdivision);
		pmp_gu_draw_sprite(&texture_subdivision);
		}
	while (texture_subdivision.output_last == 0);
	
	if (info_count)
		{
		info_count--;
		gu_font_print( (476-gu_font_width_get(info_string,0)), 2, 0, info_string );
		previous_info = 1;
		}
	
	if (show_subtitle )
		{
		int flags = 0;
		int pwidth = 0;

		struct subtitle_frame_struct *frame = 0;
		if (subtitle_parse_get_frame(&subtitle_parser[show_subtitle-1], &frame, frame_number)==0)
			{
				if (frame!=0)
					{
					if ((subtitle_format==1) && (gufont_haveflags&GU_FONT_HAS_UNICODE_CHARMAP))
						{
						flags = FLAG_UTF8;
						pwidth = gu_font_utf8_width_get(frame->p_string,0);
						}
					else
						pwidth = gu_font_width_get(frame->p_string,0);
					unsigned int sub_distance = gu_font_distance_get();
					if ( gu_font_align_get() > 0 ) 
						gu_font_print( (480-pwidth)/2, 272-(frame->p_num_lines)*gu_font_height()-sub_distance, flags | FLAG_ALIGN_CENTER, frame->p_string);
					else
						gu_font_print( (480-pwidth)/2, sub_distance, flags | FLAG_ALIGN_CENTER, frame->p_string);
					//gu_font_print( (480-pwidth)/2, gu_font_height(), flags | FLAG_ALIGN_CENTER, frame->p_string);
					
					previous_subtitle = 1;//(pwidth<=vertex_width?(vertex_height<272?1:0):1);
					}
			}
		}
	
	if (show_interface)
		{
		pmp_gu_load_interface_texture(background_8888);
		texture_subdivision_constructor(&texture_subdivision, 480, interface_height, 16, output_width, interface_height*output_height/272, output_left, output_top);
		do
			{
			texture_subdivision_get(&texture_subdivision);
			pmp_gu_draw_sprite(&texture_subdivision);
			}
		while (texture_subdivision.output_last == 0);
		}
		
	sceGuCopyImage(GU_PSM_8888, 0, 0, 480, 272, 768, pmp_gu_draw_buffer, 0, 0, 768, video_frame_buffer);
	sceGuTexSync();
	
	sceGuFinish();
	
	}

void pmp_gu_draw_tvout_interlace(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer)
	{
	short texture_width  = aspect_ratios[0].width;
	short texture_height = aspect_ratios[0].height;

	int vertex_width  = aspect_ratios[aspect_ratio].psp_width;
	int vertex_height = aspect_ratios[aspect_ratio].psp_height;

	vertex_width  = zoom * vertex_width  / 100;
	vertex_height = zoom * vertex_height / 100;

	int vertex_x = output_left + (output_width >> 1) - (vertex_width  >> 1);
	int vertex_y = output_top + (output_height >> 1) - (vertex_height >> 1);

	int filter;
	if ((texture_width == vertex_width) && (texture_height == vertex_height))
		{
		filter = GU_NEAREST;
		}
	else
		{
		filter = GU_LINEAR;
		}
		
		
	sceGuStart(GU_DIRECT, pmp_gu_list);
	if ((previous_aspect_ratio != aspect_ratio) || (previous_zoom != zoom) || (previous_interface != show_interface) ||
	    ((vertex_width < output_width || vertex_height < output_height) && (previous_subtitle || previous_info)))
		{
		sceGuClearColor(0);
		sceGuClear(GU_COLOR_BUFFER_BIT);

		previous_aspect_ratio = aspect_ratio;
		previous_zoom         = zoom;
		previous_subtitle     = 0;
		previous_info         = 0;
		previous_interface = show_interface;
		}



	struct texture_subdivision_struct texture_subdivision;


	pmp_gu_load(pmp_gu_rgb_buffer, filter);
	texture_subdivision_constructor(&texture_subdivision, texture_width, texture_height, 16, vertex_width, vertex_height, vertex_x, vertex_y);
	do
		{
		texture_subdivision_get(&texture_subdivision);
		pmp_gu_draw_sprite(&texture_subdivision);
		}
	while (texture_subdivision.output_last == 0);
	



	pmp_gu_load_luminosity_texture(luminosity_textures + 64 * luminosity_boost);
	texture_subdivision_constructor(&texture_subdivision, 4, 4, 16, vertex_width, vertex_height, vertex_x, vertex_y);
	do
		{
		texture_subdivision_get(&texture_subdivision);
		pmp_gu_draw_sprite(&texture_subdivision);
		}
	while (texture_subdivision.output_last == 0);
	
	if (info_count)
		{
		info_count--;
		gu_font_print( (476-gu_font_width_get(info_string,0)), 2, 0, info_string );
		previous_info = 1;
		}
	
	if (show_subtitle )
		{
		int flags = 0;
		int pwidth = 0;

		struct subtitle_frame_struct *frame = 0;
		if (subtitle_parse_get_frame(&subtitle_parser[show_subtitle-1], &frame, frame_number)==0)
			{
				if (frame!=0)
					{
					if ((subtitle_format==1) && (gufont_haveflags&GU_FONT_HAS_UNICODE_CHARMAP))
						{
						flags = FLAG_UTF8;
						pwidth = gu_font_utf8_width_get(frame->p_string,0);
						}
					else
						pwidth = gu_font_width_get(frame->p_string,0);
					unsigned int sub_distance = gu_font_distance_get();
					if ( gu_font_align_get() > 0 ) 
						gu_font_print( (480-pwidth)/2, 272-(frame->p_num_lines)*gu_font_height()-sub_distance, flags | FLAG_ALIGN_CENTER, frame->p_string);
					else
						gu_font_print( (480-pwidth)/2, sub_distance, flags | FLAG_ALIGN_CENTER, frame->p_string);
					//gu_font_print( (480-pwidth)/2, gu_font_height(), flags | FLAG_ALIGN_CENTER, frame->p_string);
					
					previous_subtitle = 1;//(pwidth<=vertex_width?(vertex_height<272?1:0):1);
					}
			}
		}
	
	if (show_interface)
		{
		pmp_gu_load_interface_texture(background_8888);
		texture_subdivision_constructor(&texture_subdivision, 480, interface_height, 16, output_width, interface_height*output_height/272, output_left, output_top);
		do
			{
			texture_subdivision_get(&texture_subdivision);
			pmp_gu_draw_sprite(&texture_subdivision);
			}
		while (texture_subdivision.output_last == 0);
		}
	
	int i;
	void* s0 = pmp_gu_draw_buffer;
	void* d0 = video_frame_buffer;
	void* d1 = d0 + 804864;
	for(i=0; i<240; i++)
		{
		sceGuCopyImage(GU_PSM_8888, 0, 0, 720, 1, 768, s0, 0, 0, 768, d1);
		sceGuTexSync();
		s0+=3072;
		d1+=3072;
		sceGuCopyImage(GU_PSM_8888, 0, 0, 720, 1, 768, s0, 0, 0, 768, d0);
		sceGuTexSync();
		s0+=3072;
		d0+=3072;
		}
	sceGuFinish();
	}

void pmp_gu_draw_tvout_progressive(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer)
	{
	short texture_width  = aspect_ratios[0].width;
	short texture_height = aspect_ratios[0].height;

	int vertex_width  = aspect_ratios[aspect_ratio].psp_width;
	int vertex_height = aspect_ratios[aspect_ratio].psp_height;

	vertex_width  = zoom * vertex_width  / 100;
	vertex_height = zoom * vertex_height / 100;

	int vertex_x = output_left + (output_width >> 1) - (vertex_width  >> 1);
	int vertex_y = output_top + (output_height >> 1) - (vertex_height >> 1);

	int filter;
	if ((texture_width == vertex_width) && (texture_height == vertex_height))
		{
		filter = GU_NEAREST;
		}
	else
		{
		filter = GU_LINEAR;
		}
		
		
	sceGuStart(GU_DIRECT, pmp_gu_list);
	if ((previous_aspect_ratio != aspect_ratio) || (previous_zoom != zoom) || (previous_interface != show_interface) ||
	    ((vertex_width < output_width || vertex_height < output_height) && (previous_subtitle || previous_info)))
		{
		sceGuClearColor(0);
		sceGuClear(GU_COLOR_BUFFER_BIT);

		previous_aspect_ratio = aspect_ratio;
		previous_zoom         = zoom;
		previous_subtitle     = 0;
		previous_info         = 0;
		previous_interface = show_interface;
		}



	struct texture_subdivision_struct texture_subdivision;


	pmp_gu_load(pmp_gu_rgb_buffer, filter);
	texture_subdivision_constructor(&texture_subdivision, texture_width, texture_height, 16, vertex_width, vertex_height, vertex_x, vertex_y);
	do
		{
		texture_subdivision_get(&texture_subdivision);
		pmp_gu_draw_sprite(&texture_subdivision);
		}
	while (texture_subdivision.output_last == 0);
	



	pmp_gu_load_luminosity_texture(luminosity_textures + 64 * luminosity_boost);
	texture_subdivision_constructor(&texture_subdivision, 4, 4, 16, vertex_width, vertex_height, vertex_x, vertex_y);
	do
		{
		texture_subdivision_get(&texture_subdivision);
		pmp_gu_draw_sprite(&texture_subdivision);
		}
	while (texture_subdivision.output_last == 0);
	
	if (info_count)
		{
		info_count--;
		gu_font_print( (476-gu_font_width_get(info_string,0)), 2, 0, info_string );
		previous_info = 1;
		}
	
	if (show_subtitle )
		{
		int flags = 0;
		int pwidth = 0;

		struct subtitle_frame_struct *frame = 0;
		if (subtitle_parse_get_frame(&subtitle_parser[show_subtitle-1], &frame, frame_number)==0)
			{
				if (frame!=0)
					{
					if ((subtitle_format==1) && (gufont_haveflags&GU_FONT_HAS_UNICODE_CHARMAP))
						{
						flags = FLAG_UTF8;
						pwidth = gu_font_utf8_width_get(frame->p_string,0);
						}
					else
						pwidth = gu_font_width_get(frame->p_string,0);
					unsigned int sub_distance = gu_font_distance_get();
					if ( gu_font_align_get() > 0 ) 
						gu_font_print( (480-pwidth)/2, 272-(frame->p_num_lines)*gu_font_height()-sub_distance, flags | FLAG_ALIGN_CENTER, frame->p_string);
					else
						gu_font_print( (480-pwidth)/2, sub_distance, flags | FLAG_ALIGN_CENTER, frame->p_string);
					//gu_font_print( (480-pwidth)/2, gu_font_height(), flags | FLAG_ALIGN_CENTER, frame->p_string);
					
					previous_subtitle = 1;//(pwidth<=vertex_width?(vertex_height<272?1:0):1);
					}
			}
		}
	
	if (show_interface)
		{
		pmp_gu_load_interface_texture(background_8888);
		texture_subdivision_constructor(&texture_subdivision, 480, interface_height, 16, output_width, interface_height*output_height/272, output_left, output_top);
		do
			{
			texture_subdivision_get(&texture_subdivision);
			pmp_gu_draw_sprite(&texture_subdivision);
			}
		while (texture_subdivision.output_last == 0);
		}
	
	sceGuCopyImage(GU_PSM_8888, 0, 0, 720, 480, 768, pmp_gu_draw_buffer, 0, 0, 768, video_frame_buffer);
	sceGuTexSync();
	
	sceGuFinish();
	}

typedef struct tagBITMAPFILEHEADER {
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER ;

typedef struct tagBITMAPINFOHEADER{
	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	int32_t biClrUsed;
	int32_t biClrImportant;
} BITMAPINFOHEADER;



void make_bmp_screenshot() {
	int x, y, mode, pixel_format, width, height, texture_width;
	uint32_t* frame_buffer;
	unsigned char buffer[1440];
	char filename[512];
	SceUID fd;
	
	sceDisplayGetMode(&mode, &width, &height);
	if(width > 480)
		return;
	sceDisplayGetFrameBuf(&frame_buffer, &texture_width, &pixel_format, &mode);
	
	BITMAPFILEHEADER h1;
	BITMAPINFOHEADER h2;
	
	sceIoMkdir("ms0:/PICTURE", 0777);
	sceIoMkdir("ms0:/PICTURE/PPA", 0777);
	
	memset(filename, 0, 512);
	
	pspTime current_time;
	sceRtcGetCurrentClockLocalTime(&current_time);
	
	sprintf(filename, "ms0:/PICTURE/PPA/SNAPSHOT%04d%02d%02d%05d.BMP", 
		current_time.year,
		current_time.month,
		current_time.day,
		current_time.hour*3600+current_time.minutes*60+current_time.seconds);
	fd = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if(fd < 0)
		return;
		
	h1.bfType = 0x4D42;
	h1.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 3*width*height;
	h1.bfReserved1 = 0;
	h1.bfReserved2 = 0;
	h1.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	
	h2.biSize = sizeof(BITMAPINFOHEADER);
	h2.biWidth = width;
	h2.biHeight = height;
	h2.biPlanes = 1;
	h2.biBitCount = 24;
	h2.biCompression = 0;
	h2.biSizeImage = 3*width*height;
	h2.biXPelsPerMeter = 0;
	h2.biYPelsPerMeter = 0;
	h2.biClrUsed = 0;
	h2.biClrImportant = 0;
	
	sceIoWrite(fd, &h1, sizeof(BITMAPFILEHEADER));
	sceIoWrite(fd, &h2, sizeof(BITMAPINFOHEADER));

	for(y = height-1; y >= 0; y--) {
		int i;
		for(i = 0, x = 0; x < width; x++) {
			uint32_t color = frame_buffer[x + y * texture_width];
			buffer[i+2] = (unsigned char)( color & 0xFF );
			buffer[i+1] = (unsigned char)( (color>>8) & 0xFF );
			buffer[i] = (unsigned char)( (color>>16) & 0xFF );
			i += 3;
		}
		sceIoWrite(fd, buffer, 3 * width);
	}
	sceIoClose(fd);
	
}

void make_screenshot() {
	int x, y, mode, pixel_format, width, height, texture_width;
	uint32_t* frame_buffer;
	unsigned char buffer[1440];
	char filename[512];
	FILE* fp;
	
	sceDisplayGetMode(&mode, &width, &height);
	if(width > 480)
		return;
	sceDisplayGetFrameBuf(&frame_buffer, &texture_width, &pixel_format, &mode);
	
	sceIoMkdir("ms0:/PICTURE", 0777);
	sceIoMkdir("ms0:/PICTURE/PPA", 0777);
	
	memset(filename, 0, 512);
	
	pspTime current_time;
	sceRtcGetCurrentClockLocalTime(&current_time);
	
	sprintf(filename, "ms0:/PICTURE/PPA/SNAPSHOT%04d%02d%02d%05d.PNG", 
		current_time.year,
		current_time.month,
		current_time.day,
		current_time.hour*3600+current_time.minutes*60+current_time.seconds);
	
	png_structp png_ptr = 0;
	png_infop info_ptr;
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) 
		return;
		
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return;
	}
	
	fp = fopen(filename, "wb");
	if(fp == 0) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return;
	}
	
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width, height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);


	for(y = 0; y < height; y++) {
		int i;
		for(i = 0, x = 0; x < width; x++) {
			uint32_t color = frame_buffer[x + y * texture_width];
			buffer[i++] = (unsigned char)( color & 0xFF );
			buffer[i++] = (unsigned char)( (color>>8) & 0xFF );
			buffer[i++] = (unsigned char)( (color>>16) & 0xFF );
		}
		png_write_row(png_ptr, buffer);
	}
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	fclose(fp);
}