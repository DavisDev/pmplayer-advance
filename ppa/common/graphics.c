#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <pspgu.h>

#include "graphics.h"
#include "framebuffer.h"

#define IS_ALPHA(color) (((color)&0xff000000)==0xff000000?0:1)
#ifndef PSP_FRAMEBUFFER_SIZE
#define PSP_FRAMEBUFFER_SIZE (PSP_SCREEN_TEXTURE_WIDTH*PSP_SCREEN_HEIGHT*4)
#endif
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

typedef struct
{
	unsigned short u, v;
	short x, y, z;
} Vertex;

unsigned int __attribute__((aligned(16))) list[262144];
static int displayBufferNumber;
static int initialized = 0;

static int getNextPower2(int width){
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

static void drawLine(int x0, int y0, int x1, int y1, int color, Color* destination, int width){
	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	
	if (dy < 0) { dy = -dy;  stepy = -width; } else { stepy = width; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;
	
	y0 *= width;
	y1 *= width;
	destination[x0+y0] = color;
	if (dx > dy) {
		int fraction = dy - (dx >> 1);
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			destination[x0+y0] = color;
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			destination[x0+y0] = color;
		}
	}
}

void initGraphics(){
	displayBufferNumber = 0;

	sceGuInit();

	guStart();
	sceGuDrawBuffer(GU_PSM_8888, (void*)PSP_FRAMEBUFFER_SIZE, PSP_SCREEN_TEXTURE_WIDTH);
	sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, (void*)0, PSP_SCREEN_TEXTURE_WIDTH);
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
	sceGuDepthBuffer((void*) (PSP_FRAMEBUFFER_SIZE*2), PSP_SCREEN_TEXTURE_WIDTH);
	sceGuOffset(2048 - (PSP_SCREEN_WIDTH / 2), 2048 - (PSP_SCREEN_HEIGHT / 2));
	sceGuViewport(2048, 2048, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuDepthRange(0xc350, 0x2710);
	sceGuScissor(0, 0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuAlphaFunc(GU_GREATER, 0, 0xff);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuAmbientColor(0xffffffff);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuFinish();
	sceGuSync(0, 0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	initialized = 1;
}

void disableGraphics(){
	sceGuTerm();
	initialized = 0;
}

Color* getVramDrawBuffer(){
	Color* vram = (Color*) globalVramBase;
	if (displayBufferNumber == 0) vram += PSP_FRAMEBUFFER_SIZE / sizeof(Color);
	return vram;
}

Color* getVramDisplayBuffer(){
	Color* vram = (Color*) globalVramBase;
	if (displayBufferNumber == 1) vram += PSP_FRAMEBUFFER_SIZE / sizeof(Color);
	return vram;
}

void guStart(){
	sceGuStart(GU_DIRECT, list);
}

void flipScreen(){
	if (!initialized) return;
	sceGuSwapBuffers();
	displayBufferNumber ^= 1;
}

void clearScreen(Color color){
	if (!initialized) return;
	guStart();
	sceGuClearDepth(0);
	sceGuClearColor(color);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);
}

void putPixelToScreen(Color color, int x, int y){
	Color* vram = getVramDrawBuffer();
	vram[PSP_SCREEN_TEXTURE_WIDTH * y + x] = color;
}

Color getPixelFromScreen(int x, int y){
	Color* vram = getVramDrawBuffer();
	return vram[PSP_SCREEN_TEXTURE_WIDTH * y + x];
}

void drawLineInScreen(Color color, int x0, int y0, int x1, int y1){
	drawLine(x0, y0, x1, y1, color, getVramDrawBuffer(), PSP_SCREEN_TEXTURE_WIDTH);
}

void fillScreenRect(Color color, int x0, int y0, int width, int height){
	if (!initialized) return;
	int skipX = PSP_SCREEN_TEXTURE_WIDTH - width;
	int x, y;
	Color* data = getVramDrawBuffer() + x0 + y0 * PSP_SCREEN_TEXTURE_WIDTH;
	for (y = 0; y < height; y++, data += skipX) {
		for (x = 0; x < width; x++, data++) *data = color;
	}
}

void blitImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy){
	if (!initialized) return;
	Color* vram = getVramDrawBuffer();
	sceKernelDcacheWritebackInvalidateAll();
	guStart();
	sceGuCopyImage(GU_PSM_8888, sx, sy, width, height, source->textureWidth, source->data, dx, dy, PSP_SCREEN_TEXTURE_WIDTH, vram);
	sceGuFinish();
	sceGuSync(0,0);
}

void blitAlphaImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy){
	if (!initialized) return;

	sceKernelDcacheWritebackInvalidateAll();
	guStart();
	sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, (void*) source->data);
	float u = 1.0f / ((float)source->textureWidth);
	float v = 1.0f / ((float)source->textureHeight);
	sceGuTexScale(u, v);
	
	int j = 0;
	while (j < width) {
		Vertex* vertices = (Vertex*) sceGuGetMemory(2 * sizeof(Vertex));
		int sliceWidth = 64;
		if (j + sliceWidth > width) sliceWidth = width - j;
		vertices[0].u = sx + j;
		vertices[0].v = sy;
		vertices[0].x = dx + j;
		vertices[0].y = dy;
		vertices[0].z = 0;
		vertices[1].u = sx + j + sliceWidth;
		vertices[1].v = sy + height;
		vertices[1].x = dx + j + sliceWidth;
		vertices[1].y = dy + height;
		vertices[1].z = 0;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
		j += sliceWidth;
	}
	
	sceGuFinish();
	sceGuSync(0, 0);
}

