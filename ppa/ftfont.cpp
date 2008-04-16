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
#include<pspkernel.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include "ftfont.h"

#define SBIT_HASH_SIZE (256)

typedef struct  Cache_Bitmap_{
    int width;
    int height;
    int left;
    int	top;
    char format;
    short max_grays;
    int	pitch;
    unsigned char*  buffer;
} Cache_Bitmap;

typedef struct SBit_HashItem_ {
	unsigned long ucs_code;
	int glyph_index;
	int size;
	bool anti_alias;
	bool embolden;
	int   xadvance;
	int   yadvance;
	Cache_Bitmap bitmap;
} SBit_HashItem;

static int get_next_utf8_char(unsigned long* ucs, char* s, int n) {
	const char *src = s;
	char c;
	int extra;
	unsigned long result;

	if (n == 0)
		return 0;
    
	c = *src++;
	n--;
    
	if (!(c & 0x80)) {
		result = c;
		extra = 0;
	} 
	else if (!(c & 0x40)) {
		return -1;
	}
	else if (!(c & 0x20)) {
		result = c & 0x1f;
		extra = 1;
	}
	else if (!(c & 0x10)) {
		result = c & 0xf;
		extra = 2;
	}
	else if (!(c & 0x08)) {
		result = c & 0x07;
		extra = 3;
	}
	else if (!(c & 0x04)) {
		result = c & 0x03;
		extra = 4;
	}
	else if ( ! (c & 0x02)) {
		result = c & 0x01;
		extra = 5;
	}
	else {
		return -1;
	}
	if (extra > n)
		return -1;
    
	while (extra--) {
		result <<= 6;
		c = *src++;
	
		if ((c & 0xc0) != 0x80)
			return -1;
		result |= c & 0x3f;
	}
	*ucs = result;
	return src - s;
}

class FtFontImpl : FtFont {
private:
	FT_Library library;
	FT_Face face;
	
	char* fontName;
	FT_Open_Args openArgs;
	FT_StreamRec fontStream;
	bool antiAlias;
	bool embolden;
	int pixelSize;
	
	SBit_HashItem sbitHashRoot[SBIT_HASH_SIZE];
	int cacheSize;
	int cachePop;
	
	FtFontImpl(FT_Library library);
	bool loadFace(const char* filename);
	
	SBit_HashItem* sbitCacheFind(unsigned long ucsCode);
	void sbitCacheAdd(unsigned long ucsCode, int glyphIndex, 
		FT_Bitmap *bitmap, int left, int top, int xadvance, int yadvance);
	
	void drawCachedBitmap(Image* image, Cache_Bitmap* sbt, FT_Int x, FT_Int y, int width, int height, Color color);
	void drawBitmap(Image* image, FT_Bitmap *bitmap, FT_Int x, FT_Int y, int width, int height, Color color);
	
	
	
public:
	static FtFont* loadFont(FT_Library library, const char* filename);
	virtual ~FtFontImpl();
	virtual void setAntiAlias(bool enable);
	virtual void setEmbolden(bool enable);
	virtual void setPixelSize(int size);
	virtual int getPixelSize();
	virtual void printStringToImage(Image* image, int x, int y, int width, int height, Color color, const char* s);
	virtual void onSuspend();
	virtual void onResume();
};

static unsigned long ftfont_stream_io( FT_Stream stream,
	unsigned long   offset,
	unsigned char*  buffer,
	unsigned long   count ) {
		sceIoLseek32(stream->descriptor.value, offset, PSP_SEEK_SET);
		if ( count == 0 )
			return 0;
		return sceIoRead(stream->descriptor.value, buffer, count);
}

static void ftfont_stream_close(FT_Stream  stream) {
	sceIoClose( stream->descriptor.value );
}

FtFontImpl::FtFontImpl(FT_Library library) {
	this->library = library;
	this->face = 0;
	this->fontName = NULL;
	this->fontStream.read = ftfont_stream_io;
	this->fontStream.close = ftfont_stream_close;
	this->openArgs.flags = FT_OPEN_STREAM;
	this->openArgs.stream = &(this->fontStream);
	int i;
	for (i = 0; i < SBIT_HASH_SIZE; ++i) {
		memset(&sbitHashRoot[i], 0, sizeof(SBit_HashItem));
	}
	this->cacheSize = 0;
	this->cachePop = 0;
};

