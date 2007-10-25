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


#include "pmp_gu.h"


unsigned int __attribute__((aligned(16))) pmp_gu_list[262144];


static unsigned char __attribute__((aligned(64))) luminosity_textures[64 * number_of_luminosity_boosts] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 8, 8, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 12, 12, 12, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 16, 16, 16, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 20, 20, 20, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 24, 24, 24, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 28, 28, 28, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 32, 32, 32, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 36, 36, 36, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 40, 40, 40, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 44, 44, 44, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 48, 48, 48, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 52, 52, 52, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 56, 56, 56, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0, 60, 60, 60, 0};

typedef void (*f_pmp_gu_draw)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, void *);

void *pmp_gu_draw_buffer;
void *pmp_gu_rgb_buffer;

static unsigned int previous_aspect_ratio;
static unsigned int previous_zoom;
static unsigned int previous_subtitle;
static unsigned int previous_info;
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
	}


void pmp_gu_end()
	{
	sceGuTerm();
	}


void pmp_gu_start_without_tvout_supported()
	{
	p_pmp_gu_draw = pmp_gu_draw_without_tvout_supported;
	sceGuInit();
	sceGuStart(GU_DIRECT, pmp_gu_list);
	sceGuDrawBuffer(GU_PSM_8888, 0, 512);
	sceGuDispBuffer(480, 272, 0, 512);
	sceGuOffset(2048 - (480 >> 1), 2048 - (272 >> 1));
	sceGuViewport(2048, 2048, 480, 272);
	sceGuScissor(0, 0, 480, 272);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDisable(GU_CULL_FACE);
	sceGuDisable(GU_DEPTH_TEST);	// Disable Z-compare
	sceGuDepthMask(GU_TRUE);		// Disable Z-writes
	sceGuDisable(GU_COLOR_TEST);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_LIGHTING);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuFinish();
	sceGuSync(0, 0);


	pmp_gu_draw_buffer  = (void *) 0x04000000;
	pmp_gu_rgb_buffer  = pmp_gu_draw_buffer + (4 * 512 * 272);

	memset(pmp_gu_rgb_buffer, 0, 4 * 512 * 512);
	sceKernelDcacheWritebackInvalidateAll();
	}

void pmp_gu_start_psplcd()
	{
	p_pmp_gu_draw = pmp_gu_draw_psplcd;
	sceGuInit();
	sceGuStart(GU_DIRECT, pmp_gu_list);
	sceGuDrawBuffer(GU_PSM_8888, 0, 768);
	sceGuDispBuffer(480, 272, 0, 768);
	sceGuOffset(2048 - (480 >> 1), 2048 - (272 >> 1));
	sceGuViewport(2048, 2048, 480, 272);
	sceGuScissor(0, 0, 480, 272);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDisable(GU_CULL_FACE);
	sceGuDisable(GU_DEPTH_TEST);	// Disable Z-compare
	sceGuDepthMask(GU_TRUE);		// Disable Z-writes
	sceGuDisable(GU_COLOR_TEST);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_LIGHTING);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuFinish();
	sceGuSync(0, 0);

	pmp_gu_draw_buffer  = (void *) (0x04000000);
	pmp_gu_rgb_buffer  = (void*)0x0a000000;

	memset(pmp_gu_rgb_buffer, 0, 4 * 512 * 512);
	sceKernelDcacheWritebackInvalidateAll();
	}

void pmp_gu_start_tvout_interlace()
	{
	p_pmp_gu_draw = pmp_gu_draw_tvout_interlace;
	sceGuInit();
	sceGuStart(GU_DIRECT, pmp_gu_list);
	sceGuDrawBuffer(GU_PSM_8888, 0, 768);
	sceGuDispBuffer(720, 480, 0, 768);
	sceGuOffset(2048 - (720 >> 1), 2048 - (480 >> 1));
	sceGuViewport(2048, 2048, 720, 480);
	sceGuScissor(0, 0, 720, 480);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDisable(GU_CULL_FACE);
	sceGuDisable(GU_DEPTH_TEST);	// Disable Z-compare
	sceGuDepthMask(GU_TRUE);		// Disable Z-writes
	sceGuDisable(GU_COLOR_TEST);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_LIGHTING);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuFinish();
	sceGuSync(0, 0);

	pmp_gu_draw_buffer  = (void *) (0x40000000 | 0x04000000);
	pmp_gu_rgb_buffer  = (void*)0x0a000000;

	memset(pmp_gu_rgb_buffer, 0, 4 * 512 * 512);
	sceKernelDcacheWritebackInvalidateAll();
	}

