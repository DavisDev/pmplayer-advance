/*
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/common.h"
#include "libavcodec/dsputil.h"
//#include "mmi.h"

#if 0
#define W1 2841 /* 2048*sqrt (2)*cos (1*pi/16) */
#define W2 2676 /* 2048*sqrt (2)*cos (2*pi/16) */
#define W3 2408 /* 2048*sqrt (2)*cos (3*pi/16) */
#define W4 2048 /* 2048*sqrt (2)*cos (4*pi/16) */
#define W5 1609 /* 2048*sqrt (2)*cos (5*pi/16) */
#define W6 1108 /* 2048*sqrt (2)*cos (6*pi/16) */
#define W7 565  /* 2048*sqrt (2)*cos (7*pi/16) */
#define ROW_SHIFT 8
#define COL_SHIFT 17
#else
#define W1  22725  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W2  21407  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W3  19266  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W4  16383  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W5  12873  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W6  8867   //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W7  4520   //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define ROW_SHIFT 11
#define COL_SHIFT 20 // 6
#endif

/* signed 16x16 -> 32 multiply add accumulate */
#define MAC16(rt, ra, rb) rt += (ra) * (rb)

/* signed 16x16 -> 32 multiply */
#define MUL16(rt, ra, rb) rt = (ra) * (rb)

#define BUTTERFLY(t0,t1,W0,W1,d0,d1)	\
do {					\
    int tmp = W0 * (d0 + d1);		\
    t0 = tmp + (W1 - W0) * d1;		\
    t1 = tmp - (W1 + W0) * d0;		\
} while (0)

#define BUTTERFLY0(t0,t1,W0,W1,d0,d1)	\
do {					\
    t0 = W0 * d0 + W1 * d1;		\
    t1 = W0 * d1 - W1 * d0;		\
} while (0)

#define BUTTERFLYADD0(t0,t1,W0,W1,d0,d1)\
do {					\
    t0 += W0 * d0 + W1 * d1;		\
    t1 += W0 * d1 - W1 * d0;		\
} while (0)

const static float __attribute__((aligned(64))) W1W7W5W3[] =
{
   W7, W1, W3,W5,
  -W1, W7,-W5,W3,
    0,  0,  0, 0,
    0,  0,  0, 0
};

const static float __attribute__((aligned(64))) mW5W3mW7mW1[] =
{
	  0,  0,   0,  0,
	  0,  0,   0,  0,
	-W5, W3, -W7,-W1,
	 W3, W5, -W1, W7
};

static float __attribute__((aligned(64))) fv[4];
static int __attribute__((aligned(64))) fv_ret2[4];

#define loadW1W7W5W3 \
    __asm__ volatile ( \
            "lv.q C300, 0(%0)\n" \
            "lv.q C310, 16(%0)\n" \
            "lv.q C320, 32(%0)\n" \
            "lv.q C330, 48(%0)\n" \
    :: "r"(mat_ptr) : "memory");

#define loadmW5W3mW7mW1 \
    __asm__ volatile ( \
            "lv.q C400, 0(%0)\n" \
            "lv.q C410, 16(%0)\n" \
            "lv.q C420, 32(%0)\n" \
            "lv.q C430, 48(%0)\n" \
    :: "r"(mat_ptr) : "memory");

#define multiplyWs \
    __asm__ volatile ( \
			"vmidt.q M000\n" \
			"lv.q C030, 0(%1)\n" \
			"vmmul.q M100,M300,M000\n" \
			"vmmul.q M200,M400,M000\n" \
			"vadd.q C030, C130, C230\n" \
			"vf2iz.q C530, C030, 0\n" \
			"sv.q C530,0(%0)\n" \
    :"+r"(ret_ptr2):"r"(fv_ptr): "memory");

