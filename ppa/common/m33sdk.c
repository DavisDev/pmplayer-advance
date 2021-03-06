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
#include "m33sdk.h"
#include <pspsdk.h>
#include <kubridge.h>

#define PSP_MODEL_3K   2

int m33KernelGetModel() {
	return kuKernelGetModel();
};

int m33IsTVOutSupported(int type){
	return ( (type==PSP_MODEL_SLIM_AND_LITE || type==PSP_MODEL_3K) && (!(sceKernelDevkitVersion()<0x03070110)) );
};

SceUID m33KernelLoadModule(const char *path, int flags, SceKernelLMOption *option){
	return kuKernelLoadModule(path, flags, option);
};
