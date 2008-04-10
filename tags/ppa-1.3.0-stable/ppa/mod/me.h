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


#ifndef me_h
#define me_h

int me_struct_init();
void me_start(int func, int param);
void me_wait();
int me_unused();
void me_sceKernelDcacheWritebackInvalidateAll();
void me_signal_reset();
void me_signal();
int me_signaled();

#endif
