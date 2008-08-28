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
#include <stdio.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspusb.h>
#include <string.h>
#include "usbhost.h"

#define HOSTFSDRIVER_NAME "USBHostFSDriver"
#define HOSTFSDRIVER_PID  (0x1C9)

static int build_args(char *args, const char *execfile, int argc, char **argv) {
	int loc = 0;
	int i;

	strcpy(args, execfile);
	loc += strlen(execfile) + 1;
	for(i = 0; i < argc; i++)
	{
		strcpy(&args[loc], argv[i]);
		loc += strlen(argv[i]) + 1;
	}

	return loc;
}

static int load_start_module(const char *name, int argc, char **argv) {
	SceUID modid;
	int status;
	char args[1024];
	int len;

	modid = sceKernelLoadModule(name, 0, NULL);
	if(modid >= 0) {
		len = build_args(args, name, argc, argv);
		modid = sceKernelStartModule(modid, len, (void *) args, &status, NULL);
		//FILE* fp = fopen("ms0:/debug.txt", "a+");
		//fprintf(fp,"lsm: name '%s' ret %08X\n",name, modid);
		//fclose(fp);
	}
	else{
		//FILE* fp = fopen("ms0:/debug.txt", "a+");
		//fprintf(fp,"lsm: Error loading module %s %08X\n", name, modid);
		//fclose(fp);
	}

	return modid;
}

bool UsbHost::usbHostPrxLoaded = false;
bool UsbHost::usbHostState = false;

bool UsbHost::loadUsbHostPrx(const char* prxName){
#ifdef ENABLE_USBHOST
	if ( !usbHostPrxLoaded ) {
		if ( load_start_module(prxName, 0, NULL) < 0 )
			return false;
		//load_start_module(prxName, 0, NULL);
		usbHostPrxLoaded = true;
	}
#endif
	return true;
};

bool UsbHost::startUsbHost() {
#ifdef ENABLE_USBHOST
	if ( !usbHostPrxLoaded )
		return false;
	if ( !usbHostState ) {
		int retVal;
		retVal = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
		if (retVal != 0) {
			return false;
		}
		retVal = sceUsbStart(HOSTFSDRIVER_NAME, 0, 0);
		if (retVal != 0) {
			return false;
		}

		retVal = sceUsbActivate(HOSTFSDRIVER_PID);

		if(retVal != 0) {
			return false;
		}
		sceIoAssign("usbhost0:", "host0:", NULL, IOASSIGN_RDWR, NULL, 0); 
		usbHostState = true;
	}
	return true;
#else
	return false;
#endif
};

bool UsbHost::stopUsbHost() {
#ifdef ENABLE_USBHOST
	if ( usbHostState ) {
		int retVal;
		sceIoUnassign("usbhost0:"); 
		retVal = sceUsbDeactivate(HOSTFSDRIVER_PID);
		if (retVal != 0) {
			return false;
		}

		retVal = sceUsbStop(HOSTFSDRIVER_NAME, 0, 0);
		if (retVal != 0) {
			return false;
		}
		retVal = sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
		if (retVal != 0) {
			return false;
		}
		usbHostState = false;
	}
#endif
	return true;
};

