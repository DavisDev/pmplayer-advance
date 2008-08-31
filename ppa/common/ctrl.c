#include "ctrl.h"
#include <time.h>
#include <pspkernel.h>

#define CTRL_REPEAT_TIME 0x40000
static unsigned int last_btn = 0;
static unsigned int last_tick = 0;
#ifdef ENABLE_HPRM
static int hprmenable = 0;
static u32 lasthprmkey = 0;
static u32 lastkhprmkey = 0;
static SceUID hprm_sema = -1;
#endif

void ctrl_init() {
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
#ifdef ENABLE_HPRM
	hprm_sema = sceKernelCreateSema("hprm sem", 0, 1, 1, NULL);
#endif
}

void ctrl_destroy() {
#ifdef ENABLE_HPRM
	sceKernelDeleteSema(hprm_sema);
	hprm_sema = -1;
#endif
}

void ctrl_analog(int * x, int * y) {
	SceCtrlData ctl;
	sceCtrlReadBufferPositive(&ctl,1);
	*x = ((int)ctl.Lx) - 128;
	*y = ((int)ctl.Ly) - 128;
}

u32 ctrl_read_cont() {
	SceCtrlData ctl;

	sceCtrlReadBufferPositive(&ctl,1);
#ifdef ENABLE_HPRM
	if(hprmenable && sceHprmIsRemoteExist()) {
		u32 key;
		sceKernelWaitSema(hprm_sema, 1, NULL);
		sceHprmPeekCurrentKey(&key);
		sceKernelSignalSema(hprm_sema, 1);

		if(key > 0) {
			switch(key) {
			case PSP_HPRM_FORWARD:
				if(key == lastkhprmkey)
					break;
				lastkhprmkey = key;
				return CTRL_FORWARD;
			case PSP_HPRM_BACK:
				if(key == lastkhprmkey)
					break;
				lastkhprmkey = PSP_HPRM_BACK;
				return CTRL_BACK;
			case PSP_HPRM_PLAYPAUSE:
				if(key == lastkhprmkey)
					break;
				lastkhprmkey = key;
				return CTRL_PLAYPAUSE;
			}
		}
		else
			lastkhprmkey = 0;
	}
#endif
#ifdef ENABLE_ANALOG
	if (ctl.Lx < 65 || ctl.Lx > 191 || ctl.Ly < 65 || ctl.Ly > 191)
		return CTRL_ANALOG | ctl.Buttons;
#endif
	last_btn  = ctl.Buttons;
	last_tick = ctl.TimeStamp;
	return last_btn;
}

u32 ctrl_read() {
	SceCtrlData ctl;
#ifdef ENABLE_HPRM
	if(hprmenable && sceHprmIsRemoteExist())
	{
		u32 key;
		sceHprmPeekCurrentKey(&key);

		if(key > 0)
		{
			switch(key)
			{
			case PSP_HPRM_FORWARD:
				if(key == lastkhprmkey)
					break;
				lastkhprmkey = key;
				return CTRL_FORWARD;
			case PSP_HPRM_BACK:
				if(key == lastkhprmkey)
					break;
				lastkhprmkey = key;
				return CTRL_BACK;
			case PSP_HPRM_PLAYPAUSE:
				if(key == lastkhprmkey)
					break;
				lastkhprmkey = key;
				return CTRL_PLAYPAUSE;
			}
		}
		else
			lastkhprmkey = 0;
	}
#endif
	sceCtrlReadBufferPositive(&ctl,1);
#ifdef ENABLE_ANALOG
	if (ctl.Lx < 65 || ctl.Lx > 191 || ctl.Ly < 65 || ctl.Ly > 191)
		return CTRL_ANALOG;
#endif
	if (ctl.Buttons == last_btn) {
		if (ctl.TimeStamp - last_tick < CTRL_REPEAT_TIME) return 0;
		return last_btn;
	}
	last_btn  = ctl.Buttons;
	last_tick = ctl.TimeStamp;
	return last_btn;
}

#ifdef ENABLE_HPRM
u32 ctrl_hprm() {
	if(sceKernelDevkitVersion() >= 0x02000010)
		return 0;
	if(!sceHprmIsRemoteExist())
		return 0;

	u32 key;
	sceKernelWaitSema(hprm_sema, 1, NULL);
	sceHprmPeekCurrentKey(&key);
	sceKernelSignalSema(hprm_sema, 1);
	if(key == lasthprmkey)
		return 0;

	lasthprmkey = key;
	return key;
}

void ctrl_enablehprm(int enable) {
	hprmenable = (sceKernelDevkitVersion() < 0x02000010) && enable;
}
#endif
