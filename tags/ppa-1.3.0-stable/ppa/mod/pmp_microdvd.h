/*
PMP Mod
Copyright (C) 2006 Raphael

E-mail:   raphael@fx-world.org

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
microdvd subtitle format parser
*/

#ifndef pmp_microdvd_h__
#define pmp_microdvd_h__

#include <stdio.h>
#include "pmp_parse.h"

struct pmp_sub_frame_struct* pmp_sub_parse_microdvd( FILE *f, unsigned int rate, unsigned int scale );


#endif

