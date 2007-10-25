#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <psptypes.h>

#ifndef PSP_SCREEN_TEXTURE_WIDTH
#define	PSP_SCREEN_TEXTURE_WIDTH 512
#endif
#ifndef PSP_SCREEN_WIDTH
#define PSP_SCREEN_WIDTH 480
#endif
#ifndef PSP_SCREEN_HEIGHT
#define PSP_SCREEN_HEIGHT 272
#endif

#ifndef PSP_TVOUT_TEXTURE_WIDTH
#define	PSP_TVOUT_TEXTURE_WIDTH 768
#endif
#ifndef PSP_TVOUT_WIDTH
#define PSP_TVOUT_WIDTH 720
#endif
#ifndef PSP_TVOUT_HEIGHT
#define PSP_TVOUT_HEIGHT 480
#endif

typedef u32 Color;
#define A(color) ((u8)(color >> 24 & 0x000000FF))
#define B(color) ((u8)(color >> 16 & 0x000000FF))
#define G(color) ((u8)(color >> 8 & 0x000000FF))
#define R(color) ((u8)(color & 0x000000FF))

typedef struct
{
	int textureWidth;  
	int textureHeight;
	int imageWidth;  
	int imageHeight;
	Color* data;
} Image;


#ifdef __cplusplus
extern "C" {
#endif

void initGraphics(int type, int mode);

void disableGraphics();

void setGraphicsVideoMode(int mode);

void setGraphicsTVAspectRatio(int ar);

void flipScreen();

void blitImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy);

void blitAlphaImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy);


Image* createImage(int width, int height);

void freeImage(Image* image);

void clearImage(Image* image, Color color);

void putPixelToImage(Image* image, Color color, int x, int y);

Color getPixelFromImage(Image* image, int x, int y);

void drawLineInImage(Image* image, Color color, int x0, int y0, int x1, int y1);

void fillImageRect(Image* image, Color color, int x0, int y0, int width, int height);

void fillImageEllipse(Image* image, Color color, int x0, int y0, int width, int height, int r);

void putImageToImage(Image* src, Image* dest, int dx, int dy, int dw, int dh);

#ifdef __cplusplus
}
#endif

#endif
