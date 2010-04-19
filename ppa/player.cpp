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
#include <unistd.h>
#include <string.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <psprtc.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspdebug.h>

#include "player.h"
#include "config.h"
#include "skin.h"
#include "ftfont.h"
#include "versiondialog.h"
#include "cfgdialog.h"
#include "msgdialog.h"
#include "skindialog.h"
#include "usbhost.h"
#include "nethost.h"
#include "videomode.h"

#include "common/libminiconv.h"
#include "common/libi18n.h"
#include "common/fat.h"
#include "common/directory.h"
#include "common/ctrl.h"
#include "common/imagefile.h"
#include "common/base64.h"
#include "common/m33sdk.h"


#include "mod/subtitle_charset.h"
#include "mod/cpu_clock.h"
#include "mod/mp4avcdecoder.h"
#include "mod/psp1k_frame_buffer.h"

#include "mod/pmp.h"

#include "mod/mp4.h"

#include "mod/mkv.h"

#include "mod/flv1.h"

#include "mod/codec_prx.h"
#include "mod/gu_font.h"
#include "mod/movie_file.h"
#include "mod/movie_stat.h"


#define TEXT_ITEM_BORDER 1

struct movie_file_struct currentMovie;

struct subtitle_ext_charset_struct subtitleExt[] = {
	{".srt", 4, "DEFAULT"},
	{".sub", 4, "DEFAULT"},
	{".ass", 4, "DEFAULT"},
	{".ssa", 4, "DEFAULT"},
	{".en.srt", 7, "UTF-8"},
	{".ch.srt", 7, "GBK"},
	{".eng.srt", 8, "UTF-8"},
	{".gb.srt", 7, "GBK"},
	{".chs.srt", 8, "GBK"},
	{".sc.srt", 7, "GBK"},
	{".big5.srt", 9, "BIG5"},
	{".cht.srt", 8, "BIG5"},
	{".tc.srt", 7, "BIG5"},
	{".en.ass", 7, "UTF-8"},
	{".ch.ass", 7, "GBK"},
	{".eng.ass", 8, "UTF-8"},
	{".gb.ass", 7, "GBK"},
	{".chs.ass", 8, "GBK"},
	{".sc.ass", 7, "GBK"},
	{".big5.ass", 9, "BIG5"},
	{".cht.ass", 8, "BIG5"},
	{".tc.ass", 7, "BIG5"},
	{".en.ssa", 7, "UTF-8"},
	{".ch.ssa", 7, "GBK"},
	{".eng.ssa", 8, "UTF-8"},
	{".gb.ssa", 7, "GBK"},
	{".chs.ssa", 8, "GBK"},
	{".sc.ssa", 7, "GBK"},
	{".big5.ssa", 9, "BIG5"},
	{".cht.ssa", 8, "BIG5"},
	{".tc.ssa", 7, "BIG5"},
	{NULL, 0, NULL}	
};

file_type_ext_struct movieFileFilter[] = {
	{"pmp", FS_PMP_FILE},
	{"mp4", FS_MP4_FILE},
	{"3gp", FS_MP4_FILE},
	{"mkv", FS_MKV_FILE},
	{"flv", FS_FLV1_FILE},
	{NULL, FS_UNKNOWN_FILE}
};

file_type_ext_struct movieSubtitleFilter[] = {
	{"sub", FS_SUB_FILE},
	{"srt", FS_SRT_FILE},
	{"ass", FS_ASS_FILE},
	{"ssa", FS_SSA_FILE},
	{NULL, FS_UNKNOWN_FILE}
};

file_type_ext_struct movieAttachmentFilter[] = {
	{"sub", FS_SUB_FILE},
	{"srt", FS_SRT_FILE},
	{"ass", FS_ASS_FILE},
	{"ssa", FS_SSA_FILE},
	{"png", FS_PNG_FILE},
	{NULL, FS_UNKNOWN_FILE}
};

PmpAvcPlayer::PmpAvcPlayer() {
	drawImage = NULL;
	
	filmPreviewImage = NULL;
	
	fileItems = NULL;
	
	attachmentItems = NULL;
	
	batteryPercent10 = NULL;
	batteryPercent33 = NULL;
	batteryPercent66 = NULL;
	batteryPercent100 = NULL;
	batteryCharging = NULL;

};

PmpAvcPlayer::~PmpAvcPlayer() {
	
	if ( fileItems ) {
		free(fileItems);
	}
	
	if ( attachmentItems ) {
		free(attachmentItems);
	}
	
	if ( drawImage ) {
		freeImage(drawImage);
		drawImage = NULL;
	}
	
	if ( filmPreviewImage ) {
		freeImage(filmPreviewImage);
		filmPreviewImage = NULL;
	}
	
	if ( batteryPercent10 ) {
		freeImage(batteryPercent10);
		batteryPercent10 = NULL;
	}
	
	if ( batteryPercent33 ) {
		freeImage(batteryPercent33);
		batteryPercent33 = NULL;
	}
	
	if ( batteryPercent66 ) {
		freeImage(batteryPercent66);
		batteryPercent66 = NULL;
	}
	
	if ( batteryPercent100 ) {
		freeImage(batteryPercent100);
		batteryPercent100 = NULL;
	}
	
	if ( batteryCharging ) {
		freeImage(batteryCharging);
		batteryCharging = NULL;
	}
	
	ctrl_destroy();
	fat_free();
	disableGraphics();
	sceDisplayWaitVblankStart();
	sceGuTerm();
	
	FtFontManager::freeFtFontManager();
	Skin::freeSkin();
	Config::freeConfig();
	
	gu_font_close();
	
};