static inline void idctRowCondDC (DCTELEM * row)
{
	int a0, a1, a2, a3;
	int d0,d4;
	float *fv_ptr = fv;
	int *ret_ptr2 = fv_ret2;

        uint32_t temp;
        if(sizeof(DCTELEM)==2){
            if (!(((uint32_t*)row)[1] |
                  ((uint32_t*)row)[2] |
                  ((uint32_t*)row)[3] | 
                  row[1])) {
                temp = (row[0] << 3) & 0xffff;
                temp += temp << 16;
                ((uint32_t*)row)[0]=((uint32_t*)row)[1] =
                ((uint32_t*)row)[2]=((uint32_t*)row)[3] = temp;
                return;
	    }
        }else{
            if (!(row[1]|row[2]|row[3]|row[4]|row[5]|row[6]|row[7])) {
                row[0]=row[1]=row[2]=row[3]=row[4]=row[5]=row[6]=row[7]= row[0] << 3;
                return;
            }
        }

	d4=W4*row[4];

	fv[0] = row[1];
	fv[1] = row[7];
	fv[2] = row[3];
	fv[3] = row[5];

        a0 = (W4 * row[0]) + (1 << (ROW_SHIFT - 1));
        a1 = a0;

	multiplyWs

        a0 += d4;
        a1 -= d4;
        a2 = a1;
        a3 = a0;

        BUTTERFLY(d0,d4,row[2],row[6],W2,W6);

        a0 += d0;
        a1 += d4;
        a2 -= d4;
        a3 -= d0;
		
	row[0] = (a0 + fv_ret2[1]) >> ROW_SHIFT;
	row[7] = (a0 - fv_ret2[1]) >> ROW_SHIFT;
	row[1] = (a1 + fv_ret2[2]) >> ROW_SHIFT;
	row[6] = (a1 - fv_ret2[2]) >> ROW_SHIFT;
	row[2] = (a2 + fv_ret2[3]) >> ROW_SHIFT;
	row[5] = (a2 - fv_ret2[3]) >> ROW_SHIFT;
	row[3] = (a3 + fv_ret2[0]) >> ROW_SHIFT;
	row[4] = (a3 - fv_ret2[0]) >> ROW_SHIFT;
}

static inline void idctSparseColPut (uint8_t *dest, int line_size, 
                                     DCTELEM * col)
{
	int a0, a1, a2, a3;
        uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;
	int d0,d4=W4*col[8*4];
	float *fv_ptr = fv;
	int *ret_ptr2 = fv_ret2;

        /* XXX: I did that only to give same values as previous code */
	a0 = W4 * col[8*0] + (1<<(COL_SHIFT-1));
	a1 = a0;

	fv[0] = col[8*1];
	fv[1] = col[8*7];
	fv[2] = col[8*3];
	fv[3] = col[8*5];

	multiplyWs

        a0 += d4;
        a1 -= d4;
        a2 = a1;
        a3 = a0;

        BUTTERFLY(d0,d4,col[8*2],col[8*6],W2,W6);

        a0 += d0;
        a1 += d4;
        a2 -= d4;
        a3 -= d0;

        dest[0] = cm[(a0 + fv_ret2[1]) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a1 + fv_ret2[2]) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a2 + fv_ret2[3]) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a3 + fv_ret2[0]) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a3 - fv_ret2[0]) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a2 - fv_ret2[3]) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a1 - fv_ret2[2]) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a0 - fv_ret2[1]) >> COL_SHIFT];
}

static inline void idctSparseColAdd (uint8_t *dest, int line_size, 
                                     DCTELEM * col)
{
	int a0, a1, a2, a3;
        uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;
	int d0,d4=W4*col[8*4];
	fv[0] = col[8*1];fv[1] = col[8*7];fv[2] = col[8*3];fv[3] = col[8*5];

	float *fv_ptr = fv;
	int *ret_ptr2 = fv_ret2;


        /* XXX: I did that only to give same values as previous code */

	a0 = W4 * col[8*0] + (1<<(COL_SHIFT-1));
	a1 = a0;

	multiplyWs

        a0 += d4;
        a1 -= d4;
        a3 = a0;
        a2 = a1;
        BUTTERFLY(d0,d4,col[8*2],col[8*6],W2,W6);	

        a0 += d0;
        a1 += d4;
        a2 -= d4;
        a3 -= d0;

        dest[0] = cm[dest[0] + ((a0 + fv_ret2[1]) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a1 + fv_ret2[2]) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a2 + fv_ret2[3]) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a3 + fv_ret2[0]) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a3 - fv_ret2[0]) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a2 - fv_ret2[3]) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a1 - fv_ret2[2]) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a0 - fv_ret2[1]) >> COL_SHIFT)];
}

