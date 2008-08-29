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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <psputility.h>
#include "common/ctrl.h"
#include "common/libminiconv.h"
#include "common/directory.h"
#include "common/graphics.h"

#include "mod/cpu_clock.h"
#include "mod/subtitle_charset.h"
#include "mod/gu_font.h"


#include "cfgdialog.h"
#include "config.h"
#include "skin.h"
#include "usbhost.h"
#include "nethost.h"
#include "videomode.h"

#define PPA_CONFIGDLG_ALPHA	0xD0
#define PPA_CONFIGDLG_HLALPHA	0xD0
#define PPA_CONFIGDLG_BG_COLOR 0x000000
#define PPA_CONFIGDLG_BG_HLCOLOR 0x000000
#define PPA_CONFIGDLG_LABEL_COLOR 0xFFFFFF
#define PPA_CONFIGDLG_LABEL_HLCOLOR 0x00FFFF
#define PPA_CONFIGDLG_VALUE_COLOR 0x00FF00
#define PPA_CONFIGDLG_VALUE_EDCOLOR 0x0000FF

ConfigItem::ConfigItem(ConfigDialog* dialog, Image* drawImage){
	focus = false;
	editing = false;
	this->dialog = dialog;
	this->drawImage = drawImage;
	mainFont = FtFontManager::getInstance()->getMainFont();
	fontSize = mainFont->getPixelSize();
	Skin* skin = Skin::getInstance();
	alpha = skin->getAlphaValue("skin/config_dialog/dialog/highlight_alpha", PPA_CONFIGDLG_HLALPHA);
	bgHlColor = skin->getColorValue("skin/config_dialog/dialog/background_highlight_color", PPA_CONFIGDLG_BG_HLCOLOR);
	labelColor = skin->getColorValue("skin/config_dialog/dialog/label_color", PPA_CONFIGDLG_LABEL_COLOR);
	labelHlColor = skin->getColorValue("skin/config_dialog/dialog/label_highlight_color", PPA_CONFIGDLG_LABEL_HLCOLOR);
	valueColor = skin->getColorValue("skin/config_dialog/dialog/value_color", PPA_CONFIGDLG_VALUE_COLOR);
	valueEdColor = skin->getColorValue("skin/config_dialog/dialog/value_edit_color", PPA_CONFIGDLG_VALUE_EDCOLOR);
};

ConfigItem::~ConfigItem() {
	dialog = NULL;
	drawImage = NULL;
	mainFont = NULL;
};

void ConfigItem::setFocus(bool focus) {
	this->focus = focus;
};

/********************************************************************************
 *                      CpuSpeed                                                *
 ********************************************************************************/
 class CpuSpeedConfigItem : public ConfigItem {
private:
	int values[6]; 
	char* label; 
	int currentValue, newValue;
public:
	CpuSpeedConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

CpuSpeedConfigItem::CpuSpeedConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "CPU Speed: ";
	values[0] = 66;
	values[1] = 120;
	values[2] = 133;
	values[3] = 222;
	values[4] = 266;
	values[5] = 333;
	
	Config* config = Config::getInstance();
	int speed = config->getIntegerValue("config/cpu/speed", 120);
	speed = (speed>=333)?333:((speed>=266)?266:((speed>=222)?222:((speed>=133)?133:(speed>=120?120:66))));
	int i;
	for( i = 0; i< 6; i++)
		if ( values[i] == speed ) {
			currentValue = i;
			break;
		}
	if (i == 6)
		currentValue = 0;
	newValue = currentValue;
};

void CpuSpeedConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	char valueString[4];
	memset(valueString, 0, 4);
	if ( editing ) {
		sprintf(valueString, "%3d", values[newValue]);
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, valueString);
	}
	else {
		sprintf(valueString, "%3d", values[currentValue]);
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, valueString);
	}	
};

void CpuSpeedConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			Config* config = Config::getInstance();
			config->setIntegerValue("config/cpu/speed", values[newValue]);
			cpu_clock_set_cpu_speed( values[newValue] );
			currentValue = newValue;
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = (newValue > 0 ? (newValue-1) : 0);
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = (newValue < 5 ? (newValue+1) : 5);
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};


/********************************************************************************
 *                      SubCharset                                              *
 ********************************************************************************/
class SubCharsetConfigItem : public ConfigItem {
private:
	int count;
	char* label; 
	int currentValue, newValue;
public:
	SubCharsetConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

SubCharsetConfigItem::SubCharsetConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Subtitles Charset: ";

	count = miniConvGetConvCount();

	Config* config = Config::getInstance();
	const char* configValue = config->getStringValue("config/subtitles/charset/value", "UTF-8");
	int i;
	for( i = 0; i< count; i++)
		if ( stricmp(miniConvGetConvCharset(i), configValue) == 0 ) {
			currentValue = i;
			break;
		}
	if (i == count)
		currentValue = 0;
	newValue = currentValue;
};

void SubCharsetConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	if ( editing )
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, miniConvGetConvCharset(newValue));
	else
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, miniConvGetConvCharset(currentValue));
};

void SubCharsetConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			Config* config = Config::getInstance();
			config->setStringValue("config/subtitles/charset/value", miniConvGetConvCharset(newValue));
			miniConvSetSubtitleConv( miniConvGetConvCharset(newValue) );

			currentValue = newValue;
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = (newValue > 0 ? (newValue-1) : 0);
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = (newValue < (count-1) ? (newValue+1) : (count-1));
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      SubFontSize                                             *
 ********************************************************************************/
