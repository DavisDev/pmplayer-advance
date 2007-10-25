/*
PMP Mod
Copyright (C) 2006 jonny

Homepage: http://jonny.leffe.dnsalias.com
E-mail:   jonny@leffe.dnsalias.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
simple lib to use the psp me

a lot of code is taken from here: http://forums.ps2dev.org/viewtopic.php?t=2652
thanks to crazyc, mrbrown, Brunni, xdeadbeef and anyone else involved
(major optimizations in libavcodec are done with the me)
*/


#include <pspkernel.h>
#include <string.h>
#include "me.h"
#include "common/mem64.h"


struct me_struct
	{
	int start;
	int end;
	void (*func)(int);
	int param;
	int signaled;
	};


volatile struct me_struct *nocache;


void me_stub(void);
void me_stub_end(void);


void me_startproc(u32 func, u32 param)
	{
#ifndef DEVHOOK
	memcpy((void *) 0xbfc00040, me_stub, (int) (me_stub_end - me_stub));

	_sw(func,  0xbfc00600);
	_sw(param, 0xbfc00604);

	sceKernelDcacheWritebackAll();
	sceSysregMeResetEnable();
	sceSysregMeBusClockEnable();
	sceSysregMeResetDisable();
#endif
	}


void me_function(int unused)
	{
#ifndef DEVHOOK
	while (1)
		{
		while (nocache->start == 0);
		nocache->start = 0;
		nocache->func(nocache->param);
		nocache->end = 1;
		}
#endif
	}


int me_struct_init()
	{
#ifndef DEVHOOK
	nocache = malloc_64(sizeof(struct me_struct));

	if (nocache == 0) return(0);

	nocache = (volatile struct me_struct *) (((int) nocache) | 0x40000000);
	sceKernelDcacheWritebackInvalidateAll();

	nocache->start    = 0;
	nocache->end      = 1;
	nocache->func     = 0;
	nocache->param    = 0;
	nocache->signaled = 0;

	me_startproc((u32) me_function, 0);

	return(1);
#endif
	}


void me_start(int func, int param)
	{
#ifndef DEVHOOK
	nocache->end   = 0;
	nocache->func  = (void (*)(int)) func;
	nocache->param = param;
	nocache->start = 1;
#endif
	}


void me_wait()
	{
#ifndef DEVHOOK
	while (nocache->end == 0);
#endif
	}


int me_unused()
	{
#ifndef DEVHOOK
	return(nocache->end);
#endif
	}


void me_signal_reset()
	{
#ifndef DEVHOOK
	nocache->signaled = 0;
#endif
	}


void me_signal()
	{
#ifndef DEVHOOK
	nocache->signaled = 1;
#endif
	}


int me_signaled()
	{
#ifndef DEVHOOK
	return(nocache->signaled);
#endif
	}


