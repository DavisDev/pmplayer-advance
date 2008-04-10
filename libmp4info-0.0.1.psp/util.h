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
 
#ifndef __MP4INFO_UTIL_H__
#define __MP4INFO_UTIL_H__

#include <pspiofilemgr.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t io_set_position(SceUID handle, const int32_t position);
int32_t io_get_position(SceUID handle);
uint32_t io_read_data(SceUID handle, uint8_t* data, const uint32_t size);
uint64_t io_read_int64(SceUID handle);
uint32_t io_read_int32(SceUID handle);
uint32_t io_read_int24(SceUID handle);
uint16_t io_read_int16(SceUID handle);
uint8_t io_read_int8(SceUID handle);

#ifdef __cplusplus
}
#endif

#endif