Image* createImage(int width, int height){
	Image* image = (Image*) malloc(sizeof(Image));
	if (!image) return NULL;
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth = getNextPower2(width);
	image->textureHeight = getNextPower2(height);
	image->data = (Color*) memalign(32, image->textureWidth * image->textureHeight * sizeof(Color));
	if (!image->data) {
		free(image);
		return NULL;
	}
	memset(image->data, 0, image->textureWidth * image->textureHeight * sizeof(Color));
	return image;
}

void freeImage(Image* image){
	if (image != NULL) {
		if (image->data != NULL)
			free(image->data);
		free(image);
	}
}

void clearImage(Image* image, Color color){
	int i;
	int size = image->textureWidth * image->textureHeight;
	Color* data = image->data;
	for (i = 0; i < size; i++, data++) *data = color;
}

void putPixelToImage(Image* image, Color color, int x, int y){
	image->data[x + y * image->textureWidth] = color;
}



Color getPixelFromImage(Image* image, int x, int y){
	return image->data[x + y * image->textureWidth];
}

void drawLineInImage(Image* image, Color color, int x0, int y0, int x1, int y1){
	drawLine(x0, y0, x1, y1, color, image->data, image->textureWidth);
}

void fillImageRect(Image* image, Color color, int x0, int y0, int width, int height){
	int skipX = image->textureWidth - width;
	int x, y;
	Color* data = image->data + x0 + y0 * image->textureWidth;
	for (y = 0; y < height; y++, data += skipX) {
		for (x = 0; x < width; x++, data++) *data = color;
	}
}

void fillImageEllipse(Image* image, Color color, int x0, int y0, int width, int height, int r) {
	int skipX = image->textureWidth - width;
	int x, y;
	Color* data = image->data + x0 + y0 * image->textureWidth;
	for (y = 0; y < height; y++, data += skipX) {
		for (x = 0; x < width; x++, data++) {
			if ( (x < r) && (y < r)  && ((r-x)*(r-x)+(r-y)*(r-y) > r*r) )
				continue;
			else if ( (x < r) && (y > height-r-1)  && ((r-x)*(r-x)+(y-height+r+1)*(y-height+r+1) > r*r) )
				continue;
			else if ( (x > width-r-1) && (y < r)  && ((x-width+r+1)*(x-width+r+1)+(r-y)*(r-y) > r*r) )
				continue;
			else if ( (x > width-r-1) && (y > height-r-1)  && ((x-width+r+1)*(x-width+r+1)+(y-height+r+1)*(y-height+r+1) > r*r) )
				continue;
			else
				*data = color;
		}
	}
} 

void putImageToImage(Image* src, Image* dest, int dx, int dy, int dw, int dh) {
	int width = src->imageWidth > dw ? dw : src->imageWidth;
	int height = src->imageHeight > dh ? dh : src->imageHeight;
	int x, y;
	for(y = 0; y < height; y++ )
		for( x = 0; x < width; x++ ) {
			dest->data[(dy+y)*dest->textureWidth + (dx+x)] = src->data[y*src->textureWidth+x];
		}
}

void makeScreenSnapshot(Image* dest) {
	if ( dest->imageWidth != PSP_SCREEN_WIDTH || dest->imageHeight != PSP_SCREEN_HEIGHT)
		return;
	Color* vram = getVramDrawBuffer();
	memcpy(dest->data, vram, PSP_FRAMEBUFFER_SIZE);
}


