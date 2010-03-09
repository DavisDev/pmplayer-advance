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
 
#ifndef __FLV_2_MPEG4_LIB_H__
#define __FLV_2_MPEG4_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int open_flv2mpeg4_convert(int w, int h);
void close_flv2mpeg4_convert();
void* convert_flv_frame_to_mpeg4_frame(void* flv_frame, int in_size, int* out_size);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif