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
font rendering system
*/

// TODO: Clean up again
// TODO: fix temporary cache for multi-use


#include <pspkernel.h>
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "../common/mem64.h"

#include "gu_font.h"
#include "texture_subdivision.h"

static FT_Library library = 0;

static FT_Open_Args open_args;
static FT_StreamRec font_stream;

static char font_filename[1024];
static FT_Face	face = 0;
static FT_Stroker stroker = 0;

static unsigned long gufont_font_stream_io( FT_Stream stream,
	unsigned long   offset,
	unsigned char*  buffer,
	unsigned long   count ) {
		sceIoLseek32(stream->descriptor.value, offset, PSP_SEEK_SET);
		if ( count == 0 )
			return 0;
		return sceIoRead(stream->descriptor.value, buffer, count);
}

static void gufont_font_stream_close(FT_Stream  stream) {
	sceIoClose( stream->descriptor.value );
}



int	gufont_haveflags = GU_FONT_HAS_UNICODE_CHARMAP;
int	gufont_hasitalic = 0;
int	gufont_hasbold = 0;


static unsigned int			gufont_color = 0xffffff;
static unsigned int			gufont_border_color = 0x000000;
static unsigned int			gufont_border_enable = 0;
static float				gufont_border_size = 1.0;
static unsigned int			gufont_pixel_size = 16;
static unsigned int			gufont_char_width = 16;
static unsigned int			gufont_char_height = 16;
static unsigned int			gufont_embolden_enable = 0;
static unsigned int			gufont_align = 1;
static unsigned int 			gufont_distance = 16;

static int gu_font_initialized = 0;

#define DIVWIDTH	3

#define CHARWIDTH(c) (((c > 255)?gufont_char_width:gufont_char_width/2))
#define CHARWIDTH2(c,style) (((c > 255)?gufont_char_width:gufont_char_width/2))

#define SBIT_HASH_SIZE (997)

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
	int embolden;
	int   xadvance;
	int   yadvance;
	Cache_Bitmap bitmap;
	Cache_Bitmap border_bitmap;
} SBit_HashItem;

static SBit_HashItem sbit_hash_root[SBIT_HASH_SIZE];


#define IsSet(val,flag) (val&flag)==flag

typedef u32 Color;
#define A(color) ((u8)(color >> 24 & 0x000000FF))
#define B(color) ((u8)(color >> 16 & 0x000000FF))
#define G(color) ((u8)(color >> 8 & 0x000000FF))
#define R(color) ((u8)(color & 0x000000FF))

#define DRAW_BUFFER_SIZE 262144

#define SUB_SCREEN_WIDTH 480
#define SUB_SCREEN_HEIGHT 128
#define SUB_SCREEN_TEXTURE_WIDTH 512

static unsigned char __attribute__((aligned(64))) sub_8888[DRAW_BUFFER_SIZE];
static unsigned char __attribute__((aligned(64))) border_sub_8888[DRAW_BUFFER_SIZE]; 

static char cache_string[2048];



struct vertex_struct
	{
	short texture_x;
	short texture_y;

	float vertex_x;
	float vertex_y;
	float vertex_z;
	};

void putPixelScreenBuffer(void* buffer, Color color, int x, int y)
{
	*((Color*)buffer+y*SUB_SCREEN_TEXTURE_WIDTH+x) = color;
}

void draw_cachedbitmap(void *buffer, Cache_Bitmap* sbt, FT_Int x, FT_Int y, int width, int height, Color color)
{
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
					pixel = color;
				else
					pixel = 0;
				if (pixel)
					putPixelScreenBuffer(buffer, pixel, i+x, j+y);
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
						//a = (grey * A(color)) / 255;
						r = (grey * R(color)) / 255;
						g = (grey * G(color)) / 255;
						b = (grey * B(color)) / 255;
					}
						
	  				pixel = (0xff000000) | (b << 16) | (g << 8) | r;
	  			}
				else
					pixel = 0;
				if (pixel)
					putPixelScreenBuffer(buffer, pixel, i+x, j+y);
			}
	}
}

void draw_bitmap(void *buffer, FT_Bitmap *bitmap, FT_Int x, FT_Int y, int width, int height, Color color)
{
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
					pixel = color;
				else
					pixel = 0;
				if (pixel)
					putPixelScreenBuffer(buffer, pixel, i+x, j+y);
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
						//a = (grey * A(color)) / 255;
						r = (grey * R(color)) / 255;
						g = (grey * G(color)) / 255;
						b = (grey * B(color)) / 255;
					}
					pixel = (0xff000000) | (b << 16) | (g << 8) | r;
	  			}
				else
					pixel = 0;
				if (pixel)
					putPixelScreenBuffer(buffer, pixel, i+x, j+y);
			}
	}
}