void PmpAvcPlayer::initSkinData() {
	Skin* skin = NULL;
	skin = Skin::getInstance();
	
	char tempPath[1024];
	
	//TODO init fileList
	fileListTextColor = skin->getColorValue("skin/font_color/color", 0xFFFFFF);
	fileListHLTextColor = skin->getColorValue("skin/files_list/highlight_color", 0xFFFFFF);
	fileListHLBackgroundColor = skin->getColorValue("skin/files_list/highlight_background_color", 0x000000);
	
	if ( skin->getBooleanValue("skin/files_list/highlight_alpha_enable", false) ) {
		Color alpha = skin->getColorValue("skin/files_list/highlight_alpha_value", 0xFF);
		fileListHLBackgroundColor = (alpha << 24) | fileListHLBackgroundColor;
	}
	else {
		fileListHLBackgroundColor = 0xff000000 | fileListHLBackgroundColor;
	}
	fileListScrollbarColor = fileListHLBackgroundColor;
	
	fileListBoxLeft = skin->getIntegerValue("skin/files_list/left", 0);
	fileListBoxTop = skin->getIntegerValue("skin/files_list/top", 0);
	fileListBoxWidth = skin->getIntegerValue("skin/files_list/width", 480);
	fileListBoxHeight = skin->getIntegerValue("skin/files_list/height", 272);
	
	fileItemBottom = fileListBoxHeight / ( textPixelSize + 2*TEXT_ITEM_BORDER );
	
	//TODO init datetime
	dateVisible = skin->getBooleanValue("skin/datetime_pannel/date_label/visible", false);
	dateLeft = skin->getIntegerValue("skin/datetime_pannel/date_label/left", 0);
	dateTop = skin->getIntegerValue("skin/datetime_pannel/date_label/top", 0);
	dateWidth = skin->getIntegerValue("skin/datetime_pannel/date_label/width", 144);
	dateHeight = skin->getIntegerValue("skin/datetime_pannel/date_label/height", 80);
	timeVisible = skin->getBooleanValue("skin/datetime_pannel/time_label/visible", false);
	timeLeft = skin->getIntegerValue("skin/datetime_pannel/time_label/left", 0);
	timeTop = skin->getIntegerValue("skin/datetime_pannel/time_label/top", 0);
	timeWidth = skin->getIntegerValue("skin/datetime_pannel/time_label/width", 144);
	timeHeight = skin->getIntegerValue("skin/datetime_pannel/time_label/height", 80);
	
	//TODO init battery
	batteryStatusVisible = skin->getBooleanValue("skin/battery_pannel/battery_status_label/visible", false);
	batteryStatusLeft = skin->getIntegerValue("skin/battery_pannel/battery_status_label/left", 0);
	batteryStatusTop = skin->getIntegerValue("skin/battery_pannel/battery_status_label/top", 0);
	batteryStatusWidth = skin->getIntegerValue("skin/battery_pannel/battery_status_label/width", 144);
	batteryStatusHeight = skin->getIntegerValue("skin/battery_pannel/battery_status_label/height", 80);
	batteryLifeVisible = skin->getBooleanValue("skin/battery_pannel/battery_life_label/visible", false);
	batteryLifeLeft = skin->getIntegerValue("skin/battery_pannel/battery_life_label/left", 0);
	batteryLifeTop = skin->getIntegerValue("skin/battery_pannel/battery_life_label/top", 0);
	batteryLifeWidth = skin->getIntegerValue("skin/battery_pannel/battery_life_label/width", 144);
	batteryLifeHeight = skin->getIntegerValue("skin/battery_pannel/battery_life_label/height", 80);
	
	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", skinPath, skin->getStringValue("skin/battery_pannel/battery_status_image/percent10", "10.png"));
	freeImage(batteryPercent10);
	batteryPercent10 = loadPNGImage(tempPath);
	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", skinPath, skin->getStringValue("skin/battery_pannel/battery_status_image/percent33", "33.png"));
	freeImage(batteryPercent33);
	batteryPercent33 = loadPNGImage(tempPath);
	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", skinPath, skin->getStringValue("skin/battery_pannel/battery_status_image/percent66", "66.png"));
	freeImage(batteryPercent66);
	batteryPercent66 = loadPNGImage(tempPath);
	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", skinPath, skin->getStringValue("skin/battery_pannel/battery_status_image/percent100", "100.png"));
	freeImage(batteryPercent100);
	batteryPercent100 = loadPNGImage(tempPath);
	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", skinPath, skin->getStringValue("skin/battery_pannel/battery_status_image/charging", "charging.png"));
	batteryCharging = loadPNGImage(tempPath);
	
	filmReloadEnable = true;
	
	//TODO init preview
	filmPreviewVisible = skin->getBooleanValue("skin/film_preview_pannel/preview_image/visible", false);
	filmPreviewReload = filmReloadEnable;
	filmPreviewLeft = skin->getIntegerValue("skin/film_preview_pannel/preview_image/left", 0);
	filmPreviewTop = skin->getIntegerValue("skin/film_preview_pannel/preview_image/top", 0);
	filmPreviewWidth = skin->getIntegerValue("skin/film_preview_pannel/preview_image/width", 144);
	filmPreviewHeight = skin->getIntegerValue("skin/film_preview_pannel/preview_image/height", 80);
	
	//TODO init filminformation
	filmInformationReload = filmReloadEnable;
	filmAspectRatioVisible = skin->getBooleanValue("skin/film_information_pannel/aspect_ratio_label/visible", false);
	filmAspectRatioLeft = skin->getIntegerValue("skin/film_information_pannel/aspect_ratio_label/left", 0);
	filmAspectRatioTop = skin->getIntegerValue("skin/film_information_pannel/aspect_ratio_label/top", 0);
	filmAspectRatioWidth = skin->getIntegerValue("skin/film_information_pannel/aspect_ratio_label/width", 144);
	filmAspectRatioHeight = skin->getIntegerValue("skin/film_information_pannel/aspect_ratio_label/height", 80);
	
	filmFpsVisible = skin->getBooleanValue("skin/film_information_pannel/fps_label/visible", false);
	filmFpsLeft = skin->getIntegerValue("skin/film_information_pannel/fps_label/left", 0);
	filmFpsTop = skin->getIntegerValue("skin/film_information_pannel/fps_label/top", 0);
	filmFpsWidth = skin->getIntegerValue("skin/film_information_pannel/fps_label/width", 144);
	filmFpsHeight = skin->getIntegerValue("skin/film_information_pannel/fps_label/height", 80);
	
	filmTotalTimeVisible = skin->getBooleanValue("skin/film_information_pannel/total_time_label/visible", false);
	filmTotalTimeLeft = skin->getIntegerValue("skin/film_information_pannel/total_time_label/left", 0);
	filmTotalTimeTop = skin->getIntegerValue("skin/film_information_pannel/total_time_label/top", 0);
	filmTotalTimeWidth = skin->getIntegerValue("skin/film_information_pannel/total_time_label/width", 144);
	filmTotalTimeHeight = skin->getIntegerValue("skin/film_information_pannel/total_time_label/height", 80);
	
	filmAudioStreamsVisible = skin->getBooleanValue("skin/film_information_pannel/audio_streams_label/visible", false);
	filmAudioStreamsLeft = skin->getIntegerValue("skin/film_information_pannel/audio_streams_label/left", 0);
	filmAudioStreamsTop = skin->getIntegerValue("skin/film_information_pannel/audio_streams_label/top", 0);
	filmAudioStreamsWidth = skin->getIntegerValue("skin/film_information_pannel/audio_streams_label/width", 144);
	filmAudioStreamsHeight = skin->getIntegerValue("skin/film_information_pannel/audio_streams_label/height", 80);
	
	filmSubtitlesVisible = skin->getBooleanValue("skin/film_information_pannel/subtitles_label/visible", false);
	filmSubtitlesLeft = skin->getIntegerValue("skin/film_information_pannel/subtitles_label/left", 0);
	filmSubtitlesTop = skin->getIntegerValue("skin/film_information_pannel/subtitles_label/top", 0);
	filmSubtitlesWidth = skin->getIntegerValue("skin/film_information_pannel/subtitles_label/width", 144);
	filmSubtitlesHeight = skin->getIntegerValue("skin/film_information_pannel/subtitles_label/height", 80);
	
};

