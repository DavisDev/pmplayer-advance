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
aspect ratio calculations
*/


#include "aspect_ratio.h"
#include "common/m33sdk.h"


struct aspect_ratio_struct aspect_ratios[number_of_aspect_ratios] =
	{
	{"Original",   0,   0, 0, 0},
	{"4 / 3",      4,   3, 0, 0},
	{"16 / 9",   480, 272, 0, 0},
	{"2.35",     235, 100, 0, 0}
	};


void aspect_ratio_struct_init_psplcd(unsigned int width, unsigned int height)
	{
	aspect_ratios[0].width  = width;
	aspect_ratios[0].height = height;


	int i = 0;
	for (; i < number_of_aspect_ratios; i++)
		{
		// width / height > 480 / 272

		if (272 * aspect_ratios[i].width > 480 * aspect_ratios[i].height)
			{
			// height / width = psp_height / psp_width

			aspect_ratios[i].psp_width  = 480;
			aspect_ratios[i].psp_height = aspect_ratios[i].psp_width * aspect_ratios[i].height / aspect_ratios[i].width;
			}
		else
			{
			// width / height = psp_width / psp_height

			aspect_ratios[i].psp_height = 272;
			aspect_ratios[i].psp_width  = aspect_ratios[i].psp_height * aspect_ratios[i].width / aspect_ratios[i].height;
			}
		}
	}

void aspect_ratio_struct_init_tvout(unsigned int width, unsigned int height, int tv_aspectratio)
	{
	aspect_ratios[0].width  = width;
	aspect_ratios[0].height = height;
	
	int tvout_width, tvout_height;
	if (tv_aspectratio == 0 )
		{
		tvout_width = 704;
		tvout_height = 480;
		}
	else
		{
		tvout_width = 704;
		tvout_height = 360;
		}

	int i = 0;
	for (; i < number_of_aspect_ratios; i++)
		{
		// width / height > 480 / 272

		if (272 * aspect_ratios[i].width > 480 * aspect_ratios[i].height)
			{
			// height / width = psp_height / psp_width

			aspect_ratios[i].psp_width  = tvout_width;
			aspect_ratios[i].psp_height = 480 * aspect_ratios[i].height / aspect_ratios[i].width * tvout_height / 272;
			}
		else
			{
			// width / height = psp_width / psp_height

			aspect_ratios[i].psp_height = tvout_height;
			aspect_ratios[i].psp_width  = 272 * aspect_ratios[i].width / aspect_ratios[i].height * tvout_width / 480;
			}
		}
	}

void aspect_ratio_struct_init(unsigned int width, unsigned int height, int psp_type, int tv_aspectratio, int video_mode)
	{
	if( !m33IsTVOutSupported(psp_type) )
		aspect_ratio_struct_init_psplcd(width, height);
	else
		{
		if ( video_mode == 0 )
			aspect_ratio_struct_init_psplcd(width, height);
		else
			aspect_ratio_struct_init_tvout(width, height, tv_aspectratio);
		}
	}