static inline void idctSparseCol (DCTELEM * col)
{
	int a0, a1, a2, a3, b0, b1, b2, b3;

        /* XXX: I did that only to give same values as previous code */
	a0 = W4 * (col[8*0] + ((1<<(COL_SHIFT-1))/W4));
	a1 = a0;
	a2 = a0;
	a3 = a0;

        a0 +=  + W2*col[8*2];
        a1 +=  + W6*col[8*2];
        a2 +=  - W6*col[8*2];
        a3 +=  - W2*col[8*2];

        MUL16(b0, W1, col[8*1]);
        MUL16(b1, W3, col[8*1]);
        MUL16(b2, W5, col[8*1]);
        MUL16(b3, W7, col[8*1]);

        MAC16(b0, + W3, col[8*3]);
        MAC16(b1, - W7, col[8*3]);
        MAC16(b2, - W1, col[8*3]);
        MAC16(b3, - W5, col[8*3]);

	if(col[8*4]){
            a0 += + W4*col[8*4];
            a1 += - W4*col[8*4];
            a2 += - W4*col[8*4];
            a3 += + W4*col[8*4];
	}

	if (col[8*5]) {
            MAC16(b0, + W5, col[8*5]);
            MAC16(b1, - W1, col[8*5]);
            MAC16(b2, + W7, col[8*5]);
            MAC16(b3, + W3, col[8*5]);
	}

	if(col[8*6]){
            a0 += + W6*col[8*6];
            a1 += - W2*col[8*6];
            a2 += + W2*col[8*6];
            a3 += - W6*col[8*6];
	}

	if (col[8*7]) {
            MAC16(b0, + W7, col[8*7]);
            MAC16(b1, - W5, col[8*7]);
            MAC16(b2, + W3, col[8*7]);
            MAC16(b3, - W1, col[8*7]);
	}

        col[0 ] = ((a0 + b0) >> COL_SHIFT);
        col[8 ] = ((a1 + b1) >> COL_SHIFT);
        col[16] = ((a2 + b2) >> COL_SHIFT);
        col[24] = ((a3 + b3) >> COL_SHIFT);
        col[32] = ((a3 - b3) >> COL_SHIFT);
        col[40] = ((a2 - b2) >> COL_SHIFT);
        col[48] = ((a1 - b1) >> COL_SHIFT);
        col[56] = ((a0 - b0) >> COL_SHIFT);
}

void ff_psp_idct_put(uint8_t *dest, int line_size, DCTELEM *block)
{
    int i;

    idctRowCondDC(block);
    idctRowCondDC(block + 8);
    idctRowCondDC(block + 16);
    idctRowCondDC(block + 24);
    idctRowCondDC(block + 32);
    idctRowCondDC(block + 40);
    idctRowCondDC(block + 48);
    idctRowCondDC(block + 56);
    
    for(i=0; i<8; i++)
        idctSparseColPut(dest + i, line_size, block + i);
}

void ff_psp_idct_add(uint8_t *dest, int line_size, DCTELEM *block)
{
    int i;

    idctRowCondDC(block);
    idctRowCondDC(block + 8);
    idctRowCondDC(block + 16);
    idctRowCondDC(block + 24);
    idctRowCondDC(block + 32);
    idctRowCondDC(block + 40);
    idctRowCondDC(block + 48);
    idctRowCondDC(block + 56);
   
    for(i=0; i<8; i++)
        idctSparseColAdd(dest + i, line_size, block + i);
}

void ff_psp_idct(DCTELEM *block)
{
    int i;
    for(i=0; i<8; i++)
        idctRowCondDC(block + i*8);
    
    for(i=0; i<8; i++)
        idctSparseCol(block + i);
}

void ff_psp_idct_init()
{
	float *mat_ptr;
	
	mat_ptr = W1W7W5W3;
	loadW1W7W5W3
	mat_ptr = mW5W3mW7mW1;
	loadmW5W3mW7mW1
}
