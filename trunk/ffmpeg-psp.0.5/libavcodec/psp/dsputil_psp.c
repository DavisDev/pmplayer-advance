/*
 * PSP optimized DSP utils
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

#include "libavcodec/dsputil.h"
//#include "mmi.h"
//
void ff_psp_idct_init();
void ff_psp_idct_put(uint8_t *dest, int line_size, DCTELEM *block);
void ff_psp_idct_add(uint8_t *dest, int line_size, DCTELEM *block);
void ff_psp_idct(DCTELEM *block);

static void clear_blocks_psp(DCTELEM * blocks)
{
	__asm__ volatile (
	"		.set	push"				"\n"
	"		.set noreorder"				"\n"
	"		vmzero.q M700\n"
	"		addi $9, $0, 12"			"\n"
	"1:		addiu $9, $9, -1"			"\n"
	"		sv.q C700, 0(%0)\n"
	"		sv.q C710, 16(%0)\n"
	"		sv.q C720, 32(%0)\n"
	"		sv.q C730, 48(%0)\n"
	"		bne $9, $0, 1b"				"\n"
	"		addiu %0, %0, 64"			"\n"
	"		.set	pop"				"\n"
	: "+r"(blocks) :: "$9", "memory");
}


//static void get_pixels_mmi(DCTELEM *block, const uint8_t *pixels, int line_size)
//{
//        __asm__ volatile(
//        ".set   push            \n\t"
//        ".set   mips3           \n\t"
//        "ld     $8, 0(%0)       \n\t"
//        "add    %0, %0, %2      \n\t"
//        "ld     $9, 0(%0)       \n\t"
//        "add    %0, %0, %2      \n\t"
//        "ld     $10, 0(%0)      \n\t"
//        "pextlb $8, $0, $8      \n\t"
//        "sq     $8, 0(%1)       \n\t"
//        "add    %0, %0, %2      \n\t"
//        "ld     $8, 0(%0)       \n\t"
//        "pextlb $9, $0, $9      \n\t"
//        "sq     $9, 16(%1)      \n\t"
//        "add    %0, %0, %2      \n\t"
//        "ld     $9, 0(%0)       \n\t"
//        "pextlb $10, $0, $10    \n\t"
//        "sq     $10, 32(%1)     \n\t"
//        "add    %0, %0, %2      \n\t"
//        "ld     $10, 0(%0)      \n\t"
//        "pextlb $8, $0, $8      \n\t"
//        "sq     $8, 48(%1)      \n\t"
//        "add    %0, %0, %2      \n\t"
//        "ld     $8, 0(%0)       \n\t"
//        "pextlb $9, $0, $9      \n\t"
//        "sq     $9, 64(%1)      \n\t"
//        "add    %0, %0, %2      \n\t"
//        "ld     $9, 0(%0)       \n\t"
//        "pextlb $10, $0, $10    \n\t"
//        "sq     $10, 80(%1)     \n\t"
//        "pextlb $8, $0, $8      \n\t"
//        "sq     $8, 96(%1)      \n\t"
//        "pextlb $9, $0, $9      \n\t"
//        "sq     $9, 112(%1)     \n\t"
//        ".set   pop             \n\t"
//        : "+r" (pixels) : "r" (block), "r" (line_size) : "$8", "$9", "$10", "memory" );
//}


static void put_pixels8_psp(uint8_t *block, const uint8_t *pixels, int line_size, int h)
{
	__asm__ volatile(
	"		.set	push"				"\n"
	"		.set noreorder"				"\n"
	"1:		lwl $9, 3(%1)"				"\n"
	"		addiu %2, %2, -1"			"\n"
	"		lwr $9, 0(%1)"				"\n"
	"		sw $9, 0(%0)"				"\n"
	"		lwl $10, 7(%1)"				"\n"
	"		lwr $10, 4(%1)"				"\n"
	"		addu %1, %1, %3"			"\n"
	"		sw $10, 4(%0)"				"\n"
	"		bne %2, $0, 1b"				"\n"
	"		addu %0, %0, %3"			"\n"
	"		.set	pop"				"\n"
	: "+r" (block), "+r" (pixels), "+r" (h) : "r" (line_size) : "$9", "$10", "memory" );
}


static void put_pixels16_psp(uint8_t *block, const uint8_t *pixels, int line_size, int h)
{
	__asm__ volatile(
	"		.set	push"				"\n"
	"		.set noreorder"				"\n"
	"1:		lwl $9, 3(%1)"				"\n"
	"		addiu %2, %2, -1"			"\n"
	"		lwr $9, 0(%1)"				"\n"
	"		sw $9, 0(%0)"				"\n"
	"		lwl $10, 7(%1)"				"\n"
	"		lwr $10, 4(%1)"				"\n"
	"		sw $10, 4(%0)"				"\n"
	"		lwl $9, 11(%1)"				"\n"
	"		lwr $9, 8(%1)"				"\n"
	"		sw $9, 8(%0)"				"\n"
	"		lwl $10, 15(%1)"			"\n"
	"		lwr $10, 12(%1)"			"\n"
	"		addu %1, %1, %3"			"\n"
	"		sw $10, 12(%0)"				"\n"
	"		bne %2, $0, 1b"				"\n"
	"		addu %0, %0, %3"			"\n"
	"		.set	pop"				"\n"
	: "+r" (block), "+r" (pixels), "+r" (h) : "r" (line_size) : "$9", "$10", "$11", "$12", "$13", "$14", "memory" );
}


void dsputil_init_psp(DSPContext* c, AVCodecContext *avctx)
{
    const int idct_algo= avctx->idct_algo;

    c->clear_blocks = clear_blocks_psp;

    c->put_pixels_tab[1][0] = put_pixels8_psp;
    c->put_no_rnd_pixels_tab[1][0] = put_pixels8_psp;

    c->put_pixels_tab[0][0] = put_pixels16_psp;
    c->put_no_rnd_pixels_tab[0][0] = put_pixels16_psp;

//    c->get_pixels = get_pixels_mmi;
//
//    if(idct_algo==FF_IDCT_AUTO){
//    	ff_psp_idct_init();
//        c->idct_put= ff_psp_idct_put;
//        c->idct_add= ff_psp_idct_add;
//        c->idct    = ff_psp_idct;
//        c->idct_permutation_type= FF_NO_IDCT_PERM;
//    }
}

