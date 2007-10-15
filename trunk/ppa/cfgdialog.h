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
 
#ifndef __PPA_CONFIG_DIALOG_H__
#define __PPA_CONFIG_DIALOG_H__

#include "common/graphics.h"
#include "ftfont.h"

class ConfigDialog;

class ConfigItem {
protected:
	bool focus, editing;
	ConfigDialog* dialog;
	Image* drawImage;
	FtFont* mainFont;
	int fontSize;
	Color alpha, bgHlColor, labelColor, labelHlColor, valueColor, valueEdColor;
public:
	ConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual ~ConfigItem();
	virtual void paint(int x, int y, int w, int h) = 0;
	virtual void enterEditStatus() = 0;
	void setFocus(bool focus);
};

typedef ConfigItem* PConfigItem;

class ConfigDialog {
private:
	Image* screenSnapshot;
	Image* drawImage;
	PConfigItem* items;
	int itemBottom, itemTop, itemCurrent, itemCount;
	FtFont* mainFont;
	int fontSize;
	Color alpha, bgColor, labelColor;
	const char* title;
	const char* help;
public:
	
	ConfigDialog();
	~ConfigDialog();
	bool init();
	void execute();
	void paint();
};

#endif