FtFontImpl::~FtFontImpl() {
	int i;
	if ( fontName ) {
		free(fontName);
		fontName = NULL;
	}
	for (i = 0; i < SBIT_HASH_SIZE; ++i) {
		if(sbitHashRoot[i].bitmap.buffer) {
			free(sbitHashRoot[i].bitmap.buffer);
		}
	}
	if( face ) {
		FT_Done_Face(face);
	}
};

bool FtFontImpl::loadFace(const char* filename) {
	this->fontName = (char*)malloc_64(strlen(filename)+1);
	memset(this->fontName, 0, strlen(filename)+1);
	strcpy(this->fontName, filename);
	FT_Error error;
//	error = FT_New_Face(library, filename, 0, &face);
	this->fontStream.descriptor.value = sceIoOpen(filename, PSP_O_RDONLY, 0777);
	if ( ! this->fontStream.descriptor.value ){
		face = NULL;
		return false;
	}
	this->fontStream.base = 0;
	this->fontStream.size = sceIoLseek32(this->fontStream.descriptor.value,0, PSP_SEEK_END);
	this->fontStream.pos = sceIoLseek32(this->fontStream.descriptor.value,0, PSP_SEEK_SET);
	error = FT_Open_Face(library, &(this->openArgs), 0, &face);
	if (error)
		return false;
	else 
		return true;
};

void FtFontImpl::onSuspend() {
	sceIoClose(this->fontStream.descriptor.value);
	this->fontStream.descriptor.value = 0;
};

void FtFontImpl::onResume() {
	this->fontStream.descriptor.value = sceIoOpen(this->fontName, PSP_O_RDONLY, 0777);
	sceIoLseek32(this->fontStream.descriptor.value, this->fontStream.pos, PSP_SEEK_SET);
};

SBit_HashItem* FtFontImpl::sbitCacheFind(unsigned long ucsCode) { 
	/*/
	unsigned long hashValue = ucsCode % SBIT_HASH_SIZE;
	if ( (sbitHashRoot[hashValue].ucs_code == ucsCode) &&
		(sbitHashRoot[hashValue].size == pixelSize) &&  
		(sbitHashRoot[hashValue].anti_alias == antiAlias) &&
		(sbitHashRoot[hashValue].embolden == embolden) )
		return (&sbitHashRoot[hashValue]);
	else
		return NULL;
	//*/
	for(int i=0; i<cacheSize; i++) {
		if ( (sbitHashRoot[i].ucs_code == ucsCode) &&
			(sbitHashRoot[i].size == pixelSize) &&  
			(sbitHashRoot[i].anti_alias == antiAlias) &&
			(sbitHashRoot[i].embolden == embolden) )
			return (&sbitHashRoot[i]);
	}
	return NULL;
};

void FtFontImpl::sbitCacheAdd(unsigned long ucsCode, int glyphIndex, 
	FT_Bitmap *bitmap, int left, int top, int xadvance, int yadvance) {
	/*/
	unsigned long hashValue = ucsCode % SBIT_HASH_SIZE;
	SBit_HashItem* item = &sbitHashRoot[hashValue];
	//*/
	int addIndex = 0;
	if ( cacheSize < SBIT_HASH_SIZE ) {
		addIndex = cacheSize++;
	}
	else {
		addIndex = cachePop++;
		if (cachePop == SBIT_HASH_SIZE)
			cachePop = 0;
	}
	SBit_HashItem* item = &sbitHashRoot[addIndex];
	if ( item->bitmap.buffer ) {
		free(item->bitmap.buffer);
		item->bitmap.buffer = 0;
	}
	
	item->ucs_code = ucsCode;
	item->glyph_index = glyphIndex;
	item->size = pixelSize;
	item->anti_alias = antiAlias;
	item->embolden = embolden;
	item->xadvance  = xadvance;
	item->yadvance  = yadvance;
	item->bitmap.width     = bitmap->width;
	item->bitmap.height    = bitmap->rows;
	item->bitmap.pitch     = bitmap->pitch;
	item->bitmap.left      = left;
	item->bitmap.top       = top;
	item->bitmap.format    = bitmap->pixel_mode;
	item->bitmap.max_grays = (bitmap->num_grays - 1);

	int pitch = bitmap->pitch;
	if( pitch < 0 ) pitch = -pitch;
	if ( pitch * bitmap->rows > 0) {
		item->bitmap.buffer = (unsigned char*)malloc_64(pitch * bitmap->rows);
		if ( item->bitmap.buffer ) {
			memset(item->bitmap.buffer, 0, pitch * bitmap->rows);
			memcpy(item->bitmap.buffer, bitmap->buffer, pitch * bitmap->rows);
		}
	}
};

