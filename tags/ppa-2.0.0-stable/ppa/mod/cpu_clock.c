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
cpu clock routines
*/


#include "cpu_clock.h"

#include <psppower.h>


struct speed_setting_struct current_speed;

//add by cooleyes 2006/12/19
void cpu_clock_set_cpu_speed( int cpu ) {
	if ( cpu >= 333 ) {
		current_speed.cpu = 333;
		current_speed.ram = 333;
		current_speed.bus = 166;
	}
	else if ( cpu >= 266 ) {
		current_speed.cpu = 266;
		current_speed.ram = 266;
		current_speed.bus = 133;
	}
	else if ( cpu >= 222 ) {
		current_speed.cpu = 222;
		current_speed.ram = 222;
		current_speed.bus = 111;
	}
	else if ( cpu >= 133 ) {
		current_speed.cpu = 133;
		current_speed.ram = 222;
		current_speed.bus = 66;
	}
	else if ( cpu >= 120 ){
		current_speed.cpu = 120;
		current_speed.ram = 222;
		current_speed.bus = 60;
	}
	else {
		current_speed.cpu = 66;
		current_speed.ram = 222;
		current_speed.bus = 33;
	}
	if ( current_speed.cpu > 222 )
		scePowerSetClockFrequency(current_speed.cpu, current_speed.ram, current_speed.bus);
	else {
		scePowerSetClockFrequency(222,222,111);
		scePowerSetCpuClockFrequency(current_speed.cpu);
		scePowerSetBusClockFrequency(current_speed.bus);
	}	
}
//add end

void cpu_clock_set_speed( struct speed_setting_struct *speed ) {
	current_speed = *speed;
}


void cpu_clock_set_maximum() {
	
	scePowerSetClockFrequency(333, 333, 166);
	
}


void cpu_clock_set_minimum() {
	
	//if (current_speed.cpu<120) current_speed.cpu=120;
	//if (current_speed.ram<222) current_speed.ram=222;
	//if (current_speed.bus<60) current_speed.bus=60;
	if ( current_speed.cpu > 222 )
		scePowerSetClockFrequency(current_speed.cpu, current_speed.ram, current_speed.bus);
	else {
		scePowerSetClockFrequency(222,222,111);
		scePowerSetCpuClockFrequency(current_speed.cpu);
		scePowerSetBusClockFrequency(current_speed.bus);
	}

}
