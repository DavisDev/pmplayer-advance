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

#include <pspkernel.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspdebug.h>
#include <psprtc.h>
#include <pspsdk.h>
#include "player.h"

#define VERS 3
#define REVS 2

#include "cooleyesBridge.h"

PSP_MODULE_INFO("PMPLAYER_ADVANCE", 0, VERS, REVS);
PSP_MAIN_THREAD_ATTR(0);
PSP_HEAP_SIZE_KB(18*1024);

#define printf	pspDebugScreenPrintf

static PmpAvcPlayer* player;

/* Power callbakc */
static int power_callback(int arg1, int powerInfo, void * arg){
	if((powerInfo & (PSP_POWER_CB_POWER_SWITCH | PSP_POWER_CB_STANDBY)) > 0) {
		player->enterSuspendMode();
		//sceKernelDelayThread(1000000);
	}
	else if((powerInfo & PSP_POWER_CB_RESUME_COMPLETE) > 0)	{
		sceKernelDelayThread(1500000);
		player->leaveSuspendMode();
	}
	return 0;
}

/* Exit callback */
static int exit_callback(int arg1, int arg2, void *common){
	cooleyesAudioSetFrequency(sceKernelDevkitVersion(), 44100);
	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp){
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	cbid = sceKernelCreateCallback("Power Callback", power_callback, NULL);
	scePowerRegisterCallback(0, cbid);

	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void){
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}
int main(int argc, char **argv){

	pspDebugScreenInit();
#ifdef DEBUG
	pspDebugScreenPrintf("PMPlayer Advance loading...\n");
#endif
	SetupCallbacks();

#ifdef DEBUG	
	pspDebugScreenPrintf("begin init...\n");
#endif
	player = new PmpAvcPlayer();
	if (player->init(0))
		player->run();
	else {
		pspDebugScreenPrintf("init fail, press X to exit...\n");
		SceCtrlData input;
		sceCtrlReadBufferPositive(&input, 1);
		while(!(input.Buttons & PSP_CTRL_CROSS)) {
			sceKernelDelayThread(10000);	// wait 10 milliseconds
			sceCtrlReadBufferPositive(&input, 1);
		}
	}
	delete player;
	player = NULL;
#ifdef DEBUG	
	pspDebugScreenPrintf("exit ppa...\n");
#endif
	//sceKernelDelayThread(10000000);
	sceKernelExitGame();	
	return 0; 
}