void FtFontImpl::drawCachedBitmap(Image* image, Cache_Bitmap* sbt, FT_Int x, FT_Int y, int width, int height, Color color) {
	FT_Int i, j;
	Color pixel, grey;
	int a, r, g, b;
	if( !sbt->buffer )
		return;
	if(sbt->format ==FT_PIXEL_MODE_MONO){
		for(j = 0; j< sbt->height; j++)
			for(i = 0; i< sbt->width ; i++) {
				if( i+x < 0 || i+x >= width || j+y < 0 || j+y >= height)
					continue;
				if ( sbt->buffer[j*sbt->pitch+i/8] & (0x80>>(i%8)) )
					pixel = (0xff000000) | color;
				else
					pixel = 0;
				if (pixel)
					putPixelToImage(image, pixel, i+x, j+y);
			}
	}
	else if(sbt->format ==FT_PIXEL_MODE_GRAY){
		for(j = 0; j< sbt->height; j++)
			for(i = 0; i< sbt->width ; i++) {
				if( i+x < 0 || i+x >= width || j+y < 0 || j+y >= height)
					continue;
				grey = sbt->buffer[j*sbt->pitch+i];
				
				if ( grey ) {
					{
						a = 0xff;//(grey * 0xFF) / 255;
						r = (grey * R(color)) / 255;
						g = (grey * G(color)) / 255;
						b = (grey * B(color)) / 255;
					}
						
	  				pixel = (a << 24) | (b << 16) | (g << 8) | r;
	  			}
				else
					pixel = 0;
				if (pixel)
					putPixelToImage(image, pixel, i+x, j+y);
			}
	}
};

void FtFontImpl::drawBitmap(Image* image, FT_Bitmap *bitmap, FT_Int x, FT_Int y, int width, int height, Color color) {
	FT_Int i, j;
	Color pixel, grey;
	int a, r, g, b;
	if( !bitmap->buffer )
		return;
	if(bitmap->pixel_mode ==FT_PIXEL_MODE_MONO){
		for(j = 0; j< bitmap->rows; j++)
			for(i = 0; i< bitmap->width ; i++) {
				if( i+x < 0 || i+x >= width || j+y < 0 || j+y >= height)
					continue;
				if ( bitmap->buffer[j*bitmap->pitch+i/8] & (0x80>>(i%8)) )
					pixel = (0xff000000) | color;
				else
					pixel = 0;
				if (pixel)
					putPixelToImage(image, pixel, i+x, j+y);
			}
	}
	else if(bitmap->pixel_mode ==FT_PIXEL_MODE_GRAY){
		for(j = 0; j< bitmap->rows; j++)
			for(i = 0; i< bitmap->width ; i++) {
				if( i+x < 0 || i+x >= width || j+y < 0 || j+y >= height)
					continue;
				grey = bitmap->buffer[j*bitmap->pitch+i];
				
				if ( grey ) {
					{
						a = 0xff;//(grey * 0xFF) / 255;
						r = (grey * R(color)) / 255;
						g = (grey * G(color)) / 255;
						b = (grey * B(color)) / 255;
					}
					pixel = (a << 24) | (b << 16) | (g << 8) | r;
	  			}
				else
					pixel = 0;
				if (pixel)
					putPixelToImage(image, pixel, i+x, j+y);
			}
	}	
};

FtFont* FtFontImpl::loadFont(FT_Library library, const char* filename) {
	FtFontImpl* obj = new FtFontImpl(library);
	if ( ! obj->loadFace(filename) ) {
		delete obj;
		return NULL;
	}
	else
		return obj;
};

void FtFontImpl::setAntiAlias(bool enable) {
	antiAlias = enable;
};

void FtFontImpl::setEmbolden(bool enable) {
	embolden = enable;
};

void FtFontImpl::setPixelSize(int size) {
	pixelSize = size;
	FT_Set_Pixel_Sizes(face, pixelSize, pixelSize);
};