// NOTE: the following function only handles triple byte utf8 encodings maximum

static unsigned short get_next_utf8( char** utf8 )
	{
	if ((*utf8)==0) return 0;
	
	// skip follow-bytes at start of stream
	while ((*(*utf8)&0xC0)==0x80)
		(*utf8)++;

	unsigned char u1 = *(*utf8);
	if ((u1&0x80)==0)
		return ((unsigned short)u1);		// ASCII
	else
	if ((u1&0xE0)==0xC0)
		{
		(*utf8)++;
		unsigned char u2 = *(*utf8);
		// 
		return (((unsigned short)(u1&0x1F) << 6) | (unsigned short)(u2&0x3F));
		}
	else
	if ((u1&0xF0)==0xE0)
		{
		(*utf8)++;
		unsigned char u2 = *(*utf8)++;
		unsigned char u3 = *(*utf8);
		return (((unsigned short)(u1&0xF) << 12) | ((unsigned short)(u2&0x3F)<<6) | (unsigned short)(u3&0x3F));
		}
	else
		return (unsigned short)u1;
	}


static unsigned short get_next_utf16le( short** utf16 )
	{
	if ((*utf16)==0) return 0;

	return (unsigned short)(*(*utf16));
	}


static unsigned short get_next_utf16be( short** utf16 )
	{
	if ((*utf16)==0) return 0;
	
	unsigned short u1 =*(*utf16);
	
	return (unsigned short)(((u1&0xFF)<<8)|(u1>>8));
	}


#define SWAPBYTES(c) ((((c)&0xFF)<<8)|((c)>>8))
static short* utf16be2le( short* utf16 )
	{
	if (utf16==0) return 0;
	
	short* utf16le = utf16;
	
	while (*utf16le!=0)
		{
		*utf16le++ = SWAPBYTES(*utf16le);
		}
	return(utf16);
	}

static int gu_font_parse_bbcode( char** c,  int* style )
	{
	if (c==0 || *c==0) return(0);
	register char c0 = (*c)[0], c1 = (*c)[1], c2 = (*c)[2], c3 = (*c)[3];
	if (c0=='[')
		{
		// parse bbcode formatting
		if (c1=='/' && c3==']')
			{
			if (c2=='i' || c2=='I')
				{
				*style &= ~GU_FONT_ITALIC;
				(*c)+=3;
				}
			else
			if (c2=='b' || c2=='B')
				{
				*style &= ~GU_FONT_BOLD;
				(*c)+=3;
				}
			else
				return(0);
			}
		else if (c2==']')
			{
			if (c1=='i' || c1=='I')
				{
				*style |= GU_FONT_ITALIC;
				(*c)+=2;
				}
			else
			if (c1=='b' || c1=='B')
				{
				*style |= GU_FONT_BOLD;
				(*c)+=2;
				}
			else
				return(0);
			}
		else
			return(0);
		}
	else
		return(0);
	
	return(1);
	}
	

static int gu_font_utf16le_parse_bbcode( short** c,  int* style )
	{
	if (c==0 || *c==0) return(0);
	register short c0 = (*c)[0], c1 = (*c)[1], c2 = (*c)[2], c3 = (*c)[3];
	if (c0=='[')
		{
		// parse bbcode formatting
		if (c1=='/' && c3==']')
			{
			if (c2=='i' || c2=='I')
				{
				*style &= ~GU_FONT_ITALIC;
				(*c)+=3;
				}
			else
			if (c2=='b' || c2=='B')
				{
				*style &= ~GU_FONT_BOLD;
				(*c)+=3;
				}
			else
				return(0);
			}
		else if (c2==']')
			{
			if (c1=='i' || c1=='I')
				{
				*style |= GU_FONT_ITALIC;
				(*c)+=2;
				}
			else
			if (c1=='b' || c1=='B')
				{
				*style |= GU_FONT_BOLD;
				(*c)+=2;
				}
			else
				return(0);
			}
		else
			return(0);
		}
	else
		return(0);
	
	return(1);
	}


