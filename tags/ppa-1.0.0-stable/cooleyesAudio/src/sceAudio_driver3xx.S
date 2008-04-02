	.set noreorder

#include "pspimport.s"

	IMPORT_START	"sceAudio_driver",0x00010000
	IMPORT_FUNC	"sceAudio_driver",0x442588EB,sceAudioSetFrequency371
	IMPORT_FUNC	"sceAudio_driver",0x9A34BC0D,sceAudioSetFrequency380