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
 
#ifndef __PPA_FTFONT_H__
#define __PPA_FTFONT_H__

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_SYNTHESIS_H

#include "common/graphics.h"

class FtFont {
public:
	virtual ~FtFont(){};
	virtual void setAntiAlias(bool enable) = 0;
	virtual void setEmbolden(bool enable) = 0;
	virtual void setPixelSize(int size) = 0;
	virtual int getPixelSize() = 0;
	virtual void printStringToImage(Image* image, int x, int y, int width, int height, Color color, const char* s) = 0;
	virtual void onSuspend() = 0;
	virtual void onResume() = 0;
};

class FtFontManager {
private:
	static FtFontManager* instance;
	
	FT_Library library;
	FtFont* mainFont;
	
	FtFontManager();
	~FtFontManager();
	bool init();
public:
	static FtFontManager* getInstance();
	static void freeFtFontManager();
	bool loadMainFont(const char* filename);
	void unloadMainFont();
	FtFont* getMainFont();
};

#endif