static int gu_font_utf16be_parse_bbcode( short** c,  int* style )
	{
	if (c==0 || *c==0) return(0);
	
	if (SWAPBYTES((*c)[0])=='[')
		{
		// parse bbcode formatting
		if (SWAPBYTES((*c)[1])=='/' && SWAPBYTES((*c)[3])==']')
			{
			if (SWAPBYTES((*c)[2])=='i' || SWAPBYTES((*c)[2])=='I')
				{
				*style &= ~GU_FONT_ITALIC;
				(*c)+=3;
				}
			else
			if (SWAPBYTES((*c)[2])=='b' || SWAPBYTES((*c)[2])=='B')
				{
				*style &= ~GU_FONT_BOLD;
				(*c)+=3;
				}
			else
				return(0);
			}
		else if (SWAPBYTES((*c)[2])==']')
			{
			if (SWAPBYTES((*c)[1])=='i' || SWAPBYTES((*c)[1])=='I')
				{
				*style |= GU_FONT_ITALIC;
				(*c)+=2;
				}
			else
			if (SWAPBYTES((*c)[1])=='b' || SWAPBYTES((*c)[1])=='B')
				{
				*style |= GU_FONT_BOLD;
				(*c)+=2;
				}
			else
				return(0);
			}
		else
			return(0);
		}
	else
		return(0);
	
	return(1);
	}



void sbit_cache_init(void)
{
	int i;
	for (i = 0; i < SBIT_HASH_SIZE; ++i) {
		memset(&sbit_hash_root[i], 0, sizeof(SBit_HashItem));
	}
}

void sbit_cache_done(void)
{
	int i;
	for (i = 0; i < SBIT_HASH_SIZE; ++i) {
		if(sbit_hash_root[i].bitmap.buffer) {
			free(sbit_hash_root[i].bitmap.buffer);
		}
		if(sbit_hash_root[i].border_bitmap.buffer) {
			free(sbit_hash_root[i].border_bitmap.buffer);
		}
	}
}

SBit_HashItem* sbit_cache_find(unsigned long ucs_code, int size, int embolden) {
	unsigned long hash_value = ucs_code % SBIT_HASH_SIZE;
	if ( (sbit_hash_root[hash_value].ucs_code == ucs_code) &&
		(sbit_hash_root[hash_value].size == size)  &&
		(sbit_hash_root[hash_value].embolden == embolden) )
		return (&sbit_hash_root[hash_value]);
	else
		return 0;
}


void sbit_cache_add(unsigned long ucs_code, int glyph_index, int size, int embolden,
	FT_Bitmap *bitmap, int left, int top, 
	FT_Bitmap *border_bitmap, int border_left, int border_top, int xadvance, int yadvance) {
	unsigned long hash_value = ucs_code % SBIT_HASH_SIZE;
	SBit_HashItem* item = &sbit_hash_root[hash_value];
	if ( item->bitmap.buffer ) {
		free(item->bitmap.buffer);
		item->bitmap.buffer = 0;
	}
	
	if ( item->border_bitmap.buffer ) {
		free(item->border_bitmap.buffer);
		item->border_bitmap.buffer = 0;
	}
	
	item->ucs_code = ucs_code;
	item->glyph_index = glyph_index;
	item->size = size;
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
	
	item->border_bitmap.width     = border_bitmap->width;
	item->border_bitmap.height    = border_bitmap->rows;
	item->border_bitmap.pitch     = border_bitmap->pitch;
	item->border_bitmap.left      = border_left;
	item->border_bitmap.top       = border_top;
	item->border_bitmap.format    = border_bitmap->pixel_mode;
	item->border_bitmap.max_grays = (border_bitmap->num_grays - 1);
	
	int pitch = bitmap->pitch;
	if( pitch < 0 ) pitch = -pitch;
	if ( pitch * bitmap->rows > 0) {
		item->bitmap.buffer = malloc_64(pitch * bitmap->rows);
		memset(item->bitmap.buffer, 0, pitch * bitmap->rows);
		memcpy(item->bitmap.buffer, bitmap->buffer, pitch * bitmap->rows);
	}
	pitch = border_bitmap->pitch;
	if( pitch < 0 ) pitch = -pitch;
	if ( pitch * border_bitmap->rows > 0) {
		item->border_bitmap.buffer = malloc_64(pitch * border_bitmap->rows);
		if ( item->border_bitmap.buffer ) {
			memset(item->border_bitmap.buffer, 0, pitch * border_bitmap->rows);
			memcpy(item->border_bitmap.buffer, border_bitmap->buffer, pitch * border_bitmap->rows);
		}
	}
}

char* gu_font_init()
	{
	if (gu_font_initialized==1) return(0);
	
	FT_Error error;

	error = FT_Init_FreeType( &library );              /* initialize library */
  	if(error)
		return("FT_Init_FreeType failed");
	
	memset(cache_string, 0, 2048);
	
	sbit_cache_init();
	
	font_stream.read =  gufont_font_stream_io;
	font_stream.close = gufont_font_stream_close;
	
	open_args.flags = FT_OPEN_STREAM;
	open_args.stream = &font_stream;
	
	gu_font_initialized = 1;
	return(0);
	}
	