int PmpAvcPlayer::init(char* ppaPath) {
	
	char tempPath[1024];
	SceUID modid;
	int status;

	pspType = m33KernelGetModel();
	
	if ( ppaPath == NULL ) {
		memset(applicationPath, 0, 256);
		getcwd(applicationPath, 256);
	}
	else {
		memset(applicationPath, 0, 256);
		strncpy(applicationPath, ppaPath, 255);
	}
	strcat(applicationPath, "/");
	
#ifdef DEBUG	
	pspDebugScreenPrintf("load cooleyesBridge.prx...\n");
#endif
	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", applicationPath, "cooleyesBridge.prx");
	modid = m33KernelLoadModule(tempPath, 0, NULL);
	if (modid < 0){
		return 0;
	}
	modid = sceKernelStartModule(modid, 0, 0, &status, NULL);
	if (modid < 0){
		return 0;
	}

#ifdef DEBUG	
	pspDebugScreenPrintf("load miniconv.prx...\n");
#endif
    memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", applicationPath, "miniconv.prx");
	modid = m33KernelLoadModule(tempPath, 0, NULL);
	if (modid < 0){
		return 0;
	}
	modid = sceKernelStartModule(modid, 0, 0, &status, NULL);
	if (modid < 0){
		return 0;
	}

#ifdef DEBUG	
	pspDebugScreenPrintf("load i18n.prx...\n");
#endif
    memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", applicationPath, "i18n.prx");
	modid = m33KernelLoadModule(tempPath, 0, NULL);
	if (modid < 0){
		return 0;
	}
	modid = sceKernelStartModule(modid, 0, 0, &status, NULL);
	if (modid < 0){
		return 0;
	}

	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", applicationPath, "dvemgr.prx");
#ifdef DEBUG	
	pspDebugScreenPrintf("load dvemgr...\n");
#endif
	if ( ! VideoMode::init(pspType, tempPath))
		return 0;

#ifdef ENABLE_USBHOST	
	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", applicationPath, "usbhostfs.prx");
#ifdef DEBUG	
	pspDebugScreenPrintf("load usbhost driver...\n");
#endif
	if ( ! UsbHost::loadUsbHostPrx(tempPath))
		return 0;
#endif	

#ifdef ENABLE_NETHOST	
	NetHost::setPrxBasePath(applicationPath);	
#endif

#ifdef DEBUG
	pspDebugScreenPrintf("malloc avc ddrtop ...\n");
#endif
	if( mp4_avc_init_ddrtop() != 1 )
		return 0;

#ifdef DEBUG
	pspDebugScreenPrintf("malloc psp 1k frame buffer ...\n");
#endif
	if (!m33IsTVOutSupported(pspType))
		if( psp1k_init_frame_buffer() != 1 )
			return 0;


	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", applicationPath, "moviestat.dat");
	init_movie_stat(tempPath);
	
	memset(fontPath, 0, 1024);
	sprintf(fontPath, "%s%s", applicationPath, "fonts/");
	
	memset(skinPath, 0, 1024);
	sprintf(skinPath, "%s%s", applicationPath, "skins/");
#ifdef DEBUG	
	pspDebugScreenPrintf("init draw_image...\n");
#endif
	drawImage = createImage(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	if (!drawImage)
		return 0;
	
	Config* config = NULL;
	FtFontManager* fontManager = NULL;
	FtFont* mainFont = NULL;
	
	//TODO load config
#ifdef DEBUG
	pspDebugScreenPrintf("init config...\n");
#endif
	if ( !Config::loadConfig("config.xml") ) 
		return 0;
	config = Config::getInstance();
	
	idleSecond = 300;
		
	//TODO load skin
#ifdef DEBUG
	pspDebugScreenPrintf("init skin...\n");
#endif
	strcat(skinPath, config->getStringValue("config/windows/skin/current", "default") ) ;
	strcat(skinPath, "/") ;
	
	if ( !Skin::loadSkin( skinPath ) )
		return 0;
	
		
	//TODO load main font
#ifdef DEBUG
	pspDebugScreenPrintf("init main_font...\n");
#endif
	if ( (fontManager = FtFontManager::getInstance()) == NULL )
		return 0;
	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", fontPath, config->getStringValue("config/windows/font/face", "mainfont.ttf"));
#ifdef DEBUG
	pspDebugScreenPrintf("load main_font...\n");
#endif
	if ( ! fontManager->loadMainFont( tempPath ) )
		return 0;
#ifdef DEBUG
	pspDebugScreenPrintf("load main_font finished...\n");
#endif
	mainFont = fontManager->getMainFont();
	textPixelSize = config->getIntegerValue("config/windows/font/size", 12);
	mainFont->setPixelSize( textPixelSize );
	mainFont->setAntiAlias( config->getBooleanValue("config/windows/font/anti_alias", true) );
	mainFont->setEmbolden( config->getBooleanValue("config/windows/font/embolden", false) );
	
	fileItemBottom = fileItemTop = fileItemCurrent = fileItemCount = 0;
	
	initSkinData();
	
	//TODO init FileList
	fileShowHidden = 0;//( config->getBooleanValue("config/filesystem/file_filter/show_hidden", false) ? 1 : 0 );
	fileShowHidden = 0;//( config->getBooleanValue("config/filesystem/file_filter/show_unknown", false) ? 1 : 0 );
		
	miniConvSetFileSystemConv( config->getStringValue("config/filesystem/charset/value", "UTF-8") );
	const char* last_path = config->getStringValue("config/filesystem/browser/last_path", "");
	base64decode((unsigned char*)filePath, last_path, strlen(last_path));
	base64decode((unsigned char*)fileShortPath, last_path, strlen(last_path));
	
#ifdef DEBUG
	pspDebugScreenPrintf("init fat,ctrl...\n");
#endif	
	//TODO init fat
	fat_init(sceKernelDevkitVersion());
	ctrl_init();
#ifdef ENABLE_HPRM
	ctrl_enablehprm(1);
#endif	
	//TODO init pmpavc mod kernel
#ifdef DEBUG
	pspDebugScreenPrintf("init pmpavc_kernel(cpu)...\n");
#endif
	cpu_clock_set_cpu_speed( config->getIntegerValue("config/cpu/speed", 120 ) );
	
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

//*/	
#ifdef DEBUG	
	pspDebugScreenPrintf("init pmpavc_kernel(avcodec)...\n");
#endif
	char* result;
	result = load_codec_prx(applicationPath, sceKernelDevkitVersion());
	if (result!=0){
#ifdef DEBUG
		pspDebugScreenPrintf("%s\n",result);
#endif
		return 0;
	}
	
#ifdef DEBUG	
	pspDebugScreenPrintf("init pmpavc_kernel(GU_FONT)...\n");
#endif
	gu_font_init();
	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", fontPath, config->getStringValue("config/subtitles/font/face","subfont.ttf"));
	result=gu_font_load(tempPath);
	if (result==0){
		gu_font_pixelsize_set(config->getIntegerValue("config/subtitles/font/size",16));
		gu_font_scale_set(config->getFloatValue("config/subtitles/font/asc_scale", 2.0),
			config->getFloatValue("config/subtitles/font/multcode_scale", 1.0) );
		float subBorder = config->getFloatValue("config/subtitles/font/border",0.0);
		if ( subBorder > 0.0 ) {
			gu_font_border_enable(1);
			gu_font_border_set(subBorder);
		}
		else
			gu_font_border_enable(0);
	
		gu_font_color_set(config->getColorValue("config/subtitles/font/color",0xffffff));
		gu_font_border_color_set(config->getColorValue("config/subtitles/font/border_color",0x000000));
		if ( config->getBooleanValue("config/subtitles/font/embolden",true) )
			gu_font_embolden_enable(1);
		else
			gu_font_embolden_enable(0);
		int sub_distance = config->getIntegerValue("config/subtitles/position/distance", 16 );
		sub_distance = (sub_distance<0)?(-sub_distance):sub_distance;
		
		if ( stricmp("top", config->getStringValue("config/subtitles/position/align", "bottom") ) == 0 )
			gu_font_align_set(0);
		else
			gu_font_align_set(1);
			
		gu_font_distance_set(sub_distance);
		miniConvSetDefaultSubtitleConv(config->getStringValue("config/subtitles/charset/value","UTF-8"));
	}
//*/
#ifdef DEBUG
	pspDebugScreenPrintf("init pmpavc_kernel(GU)...\n");
#endif
	//TODO init Graphics
	if ( stricmp("4:3", config->getStringValue("config/tvout/aspect_ratio", "16:9") ) == 0 ) {
		VideoMode::setTVAspectRatio(1);
		setGraphicsTVAspectRatio(1);
	}
	else {
		VideoMode::setTVAspectRatio(0);
		setGraphicsTVAspectRatio(0);
	}
	char overScanValue[16+1];
	memset( overScanValue, 0, 16+1 );
	strncpy( overScanValue, config->getStringValue("config/tvout/over_scan", "8.0.8.0"), 16);
	int osLeft, osTop, osRight, osBottom;
	osLeft = osTop = osRight = osBottom = 0;
	sscanf(overScanValue, "%d.%d.%d.%d", &osLeft, &osTop, &osRight, &osBottom);
	VideoMode::setTVOverScan(osLeft, osTop, osRight, osBottom);
	setGraphicsTVOverScan(osLeft, osTop, osRight, osBottom);
	setGraphicsTVOutScreen();
	sceGuInit();
	initGraphics(pspType, VideoMode::getVideoMode());
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	
	return 1;
};

void PmpAvcPlayer::run() {

	if( listDirectory() == false) {
		memset(filePath, 0, 512);
		memset(fileShortPath, 0, 512);
		strcpy(filePath, "ms0:/PSP/VIDEO/");
	        strcpy(fileShortPath, "ms0:/PSP/VIDEO/");
		if ( listDirectory() == false ) {
			memset(filePath, 0, 512);
			memset(fileShortPath, 0, 512);
			strcpy(filePath, "ms0:/VIDEO/");
	        	strcpy(fileShortPath, "ms0:/VIDEO/");
			if ( listDirectory() == false ) {
				memset(filePath, 0, 512);
				memset(fileShortPath, 0, 512);
				listDirectory();
			}
		}
	}
	if ( fileItemCount > 1 && fileItems[1].filetype != FS_DIRECTORY )
		fileItemCurrent = 1;
	
	activeTime = time(NULL);
	isSuspended = false;
	
	while( true ) {
		
		u32 key = ctrl_read();
		//if ( (key & PSP_CTRL_SELECT) && (key &PSP_CTRL_START) ) {
		//	break;
		//}
		if ( (key & PSP_CTRL_LTRIGGER) && (key & PSP_CTRL_UP) ) {
			fileItemCurrent = 0;
			filmPreviewReload = filmInformationReload = filmReloadEnable;
			activeTime = time(NULL);
		}
		else if ( (key & PSP_CTRL_LTRIGGER) && (key & PSP_CTRL_DOWN) ) {
			fileItemCurrent = fileItemCount - 1;
			filmPreviewReload = filmInformationReload = filmReloadEnable;
			activeTime = time(NULL);
		}
		else if ( (key & PSP_CTRL_LTRIGGER) && (key & PSP_CTRL_SQUARE) ) {
			filmReloadEnable = !filmReloadEnable;
			filmPreviewReload = filmInformationReload = filmReloadEnable;
			activeTime = time(NULL);
		}
		else if ( (key & PSP_CTRL_LTRIGGER) && (key & PSP_CTRL_SELECT) ) {
			char tempPath[1024];
			memset(tempPath, 0, 1024);
			sprintf(tempPath, "%s%s", applicationPath, "skins/");
			char tempSkinName[256];
			memset(tempSkinName, 0, 256);
			SkinDialog* dialog = new SkinDialog(Skin::getInstance()->getBackground(skinPath), drawImage);
			if ( dialog ) {
				if ( dialog->init(tempPath, tempSkinName) ) {
					bool res = dialog->execute();
					if ( res ) {
						sprintf(tempPath, "%s%s%s/", applicationPath, "skins/", tempSkinName);
						if ( Skin::loadSkin(tempPath) ) {
							strcpy(skinPath, tempPath);
							initSkinData();
							Config* config = Config::getInstance();
							if ( config ) {
								config->setStringValue("config/windows/skin/current", tempSkinName);
								memset(tempPath, 0, 1024);
								sprintf(tempPath, "%s%s", applicationPath, "config.xml");
								config->save(tempPath);
							}
						}
					}
				}
				delete dialog;
			}
			activeTime = time(NULL);
		}
		else if ( (key & PSP_CTRL_LTRIGGER) && (key & PSP_CTRL_TRIANGLE) ) {
			VersionDialog* dialog = new VersionDialog(Skin::getInstance()->getBackground(skinPath), drawImage);
			if (dialog) {
				if ( dialog->init() ) {
					dialog->execute();
				}
				delete dialog;
			}
			activeTime = time(NULL);
		} 
		else if ( (key & PSP_CTRL_LTRIGGER) && (key & PSP_CTRL_START) ) {
			MessageDialog* dialog = new MessageDialog(Skin::getInstance()->getBackground(skinPath), drawImage);
			if ( dialog ) {
				if ( dialog->init(i18nGetText(I18N_MSG_DLG_QUIT_TITLE), MESSAGE_TYPE_YES_NO) ) {
					u32 res = dialog->execute();
					if ( res == MESSAGE_RESULT_YES ) {
						break;
					}
				}
				delete dialog;
			}
			activeTime = time(NULL);
		}
		else if ( key & PSP_CTRL_TRIANGLE ) {
			showPadHelp();
			activeTime = time(NULL);
		}
		else if ( key & PSP_CTRL_SQUARE ) {
			ConfigDialog* dialog = new ConfigDialog(Skin::getInstance()->getBackground(skinPath), drawImage);
			if ( dialog ) {
				if ( dialog->init() ) {
					dialog->execute();
					char configPath[1024];
					memset(configPath, 0, 1024);
					sprintf(configPath, "%s%s", applicationPath, "config.xml");
					Config::getInstance()->save(configPath);
				}
				delete dialog;
			}
			activeTime = time(NULL);
		}
		else if ( key & PSP_CTRL_SELECT ) {
			if ( fileItems[fileItemCurrent].filetype != FS_DIRECTORY ) {
				MessageDialog* dialog = new MessageDialog(Skin::getInstance()->getBackground(skinPath), drawImage);
				if ( dialog ) {
					if ( dialog->init(i18nGetText(I18N_MSG_DLG_DELETE_TITLE), MESSAGE_TYPE_YES_NO) ) {
						u32 res = dialog->execute();
						if ( res == MESSAGE_RESULT_YES ) {
							deleteSelectMovie();
							listDirectory();
//							if (fileItemCurrent != 0){
//								fileItemCurrent--;
//							}
//							else {
//								fileItemCurrent = fileItemCount - 1;
//							}
							if (fileItemCurrent >= fileItemCount)
								fileItemCurrent = 0;
							filmPreviewReload = filmInformationReload = filmReloadEnable;
						}
					}
					delete dialog;
				}
			}
			activeTime = time(NULL);
		}
		else if (key & PSP_CTRL_START) {
			memset(filePath, 0, 512);
			memset(fileShortPath, 0, 512);
			listDirectory();
			fileItemCurrent = fileItemTop = 0;
			filmPreviewReload = filmInformationReload = filmReloadEnable;
			activeTime = time(NULL);
		}
		else if( (key & PSP_CTRL_UP) || (key & CTRL_BACK) ) {
			if (fileItemCurrent != 0){
				fileItemCurrent--;
			}
			else {
				fileItemCurrent = fileItemCount - 1;
			}
			filmPreviewReload = filmInformationReload = filmReloadEnable;
			activeTime = time(NULL);
		}
		else if( (key & PSP_CTRL_DOWN) || (key & CTRL_FORWARD) ){
			if (fileItemCurrent + 1 < fileItemCount) {
				fileItemCurrent++;
			}
			else 
				fileItemCurrent = 0;
			filmPreviewReload = filmInformationReload = filmReloadEnable;
			activeTime = time(NULL);
		}
		else if (key & PSP_CTRL_CIRCLE) {
			if ( fileItems[fileItemCurrent].filetype == FS_DIRECTORY ) {
				bool isUp = false;
				char dirName[512];
				if(strcmp(fileItems[fileItemCurrent].compname, "..") == 0) {
					isUp = true;
					int ll;
					if((ll = strlen(filePath) - 1) >= 0)
					while(filePath[ll] == '/' && ll >= 0) {
						filePath[ll] = 0;
						ll --;
					}
					char * lps;
					if((lps = strrchr(filePath, '/')) != NULL) {
						lps ++;
						strcpy(dirName, lps);
						*lps = 0;
					}
					else {
						strcpy(dirName, filePath);
						filePath[0] = 0;
					}
				}
				else {
					strcat(filePath, fileItems[fileItemCurrent].compname);
					strcat(filePath, "/");	
				}
				if( listDirectory() == false) {
					memset(filePath, 0, 512);
					memset(fileShortPath, 0, 512);
					listDirectory();
				}
				fileItemTop = 0;
				if (isUp) {
					for(fileItemCurrent = 0; fileItemCurrent < fileItemCount; fileItemCurrent ++) {
						if(stricmp(fileItems[fileItemCurrent].compname, dirName) == 0)
							break;
					}
					if(fileItemCurrent == fileItemCount)
						fileItemCurrent = 0;
				}
				else {
					fileItemCurrent = 0;
				}
				filmPreviewReload = filmInformationReload = filmReloadEnable;
			}
			else {
				paintLoading();
				playMovie(false);
				filmPreviewReload = filmInformationReload = filmReloadEnable;
			}
			activeTime = time(NULL);
		}
		else if (key & PSP_CTRL_CROSS) {
			if ( fileItems[fileItemCurrent].filetype != FS_DIRECTORY ) {
				paintLoading();
				playMovie(true);
				filmPreviewReload = filmInformationReload = filmReloadEnable;
			}
			activeTime = time(NULL);
		}
		

		if (!isSuspended && ( time(NULL) - activeTime >= idleSecond) ) {
			isSuspended = true;
			activeTime = time(NULL);
			scePowerRequestSuspend();
		}

		paint();
		sceKernelDelayThread(12500);
	}
	saveConfig();
};

void PmpAvcPlayer::saveConfig() {
	char last_path[2048];
	base64encode(last_path, (const unsigned char*)filePath, strlen(filePath));
	
	Config* config = Config::getInstance();
	if ( config ) {
		config->setStringValue("config/filesystem/browser/last_path", last_path);
	
		char configPath[1024];
		memset(configPath, 0, 1024);
		sprintf(configPath, "%s%s", applicationPath, "config.xml");
		config->save(configPath);
	}
}

void PmpAvcPlayer::paint() {
	Image* mainWindow = Skin::getInstance()->getBackground(skinPath);
	clearImage(drawImage, 0);
	
	paintFileListBox();
	paintDateTime();
	paintBattery();
	paintFilmPreview();
	paintFilmInformation();
	
	guStart();
	clearScreen();
	blitImageToScreen(0, 0, mainWindow->imageWidth, mainWindow->imageHeight, mainWindow, 0, 0);
	blitAlphaImageToScreen(0, 0, drawImage->imageWidth, drawImage->imageHeight, drawImage, 0, 0);
	flipScreen();
};

bool PmpAvcPlayer::listDirectory(){
	fileItemCount = open_directory(filePath, fileShortPath, fileShowHidden, fileShowUnknown, movieFileFilter, &fileItems);
	if ( fileItemCount < 1)
		return false;
	return true;
};

void PmpAvcPlayer::paintFileListBox(){
	FtFont* mainFont = FtFontManager::getInstance()->getMainFont();
	char stringBuffer[512];
	Color textColor;
	if ( fileItemCurrent < fileItemTop ) {
		fileItemTop = fileItemCurrent;
	}
	else if (fileItemCurrent - fileItemTop >= fileItemBottom ){
		fileItemTop = fileItemCurrent - fileItemBottom + 1;
	}
	
	int scrollbarWidth = 0;
	if ( fileItemCount > fileItemBottom ) 
		scrollbarWidth = 5;
	
	int scrollbarPosHeight = (fileListBoxHeight + fileItemCount/2) / fileItemCount ;
	
	if ( scrollbarWidth > 0 ) {
		Color lineColor = fileListScrollbarColor;
		//drawLineInImage( drawImage, lineColor, fileListBoxLeft+fileListBoxWidth-4, fileListBoxTop, 
		//	fileListBoxLeft+fileListBoxWidth-1, fileListBoxTop);
		//drawLineInImage( drawImage, lineColor, fileListBoxLeft+fileListBoxWidth-4, fileListBoxTop+fileListBoxHeight-1, 
		//	fileListBoxLeft+fileListBoxWidth-1, fileListBoxTop+fileListBoxHeight-1);
		drawLineInImage( drawImage, lineColor, fileListBoxLeft+fileListBoxWidth-4, fileListBoxTop, 
			fileListBoxLeft+fileListBoxWidth-4, fileListBoxTop+fileListBoxHeight-1);
		drawLineInImage( drawImage, lineColor, fileListBoxLeft+fileListBoxWidth-1, fileListBoxTop, 
			fileListBoxLeft+fileListBoxWidth-1, fileListBoxTop+fileListBoxHeight-1);	
			
		int x = fileListBoxLeft+fileListBoxWidth-4;
		int y = fileItemCurrent * scrollbarPosHeight;
		if ( ( y > fileListBoxHeight-scrollbarPosHeight ) || (fileItemCurrent == fileItemCount-1) )
			y = fileListBoxHeight - scrollbarPosHeight;
		y += fileListBoxTop;
		fillImageRect(drawImage, lineColor, x, y, 4, scrollbarPosHeight);	
	}
	

	int i;
	for(i=0;i<fileItemBottom;i++) {
		if( fileItemTop + i < fileItemCount) {
			textColor = fileListTextColor;
			if( fileItemTop + i == fileItemCurrent ) {
				fillImageRect(drawImage, fileListHLBackgroundColor, 
					fileListBoxLeft, fileListBoxTop+i*(2*TEXT_ITEM_BORDER+textPixelSize), 
					fileListBoxWidth - scrollbarWidth , 2*TEXT_ITEM_BORDER+textPixelSize);
				textColor = fileListHLTextColor;
			}
			memset(stringBuffer, 0, 512);
			if ( fileItems[fileItemTop + i].filetype == FS_DIRECTORY) {
				sprintf(stringBuffer, "<%s>", fileItems[fileItemTop + i].longname);
			}
			else {
				sprintf(stringBuffer, "%s", fileItems[fileItemTop + i].longname);
			}
			mainFont->printStringToImage(drawImage, fileListBoxLeft+TEXT_ITEM_BORDER, 
				fileListBoxTop+i*(2*TEXT_ITEM_BORDER+textPixelSize) + textPixelSize-TEXT_ITEM_BORDER,
				fileListBoxWidth - 2*TEXT_ITEM_BORDER - scrollbarWidth,
				textPixelSize,
				textColor,
				stringBuffer); 
		}
	}
};

void PmpAvcPlayer::paintDateTime() {
	FtFont* mainFont = FtFontManager::getInstance()->getMainFont();
	Color color = Skin::getInstance()->getColorValue("skin/font_color/color", 0xFFFFFF);
	char stringBuffer[64];
	pspTime currentPSPTime;
	sceRtcGetCurrentClockLocalTime(&currentPSPTime);
	if ( dateVisible ) {
		memset(stringBuffer, 0, 64);
		sprintf(stringBuffer, "%02d/%02d" , currentPSPTime.month, currentPSPTime.day);
		mainFont->printStringToImage(drawImage,
			dateLeft, dateTop+textPixelSize-TEXT_ITEM_BORDER, dateWidth, dateHeight,
			color, stringBuffer);
	}
	if ( timeVisible ) {
		memset(stringBuffer, 0, 64);
		sprintf(stringBuffer, "%02d:%02d" , currentPSPTime.hour, currentPSPTime.minutes);
		mainFont->printStringToImage(drawImage,
			timeLeft, timeTop+textPixelSize-TEXT_ITEM_BORDER, timeWidth, timeHeight,
			color, stringBuffer);
	}	
};

void PmpAvcPlayer::paintBattery() {
	FtFont* mainFont = FtFontManager::getInstance()->getMainFont();
	Color color = Skin::getInstance()->getColorValue("skin/font_color/color", 0xFFFFFF);
	char stringBuffer[64];
	int batteryLifePercent = scePowerGetBatteryLifePercent();
	if ( (batteryLifePercent < 0) || (batteryLifePercent > 100) )
		batteryLifePercent = 0;
	//*/
	if ( batteryStatusVisible ) {
		int status = scePowerGetBatteryChargingStatus();
		Image* statusImage;
		if ( (status & PSP_POWER_CB_BATTPOWER) || (status & PSP_POWER_CB_AC_POWER) )
			statusImage = batteryCharging;
		else if ( batteryLifePercent > 66 )
			statusImage = batteryPercent100;
		else if ( batteryLifePercent > 33 )
			statusImage = batteryPercent66;
		else if ( batteryLifePercent > 10 )
			statusImage = batteryPercent33;
		else 
			statusImage = batteryPercent10;
			
		if ( statusImage ) 
			putImageToImage(statusImage, drawImage, batteryStatusLeft, batteryStatusTop, batteryStatusWidth, batteryStatusHeight);
		
	}
	//*/
	//*/
	if ( batteryLifeVisible ) {
		memset(stringBuffer, 0, 64);
		sprintf(stringBuffer, "%3d%%" , batteryLifePercent);
		mainFont->printStringToImage(drawImage,
			batteryLifeLeft, batteryLifeTop+textPixelSize-TEXT_ITEM_BORDER, batteryLifeWidth, batteryLifeHeight,
			color, stringBuffer);
	}
	//*/
};

void PmpAvcPlayer::paintFilmPreview(){
	if ( !filmReloadEnable )
		return;
	if ( !filmPreviewVisible ) 
		return;
	
	if ( filmPreviewReload ) {
		
		if ( filmPreviewImage ) {
			freeImage(filmPreviewImage);
			filmPreviewImage = NULL;
		}
		
		if ( fileItems[fileItemCurrent].filetype == FS_PMP_FILE 
			|| fileItems[fileItemCurrent].filetype == FS_MP4_FILE 
			|| fileItems[fileItemCurrent].filetype == FS_MKV_FILE
			|| fileItems[fileItemCurrent].filetype == FS_FLV1_FILE) {
			char previewFileName[512];
			memset(previewFileName, 0, 512);
	
			sprintf(previewFileName,"%s%s", fileShortPath, fileItems[fileItemCurrent].shortname);
			int filenameEnd = strlen(previewFileName);
			previewFileName[filenameEnd-1] = 'g';
			previewFileName[filenameEnd-2] = 'n';
			previewFileName[filenameEnd-3] = 'p';
			filmPreviewImage = loadPNGImage(previewFileName);
			if ( filmPreviewImage == NULL) {
				char tempPath[1024];
				memset(tempPath, 0, 1024);
				sprintf(tempPath, "%s%s", skinPath, Skin::getInstance()->getStringValue("skin/film_preview_pannel/preview_image/default_image", "preview.png"));
				filmPreviewImage = loadPNGImage( tempPath );
			}
		}
		filmPreviewReload = false;
	}
	if ( filmPreviewImage )
		putImageToImage(filmPreviewImage, drawImage, filmPreviewLeft, filmPreviewTop, filmPreviewWidth, filmPreviewHeight);
};

void PmpAvcPlayer::initFilmInformation() {
	filmTotalFrames = 0;
	filmWidth = 0;
	filmHeight = 0;
	filmScale = 1;
	filmRate = 1;
	filmAudioStreams = 0;
	filmSubtitles = 0;
}

void PmpAvcPlayer::getCurrentPmpFilmInformation() {
	char previewFileName[512];
	memset(previewFileName, 0, 512);
	
	sprintf(previewFileName,"%s%s", fileShortPath, fileItems[fileItemCurrent].shortname);
	SceUID fp = sceIoOpen(previewFileName, PSP_O_RDONLY, 0777);
	if ( !fp ) {
		initFilmInformation();
		return;
	}
			
	u32 inforBuffer[10];
	memset(&inforBuffer, 0, 10*sizeof(u32));
	sceIoRead(fp, &inforBuffer, 10*sizeof(u32));
	sceIoClose(fp);
			
	if ( inforBuffer[0] != 0x6D706D70 || inforBuffer[1] != 1 ) {
		initFilmInformation();
	}
	else {			
		filmTotalFrames = inforBuffer[3];
		filmWidth = inforBuffer[4];
		filmHeight = inforBuffer[5];
		filmScale = inforBuffer[6];
		filmRate = inforBuffer[7];
		filmAudioStreams = inforBuffer[9];
		filmSubtitles = getSelectMovieSubtitles();
	}
}

void PmpAvcPlayer::getCurrentMp4FilmInformation() {
	initFilmInformation();
	char previewFileName[512];
	memset(previewFileName, 0, 512);
	
	sprintf(previewFileName,"%s%s", fileShortPath, fileItems[fileItemCurrent].shortname);
	
	mp4info_t* info = mp4info_open(previewFileName);
	if ( info == 0 ) {
		initFilmInformation();
	}
	else {
		int i;
		int video_track_id = -1;
		for(i = 0; i < info->total_tracks; i++) {
			mp4info_track_t* track = info->tracks[i];
			if (track->type != MP4_TRACK_VIDEO)
				continue;
			if ( track->width < 1 || track->height < 1 )
				continue;
			if ( track->width > 720 || track->height > 480 ) 
				continue;
			if ( track->video_type == 0x61766331 /*avc1*/) {
				if ( track->avc_profile==0x42 && (track->width > 480 || track->height > 272) ) 
					continue;
			}
			else {
				if ( track->width > 480 || track->height > 272 ) 
					continue;
			}
			video_track_id = i;
			break;
		}
		if ( video_track_id < 0 ) {
			mp4info_close(info);
			initFilmInformation();
			return;
		} 
		
		int audio_tracks = 0;
		int first_audio_track_id = 0;
		for(i = 0; i < info->total_tracks; i++) {
			mp4info_track_t* track = info->tracks[i];
			if (track->type != MP4_TRACK_AUDIO)
				continue;
			if ( audio_tracks == 0 ) {
				if ( track->audio_type != 0x6D703461 && track->audio_type != 0x73616D72 )
					continue;
//				if ( track->channels != 2 )
//					continue;
				if ( track->samplerate != 8000 && track->samplerate != 22050 && track->samplerate != 24000 && track->samplerate != 44100 && track->samplerate != 48000 )
					continue;
//				if ( track->samplebits != 16 )
//					continue;
				first_audio_track_id = i;
				audio_tracks++;
			}
			else {
				mp4info_track_t* old_track = info->tracks[first_audio_track_id];
				if ( old_track->audio_type != track->audio_type )
					continue;
//				if ( old_track->channels != track->channels )
//					continue;
				if ( old_track->samplerate != track->samplerate )
					continue;
//				if ( old_track->samplebits != track->samplebits )
//					continue;
				audio_tracks++;
			}
			if ( audio_tracks == 6 )
				break;
		}
		if ( audio_tracks == 0 ) {
			mp4info_close(info);
			initFilmInformation();
			return;
		}
		
		for(i = 0; i < info->total_tracks; i++) {
			mp4info_track_t* track = info->tracks[i];
			if (track->type != MP4_TRACK_SUBTITLE)
				continue;
			filmSubtitles++;
		}
		
		filmTotalFrames = 0;
		for( i = 0; i < info->tracks[video_track_id]->stts_entry_count; i++)
			filmTotalFrames += info->tracks[video_track_id]->stts_sample_count[i];
		
		filmWidth = info->tracks[video_track_id]->width;
		filmHeight = info->tracks[video_track_id]->height;
		filmScale = info->tracks[video_track_id]->stts_sample_duration[0];
		filmRate = info->tracks[video_track_id]->time_scale;
		filmAudioStreams = audio_tracks;
		mp4info_close(info);
		
		filmSubtitles += getSelectMovieSubtitles();
	}
}

void PmpAvcPlayer::getCurrentMkvFilmInformation() {
	initFilmInformation();
	char previewFileName[512];
	memset(previewFileName, 0, 512);
	
	sprintf(previewFileName,"%s%s", fileShortPath, fileItems[fileItemCurrent].shortname);
	
	mkvinfo_t* info = mkvinfo_open(previewFileName);
	if ( info == 0 ) {
		initFilmInformation();
	}
	else {
		int i;
		int video_track_id = -1;
		for(i = 0; i < info->total_tracks; i++) {
			mkvinfo_track_t* track = info->tracks[i];
			if (track->type != MATROSKA_TRACK_VIDEO)
				continue;
			if ( track->width < 1 || track->height < 1 )
				continue;
			if ( track->width > 720 || track->height > 480 ) 
				continue;
			if ( track->video_type == 0x61766331 /*avc1*/) {
				if ( track->private_size < 2 )
					continue;
				uint8_t avc_profile = track->private_data[1];
				if ( avc_profile==0x42 && (track->width > 480 || track->height > 272) ) 
					continue;
			}
			else {
				if ( track->width > 480 || track->height > 272 ) 
					continue;
			}
			video_track_id = i;
			break;
		}
		if ( video_track_id < 0 ) {
			mkvinfo_close(info);
			initFilmInformation();
			return;
		} 
		
		int audio_tracks = 0;
		int first_audio_track_id = 0;
		for(i = 0; i < info->total_tracks; i++) {
			mkvinfo_track_t* track = info->tracks[i];
			if (track->type != MATROSKA_TRACK_AUDIO)
				continue;
			if ( audio_tracks == 0 ) {
				if ( track->audio_type != 0x6D703461 /*mp4a*/)
					continue;
//				if ( track->channels != 2 )
//					continue;
				if ( track->samplerate != 22050 && track->samplerate != 24000 && track->samplerate != 44100 && track->samplerate != 48000 )
					continue;
//				if ( track->samplebits != 16 )
//					continue;
				first_audio_track_id = i;
				audio_tracks++;
			}
			else {
				mkvinfo_track_t* old_track = info->tracks[first_audio_track_id];
				if ( old_track->audio_type != track->audio_type )
					continue;
//				if ( old_track->channels != track->channels )
//					continue;
				if ( old_track->samplerate != track->samplerate )
					continue;
//				if ( old_track->samplebits != track->samplebits )
//					continue;
				audio_tracks++;
			}
			if ( audio_tracks == 6 )
				break;
		}
		if ( audio_tracks == 0 ) {
			mkvinfo_close(info);
			initFilmInformation();
			return;
		}
		
		for(i = 0; i < info->total_tracks; i++) {
			mkvinfo_track_t* track = info->tracks[i];
			if (track->type != MATROSKA_TRACK_SUBTITLE)
				continue;
			if ( (track->video_type == 0x74787475) || (track->video_type == 0x7478746C) 
				|| (track->video_type == 0x73736175) || (track->video_type == 0x61737375) /*txtu & txtl & ssau & assu*/)
				filmSubtitles++;
		}
		
		filmTotalFrames = (u32)(1LL*info->duration/(1000LL*info->tracks[video_track_id]->duration/info->tracks[video_track_id]->time_scale));
		filmWidth = info->tracks[video_track_id]->width;
		filmHeight = info->tracks[video_track_id]->height;
		filmScale = info->tracks[video_track_id]->duration;
		filmRate = info->tracks[video_track_id]->time_scale;
		filmAudioStreams = audio_tracks;
		mkvinfo_close(info);
		
		filmSubtitles += getSelectMovieSubtitles();
	}
}

void PmpAvcPlayer::getCurrentFlv1FilmInformation() {
	initFilmInformation();
	char previewFileName[512];
	memset(previewFileName, 0, 512);
	
	sprintf(previewFileName,"%s%s", fileShortPath, fileItems[fileItemCurrent].shortname);
	
	flv1info_t* info = flv1info_open(previewFileName, 0);
	if ( info == 0 ) {
		initFilmInformation();
	}
	else {
		if ( info->video_type != 0x666C7631 && info->video_type	!= 0x61766331 ) {
			flv1info_close(info);
			initFilmInformation();
			return;
		} 
		
		if ( info->audio_type != 0x6D703320 &&  info->audio_type != 0x6D703461) {
			flv1info_close(info);
			initFilmInformation();
			return;
		}
		
		filmTotalFrames = (u32)(1000LL*info->duration/info->video_frame_duration);
		filmWidth = info->width;
		filmHeight = info->height;
		filmScale = info->video_frame_duration;
		filmRate = info->video_scale;
		filmAudioStreams = 1;
		flv1info_close(info);
		
		filmSubtitles += getSelectMovieSubtitles();
	}
}

void PmpAvcPlayer::paintFilmInformation() {
	if ( !filmReloadEnable )
		return;
	if ( !filmAspectRatioVisible && !filmFpsVisible && !filmTotalTimeVisible && !filmSubtitlesVisible )
		return;
	if ( filmInformationReload ) {
		if ( fileItems[fileItemCurrent].filetype == FS_PMP_FILE ) {
			getCurrentPmpFilmInformation();
		}
		else if ( fileItems[fileItemCurrent].filetype == FS_MP4_FILE ) {
			getCurrentMp4FilmInformation();
		}
		else if ( fileItems[fileItemCurrent].filetype == FS_MKV_FILE ) {
			getCurrentMkvFilmInformation();
		}
		else if ( fileItems[fileItemCurrent].filetype == FS_FLV1_FILE ) {
			getCurrentFlv1FilmInformation();
		}
		filmInformationReload = false;
	}
	if ( fileItems[fileItemCurrent].filetype == FS_PMP_FILE 
		|| fileItems[fileItemCurrent].filetype == FS_MP4_FILE 
		|| fileItems[fileItemCurrent].filetype == FS_MKV_FILE
		|| fileItems[fileItemCurrent].filetype == FS_FLV1_FILE) {
			
		FtFont* mainFont = FtFontManager::getInstance()->getMainFont();
		Color color = Skin::getInstance()->getColorValue("skin/font_color/color", 0xFFFFFF);
		char stringBuffer[64];
		if ( filmAspectRatioVisible ) {
			memset(stringBuffer, 0, 64);
			sprintf(stringBuffer, "%d : %d" , filmWidth, filmHeight);
			mainFont->printStringToImage(drawImage,
				filmAspectRatioLeft, filmAspectRatioTop+textPixelSize-TEXT_ITEM_BORDER, filmAspectRatioWidth, filmAspectRatioHeight,
				color, stringBuffer);
		}
		if ( filmFpsVisible ) {
			memset(stringBuffer, 0, 64);
			sprintf(stringBuffer, "%6.3f" , filmRate*1.0/filmScale);
			mainFont->printStringToImage(drawImage,
				filmFpsLeft, filmFpsTop+textPixelSize-TEXT_ITEM_BORDER, filmFpsWidth, filmFpsHeight,
				color, stringBuffer);
		}
		if ( filmTotalTimeVisible ) {
			memset(stringBuffer, 0, 64);
			u64 totalTime = filmTotalFrames;
			totalTime *= filmScale;
			totalTime /= filmRate;
			u32 second = totalTime % 60;
			totalTime /= 60;
			u32 minute = totalTime % 60;
			u32 hour = totalTime / 60;			 
			sprintf(stringBuffer, "%02d:%02d:%02d" , hour, minute, second);
			mainFont->printStringToImage(drawImage,
				filmTotalTimeLeft, filmTotalTimeTop+textPixelSize-TEXT_ITEM_BORDER, filmTotalTimeWidth, filmTotalTimeHeight,
				color, stringBuffer);
		}
		if ( filmAudioStreamsVisible ) {
			memset(stringBuffer, 0, 64);
			sprintf(stringBuffer, "%d" , filmAudioStreams);
			mainFont->printStringToImage(drawImage,
				filmAudioStreamsLeft, filmAudioStreamsTop+textPixelSize-TEXT_ITEM_BORDER, filmAudioStreamsWidth, filmAudioStreamsHeight,
				color, stringBuffer);
		}
		if ( filmSubtitlesVisible ) {
			memset(stringBuffer, 0, 64);
			sprintf(stringBuffer, "%d" , filmSubtitles);
			mainFont->printStringToImage(drawImage,
				filmSubtitlesLeft, filmSubtitlesTop+textPixelSize-TEXT_ITEM_BORDER, filmSubtitlesWidth, filmSubtitlesHeight,
				color, stringBuffer);
		}	
	}
	
};

void PmpAvcPlayer::paintLoading() {
	char tempPath[1024];
	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", skinPath, Skin::getInstance()->getStringValue("skin/extra/loading_image", "loading.png") );
	Image* img = loadPNGImage(tempPath);
	if ( img ) {
		guStart();
		clearScreen();
		blitAlphaImageToScreen(0, 0, img->imageWidth, img->imageHeight, img, 
			(PSP_SCREEN_WIDTH - img->imageWidth)/2, (PSP_SCREEN_HEIGHT - img->imageHeight)/2);
		flipScreen();
		freeImage(img);
		sceDisplayWaitVblankStart();
	}
};

void PmpAvcPlayer::showPadHelp() {
	char tempPath[1024];
	memset(tempPath, 0, 1024);
	sprintf(tempPath, "%s%s", skinPath, Skin::getInstance()->getStringValue("skin/extra/pad_help_image", "pad.png") );
	Image* img = loadPNGImage(tempPath);
	if ( img ) {
		guStart();
		clearScreen();
		blitImageToScreen(0, 0, img->imageWidth, img->imageHeight, img, 0, 0 );
		flipScreen();
		freeImage(img);
	}
	while( !( ctrl_read() & PSP_CTRL_TRIANGLE )) {
		sceKernelDelayThread(12500);
	};
};

void PmpAvcPlayer::deleteSelectMovie() {
	if ( fileItems[fileItemCurrent].filetype != FS_PMP_FILE 
		&& fileItems[fileItemCurrent].filetype != FS_MP4_FILE 
		&& fileItems[fileItemCurrent].filetype != FS_MKV_FILE
		&& fileItems[fileItemCurrent].filetype != FS_FLV1_FILE)
		return;
	int deleteFileCount = 1;
	char deleteFiles[5120];
	
	memset(deleteFiles, 0, 5120);
	
	strncpy( &deleteFiles[0], fileItems[fileItemCurrent].shortname, 511);
	
	int items = open_directory(filePath, fileShortPath, fileShowHidden, fileShowUnknown, movieAttachmentFilter, &attachmentItems);
	int i, filename_size;
	
	filename_size = strlen(fileItems[fileItemCurrent].longname)-4;
	for(i=0; i<items; i++) {
		if ( strnicmp(fileItems[fileItemCurrent].longname, attachmentItems[i].longname, filename_size) == 0 ) {
			if(strnicmp(&(attachmentItems[i].longname[filename_size]), ".png", 4) == 0) {
				strncpy( &deleteFiles[(deleteFileCount++)*512], attachmentItems[i].shortname, 511); 
			}
			else {
				struct subtitle_ext_charset_struct* exts = subtitleExt;
				while(exts->ext != NULL){
					if(strnicmp(&(attachmentItems[i].longname[filename_size]), exts->ext, exts->ext_len) == 0) {
						strncpy( &deleteFiles[(deleteFileCount++)*512], attachmentItems[i].shortname, 511);
						break;
					}
					exts++;
				}
			}
		}
		if (deleteFileCount >= 10 )
			break;
	}
	
	char deleteFileName[1024];
	
	for(i=0;i<deleteFileCount;i++) {
		memset(deleteFileName, 0, 1024);
		sprintf(deleteFileName, "%s%s", fileShortPath, &deleteFiles[i*512]);
		sceIoRemove(deleteFileName);
	}
};

int PmpAvcPlayer::getSelectMovieSubtitles() {
	int items = open_directory(filePath, fileShortPath, fileShowHidden, fileShowUnknown, movieSubtitleFilter, &attachmentItems);
	if ( items < 1)
		return 0;
	int i, subtitles, filename_size;
	subtitles = 0;
	filename_size = strlen(fileItems[fileItemCurrent].longname)-4;
	for(i=0; i<items; i++) {
		if ( strnicmp(fileItems[fileItemCurrent].longname, attachmentItems[i].longname, filename_size) == 0 ) {
			struct subtitle_ext_charset_struct* exts = subtitleExt;
			while(exts->ext != NULL){
				if(strnicmp(&(attachmentItems[i].longname[filename_size]), exts->ext, exts->ext_len) == 0) {
					subtitles++;
					break;
				}
				exts++;
			}
		}
	}
	return subtitles;
};

void PmpAvcPlayer::fillSelectMovieInfo() {
	memset(currentMovie.movie_file, 0, 512);
	sprintf(currentMovie.movie_file, "%s%s", fileShortPath, fileItems[fileItemCurrent].shortname);
	
	memset(currentMovie.movie_hash, 0, 16);
	sceKernelUtilsMd5Digest((u8*)(fileItems[fileItemCurrent].longname), strlen(fileItems[fileItemCurrent].longname), (u8*)(currentMovie.movie_hash));
	
	currentMovie.movie_subtitle_num = 0;
	int items = open_directory(filePath, fileShortPath, fileShowHidden, fileShowUnknown, movieSubtitleFilter, &attachmentItems);
	if ( items < 1)
		return ;
	int i, filename_size;
	
	filename_size = strlen(fileItems[fileItemCurrent].longname)-4;
	for(i=0; i<items; i++) {
		if ( strnicmp(fileItems[fileItemCurrent].longname, attachmentItems[i].longname, filename_size) == 0 ) {
			struct subtitle_ext_charset_struct* exts = subtitleExt;
			while(exts->ext != NULL){
				if(strnicmp(&(attachmentItems[i].longname[filename_size]), exts->ext, exts->ext_len) == 0) {
					memset(currentMovie.movie_subtitles[currentMovie.movie_subtitle_num].subtitle_file, 0, 512);
					sprintf(currentMovie.movie_subtitles[currentMovie.movie_subtitle_num].subtitle_file, "%s%s", fileShortPath, attachmentItems[i].shortname);
					memset(currentMovie.movie_subtitles[currentMovie.movie_subtitle_num].subtitle_charset, 0, 32);
					strncpy(currentMovie.movie_subtitles[currentMovie.movie_subtitle_num].subtitle_charset, exts->charset, 31);
					currentMovie.movie_subtitle_num++;
					break;
				}
				exts++;
			}
		}
		if (currentMovie.movie_subtitle_num >= MAX_MOVIE_SUBTITLES )
			break;
	}
};

void PmpAvcPlayer::playMovie(bool resume) {

#ifdef ENABLE_SUSPEND	
	scePowerLock(0);
#endif
	
	fillSelectMovieInfo();
	
	char* result = NULL;
	
	int left, top, right, bottom;
	VideoMode::getTVOverScan(left, top, right, bottom);
	
	int usePos = resume ? 1 : 0;
	
	if ( fileItems[fileItemCurrent].filetype == FS_PMP_FILE )
		result = pmp_play(&currentMovie, usePos, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode() );
	else if ( fileItems[fileItemCurrent].filetype == FS_MP4_FILE )
		result = mp4_play(&currentMovie, usePos, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode() );
	else if ( fileItems[fileItemCurrent].filetype == FS_MKV_FILE )
		result = mkv_play(&currentMovie, usePos, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode() );
	else if ( fileItems[fileItemCurrent].filetype == FS_FLV1_FILE )
		result = flv1_play(&currentMovie, usePos, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode() );
	else
		result = "unsupported movie";
	
	sceKernelDcacheWritebackInvalidateAll();
	sceKernelDelayThread(1000000);
	if ( result ) {

		char tempPath[1024];
		memset(tempPath, 0, 1024);
		sprintf(tempPath, "%s%s", applicationPath, "playlog.txt");
		FILE* log = fopen(tempPath,"w+");
		fprintf(log, "%s\n", result);
		fclose(log);

#ifdef ENABLE_SUSPEND	
		scePowerUnlock(0);
#endif
		return;
	}
	
	Config* config = Config::getInstance();
	const char* playMode = config->getStringValue("config/player/play_mode", "group");
	
	if ( stricmp("single", playMode) == 0  ){
		scePowerUnlock(0);
		return;
	}
	else if ( stricmp("group", playMode) == 0  ) {
		while(fileItemCurrent < fileItemCount - 1) {
			if ( is_next_movie( fileItems[fileItemCurrent].longname, fileItems[fileItemCurrent+1].longname ) ) {
				fileItemCurrent++;
				fillSelectMovieInfo();
				int left, top, right, bottom;
				VideoMode::getTVOverScan(left, top, right, bottom);
				if ( fileItems[fileItemCurrent].filetype == FS_PMP_FILE )
					result = pmp_play(&currentMovie, 0, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode());
				else if ( fileItems[fileItemCurrent].filetype == FS_MP4_FILE )
					result = mp4_play(&currentMovie, 0, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode());
				else if ( fileItems[fileItemCurrent].filetype == FS_MKV_FILE )
					result = mkv_play(&currentMovie, 0, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode());
				else if ( fileItems[fileItemCurrent].filetype == FS_FLV1_FILE )
					result = flv1_play(&currentMovie, 0, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode());						
				else
					result = "unsupported movie";
					
				sceKernelDcacheWritebackInvalidateAll();
				sceKernelDelayThread(1000000);
				if ( result ) {
					char tempPath[1024];
					memset(tempPath, 0, 1024);
					sprintf(tempPath, "%s%s", applicationPath, "playlog.txt");
					FILE* log = fopen(tempPath,"w+");
					fprintf(log, "%s\n", result);
					fclose(log);
					break;
				}
			}
			else
				break;
		} 
	}
	else if ( stricmp("all", playMode) == 0 ) {
		while(fileItemCurrent < fileItemCount - 1) {
			fileItemCurrent++;
			fillSelectMovieInfo(); 
			int left, top, right, bottom;
			VideoMode::getTVOverScan(left, top, right, bottom);
			if ( fileItems[fileItemCurrent].filetype == FS_PMP_FILE )
				result = pmp_play(&currentMovie, 0, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode());
			else if ( fileItems[fileItemCurrent].filetype == FS_MP4_FILE )
				result = mp4_play(&currentMovie, 0, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode());
			else if ( fileItems[fileItemCurrent].filetype == FS_MKV_FILE )
				result = mkv_play(&currentMovie, 0, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode());
			else if ( fileItems[fileItemCurrent].filetype == FS_FLV1_FILE )
				result = flv1_play(&currentMovie, 0, pspType, VideoMode::getTVAspectRatio(), left, top, right, bottom, VideoMode::getVideoMode());
			else
				result = "unsupported movie";
			sceKernelDcacheWritebackInvalidateAll();
			sceKernelDelayThread(1000000); 
			if ( result ) {
				char tempPath[1024];
				memset(tempPath, 0, 1024);
				sprintf(tempPath, "%s%s", applicationPath, "playlog.txt");
				FILE* log = fopen(tempPath,"w+");
				fprintf(log, "%s\n", result);
				fclose(log);
				break;
			}
		} 
	} 
	scePowerUnlock(0);
	
	guStart();
	flipScreen();
};


void PmpAvcPlayer::enterSuspendMode(){
	isSuspended = true;
	
	gu_font_on_suspend();
	
	FtFontManager::getInstance()->getMainFont()->onSuspend();
	
};

void PmpAvcPlayer::leaveSuspendMode(){
	
	FtFontManager::getInstance()->getMainFont()->onResume();
	
	gu_font_on_resume();
	
	activeTime = time(NULL);
	isSuspended = false;
	
};