void pmp_gu_start_tvout_progressive()
	{
	p_pmp_gu_draw = pmp_gu_draw_tvout_progressive;
	sceGuInit();
	sceGuStart(GU_DIRECT, pmp_gu_list);
	sceGuDrawBuffer(GU_PSM_8888, 0, 768);
	sceGuDispBuffer(720, 480, 0, 768);
	sceGuOffset(2048 - (720 >> 1), 2048 - (480 >> 1));
	sceGuViewport(2048, 2048, 720, 480);
	sceGuScissor(0, 0, 720, 480);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDisable(GU_CULL_FACE);
	sceGuDisable(GU_DEPTH_TEST);	// Disable Z-compare
	sceGuDepthMask(GU_TRUE);		// Disable Z-writes
	sceGuDisable(GU_COLOR_TEST);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_LIGHTING);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuFinish();
	sceGuSync(0, 0);

	pmp_gu_draw_buffer  = (void *) (0x04000000);
	pmp_gu_rgb_buffer  = (void*)0x0a000000;

	memset(pmp_gu_rgb_buffer, 0, 4 * 512 * 512);
	sceKernelDcacheWritebackInvalidateAll();
	}

void pmp_gu_start(int psp_type, int tv_aspectratio, int video_mode)
	{
	if (!m33IsTVOutSupported(psp_type))
		{
		gu_font_output_set(0, 0, 480, 272);
		pmp_gu_start_without_tvout_supported();
		}
	else
		{
			if (video_mode == 0 )
				{
				pmp_gu_start_psplcd();
				gu_font_output_set(0, 0, 480, 272);
				}
			else
				{
				if (tv_aspectratio == 0)
					{
					gu_font_output_set(8, 0, 704, 480);
					}
				else
					{
					gu_font_output_set(8, 60, 704, 360);
					}
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

	int vertex_x = (480 >> 1) - (vertex_width  >> 1);
	int vertex_y = (272 >> 1) - (vertex_height >> 1);

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
	if ((previous_aspect_ratio != aspect_ratio) || (previous_zoom != zoom) || 
	    ((vertex_width < 480 || vertex_height < 272) && (previous_subtitle || previous_info)))
		{
		sceGuClear(GU_COLOR_BUFFER_BIT);

		previous_aspect_ratio = aspect_ratio;
		previous_zoom         = zoom;
		previous_subtitle     = 0;
		previous_info         = 0;
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

		struct pmp_sub_frame_struct *frame = 0;
		if (pmp_sub_parse_get_frame(&subtitle_parser[show_subtitle-1], &frame, frame_number)==0)
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
					
					previous_subtitle = (pwidth<=vertex_width?(vertex_height<272?1:0):1);
					}
			}
		}


	if (show_interface == 0)
		{
		sceGuCopyImage(GU_PSM_8888, 0, 0, 480, 272, 512, pmp_gu_draw_buffer, 0, 0, 512, video_frame_buffer);
		sceGuTexSync();
		}
	else
		{
		sceGuCopyImage(GU_PSM_8888, 0, interface_height, 480, 272 - interface_height, 512, pmp_gu_draw_buffer, 0, 0, 512, video_frame_buffer + 2048 * interface_height);
		//sceGuCopyImage(GU_PSM_8888, 0, 0, 480, 272 - interface_height, 512, pmp_gu_draw_buffer, 0, 0, 512, video_frame_buffer);
		sceGuTexSync();
		sceGuCopyImage(GU_PSM_8888, 0, 0, 480, interface_height, 512, background_8888, 0, 0, 512, video_frame_buffer );
		//sceGuCopyImage(GU_PSM_8888, 0, 0, 480, interface_height, 512, background_8888, 0, 0, 512, video_frame_buffer + 557056 - 2048 * interface_height);
		sceGuTexSync();
		}


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

	int vertex_x = (480 >> 1) - (vertex_width  >> 1);
	int vertex_y = (272 >> 1) - (vertex_height >> 1);

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
	if ((previous_aspect_ratio != aspect_ratio) || (previous_zoom != zoom) || 
	    ((vertex_width < 480 || vertex_height < 272) && (previous_subtitle || previous_info)))
		{
		sceGuClear(GU_COLOR_BUFFER_BIT);

		previous_aspect_ratio = aspect_ratio;
		previous_zoom         = zoom;
		previous_subtitle     = 0;
		previous_info         = 0;
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

		struct pmp_sub_frame_struct *frame = 0;
		if (pmp_sub_parse_get_frame(&subtitle_parser[show_subtitle-1], &frame, frame_number)==0)
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
					
					previous_subtitle = (pwidth<=vertex_width?(vertex_height<272?1:0):1);
					}
			}
		}
	
	if (show_interface == 0)
		{
		sceGuCopyImage(GU_PSM_8888, 0, 0, 480, 272, 768, pmp_gu_draw_buffer, 0, 0, 768, video_frame_buffer);
		sceGuTexSync();
		}
	else
		{
		sceGuCopyImage(GU_PSM_8888, 0, interface_height, 480, 272 - interface_height, 768, pmp_gu_draw_buffer, 0, 0, 768, video_frame_buffer + 3072 * interface_height);
		//sceGuCopyImage(GU_PSM_8888, 0, 0, 480, 272 - interface_height, 768, pmp_gu_draw_buffer, 0, 0, 768, video_frame_buffer);
		sceGuTexSync();
		sceGuCopyImage(GU_PSM_8888, 0, 0, 480, interface_height, 512, background_8888, 0, 0, 768, video_frame_buffer );
		//sceGuCopyImage(GU_PSM_8888, 0, 0, 480, interface_height, 512, background_8888, 0, 0, 768, video_frame_buffer + 835584 - 3072 * interface_height);
		sceGuTexSync();
		}

	sceGuFinish();
	
	}

