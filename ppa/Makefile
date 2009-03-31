all:mkver
	$(MAKE) -f Makefile.3xx
ppa3xx:mkver
	$(MAKE) -f Makefile.3xx
	
clean:
	$(MAKE) -f Makefile.3xx clean
	rm -f *.bak
	rm -f mod/*.bak
	rm -f common/*.bak
mkver:
	echo "/* \n\
 *	Copyright (C) 2006 cooleyes  \n\
 *	eyes.cooleyes@gmail.com \n\
 * \n\
 *  This Program is free software; you can redistribute it and/or modify \n\
 *  it under the terms of the GNU General Public License as published by \n\
 *  the Free Software Foundation; either version 2, or (at your option) \n\
 *  any later version. \n\
 *   \n\
 *  This Program is distributed in the hope that it will be useful, \n\
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of \n\
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the \n\
 *  GNU General Public License for more details. \n\
 *   \n\
 *  You should have received a copy of the GNU General Public License \n\
 *  along with GNU Make; see the file COPYING.  If not, write to \n\
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. \n\
 *  http://www.gnu.org/copyleft/gpl.html \n\
 * \n\
 */ \n\n\
#ifndef __PPA_VERSION__ \n\
#define __PPA_VERSION__ \n\
\n\
#define PPA_NAME	\"PMPlayer advance\" \n\
#define PPA_VERSION	\"Version `tail version.txt` Built(`date +"%Y%m%d"`)\" \n\
#define PPA_COPYRIGHT	\"Copyright(C) cooleyes(eyes.cooleyes@gmail.com)\" \n\
\n\
#endif" > version.h
