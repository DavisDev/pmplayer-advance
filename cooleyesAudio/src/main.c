#include <pspsdk.h>
#include <pspkernel.h>
#include <string.h>
#include <pspaudio.h>
#include <pspaudio_kernel.h>


#define VERS 1
#define REVS 0


PSP_MODULE_INFO("cooleyesAudio", 0x1006, VERS, REVS);
PSP_MAIN_THREAD_ATTR(0);

int sceAudioSetFrequency371(int frequency);

int cooleyesAudioSetFrequency(int devkitVersion, int frequency) {
	u32 k1; 
   	k1 = pspSdkSetK1(0);
   	int ret; 
	if (devkitVersion < 0x03070110)
		ret = sceAudioSetFrequency(frequency);
	else
		ret = sceAudioSetFrequency371(frequency);
	pspSdkSetK1(k1);
	return ret;
}

int module_start(SceSize args, void *argp){
	return 0;
}

int module_stop(){
	return 0;
}
