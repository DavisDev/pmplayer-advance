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
 
#ifndef __PPA_SKIN__
#define __PPA_SKIN__

#include <psptypes.h>
#include "common/graphics.h"
#include "tinyxml/tinyxml.h"

class Skin {
private:
	static Skin* instance;
	
	TiXmlDocument xmldoc;
	
	Image* background;
	
	Skin();
	~Skin();
	bool load(const char* filename);
	
public:
	static bool loadSkin(const char* skin_path);
	static void freeSkin();
	static Skin* getInstance();
	
	bool getBooleanValue(const char* name, bool default_value);
	int getIntegerValue(const char* name, int default_value);
	float getFloatValue(const char* name, float default_value);
	const char* getStringValue(const char* name, const char* default_value);
	Color getColorValue(const char* name, Color default_value);
	Color getAlphaValue(const char* name, Color default_value);
	Image* getBackground(const char* skin_path);	
};

#endif