class SubFontSizeConfigItem : public ConfigItem {
private:
	char* label; 
	int currentValue, newValue;
public:
	SubFontSizeConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

SubFontSizeConfigItem::SubFontSizeConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Subtitles FontSize: ";

	Config* config = Config::getInstance();
	currentValue = config->getIntegerValue("config/subtitles/font/size", 16);
	currentValue = (currentValue <= 0 || currentValue > 48)?16:currentValue;
	newValue = currentValue;
};

void SubFontSizeConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	char valueString[64];
	memset(valueString, 0, 64);
	if ( editing ) {
		sprintf(valueString, "%d", newValue);
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, valueString);
	}
	else {
		sprintf(valueString, "%d", currentValue);
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, valueString);
	}
};

void SubFontSizeConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			Config* config = Config::getInstance();
			config->setIntegerValue("config/subtitles/font/size", newValue);
			gu_font_pixelsize_set(newValue);
			currentValue = newValue;
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = (newValue > 1 ? (newValue-1) : 1);
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = (newValue < 48 ? (newValue+1) : 48);
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      SubEmbolden                                             *
 ********************************************************************************/
class SubEmboldenConfigItem : public ConfigItem {
private:
	char* label; 
	bool currentValue, newValue;
public:
	SubEmboldenConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

SubEmboldenConfigItem::SubEmboldenConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Subtitles Embolden: ";

	Config* config = Config::getInstance();
	currentValue = config->getBooleanValue("config/subtitles/font/embolden", false);
	newValue = currentValue;
};

void SubEmboldenConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	char valueString[64];
	memset(valueString, 0, 64);
	if ( editing ) {
		if ( newValue )
			sprintf(valueString, "TRUE");
		else
			sprintf(valueString, "FALSE");
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, valueString);
	}
	else {
		if ( currentValue )
			sprintf(valueString, "TRUE");
		else
			sprintf(valueString, "FALSE");
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, valueString);
	}
};

void SubEmboldenConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			Config* config = Config::getInstance();
			config->setBooleanValue("config/subtitles/font/embolden", newValue);
			if ( newValue )
				gu_font_embolden_enable(1);
			else
				gu_font_embolden_enable(0);
			currentValue = newValue;
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = true;
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = false;
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      SubAlign                                                *
 ********************************************************************************/
class SubAlignConfigItem : public ConfigItem {
private:
	char* values[2]; 
	char* label; 
	int currentValue, newValue;
public:
	SubAlignConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

SubAlignConfigItem::SubAlignConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Subtitles Position Align: ";
	values[0] = "TOP";
	values[1] = "BOTTOM";
	
	Config* config = Config::getInstance();
	const char* configValue = config->getStringValue("config/subtitles/position/align", "bottom");
	int i;
	for( i = 0; i< 2; i++)
		if ( stricmp(values[i], configValue) == 0 ) {
			currentValue = i;
			break;
		}
	if (i == 2)
		currentValue = 1;
	newValue = currentValue;
};

void SubAlignConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	if ( editing )
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, values[newValue]);
	else
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, values[currentValue]);
	
};

void SubAlignConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			Config* config = Config::getInstance();
			config->setStringValue("config/subtitles/position/align", values[newValue]);
			gu_font_align_set( newValue );
			currentValue = newValue;
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = (newValue > 0 ? (newValue-1) : 0);
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = (newValue < 1 ? (newValue+1) : 1);
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      SubDistance                                             *
 ********************************************************************************/
class SubDistanceConfigItem : public ConfigItem {
private:
	char* label; 
	int currentValue, newValue;
public:
	SubDistanceConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

SubDistanceConfigItem::SubDistanceConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Subtitles Position Distance: ";

	Config* config = Config::getInstance();
	currentValue = config->getIntegerValue("config/subtitles/position/distance", 16);
	currentValue = (currentValue <= 0 || currentValue > 48)?16:currentValue;
	newValue = currentValue;
};

void SubDistanceConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	char valueString[64];
	memset(valueString, 0, 64);
	if ( editing ) {
		sprintf(valueString, "%d", newValue);
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, valueString);
	}
	else {
		sprintf(valueString, "%d", currentValue);
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, valueString);
	}
	
};

void SubDistanceConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			Config* config = Config::getInstance();
			config->setIntegerValue("config/subtitles/position/distance", newValue);
			gu_font_distance_set(newValue);
			currentValue = newValue;
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = (newValue > 1 ? (newValue-1) : 1);
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = (newValue < 48 ? (newValue+1) : 48);
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      FilesystemCharset                                       *
 ********************************************************************************/
class FilesystemCharsetConfigItem : public ConfigItem {
private:
	int count;
	char* label; 
	int currentValue, newValue;
public:
	FilesystemCharsetConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

FilesystemCharsetConfigItem::FilesystemCharsetConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "FileSystem Charset: ";
	count = miniConvGetConvCount();
	
	Config* config = Config::getInstance();
	const char* configValue = config->getStringValue("config/filesystem/charset/value", "UTF-8");
	int i;
	for( i = 0; i< count; i++)
		if ( stricmp(miniConvGetConvCharset(i), configValue) == 0 ) {
			currentValue = i;
			break;
		}
	if (i == count)
		currentValue = 0;
	newValue = currentValue;
};

void FilesystemCharsetConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	if ( editing )
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, miniConvGetConvCharset(newValue));
	else
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, miniConvGetConvCharset(currentValue));
};

void FilesystemCharsetConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			Config* config = Config::getInstance();
			config->setStringValue("config/filesystem/charset/value", miniConvGetConvCharset(newValue));
			miniConvSetFileSystemConv( miniConvGetConvCharset(newValue) );
			currentValue = newValue;
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = (newValue > 0 ? (newValue-1) : 0);
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = (newValue < (count-1) ? (newValue+1) : (count-1));
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      UsbHost                                                 *
 ********************************************************************************/
class UsbHostConfigItem : public ConfigItem {
private:
	char* label; 
	bool currentValue, newValue;
public:
	UsbHostConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

UsbHostConfigItem::UsbHostConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "USB Host: ";

	currentValue = UsbHost::getUsbHostState();
	newValue = currentValue;
};

void UsbHostConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	char valueString[64];
	memset(valueString, 0, 64);
	if ( editing ) {
		if ( newValue )
			sprintf(valueString, "ON");
		else
			sprintf(valueString, "OFF");
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, valueString);
	}
	else {
		if ( currentValue )
			sprintf(valueString, "ON");
		else
			sprintf(valueString, "OFF");
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, valueString);
	}
};

void UsbHostConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			bool retVal;
			if ( newValue != currentValue) {
				if ( newValue )
					retVal = UsbHost::startUsbHost();
				else
					retVal = UsbHost::stopUsbHost();
			}
			currentValue = newValue = UsbHost::getUsbHostState();
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = false;
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = true;
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      NetEntry                                             *
 ********************************************************************************/
class NetEntryConfigItem : public ConfigItem {
private:
	char* label; 
	int currentValue, newValue;
public:
	NetEntryConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

NetEntryConfigItem::NetEntryConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Net Entry: ";

	Config* config = Config::getInstance();
	currentValue = config->getIntegerValue("config/filesystem/net_filesystem/entry", 1);
	currentValue = (currentValue < 1 || currentValue>100)?1:currentValue;
	config->setIntegerValue("config/filesystem/net_filesystem/entry", currentValue);
	newValue = currentValue;
};

void NetEntryConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	char valueString[256];
	memset(valueString, 0, 256);
	if ( editing ) {
#ifdef ENABLE_NETHOST
		sceUtilityGetNetParam(newValue, 0, (netData*)valueString);
#endif
		if ( strlen(valueString) == 0 ) 
			sprintf(valueString, "<Not used>");
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, valueString);
	}
	else {
#ifdef ENABLE_NETHOST
		sceUtilityGetNetParam(currentValue, 0, (netData*)valueString);
#endif
		if ( strlen(valueString) == 0 ) 
			sprintf(valueString, "<Not used>");
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, valueString);
	}
};

void NetEntryConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			Config* config = Config::getInstance();
			config->setIntegerValue("config/filesystem/net_filesystem/entry", newValue);
			currentValue = newValue;
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = (newValue > 1 ? (newValue-1) : 1);
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = (newValue < 100 ? (newValue+1) : 100);
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      NetAddress                                              *
 ********************************************************************************/
class NetAddressConfigItem : public ConfigItem {
private:
	char* label; 
	char currentValue[16+1];
	u8 newValue[4];
	int valueIndex;
public:
	NetAddressConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

NetAddressConfigItem::NetAddressConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Net Address: ";

	Config* config = Config::getInstance();
	memset( currentValue, 0, 16+1 );
	strncpy( currentValue, config->getStringValue("config/filesystem/net_filesystem/address", "192.168.0.1"), 16);
	int a0, a1, a2, a3;
	a0 = a1 = a2 = a3 = 0;
	sscanf(currentValue, "%d.%d.%d.%d", &a0, &a1, &a2, &a3);
	newValue[0] = (u8)a0;
	newValue[1] = (u8)a1;
	newValue[2] = (u8)a2;
	newValue[3] = (u8)a3;
	sprintf( currentValue, "%d.%d.%d.%d", newValue[0], newValue[1], newValue[2], newValue[3]);
};

void NetAddressConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	if ( editing ) {
		char s0[4], s1[4], s2[4], s3[4];
		memset(s0, 0, 4);
		sprintf(s0, "%d", newValue[0]);
		memset(s1, 0, 4);
		sprintf(s1, "%d", newValue[1]);
		memset(s2, 0, 4);
		sprintf(s2, "%d", newValue[2]);
		memset(s3, 0, 4);
		sprintf(s3, "%d", newValue[3]);
		int sx0, sx1, sx2, sx3;
		sx0 = x1;
		sx1 = sx0 + strlen(s0)*fontSize / 2 + fontSize / 2;
		sx2 = sx1 + strlen(s1)*fontSize / 2 + fontSize / 2;
		sx3 = sx2 + strlen(s2)*fontSize / 2 + fontSize / 2;
		if ( valueIndex == 0 )
			mainFont->printStringToImage(drawImage, sx0, y+fontSize-1, w-2-sx0+x, h-2, valueEdColor, s0);
		else
			mainFont->printStringToImage(drawImage, sx0, y+fontSize-1, w-2-sx0+x, h-2, valueColor, s0);
		
		mainFont->printStringToImage(drawImage, sx1-fontSize / 2, y+fontSize-1, fontSize / 2, h-2, valueColor, ".");
		
		if ( valueIndex == 1 )
			mainFont->printStringToImage(drawImage, sx1, y+fontSize-1, w-2-sx1+x, h-2, valueEdColor, s1);
		else
			mainFont->printStringToImage(drawImage, sx1, y+fontSize-1, w-2-sx1+x, h-2, valueColor, s1);
		
		mainFont->printStringToImage(drawImage, sx2-fontSize / 2, y+fontSize-1, fontSize / 2, h-2, valueColor, ".");
		
		if ( valueIndex == 2 )
			mainFont->printStringToImage(drawImage, sx2, y+fontSize-1, w-2-sx2+x, h-2, valueEdColor, s2);
		else
			mainFont->printStringToImage(drawImage, sx2, y+fontSize-1, w-2-sx2+x, h-2, valueColor, s2);
		
		mainFont->printStringToImage(drawImage, sx3-fontSize / 2, y+fontSize-1, fontSize / 2, h-2, valueColor, ".");
		
		if ( valueIndex == 3 )
			mainFont->printStringToImage(drawImage, sx3, y+fontSize-1, w-2-sx3+x, h-2, valueEdColor, s3);
		else
			mainFont->printStringToImage(drawImage, sx3, y+fontSize-1, w-2-sx3+x, h-2, valueColor, s3);
	}
	else {
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, currentValue);
	}
};

void NetAddressConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	valueIndex = 0;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			int a0, a1, a2, a3;
			a0 = a1 = a2 = a3 = 0;
			sscanf(currentValue, "%d.%d.%d.%d", &a0, &a1, &a2, &a3);
			newValue[0] = (u8)a0;
			newValue[1] = (u8)a1;
			newValue[2] = (u8)a2;
			newValue[3] = (u8)a3;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			char valueString[16+1];
			memset(valueString, 0, 16+1);
			sprintf(valueString, "%d.%d.%d.%d", newValue[0], newValue[1], newValue[2], newValue[3]);
			Config* config = Config::getInstance();
			config->setStringValue("config/filesystem/net_filesystem/address", valueString);
			memset( currentValue, 0, 16+1);
			strncpy(currentValue, valueString, 16);
			break;
		}
		else if ( key & PSP_CTRL_LEFT) {
			if ( valueIndex > 0 ) valueIndex--;
		}
		else if ( key & PSP_CTRL_RIGHT) {
			if ( valueIndex < 3 ) valueIndex++;
		}
		else if ( key & PSP_CTRL_UP ) {
			if ( newValue[valueIndex] > 0 ) newValue[valueIndex]--;
		}
		else if ( key & PSP_CTRL_DOWN ) {
			if ( newValue[valueIndex] < 255 ) newValue[valueIndex]++;
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      NetAdhocAddress                                              *
 ********************************************************************************/
class NetAdhocAddressConfigItem : public ConfigItem {
private:
	char* label; 
	char currentValue[16+1];
	u8 newValue[4];
	int valueIndex;
public:
	NetAdhocAddressConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

NetAdhocAddressConfigItem::NetAdhocAddressConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Net Adhoc Address: ";

	Config* config = Config::getInstance();
	memset( currentValue, 0, 16+1 );
	strncpy( currentValue, config->getStringValue("config/filesystem/net_filesystem/adhoc_address", "192.168.0.1"), 16);
	int a0, a1, a2, a3;
	a0 = a1 = a2 = a3 = 0;
	sscanf(currentValue, "%d.%d.%d.%d", &a0, &a1, &a2, &a3);
	newValue[0] = (u8)a0;
	newValue[1] = (u8)a1;
	newValue[2] = (u8)a2;
	newValue[3] = (u8)a3;
	sprintf( currentValue, "%d.%d.%d.%d", newValue[0], newValue[1], newValue[2], newValue[3]);
};

void NetAdhocAddressConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	if ( editing ) {
		char s0[4], s1[4], s2[4], s3[4];
		memset(s0, 0, 4);
		sprintf(s0, "%d", newValue[0]);
		memset(s1, 0, 4);
		sprintf(s1, "%d", newValue[1]);
		memset(s2, 0, 4);
		sprintf(s2, "%d", newValue[2]);
		memset(s3, 0, 4);
		sprintf(s3, "%d", newValue[3]);
		int sx0, sx1, sx2, sx3;
		sx0 = x1;
		sx1 = sx0 + strlen(s0)*fontSize / 2 + fontSize / 2;
		sx2 = sx1 + strlen(s1)*fontSize / 2 + fontSize / 2;
		sx3 = sx2 + strlen(s2)*fontSize / 2 + fontSize / 2;
		if ( valueIndex == 0 )
			mainFont->printStringToImage(drawImage, sx0, y+fontSize-1, w-2-sx0+x, h-2, valueEdColor, s0);
		else
			mainFont->printStringToImage(drawImage, sx0, y+fontSize-1, w-2-sx0+x, h-2, valueColor, s0);
		
		mainFont->printStringToImage(drawImage, sx1-fontSize / 2, y+fontSize-1, fontSize / 2, h-2, valueColor, ".");
		
		if ( valueIndex == 1 )
			mainFont->printStringToImage(drawImage, sx1, y+fontSize-1, w-2-sx1+x, h-2, valueEdColor, s1);
		else
			mainFont->printStringToImage(drawImage, sx1, y+fontSize-1, w-2-sx1+x, h-2, valueColor, s1);
		
		mainFont->printStringToImage(drawImage, sx2-fontSize / 2, y+fontSize-1, fontSize / 2, h-2, valueColor, ".");
		
		if ( valueIndex == 2 )
			mainFont->printStringToImage(drawImage, sx2, y+fontSize-1, w-2-sx2+x, h-2, valueEdColor, s2);
		else
			mainFont->printStringToImage(drawImage, sx2, y+fontSize-1, w-2-sx2+x, h-2, valueColor, s2);
		
		mainFont->printStringToImage(drawImage, sx3-fontSize / 2, y+fontSize-1, fontSize / 2, h-2, valueColor, ".");
		
		if ( valueIndex == 3 )
			mainFont->printStringToImage(drawImage, sx3, y+fontSize-1, w-2-sx3+x, h-2, valueEdColor, s3);
		else
			mainFont->printStringToImage(drawImage, sx3, y+fontSize-1, w-2-sx3+x, h-2, valueColor, s3);
	}
	else {
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, currentValue);
	}
};

void NetAdhocAddressConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	valueIndex = 0;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			int a0, a1, a2, a3;
			a0 = a1 = a2 = a3 = 0;
			sscanf(currentValue, "%d.%d.%d.%d", &a0, &a1, &a2, &a3);
			newValue[0] = (u8)a0;
			newValue[1] = (u8)a1;
			newValue[2] = (u8)a2;
			newValue[3] = (u8)a3;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			char valueString[16+1];
			memset(valueString, 0, 16+1);
			sprintf(valueString, "%d.%d.%d.%d", newValue[0], newValue[1], newValue[2], newValue[3]);
			Config* config = Config::getInstance();
			config->setStringValue("config/filesystem/net_filesystem/adhoc_address", valueString);
			memset( currentValue, 0, 16+1);
			strncpy(currentValue, valueString, 16);
			break;
		}
		else if ( key & PSP_CTRL_LEFT) {
			if ( valueIndex > 0 ) valueIndex--;
		}
		else if ( key & PSP_CTRL_RIGHT) {
			if ( valueIndex < 3 ) valueIndex++;
		}
		else if ( key & PSP_CTRL_UP ) {
			if ( newValue[valueIndex] > 0 ) newValue[valueIndex]--;
		}
		else if ( key & PSP_CTRL_DOWN ) {
			if ( newValue[valueIndex] < 255 ) newValue[valueIndex]++;
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      NetPort                                             *
 ********************************************************************************/
class NetPortConfigItem : public ConfigItem {
private:
	char* label; 
	int currentValue, newValue;
public:
	NetPortConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

NetPortConfigItem::NetPortConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Net Port: ";

	Config* config = Config::getInstance();
	currentValue = config->getIntegerValue("config/filesystem/net_filesystem/port", 7513);
	currentValue = (currentValue <= 1000 || currentValue > 100000)?7513:currentValue;
	newValue = currentValue;
};

void NetPortConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	char valueString[64];
	memset(valueString, 0, 64);
	if ( editing ) {
		sprintf(valueString, "%d", newValue);
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, valueString);
	}
	else {
		sprintf(valueString, "%d", currentValue);
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, valueString);
	}
};

void NetPortConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			Config* config = Config::getInstance();
			config->setIntegerValue("config/filesystem/net_filesystem/port", newValue);
			currentValue = newValue;
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = (newValue > 1000 ? (newValue-1) : 1000);
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = (newValue < 100000 ? (newValue+1) : 100000);
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      NetHost                                                 *
 ********************************************************************************/
class NetHostConfigItem : public ConfigItem {
private:
	char* label; 
	bool currentValue, newValue;
public:
	NetHostConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

NetHostConfigItem::NetHostConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Net Host: ";

	currentValue = NetHost::getNetHostState();
	newValue = currentValue;
};

void NetHostConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	char valueString[64];
	memset(valueString, 0, 64);
	if ( editing ) {
		if ( newValue )
			sprintf(valueString, "ON");
		else
			sprintf(valueString, "OFF");
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, valueString);
	}
	else {
		if ( currentValue )
			sprintf(valueString, "ON");
		else
			sprintf(valueString, "OFF");
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, valueString);
	}
};

void NetHostConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			bool retVal;
			if ( newValue != currentValue) {
				if ( newValue ) {
					Config* config = Config::getInstance();
					char address[16+1];
					char port[8+1];
					char entry[9+1];
					char password[] = "";
					char adhocAddress[16+1];
					strncpy( address, config->getStringValue("config/filesystem/net_filesystem/address", "192.168.0.1"), 16);
					strncpy( port, config->getStringValue("config/filesystem/net_filesystem/port", "7513"), 8);
					strncpy( entry, config->getStringValue("config/filesystem/net_filesystem/entry", "1"), 9);
					strncpy( adhocAddress, config->getStringValue("config/filesystem/net_filesystem/adhoc_address", "192.168.0.1"), 16);
					retVal = NetHost::startNetHost(address, port, entry, password, adhocAddress);
				}
				else
					retVal = NetHost::stopNetHost();
			}
			currentValue = newValue = NetHost::getNetHostState();
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = false;
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = true;
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      PlayMode                                                *
 ********************************************************************************/
class PlayModeConfigItem : public ConfigItem {
private:
	char* values[3]; 
	char* label; 
	int currentValue, newValue;
public:
	PlayModeConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

PlayModeConfigItem::PlayModeConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Play Mode: ";
	values[0] = "SINGLE";
	values[1] = "GROUP";
	values[2] = "ALL";
	
	Config* config = Config::getInstance();
	const char* configValue = config->getStringValue("config/player/play_mode", "group");
	int i;
	for( i = 0; i< 3; i++)
		if ( stricmp(values[i], configValue) == 0 ) {
			currentValue = i;
			break;
		}
	if (i == 3)
		currentValue = 1;
	newValue = currentValue;
};

void PlayModeConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	if ( editing )
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, values[newValue]);
	else
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, values[currentValue]);
	
};

void PlayModeConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			Config* config = Config::getInstance();
			config->setStringValue("config/player/play_mode", values[newValue]);
			currentValue = newValue;
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = (newValue > 0 ? (newValue-1) : 0);
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = (newValue < 2 ? (newValue+1) : 2);
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      TVAspectRatio                                                *
 ********************************************************************************/