void gu_font_close()
	{
	if (gu_font_initialized==0) return;
	
	if( stroker )
		FT_Stroker_Done(stroker);
		
	if (face)
		FT_Done_Face(face);
	
	FT_Done_FreeType(library);
	library = 0;
	sbit_cache_done();
	gu_font_initialized=0;
	}
	
char* gu_font_load( char* name )
	{
	if (gu_font_initialized==0) return("initialized : fail");
	//*/
	if ( strcmp(font_filename, name) == 0) {
		return(0);
	}
	
	//*/
	memset(font_filename, 0, 1024);
	FT_Error error;
	
	//error = FT_New_Face(library, name, 0, &face);
	font_stream.descriptor.value = sceIoOpen(name, PSP_O_RDONLY, 0777);
	if ( ! font_stream.descriptor.value )
		{
		face = NULL;
		return("gu_font_load: FT_New_Face failed");
		}
	font_stream.base = 0;
	font_stream.size = sceIoLseek32(font_stream.descriptor.value,0, PSP_SEEK_END);
	font_stream.pos = sceIoLseek32(font_stream.descriptor.value,0, PSP_SEEK_SET);
	error = FT_Open_Face(library, &open_args, 0, &face);
	if(error)
		{
		face = NULL;
		return("gu_font_load: FT_New_Face failed");
		}
	
	error = FT_Stroker_New(face->memory, &stroker);
	if(error)
		{
		FT_Done_Face(face);
		face = NULL;
		return("gu_font_load: FT_Stroker_New failed");
		}
	
	strncpy(font_filename, name, 1023);
	
	gu_font_border_enable( 1 );
	gu_font_border_set(1.2);
	gu_font_pixelsize_set( 16 );
	gu_font_color_set( 0xffffffff );
	gu_font_border_color_set( 0xff000000 );

	return(0);
	}

void gu_font_on_suspend()
	{
	sceIoClose(font_stream.descriptor.value);
	font_stream.descriptor.value = 0;
	}

void gu_font_on_resume()
	{
	font_stream.descriptor.value = sceIoOpen(font_filename, PSP_O_RDONLY, 0777);
	sceIoLseek32(font_stream.descriptor.value, font_stream.pos, PSP_SEEK_SET);
	}

FT_Face gu_font_getface() 
	{
	return(face);
	}

void gu_font_pixelsize_set( int size ) 
	{
	
	gufont_pixel_size = size;
	gufont_char_width = size;
	gufont_char_height = size;
	
	FT_Set_Pixel_Sizes(face, gufont_char_width, gufont_char_height);
	
	}

void gu_font_border_enable( int enable )
	{
	//if (face==0) return;
	if (gufont_border_enable==enable) return;
	gufont_border_enable = enable;
	}
	
void gu_font_border_set(float border)
	{
	FT_Stroker_Set( stroker,
		(int)(64 * border),
		FT_STROKER_LINECAP_ROUND,
		FT_STROKER_LINEJOIN_ROUND,
		0 );
	gufont_border_size = border;
	}

void gu_font_border_color_set( unsigned int color )
	{
	//if (face==0) return;
	gufont_border_color = 0xff000000 | color;
	if (gufont_border_enable==0) return;

	}
	

void gu_font_color_set( unsigned int color )
	{
	//if (face==0) return;
	gufont_color = 0xff000000 | color;
	
	}

void gu_font_embolden_enable( int enable )
	{
	if (gufont_embolden_enable == enable) return;
	gufont_embolden_enable = enable;
	}

void gu_font_align_set(unsigned int align) 
	{
	gufont_align = align;
	}

unsigned int gu_font_align_get()
	{
	return gufont_align;
	}

void gu_font_distance_set(unsigned int distance) 
	{
	gufont_distance = distance;
	}

unsigned int gu_font_distance_get()
	{
	return gufont_distance;
	}


int gu_font_line_width_get( char* s )
	{
	if (s==0) return 0;
	char* c = s;
	int x = 0;
	int style = 0;
	int lastcharstyle = 0;

	while (*c!='\0' && *c!='\n')
		{
		if (*c==' ')
			{
			x += CHARWIDTH('t');
			}
		else if ((unsigned char)*c>32)
			{
			if (gu_font_parse_bbcode( &c, &style )==0)
				{
				x += CHARWIDTH2((unsigned char)*c,style);
				lastcharstyle = style;
				}
			}
		c++;
		}
	if (lastcharstyle!=0) x+=4;
	x+=8;
	return (x);
	}
	

