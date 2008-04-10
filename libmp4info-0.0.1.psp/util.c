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

#include <stdlib.h>
#include "util.h"
 
 
int32_t io_set_position(SceUID handle, const int32_t position) {
	return sceIoLseek32(handle, position, PSP_SEEK_SET);
}

int32_t io_get_position(SceUID handle) {
	return sceIoLseek32(handle, 0, PSP_SEEK_CUR);
}

uint32_t io_read_data(SceUID handle, uint8_t* data, uint32_t size) {
	return sceIoRead(handle, data, size);
}

uint64_t io_read_int64(SceUID handle) {
	uint8_t data[8];
	uint64_t result = 0;
	int i;
	
	sceIoRead(handle, data, 8);
	for (i = 0; i < 8; i++) {
		result |= ((uint64_t)data[i]) << ((7 - i) * 8);
	}

	return result;
}

uint32_t io_read_int32(SceUID handle) {
	uint8_t data[4];
	uint32_t result = 0;
	uint32_t a, b, c, d;
	
	sceIoRead(handle, data, 4);
	a = (uint8_t)data[0];
	b = (uint8_t)data[1];
	c = (uint8_t)data[2];
	d = (uint8_t)data[3];
	result = (a<<24) | (b<<16) | (c<<8) | d;

	return result;
}

uint32_t io_read_int24(SceUID handle) {
	uint8_t data[4];
	uint32_t result = 0;
	uint32_t a, b, c;
	
	sceIoRead(handle, data, 3);
	a = (uint8_t)data[0];
	b = (uint8_t)data[1];
	c = (uint8_t)data[2];
	result = (a<<16) | (b<<8) | c;

	return result;
}

uint16_t io_read_int16(SceUID handle) {
	uint8_t data[2];
	uint16_t result = 0;
	uint16_t a, b;
	
	sceIoRead(handle, data, 2);
	a = (uint8_t)data[0];
	b = (uint8_t)data[1];
	result = (a<<8) | b;

	return result;
}

uint8_t io_read_int8(SceUID handle){
	uint8_t result;
    
	sceIoRead(handle, &result, 1);
    
	return result;
}