void me_sceKernelDcacheWritebackInvalidateAll()
	{
#ifndef DEVHOOK
	asm __volatile__
		(
		"cache 20, 0($0)\n"
		"cache 20, 0($0)\n"
		"cache 20, 64($0)\n"
		"cache 20, 64($0)\n"
		"cache 20, 128($0)\n"
		"cache 20, 128($0)\n"
		"cache 20, 192($0)\n"
		"cache 20, 192($0)\n"
		"cache 20, 256($0)\n"
		"cache 20, 256($0)\n"
		"cache 20, 320($0)\n"
		"cache 20, 320($0)\n"
		"cache 20, 384($0)\n"
		"cache 20, 384($0)\n"
		"cache 20, 448($0)\n"
		"cache 20, 448($0)\n"
		"cache 20, 512($0)\n"
		"cache 20, 512($0)\n"
		"cache 20, 576($0)\n"
		"cache 20, 576($0)\n"
		"cache 20, 640($0)\n"
		"cache 20, 640($0)\n"
		"cache 20, 704($0)\n"
		"cache 20, 704($0)\n"
		"cache 20, 768($0)\n"
		"cache 20, 768($0)\n"
		"cache 20, 832($0)\n"
		"cache 20, 832($0)\n"
		"cache 20, 896($0)\n"
		"cache 20, 896($0)\n"
		"cache 20, 960($0)\n"
		"cache 20, 960($0)\n"
		"cache 20, 1024($0)\n"
		"cache 20, 1024($0)\n"
		"cache 20, 1088($0)\n"
		"cache 20, 1088($0)\n"
		"cache 20, 1152($0)\n"
		"cache 20, 1152($0)\n"
		"cache 20, 1216($0)\n"
		"cache 20, 1216($0)\n"
		"cache 20, 1280($0)\n"
		"cache 20, 1280($0)\n"
		"cache 20, 1344($0)\n"
		"cache 20, 1344($0)\n"
		"cache 20, 1408($0)\n"
		"cache 20, 1408($0)\n"
		"cache 20, 1472($0)\n"
		"cache 20, 1472($0)\n"
		"cache 20, 1536($0)\n"
		"cache 20, 1536($0)\n"
		"cache 20, 1600($0)\n"
		"cache 20, 1600($0)\n"
		"cache 20, 1664($0)\n"
		"cache 20, 1664($0)\n"
		"cache 20, 1728($0)\n"
		"cache 20, 1728($0)\n"
		"cache 20, 1792($0)\n"
		"cache 20, 1792($0)\n"
		"cache 20, 1856($0)\n"
		"cache 20, 1856($0)\n"
		"cache 20, 1920($0)\n"
		"cache 20, 1920($0)\n"
		"cache 20, 1984($0)\n"
		"cache 20, 1984($0)\n"
		"cache 20, 2048($0)\n"
		"cache 20, 2048($0)\n"
		"cache 20, 2112($0)\n"
		"cache 20, 2112($0)\n"
		"cache 20, 2176($0)\n"
		"cache 20, 2176($0)\n"
		"cache 20, 2240($0)\n"
		"cache 20, 2240($0)\n"
		"cache 20, 2304($0)\n"
		"cache 20, 2304($0)\n"
		"cache 20, 2368($0)\n"
		"cache 20, 2368($0)\n"
		"cache 20, 2432($0)\n"
		"cache 20, 2432($0)\n"
		"cache 20, 2496($0)\n"
		"cache 20, 2496($0)\n"
		"cache 20, 2560($0)\n"
		"cache 20, 2560($0)\n"
		"cache 20, 2624($0)\n"
		"cache 20, 2624($0)\n"
		"cache 20, 2688($0)\n"
		"cache 20, 2688($0)\n"
		"cache 20, 2752($0)\n"
		"cache 20, 2752($0)\n"
		"cache 20, 2816($0)\n"
		"cache 20, 2816($0)\n"
		"cache 20, 2880($0)\n"
		"cache 20, 2880($0)\n"
		"cache 20, 2944($0)\n"
		"cache 20, 2944($0)\n"
		"cache 20, 3008($0)\n"
		"cache 20, 3008($0)\n"
		"cache 20, 3072($0)\n"
		"cache 20, 3072($0)\n"
		"cache 20, 3136($0)\n"
		"cache 20, 3136($0)\n"
		"cache 20, 3200($0)\n"
		"cache 20, 3200($0)\n"
		"cache 20, 3264($0)\n"
		"cache 20, 3264($0)\n"
		"cache 20, 3328($0)\n"
		"cache 20, 3328($0)\n"
		"cache 20, 3392($0)\n"
		"cache 20, 3392($0)\n"
		"cache 20, 3456($0)\n"
		"cache 20, 3456($0)\n"
		"cache 20, 3520($0)\n"
		"cache 20, 3520($0)\n"
		"cache 20, 3584($0)\n"
		"cache 20, 3584($0)\n"
		"cache 20, 3648($0)\n"
		"cache 20, 3648($0)\n"
		"cache 20, 3712($0)\n"
		"cache 20, 3712($0)\n"
		"cache 20, 3776($0)\n"
		"cache 20, 3776($0)\n"
		"cache 20, 3840($0)\n"
		"cache 20, 3840($0)\n"
		"cache 20, 3904($0)\n"
		"cache 20, 3904($0)\n"
		"cache 20, 3968($0)\n"
		"cache 20, 3968($0)\n"
		"cache 20, 4032($0)\n"
		"cache 20, 4032($0)\n"
		"cache 20, 4096($0)\n"
		"cache 20, 4096($0)\n"
		"cache 20, 4160($0)\n"
		"cache 20, 4160($0)\n"
		"cache 20, 4224($0)\n"
		"cache 20, 4224($0)\n"
		"cache 20, 4288($0)\n"
		"cache 20, 4288($0)\n"
		"cache 20, 4352($0)\n"
		"cache 20, 4352($0)\n"
		"cache 20, 4416($0)\n"
		"cache 20, 4416($0)\n"
		"cache 20, 4480($0)\n"
		"cache 20, 4480($0)\n"
		"cache 20, 4544($0)\n"
		"cache 20, 4544($0)\n"
		"cache 20, 4608($0)\n"
		"cache 20, 4608($0)\n"
		"cache 20, 4672($0)\n"
		"cache 20, 4672($0)\n"
		"cache 20, 4736($0)\n"
		"cache 20, 4736($0)\n"
		"cache 20, 4800($0)\n"
		"cache 20, 4800($0)\n"
		"cache 20, 4864($0)\n"
		"cache 20, 4864($0)\n"
		"cache 20, 4928($0)\n"
		"cache 20, 4928($0)\n"
		"cache 20, 4992($0)\n"
		"cache 20, 4992($0)\n"
		"cache 20, 5056($0)\n"
		"cache 20, 5056($0)\n"
		"cache 20, 5120($0)\n"
		"cache 20, 5120($0)\n"
		"cache 20, 5184($0)\n"
		"cache 20, 5184($0)\n"
		"cache 20, 5248($0)\n"
		"cache 20, 5248($0)\n"
		"cache 20, 5312($0)\n"
		"cache 20, 5312($0)\n"
		"cache 20, 5376($0)\n"
		"cache 20, 5376($0)\n"
		"cache 20, 5440($0)\n"
		"cache 20, 5440($0)\n"
		"cache 20, 5504($0)\n"
		"cache 20, 5504($0)\n"
		"cache 20, 5568($0)\n"
		"cache 20, 5568($0)\n"
		"cache 20, 5632($0)\n"
		"cache 20, 5632($0)\n"
		"cache 20, 5696($0)\n"
		"cache 20, 5696($0)\n"
		"cache 20, 5760($0)\n"
		"cache 20, 5760($0)\n"
		"cache 20, 5824($0)\n"
		"cache 20, 5824($0)\n"
		"cache 20, 5888($0)\n"
		"cache 20, 5888($0)\n"
		"cache 20, 5952($0)\n"
		"cache 20, 5952($0)\n"
		"cache 20, 6016($0)\n"
		"cache 20, 6016($0)\n"
		"cache 20, 6080($0)\n"
		"cache 20, 6080($0)\n"
		"cache 20, 6144($0)\n"
		"cache 20, 6144($0)\n"
		"cache 20, 6208($0)\n"
		"cache 20, 6208($0)\n"
		"cache 20, 6272($0)\n"
		"cache 20, 6272($0)\n"
		"cache 20, 6336($0)\n"
		"cache 20, 6336($0)\n"
		"cache 20, 6400($0)\n"
		"cache 20, 6400($0)\n"
		"cache 20, 6464($0)\n"
		"cache 20, 6464($0)\n"
		"cache 20, 6528($0)\n"
		"cache 20, 6528($0)\n"
		"cache 20, 6592($0)\n"
		"cache 20, 6592($0)\n"
		"cache 20, 6656($0)\n"
		"cache 20, 6656($0)\n"
		"cache 20, 6720($0)\n"
		"cache 20, 6720($0)\n"
		"cache 20, 6784($0)\n"
		"cache 20, 6784($0)\n"
		"cache 20, 6848($0)\n"
		"cache 20, 6848($0)\n"
		"cache 20, 6912($0)\n"
		"cache 20, 6912($0)\n"
		"cache 20, 6976($0)\n"
		"cache 20, 6976($0)\n"
		"cache 20, 7040($0)\n"
		"cache 20, 7040($0)\n"
		"cache 20, 7104($0)\n"
		"cache 20, 7104($0)\n"
		"cache 20, 7168($0)\n"
		"cache 20, 7168($0)\n"
		"cache 20, 7232($0)\n"
		"cache 20, 7232($0)\n"
		"cache 20, 7296($0)\n"
		"cache 20, 7296($0)\n"
		"cache 20, 7360($0)\n"
		"cache 20, 7360($0)\n"
		"cache 20, 7424($0)\n"
		"cache 20, 7424($0)\n"
		"cache 20, 7488($0)\n"
		"cache 20, 7488($0)\n"
		"cache 20, 7552($0)\n"
		"cache 20, 7552($0)\n"
		"cache 20, 7616($0)\n"
		"cache 20, 7616($0)\n"
		"cache 20, 7680($0)\n"
		"cache 20, 7680($0)\n"
		"cache 20, 7744($0)\n"
		"cache 20, 7744($0)\n"
		"cache 20, 7808($0)\n"
		"cache 20, 7808($0)\n"
		"cache 20, 7872($0)\n"
		"cache 20, 7872($0)\n"
		"cache 20, 7936($0)\n"
		"cache 20, 7936($0)\n"
		"cache 20, 8000($0)\n"
		"cache 20, 8000($0)\n"
		"cache 20, 8064($0)\n"
		"cache 20, 8064($0)\n"
		"cache 20, 8128($0)\n"
		"cache 20, 8128($0)\n"
		);
#endif
	}	