int gu_font_width_get( char* s, int flag )
	{
	if (s==0 ) return 0;
	char* c = s;
	int x = 0;
	int width = 0;
	if (flag == 0) flag--;
	int style=0;
	int lastcharstyle=0;
	
	while (*c!='\0' && flag!=0)
		{
		if (*c==' ')
			{
			flag--;
			x += CHARWIDTH('t');
			}
		else if (*c=='\n')
			{
			if (lastcharstyle!=0) x+=4;
			x+=8;
			if (x>width) width=x;
			x = 0;
			}
		else if ((unsigned char)*c>32)
			{
			if (gu_font_parse_bbcode( &c, &style )==0)
				{
				x += CHARWIDTH2((unsigned char)*c,style);
				lastcharstyle = style;
				}
			}
		c++;
		}
	if (lastcharstyle!=0) x+=4;
	x+=8;
	return (x>width?x:width);
	}
	

int gu_font_utf8_line_width_get( char* s )
	{
	if (s==0 ) return 0;
	char* c = s;
	int x = 0;
	int style = 0;
	int lastcharstyle = 0;

	while (*c!='\0' && *c!='\n')
		{
		if (*c==' ')
			{
			x += CHARWIDTH('t');
			}
		else if ((unsigned char)*c>32)
			{
			if (gu_font_parse_bbcode( &c, &style )==0)
				{
				unsigned short ucs = get_next_utf8(&c);
				x += CHARWIDTH2(ucs,style);
				lastcharstyle = style;
				}
			}
		c++;
		}
	if (lastcharstyle!=0) x+=4;
	x+=8;
	return (x);
	}

int gu_font_utf8_width_get( char* s, int flag )
	{
	if (s==0 ) return 0;
	char* c = s;
	int x = 0;
	int width = 0;
	if (flag == 0) flag--;
	int style=0;
	int lastcharstyle = 0;
	unsigned char ch;
	
	while ((ch=(unsigned char)*c)!='\0' && flag!=0)
		{
		if (ch==' ')
			{
			flag--;
			x += CHARWIDTH('t');
			}
		else if (ch=='\n')
			{
			if (lastcharstyle!=0) x+=4;
			x+=8;
			if (x>width) width=x;
			x = 0;
			}
		else if (ch>32)
			{
			if (gu_font_parse_bbcode( &c, &style )==0)
				{
				unsigned short ucs = get_next_utf8(&c);
				x += CHARWIDTH2(ucs,style);
				lastcharstyle = style;
				}
			}
		c++;
		}
	if (lastcharstyle!=0) x+=4;
	x+=8;
	return (x>width?x:width);
	}



int gu_font_utf16le_line_width_get( short* s )
	{
	if (s==0 ) return 0;
	short* c = s;
	int x = 0;
	int style = 0;
	int lastcharstyle = 0;

	while (*c!=0 && *c!='\n')
		{
		if (*c==' ')
			{
			x += CHARWIDTH('t');
			}
		else if ((unsigned short)*c>32)
			{
			if (gu_font_utf16le_parse_bbcode( &c, &style )==0)
				{
				unsigned short ucs = get_next_utf16le(&c);
				x += CHARWIDTH2(ucs,style);
				lastcharstyle = style;
				}
			}
		c++;
		}
	if (lastcharstyle!=0) x+=4;
	x+=8;
	return (x);
	}
	
int gu_font_utf16le_width_get( short* s, int flag )
	{
	if (s==0 ) return 0;
	short* c = s;
	int x = 0;
	int width = 0;
	if (flag == 0) flag--;
	int style=0;
	int lastcharstyle=0;
	unsigned short ch;
	
	while ((ch=(unsigned short)*c)!=0 && flag!=0)
		{
		if (ch==' ')
			{
			flag--;
			x += CHARWIDTH('t');
			}
		else if (ch=='\n')
			{
			if (lastcharstyle!=0) x+=4;
			x+=8;
			if (x>width) width=x;
			x = 0;
			}
		else if (ch>32)
			{
			if (gu_font_utf16le_parse_bbcode( &c, &style )==0)
				{
				unsigned short ucs = ch;
				x += CHARWIDTH2(ucs,style);
				lastcharstyle = style;
				}
			}
		c++;
		}
	if (lastcharstyle!=0) x+=4;
	x+=8;
	return (x>width?x:width);
	}