class TVAspectRatioConfigItem : public ConfigItem {
private:
	char* values[2]; 
	char* label; 
	int currentValue, newValue;
public:
	TVAspectRatioConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

TVAspectRatioConfigItem::TVAspectRatioConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "TV AspectRatio: ";
	values[0] = "16:9";
	values[1] = "4:3";
		
	Config* config = Config::getInstance();
	const char* configValue = config->getStringValue("config/tvout/aspect_ratio", "16:9");
	int i;
	for( i = 0; i< 2; i++)
		if ( stricmp(values[i], configValue) == 0 ) {
			currentValue = i;
			break;
		}
	if (i == 2)
		currentValue = 0;
	newValue = currentValue;
};

void TVAspectRatioConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	if ( editing )
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, values[newValue]);
	else
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, values[currentValue]);
	
};

void TVAspectRatioConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			if ( newValue != currentValue) {
				Config* config = Config::getInstance();
				config->setStringValue("config/tvout/aspect_ratio", values[newValue]);
				VideoMode::setTVAspectRatio(newValue);
				setGraphicsTVAspectRatio(newValue);
				setGraphicsTVOutScreen();
			}
			currentValue = newValue = VideoMode::getTVAspectRatio();
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = (newValue > 0 ? (newValue-1) : 0);
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = (newValue < 1 ? (newValue+1) : 1);
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      TV OverScan                                              *
 ********************************************************************************/
class TVOverScanConfigItem : public ConfigItem {
private:
	char* label; 
	char currentValue[16+1];
	u8 newValue[4];
	int valueIndex;
public:
	TVOverScanConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

TVOverScanConfigItem::TVOverScanConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "TV OverScan(L.T.R.B): ";

	Config* config = Config::getInstance();
	memset( currentValue, 0, 16+1 );
	strncpy( currentValue, config->getStringValue("config/tvout/over_scan", "8.0.8.0"), 16);
	int a0, a1, a2, a3;
	a0 = a1 = a2 = a3 = 0;
	sscanf(currentValue, "%d.%d.%d.%d", &a0, &a1, &a2, &a3);
	newValue[0] = (u8)a0;
	newValue[1] = (u8)a1;
	newValue[2] = (u8)a2;
	newValue[3] = (u8)a3;
	sprintf( currentValue, "%d.%d.%d.%d", newValue[0], newValue[1], newValue[2], newValue[3]);
};

void TVOverScanConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	if ( editing ) {
		char s0[4], s1[4], s2[4], s3[4];
		memset(s0, 0, 4);
		sprintf(s0, "%d", newValue[0]);
		memset(s1, 0, 4);
		sprintf(s1, "%d", newValue[1]);
		memset(s2, 0, 4);
		sprintf(s2, "%d", newValue[2]);
		memset(s3, 0, 4);
		sprintf(s3, "%d", newValue[3]);
		int sx0, sx1, sx2, sx3;
		sx0 = x1;
		sx1 = sx0 + strlen(s0)*fontSize / 2 + fontSize / 2;
		sx2 = sx1 + strlen(s1)*fontSize / 2 + fontSize / 2;
		sx3 = sx2 + strlen(s2)*fontSize / 2 + fontSize / 2;
		if ( valueIndex == 0 )
			mainFont->printStringToImage(drawImage, sx0, y+fontSize-1, w-2-sx0+x, h-2, valueEdColor, s0);
		else
			mainFont->printStringToImage(drawImage, sx0, y+fontSize-1, w-2-sx0+x, h-2, valueColor, s0);
		
		mainFont->printStringToImage(drawImage, sx1-fontSize / 2, y+fontSize-1, fontSize / 2, h-2, valueColor, ".");
		
		if ( valueIndex == 1 )
			mainFont->printStringToImage(drawImage, sx1, y+fontSize-1, w-2-sx1+x, h-2, valueEdColor, s1);
		else
			mainFont->printStringToImage(drawImage, sx1, y+fontSize-1, w-2-sx1+x, h-2, valueColor, s1);
		
		mainFont->printStringToImage(drawImage, sx2-fontSize / 2, y+fontSize-1, fontSize / 2, h-2, valueColor, ".");
		
		if ( valueIndex == 2 )
			mainFont->printStringToImage(drawImage, sx2, y+fontSize-1, w-2-sx2+x, h-2, valueEdColor, s2);
		else
			mainFont->printStringToImage(drawImage, sx2, y+fontSize-1, w-2-sx2+x, h-2, valueColor, s2);
		
		mainFont->printStringToImage(drawImage, sx3-fontSize / 2, y+fontSize-1, fontSize / 2, h-2, valueColor, ".");
		
		if ( valueIndex == 3 )
			mainFont->printStringToImage(drawImage, sx3, y+fontSize-1, w-2-sx3+x, h-2, valueEdColor, s3);
		else
			mainFont->printStringToImage(drawImage, sx3, y+fontSize-1, w-2-sx3+x, h-2, valueColor, s3);
	}
	else {
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, currentValue);
	}
};

void TVOverScanConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	valueIndex = 0;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			int a0, a1, a2, a3;
			a0 = a1 = a2 = a3 = 0;
			sscanf(currentValue, "%d.%d.%d.%d", &a0, &a1, &a2, &a3);
			newValue[0] = (u8)a0;
			newValue[1] = (u8)a1;
			newValue[2] = (u8)a2;
			newValue[3] = (u8)a3;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			char valueString[16+1];
			memset(valueString, 0, 16+1);
			sprintf(valueString, "%d.%d.%d.%d", newValue[0], newValue[1], newValue[2], newValue[3]);
			Config* config = Config::getInstance();
			config->setStringValue("config/tvout/over_scan", valueString);
			memset( currentValue, 0, 16+1);
			strncpy(currentValue, valueString, 16);
			VideoMode::setTVOverScan(newValue[0], newValue[1], newValue[2], newValue[3]);
			setGraphicsTVOverScan(newValue[0], newValue[1], newValue[2], newValue[3]);
			setGraphicsTVOutScreen();
			break;
		}
		else if ( key & PSP_CTRL_LEFT) {
			if ( valueIndex > 0 ) valueIndex--;
		}
		else if ( key & PSP_CTRL_RIGHT) {
			if ( valueIndex < 3 ) valueIndex++;
		}
		else if ( key & PSP_CTRL_UP ) {
			if ( newValue[valueIndex] > 0 ) newValue[valueIndex]--;
		}
		else if ( key & PSP_CTRL_DOWN ) {
			if ( newValue[valueIndex] < 50 ) newValue[valueIndex]++;
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      VideoMode                                                *
 ********************************************************************************/
class VideoModeConfigItem : public ConfigItem {
private:
	char* values[4]; 
	char* label; 
	int currentValue, newValue;
public:
	VideoModeConfigItem(ConfigDialog* dialog, Image* drawImage);
	virtual void paint(int x, int y, int w, int h) ;
	virtual void enterEditStatus();
};

VideoModeConfigItem::VideoModeConfigItem(ConfigDialog* dialog, Image* drawImage) : ConfigItem(dialog, drawImage) {
	label = "Video Mode: ";
	values[0] = "PSP LCD";
	values[1] = "Composite";
	values[2] = "Component Interlace";
	values[3] = "Component Progressive";
	currentValue = 0;
	currentValue = VideoMode::getVideoMode();
	newValue = currentValue;
};

void VideoModeConfigItem::paint(int x, int y, int w, int h) {
	if ( focus ) {
		fillImageRect(drawImage, (alpha << 24) | bgHlColor, x, y, w, h);
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelHlColor, label );  
	}
	else
		mainFont->printStringToImage(drawImage, x+1, y+fontSize-1, w-2, h-2, labelColor, label );
	
	int x1 = x+1 + strlen(label)*fontSize / 2;
	
	if ( editing )
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueEdColor, values[newValue]);
	else
		mainFont->printStringToImage(drawImage, x1, y+fontSize-1, w-2-x1+x, h-2, valueColor, values[currentValue]);
	
};

void VideoModeConfigItem::enterEditStatus() {
	if ( focus == false) 
		return;
	editing = true;
	while( true ) {
		u32 key = ctrl_read();
		if ( key & PSP_CTRL_CROSS ) {
			newValue = currentValue;
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			if ( newValue != currentValue) {
				if ( VideoMode::setVideoMode(newValue) == newValue ) {
					setGraphicsVideoMode(VideoMode::getVideoMode());
				}
			}
			currentValue = newValue = VideoMode::getVideoMode();
			break;
		}
		else if ( (key & PSP_CTRL_LEFT) || (key & PSP_CTRL_UP) ) {
			newValue = (newValue > 0 ? (newValue-1) : 0);
		}
		else if ( (key & PSP_CTRL_RIGHT) || (key & PSP_CTRL_DOWN) ) {
			newValue = (newValue < 3 ? (newValue+1) : 3);
		}
		dialog->paint();
		sceKernelDelayThread(12500);
	};
	editing = false;
};

/********************************************************************************
 *                      ConfigDialog                                            *
 ********************************************************************************/
#define PPA_CONFIG_ITEMS 17

#define CONFIG_DIALOG_X		60
#define CONFIG_DIALOG_Y		4	
#define CONFIG_DIALOG_W		360
#define CONFIG_DIALOG_H		264
#define CONFIG_DIALOG_R		6
#define CONFIG_DIALOG_ITEM_X    90
#define CONFIG_DIALOG_ITEM_Y	24
#define CONFIG_DIALOG_ITEM_W	300
#define CONFIG_DIALOG_ITEM_H	238

