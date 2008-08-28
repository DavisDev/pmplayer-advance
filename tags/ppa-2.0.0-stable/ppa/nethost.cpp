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

/*
 * Special thanks to AhMan for nethostfs.prx source and support
 */
 
#include <stdio.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <string.h>
#include "nethost.h"


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
//		FILE* fp = fopen("ms0:/debug.txt", "a+");
//		fprintf(fp,"lsm: name '%s' ret %08X\n",name, modid);
//		fclose(fp);
	}
	else{
//		FILE* fp = fopen("ms0:/debug.txt", "a+");
//		fprintf(fp,"lsm: Error loading module %s %08X\n", name, modid);
//		fclose(fp);
	}

	return modid;
}
#ifdef ENABLE_NETHOST

#ifdef __cplusplus
extern "C" {
#endif

int netHostFSWaitDriverInited();
int netHostFSCleanup();

#ifdef __cplusplus
}
#endif

#endif

u32 NetHost::modID;
bool NetHost::netHostState = false;
char NetHost::basePath[1024];

#define PRX_NAME "nethostfs.prx"
#define PRX_NAME302 "nethostfs_302.prx"
#define PRX_NAME310 "nethostfs_310.prx"
#define PRX_NAME371 "nethostfs_371.prx"

void NetHost::setPrxBasePath(const char* path) {
	memset(basePath, 0, 1024);
	strncpy(basePath, path, 1023);
};

bool NetHost::startNetHost(const char* address, const char* port, const char* entry, const char* password, const char* adhocAddress) {
#ifdef ENABLE_NETHOST	
	if ( !netHostState ) {
		char prxName[1024];
		memset(prxName, 0, 1024);
		int fwVer;
		fwVer = sceKernelDevkitVersion();
		if (fwVer < 0x02000000)
			sprintf(prxName, "%s%s", basePath, PRX_NAME);
		else if ((fwVer >= 0x03000000) && (fwVer <= 0x030002FF))
			sprintf(prxName, "%s%s", basePath, PRX_NAME302);
		else if ((fwVer > 0x030002FF) && (fwVer < 0x03070110))
			sprintf(prxName, "%s%s", basePath, PRX_NAME310);
		else if (fwVer >= 0x03070110)
			sprintf(prxName, "%s%s", basePath, PRX_NAME371);
		
		char* argv[5];
		argv[0] = (char*)address;
		argv[1] = (char*)port;
		argv[2] = (char*)entry;
		argv[3] = (char*)password;
		argv[4] = (char*)adhocAddress;
		
		modID = load_start_module(prxName, 5, argv); 
		if ( modID < 0 )
			return false;
		int res = netHostFSWaitDriverInited() ;
		if ( res < 0 ) {
			netHostFSCleanup();
			sceKernelStopModule(modID, 0, NULL, NULL, NULL);
			//sceKernelUnloadModule(modID);
			return false;
		}
		netHostState = true;
	}
	return true;
#else
	return false;
#endif
};

bool NetHost::stopNetHost() {
#ifdef ENABLE_NETHOST
	if ( netHostState ) {
		netHostFSCleanup();
		sceKernelStopModule(modID, 0, NULL, NULL, NULL);
		//sceKernelUnloadModule(modID);
		netHostState = false;
	}
#endif
	return true;
};