int FtFontImpl::getPixelSize() {
	return pixelSize;
};

void FtFontImpl::printStringToImage(Image* image, int x, int y, int width, int height, Color color, const char* s) {
	width = x + width;
	height = y + height;
	if( width < 1 || width > image->imageWidth )
		width = image->imageWidth;
	if( height < 1 || height > image->imageHeight )
		height = image->imageHeight;
		
	int stringLength = strlen(s);
	if ( stringLength <= 0 )
		return;
	char* p = (char*)s;
	
	FT_Error error;
	FT_GlyphSlot slot;
	FT_UInt glyphIndex;
	
	FT_Bool useKerning; 
	FT_UInt previous; 
	
	useKerning = FT_HAS_KERNING( face ); 
	previous = 0;
	
	int n = 0, i = 0;
	unsigned long ucs = 0;
	while(n<stringLength) {
		i = get_next_utf8_char(&ucs, p, stringLength-n);
		if ( i < 0 ) {
			p++;
			n++;
			ucs = (unsigned long)'?';
		}
		else {
			p += i;
			n += i;
		}
		SBit_HashItem* cache =  sbitCacheFind(ucs);
		if ( cache) {
			if ( useKerning && previous && cache->glyph_index ) { 
				FT_Vector delta; 
				FT_Get_Kerning( face, previous, cache->glyph_index, FT_KERNING_DEFAULT, &delta ); 
				x += delta.x >> 6; 
			}
			drawCachedBitmap(image, &cache->bitmap, x + cache->bitmap.left,
				y - cache->bitmap.top, width,
				height, color);
			
			
			x += cache->xadvance >> 6;
			previous = cache->glyph_index;
		}
		else 
		{ 
			glyphIndex = FT_Get_Char_Index( face, ucs);
			error = FT_Load_Glyph( face, glyphIndex, FT_LOAD_NO_HINTING );
			if (error)
				continue;
			if ( antiAlias )
				error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
			else
				error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
			if (error) {
				continue;
			}
			slot = face->glyph;
			
			if ( embolden )
				FT_GlyphSlot_Embolden(slot);
			
			if ( useKerning && previous && glyphIndex ) { 
				FT_Vector delta; 
				FT_Get_Kerning( face, previous, glyphIndex, FT_KERNING_DEFAULT, &delta ); 
				x += delta.x >> 6; 
			}
					
			
			drawBitmap(image, &slot->bitmap, x + slot->bitmap_left,
				y - slot->bitmap_top, width,
				height, color);
			
			x += slot->advance.x >> 6;
			previous = glyphIndex;
			
			sbitCacheAdd(ucs, glyphIndex,  
				&slot->bitmap, slot->bitmap_left, slot->bitmap_top,  slot->advance.x, slot->advance.y);					
		}
	}
	
};


FtFontManager* FtFontManager::instance = NULL;

FtFontManager::FtFontManager() {
	library = 0;
};

FtFontManager::~FtFontManager() {
	if( mainFont ) {
		delete mainFont;
		mainFont = NULL;
	}
	if( library ) {
		FT_Done_FreeType(library);
		library = 0;
	}
};

bool FtFontManager::init() {
	FT_Error error;
	error = FT_Init_FreeType( &library );              /* initialize library */
  	if(error)
  		return false;
  	else
  		return true;
};

FtFontManager* FtFontManager::getInstance() {
	if( FtFontManager::instance )
		return FtFontManager::instance;
	else {
		FtFontManager::instance = new FtFontManager();
		if (!FtFontManager::instance->init()) {
			delete FtFontManager::instance;
			FtFontManager::instance = NULL;
		}
		return FtFontManager::instance;
	}
};

void FtFontManager::freeFtFontManager() {
	if ( FtFontManager::instance ) {
		delete FtFontManager::instance;
		FtFontManager::instance = NULL;
	}
};

bool FtFontManager::loadMainFont(const char* filename) {
	mainFont = FtFontImpl::loadFont(library, filename);
	if (mainFont)
		return true;
	else
		return false;
};

void FtFontManager::unloadMainFont() {
	if( mainFont ) {
		delete mainFont;
		mainFont = NULL;
	}
};

FtFont* FtFontManager::getMainFont() {
	return mainFont;
};