ConfigDialog::ConfigDialog(Image* mainWindow, Image* mainDrawImage) {
	
	itemBottom = itemTop = itemCurrent = itemCount = 0;
	itemCount = PPA_CONFIG_ITEMS;
	
	items = (PConfigItem*)malloc( sizeof(PConfigItem)*itemCount);
	int i;
	for(i = 0; i < itemCount; i++) 
		items[i] = NULL;
	
//	screenSnapshot = createImage(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	this->mainWindow = mainWindow;
	this->mainDrawImage = mainDrawImage;
	drawImage = createImage(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
};

ConfigDialog::~ConfigDialog() {
//	freeImage(screenSnapshot);
	mainWindow = NULL;
	mainDrawImage = NULL;
	freeImage(drawImage);
	int i;
	for(i = 0; i < itemCount; i++) 
		if (items[i])
			delete items[i];
	free(items);
	
	mainFont = NULL;
};

bool ConfigDialog::init() {
//	if ( screenSnapshot == NULL )
//		return false;
	if ( drawImage == NULL )
		return false;
		
//	makeScreenSnapshot(screenSnapshot);
	
	items[0] = new CpuSpeedConfigItem(this, this->drawImage);
	items[1] = new SubCharsetConfigItem(this, this->drawImage);
	items[2] = new SubFontSizeConfigItem(this, this->drawImage);
	items[3] = new SubEmboldenConfigItem(this, this->drawImage);
	items[4] = new SubAlignConfigItem(this, this->drawImage);
	items[5] = new SubDistanceConfigItem(this, this->drawImage);
	items[6] = new FilesystemCharsetConfigItem(this, this->drawImage);
	items[7] = new UsbHostConfigItem(this, this->drawImage);
	items[8] = new NetEntryConfigItem(this, this->drawImage);
	items[9] = new NetAddressConfigItem(this, this->drawImage);
	items[10] = new NetAdhocAddressConfigItem(this, this->drawImage);
	items[11] = new NetPortConfigItem(this, this->drawImage);
	items[12] = new NetHostConfigItem(this, this->drawImage);
	items[13] = new PlayModeConfigItem(this, this->drawImage);
	items[14] = new TVAspectRatioConfigItem(this, this->drawImage);
	items[15] = new TVOverScanConfigItem(this, this->drawImage);
	items[16] = new VideoModeConfigItem(this, this->drawImage);
	
	mainFont = FtFontManager::getInstance()->getMainFont();
	fontSize = (Config::getInstance())->getIntegerValue("config/windows/font/size",12);
	
	itemBottom = CONFIG_DIALOG_ITEM_H / ( fontSize + 2 );
	
	items[itemCurrent]->setFocus(true);
	
	Skin* skin = Skin::getInstance();
	alpha = skin->getAlphaValue("skin/config_dialog/dialog/alpha", PPA_CONFIGDLG_ALPHA);
	bgColor = skin->getColorValue("skin/config_dialog/dialog/background_color", PPA_CONFIGDLG_BG_COLOR);
	labelColor = skin->getColorValue("skin/config_dialog/dialog/label_color", PPA_CONFIGDLG_LABEL_COLOR);
	
	title = "Configuration";
	help = "";
	return true;
};

void ConfigDialog::execute() {
	while( true ) {
		u32 key = ctrl_read();
		if (( key & PSP_CTRL_TRIANGLE ) || (key & PSP_CTRL_SQUARE)) {
			break;
		}
		else if ( key & PSP_CTRL_CIRCLE ) {
			items[itemCurrent]->enterEditStatus();
		}
		else if( (key & PSP_CTRL_UP) || (key & CTRL_BACK) ) {
			items[itemCurrent]->setFocus(false);
			if (itemCurrent != 0){
				itemCurrent--;
			}
			else {
				itemCurrent = itemCount - 1;
			}
			items[itemCurrent]->setFocus(true);
		}
		else if( (key & PSP_CTRL_DOWN) || (key & CTRL_FORWARD) ){
			items[itemCurrent]->setFocus(false);
			if (itemCurrent + 1 < itemCount) {
				itemCurrent++;
			}
			else 
				itemCurrent = 0;
			items[itemCurrent]->setFocus(true);
		}
		else if (key & PSP_CTRL_LTRIGGER ) {
			items[itemCurrent]->setFocus(false);
			itemCurrent = 0;
			items[itemCurrent]->setFocus(true);
		}
		else if (key & PSP_CTRL_RTRIGGER ) {
			items[itemCurrent]->setFocus(false);
			itemCurrent = itemCount - 1;
			items[itemCurrent]->setFocus(true);
		}
		paint();
		sceKernelDelayThread(12500);
	};
};

void ConfigDialog::paint() {
	
	clearImage(drawImage, 0);
	
//	fillImageRect(drawImage, (alpha << 24) | bgColor, CONFIG_DIALOG_X, CONFIG_DIALOG_Y, CONFIG_DIALOG_W, CONFIG_DIALOG_H);
	fillImageEllipse(drawImage, (alpha << 24) | bgColor, CONFIG_DIALOG_X, CONFIG_DIALOG_Y, CONFIG_DIALOG_W, CONFIG_DIALOG_H, CONFIG_DIALOG_R);
	
	int titleX = CONFIG_DIALOG_X + (CONFIG_DIALOG_W - strlen(title)*fontSize/2)/2;
	mainFont->printStringToImage(drawImage, titleX, CONFIG_DIALOG_Y+2+fontSize-1, CONFIG_DIALOG_W-titleX+CONFIG_DIALOG_X, fontSize+2, labelColor, title);
	
	int helpX = CONFIG_DIALOG_X + (CONFIG_DIALOG_W - strlen(help)*fontSize/2)/2;
	mainFont->printStringToImage(drawImage, helpX, CONFIG_DIALOG_Y+CONFIG_DIALOG_H-4-fontSize+fontSize-1, CONFIG_DIALOG_W-helpX+CONFIG_DIALOG_X, fontSize+2, labelColor, help);
	
	if ( itemCurrent < itemTop ) {
		itemTop = itemCurrent;
	}
	else if (itemCurrent - itemTop >= itemBottom ){
		itemTop = itemCurrent - itemBottom + 1;
	}
	
	int i;
	for(i=0;i<itemBottom;i++) {
		if( itemTop + i < itemCount) {
			items[i]->paint(CONFIG_DIALOG_ITEM_X, CONFIG_DIALOG_ITEM_Y + i*(fontSize+2), CONFIG_DIALOG_ITEM_W, fontSize + 2 );
		}
	}
	
	guStart();	
	clearScreen();	
//	blitImageToScreen(0, 0, screenSnapshot->imageWidth, screenSnapshot->imageHeight, screenSnapshot, 0, 0);
	blitImageToScreen(0, 0, mainWindow->imageWidth, mainWindow->imageHeight, mainWindow, 0, 0);
	blitAlphaImageToScreen(0, 0, mainDrawImage->imageWidth, mainDrawImage->imageHeight, mainDrawImage, 0, 0);
	blitAlphaImageToScreen(0, 0, drawImage->imageWidth, drawImage->imageHeight, drawImage, 0, 0);
	flipScreen();
};