int gu_font_utf16be_line_width_get( short* s )
	{
	if (s==0 ) return 0;
	short* c = s;
	int x = 0;
	int style = 0;
	int lastcharstyle = 0;
	
	while (*c!=0 && SWAPBYTES(*c)!='\n')
		{
		if (SWAPBYTES(*c)==' ')
			{
			x += CHARWIDTH('t');
			}
		else if ((unsigned short)SWAPBYTES(*c)>32)
			{
			if (gu_font_utf16be_parse_bbcode( &c, &style )==0)
				{
				unsigned short ucs = get_next_utf16be(&c);
				x += CHARWIDTH2(ucs,style);
				lastcharstyle = style;
				}
			}
		c++;
		}
	if (lastcharstyle!=0) x+=4;
	x+=8;
	return (x);
	}

int gu_font_utf16be_width_get( short* s, int flag )
	{
	if (s==0 ) return 0;
	short* c = s;
	int x = 0;
	int width = 0;
	if (flag == 0) flag--;
	int style=0;
	int lastcharstyle=0;
	
	unsigned short ch;
	while ((ch=(unsigned short)SWAPBYTES((*c)))!=0 && flag!=0)
		{
		if (ch==' ')
			{
			flag--;
			x += CHARWIDTH('t');
			}
		else if (ch=='\n')
			{
			if (lastcharstyle!=0) x+=4;
			x+=8;
			if (x>width) width=x;
			x = 0;
			}
		else if (ch>32)
			{
			if (gu_font_utf16be_parse_bbcode( &c, &style )==0)
				{
				unsigned short ucs = ch;
				x += CHARWIDTH2(ucs,style);
				lastcharstyle = style;
				}
			}
		c++;
		}
	if (lastcharstyle!=0) x+=4;
	x+=8;
	return (x>width?x:width);
	}


int gu_font_height_get( char* s )
	{
	if (s == 0 ) return 0;
	char* c = s;
	int height = gufont_char_height;//FONTHEIGHT;
	
	while (*c!='\0')
		{
		if (*c=='\n')
			{
			height += gufont_char_height;//FONTHEIGHT;
			}
		c++;
		}
	return (height);
	}


int gu_font_utf16be_height_get( short* s )
	{
	if (s == 0 ) return 0;
	short* c = s;
	int height = gufont_char_height;//FONTHEIGHT;
	
	while (*c!=0)
		{
		if (*c=='\n')
			{
			height += gufont_char_height;//FONTHEIGHT;
			}
		c++;
		}
	return (height);
	}


int gu_font_utf16le_height_get( short* s )
	{
	if (s == 0 ) return 0;
	short* c = s;
	int height = gufont_char_height;//FONTHEIGHT;
	
	while (*c!=0)
		{
		if ((*c>>8)=='\n')
			{
			height += gufont_char_height;//FONTHEIGHT;
			}
		c++;
		}
	return (height);
	}


inline int gu_font_height()
	{
	return gufont_char_height;//FONTHEIGHT;
	}