void pmp_gu_draw_tvout_interlace(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer)
	{
	}

void pmp_gu_draw_tvout_progressive(unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int frame_number, void *video_frame_buffer)
	{
	short texture_width  = aspect_ratios[0].width;
	short texture_height = aspect_ratios[0].height;

	int vertex_width  = aspect_ratios[aspect_ratio].psp_width;
	int vertex_height = aspect_ratios[aspect_ratio].psp_height;

	vertex_width  = zoom * vertex_width  / 100;
	vertex_height = zoom * vertex_height / 100;

	int vertex_x = (720 >> 1) - (vertex_width  >> 1);
	int vertex_y = (480 >> 1) - (vertex_height >> 1);

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
	if ((previous_aspect_ratio != aspect_ratio) || (previous_zoom != zoom) || 
	    ((vertex_width < 704 || vertex_height < 480) && (previous_subtitle || previous_info)))
		{
		sceGuClear(GU_COLOR_BUFFER_BIT);

		previous_aspect_ratio = aspect_ratio;
		previous_zoom         = zoom;
		previous_subtitle     = 0;
		previous_info         = 0;
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

		struct pmp_sub_frame_struct *frame = 0;
		if (pmp_sub_parse_get_frame(&subtitle_parser[show_subtitle-1], &frame, frame_number)==0)
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
					
					previous_subtitle = (pwidth<=vertex_width?(vertex_height<272?1:0):1);
					}
			}
		}
	
	if (show_interface == 0)
		{
		sceGuCopyImage(GU_PSM_8888, 0, 0, 720, 480, 768, pmp_gu_draw_buffer, 0, 0, 768, video_frame_buffer);
		sceGuTexSync();
		}
	else
		{
		sceGuCopyImage(GU_PSM_8888, 0, interface_height, 720, 480 - interface_height, 768, pmp_gu_draw_buffer, 0, 0, 768, video_frame_buffer + 3072 * interface_height);
		//sceGuCopyImage(GU_PSM_8888, 0, 0, 480, 272 - interface_height, 768, pmp_gu_draw_buffer, 0, 0, 768, video_frame_buffer);
		sceGuTexSync();
		sceGuCopyImage(GU_PSM_8888, 0, 0, 480, interface_height, 512, background_8888, 0, 0, 768, video_frame_buffer );
		//sceGuCopyImage(GU_PSM_8888, 0, 0, 480, interface_height, 512, background_8888, 0, 0, 768, video_frame_buffer + 835584 - 3072 * interface_height);
		sceGuTexSync();
		}

	sceGuFinish();
	}

