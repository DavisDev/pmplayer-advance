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
 
#ifndef __PPA_VIDEO_OUT_H__
#define __PPA_VIDEO_OUT_H__

class VideoMode {
	static int videoMode;
	static int pspType;
	static int tvAspectRatio;
private:
	VideoMode() {};
public:
	static bool init(int type, const char* prx);
	static int setVideoMode(int mode);
	static int getVideoMode() { return videoMode; };
	static void setTVAspectRatio(int ar) { tvAspectRatio = ar; };
	static int getTVAspectRatio() { return tvAspectRatio; };
};

#endif