void render_string(int x, int y, char* s, int flags){
	
	int (*gu_font_line_width)( char* );
	int (*gu_font_width)( char* s, int flag );
	
	if (IsSet(flags,FLAG_UTF8)) {
		gu_font_line_width = gu_font_utf8_line_width_get;
		gu_font_width = gu_font_utf8_width_get;
	}
	else {
		gu_font_line_width = gu_font_line_width_get;
		gu_font_width = gu_font_width_get;
	}
	
	int max_width = gu_font_width( s, 0 );
	
	int x_x, y_y;
	
	if (IsSet(flags,FLAG_ALIGN_CENTER)){
		x_x = x + ((max_width-gu_font_line_width( s ))/2);
	}
	else if (IsSet(flags,FLAG_ALIGN_RIGHT)){
		x_x = x + (max_width-gu_font_line_width( s ));
	}
	else
		x_x = x;
	
	y_y = y;
	
	int width = SUB_SCREEN_WIDTH;
	int height = SUB_SCREEN_HEIGHT;
	
	int style = 0;
	
	FT_Error error;
	FT_GlyphSlot slot;
	FT_UInt glyph_index;
	
	FT_Bool use_kerning; 
	FT_UInt previous; 
	
	use_kerning = FT_HAS_KERNING( gu_font_getface() ); 
	previous = 0;

	//*/
	while (*s!='\0') {
		if (*s=='\n') {
			if (IsSet(flags,FLAG_ALIGN_CENTER)){
				x_x = x + ((max_width-gu_font_line_width( &s[1] ))/2);
			}
			else if (IsSet(flags,FLAG_ALIGN_RIGHT)){
				x_x = x + (max_width-gu_font_line_width( &s[1] ));
			}
			else
				x_x = x;
			y_y += gufont_char_height + 2;
			previous = 0;
		}
		else if ((unsigned char)*s>=32) {
			if (gu_font_parse_bbcode( &s, &style )==0) {
				unsigned short ucs;
				if (IsSet(flags,FLAG_UTF8)){
					ucs = get_next_utf8(&s);	
				}
				else{
					ucs = *s;
				}
				//*/
				SBit_HashItem* cache_item =  sbit_cache_find(ucs, gufont_pixel_size, gufont_embolden_enable);
				if ( cache_item ) {
					if ( use_kerning && previous && cache_item->glyph_index ) { 
						FT_Vector delta; 
						FT_Get_Kerning( gu_font_getface(), previous, cache_item->glyph_index, FT_KERNING_DEFAULT, &delta ); 
						x_x += delta.x >> 6; 
					}
					draw_cachedbitmap(sub_8888, &cache_item->bitmap, x_x + cache_item->bitmap.left,
						y_y - cache_item->bitmap.top, width,
						height, gufont_color);
					draw_cachedbitmap(border_sub_8888, &cache_item->border_bitmap, x_x + cache_item->border_bitmap.left,
						y_y - cache_item->border_bitmap.top, width,
						height, gufont_border_color);
							
					x_x += cache_item->xadvance >> 6;
					previous = cache_item->glyph_index;
					//x_x += DIVWIDTH;
					//y_y += cache_item->yadvance >> 6;
				}
				else { 
					FT_Int glyph_index = FT_Get_Char_Index( gu_font_getface(), ucs);
					error = FT_Load_Glyph( gu_font_getface(), glyph_index, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP );
					if (error)
						goto render_next;
					FT_Glyph glyph;
					error = FT_Get_Glyph(gu_font_getface()->glyph, &glyph);
					if (error)
						goto render_next;
					error = FT_Glyph_Stroke( &glyph, stroker, 1 );
					if (error) {
						FT_Done_Glyph(glyph);
						goto render_next;
					}
					error = FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL, 0, 1);
					if (error) {
						FT_Done_Glyph(glyph);
						goto render_next;
					}
					error = FT_Render_Glyph(gu_font_getface()->glyph, FT_RENDER_MODE_NORMAL);
					if (error) {
						FT_Done_Glyph(glyph);
						goto render_next;
					}
					slot = gu_font_getface()->glyph;
					
					if ( gufont_embolden_enable )
						FT_GlyphSlot_Embolden(slot);
					
					if ( use_kerning && previous && glyph_index ) { 
						FT_Vector delta; 
						FT_Get_Kerning( gu_font_getface(), previous, glyph_index, FT_KERNING_DEFAULT, &delta ); 
						x_x += delta.x >> 6; 
					}
					
					FT_BitmapGlyph bit = (FT_BitmapGlyph)glyph;
					draw_bitmap(sub_8888, &slot->bitmap, x_x + slot->bitmap_left,
						y_y - slot->bitmap_top, width,
						height, gufont_color);
					draw_bitmap(border_sub_8888, &bit->bitmap, x_x + bit->left,
						y_y - bit->top, width,
						height, gufont_border_color);
										
					x_x += slot->advance.x >> 6;
					previous = glyph_index;
					//x_x += DIVWIDTH;
					//y_y += slot->advance.y >> 6;
					sbit_cache_add(ucs, glyph_index, gufont_pixel_size, gufont_embolden_enable, 
						&slot->bitmap, slot->bitmap_left, slot->bitmap_top,  
						&bit->bitmap, bit->left, bit->top, slot->advance.x, slot->advance.y);
					
					FT_Done_Glyph(glyph);
					
					
					/*/
					error = FT_Load_Char( gu_font_getface(), ucs, FT_LOAD_RENDER);
					if (error)
						goto render_next;
					slot = gu_font_getface()->glyph;
					draw_bitmap(image, &slot->bitmap, x_x + slot->bitmap_left,
						y_y - slot->bitmap_top, width,
						height, gufont_color);
					x_x += slot->advance.x >> 6;
					y_y += slot->advance.y >> 6;
					sbit_cache_add(ucs, gufont_pixel_size, gufont_border_enable,  
						&slot->bitmap, slot->bitmap_left, slot->bitmap_top, slot->advance.x, slot->advance.y);
					//*/
					
				}
				//*/
			}
		}
render_next:
		s++;
	}
	//*/
}

