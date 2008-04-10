/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspvideocodec.h - Prototypes for the sceVideocodec library.
 *
 * Copyright (c) 2007 cooleyes
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

int sceVideocodecOpen(unsigned long *Buffer, int Type);
int sceVideocodecGetEDRAM(unsigned long *Buffer, int Type);
int sceVideocodecInit(unsigned long *Buffer, int Type);
int sceVideocodecDecode(unsigned long *Buffer, int Type);
int sceVideocodecStop(unsigned long *Buffer, int Type);
int sceVideocodecDelete(unsigned long *Buffer, int Type);
int sceVideocodecReleaseEDRAM(unsigned long *Buffer);

#ifdef __cplusplus
}
#endif