static void load_subtitle_texture(void *image)
	{
	sceGuEnable(GU_BLEND);
	//sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0xffffff, 0xffffff);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexImage(0, SUB_SCREEN_TEXTURE_WIDTH, SUB_SCREEN_HEIGHT, SUB_SCREEN_TEXTURE_WIDTH, image);
	//sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	}

static void gu_font_draw_sprite(struct texture_subdivision_struct *t)
	{
	struct vertex_struct *v = sceGuGetMemory(2 * sizeof(struct vertex_struct));

	v[0].texture_x = t->output_texture_x_start;
	v[0].texture_y = t->output_texture_y_start;
	v[0].vertex_x  = (int) t->output_vertex_x_start;
	v[0].vertex_y  = t->output_vertex_y_start;
	v[0].vertex_z  = 0.0;

	v[1].texture_x = t->output_texture_x_end;
	v[1].texture_y = t->output_texture_y_end;
	v[1].vertex_x  = (int) t->output_vertex_x_end;
	v[1].vertex_y  = t->output_vertex_y_end;
	v[1].vertex_z  = 0.0;

	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, v);
	}



void gu_font_printf( int x, int y, int flags, char* fmt, ... )
	{
		
	va_list         ap;
	char   p[512];

	va_start( ap,fmt );
	vsnprintf( p,512,fmt,ap );
	va_end( ap );

	gu_font_print(x, y, flags, p );
	}


void gu_font_print( int x, int y, int flags, char* s )
	{
	if (face==NULL) return;
	
	if (s==0) return;
	
	if (!IsSet(gufont_haveflags,GU_FONT_HAS_UNICODE_CHARMAP))
		{
		flags &= ~FLAG_UTF8;
		flags &= ~FLAG_UTF16BE;
		flags &= ~FLAG_UTF16LE;
		}
	
	/*/
	int width, height;
	if (IsSet(flags,FLAG_UTF16BE))
		{
		if (s[0]=='\0'&&s[1]==0) return;
		width = gu_font_utf16be_width_get( (short*)s, 0 );
		height = gu_font_utf16be_height_get( (short*)s );
		}
	else
	if (IsSet(flags,FLAG_UTF16LE))
		{
		if (s[0]=='\0'&&s[1]==0) return;
		width = gu_font_utf16le_width_get( (short*)s, 0 );
		height = gu_font_utf16le_height_get( (short*)s );
		}
	else
	if (IsSet(flags,FLAG_UTF8))
		{
		if (s[0]=='\0') return;
		width = gu_font_utf8_width_get( s, 0 );
		height = gu_font_height_get( s );
		}
	else
		{
		if (s[0]=='\0') return;
		width = gu_font_width_get( s, 0 );
		height = gu_font_height_get( s );
		}
	if (height>512) height=512;
	//*/
	
	
	if (IsSet(flags,FLAG_UTF16BE)||IsSet(flags,FLAG_UTF16LE))
		;
	else {
		//render_string( 0, x, y, s, flags );
		if ( strcmp(s, cache_string) != 0) {
			memset(sub_8888, 0, DRAW_BUFFER_SIZE);
			memset(border_sub_8888, 0, DRAW_BUFFER_SIZE);
			memset(cache_string, 0, 2048);
			strncpy(cache_string, s, 2047);
			render_string(x, gu_font_height(), cache_string, flags);
			sceKernelDcacheWritebackInvalidateAll();
			sceGuTexFlush();
		}
		
	}
	
	int status = sceGuGetAllStatus();
	sceGuEnable(GU_TEXTURE_2D);
	
	struct texture_subdivision_struct texture_subdivision;
	
	if (gufont_border_enable) {
		load_subtitle_texture(border_sub_8888);
		texture_subdivision_constructor(&texture_subdivision, SUB_SCREEN_WIDTH, SUB_SCREEN_HEIGHT, 16, SUB_SCREEN_WIDTH, SUB_SCREEN_HEIGHT, 0, y);
		do
			{
			texture_subdivision_get(&texture_subdivision);
			gu_font_draw_sprite(&texture_subdivision);
			}
		while (texture_subdivision.output_last == 0);
	}
	//*/
	load_subtitle_texture(sub_8888);
	texture_subdivision_constructor(&texture_subdivision, SUB_SCREEN_WIDTH, SUB_SCREEN_HEIGHT, 16, SUB_SCREEN_WIDTH, SUB_SCREEN_HEIGHT, 0, y);
	do
		{
		texture_subdivision_get(&texture_subdivision);
		gu_font_draw_sprite(&texture_subdivision);
		}
	while (texture_subdivision.output_last == 0);
	//*/
	
	sceGuSetAllStatus(status);		
	}
	
