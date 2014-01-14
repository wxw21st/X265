/*****************************************************************************
 * Copyright (C) 2013 x265 project
 *
 * Authors: Steve Borho <steve@borho.org>
 *          Praveen Kumar Tiwari <praveen@multicorewareinc.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@multicorewareinc.com.
 *****************************************************************************/

#if 0
#include "primitives.h"
#include "x265.h"

extern "C" {
#include "pixel.h"
#include "mc.h"
#include "ipfilter8.h"
#include "blockcopy8.h"
}

#define INIT2_NAME(name1, name2, cpu) \
    p.name1[LUMA_16x16] = x265_pixel_ ## name2 ## _16x16 ## cpu; \
    p.name1[LUMA_16x8]  = x265_pixel_ ## name2 ## _16x8 ## cpu;
#define INIT4_NAME(name1, name2, cpu) \
    INIT2_NAME(name1, name2, cpu) \
    p.name1[LUMA_8x16]  = x265_pixel_ ## name2 ## _8x16 ## cpu; \
    p.name1[LUMA_8x8]   = x265_pixel_ ## name2 ## _8x8 ## cpu;
#define INIT5_NAME(name1, name2, cpu) \
    INIT4_NAME(name1, name2, cpu) \
    p.name1[LUMA_8x4]   = x265_pixel_ ## name2 ## _8x4 ## cpu;
#define INIT6_NAME(name1, name2, cpu) \
    INIT5_NAME(name1, name2, cpu) \
    p.name1[LUMA_4x8]   = x265_pixel_ ## name2 ## _4x8 ## cpu;
#define INIT7_NAME(name1, name2, cpu) \
    INIT6_NAME(name1, name2, cpu) \
    p.name1[LUMA_4x4]   = x265_pixel_ ## name2 ## _4x4 ## cpu;
#define INIT8_NAME(name1, name2, cpu) \
    INIT7_NAME(name1, name2, cpu) \
    p.name1[LUMA_4x16]  = x265_pixel_ ## name2 ## _4x16 ## cpu;
#define INIT2(name, cpu) INIT2_NAME(name, name, cpu)
#define INIT4(name, cpu) INIT4_NAME(name, name, cpu)
#define INIT5(name, cpu) INIT5_NAME(name, name, cpu)
#define INIT6(name, cpu) INIT6_NAME(name, name, cpu)
#define INIT7(name, cpu) INIT7_NAME(name, name, cpu)
#define INIT8(name, cpu) INIT8_NAME(name, name, cpu)

#define HEVC_SATD(cpu) \
    p.satd[LUMA_32x32] = x265_pixel_satd_32x32_ ## cpu; \
    p.satd[LUMA_24x32] = x265_pixel_satd_24x32_ ## cpu; \
    p.satd[LUMA_64x64] = x265_pixel_satd_64x64_ ## cpu; \
    p.satd[LUMA_64x32] = x265_pixel_satd_64x32_ ## cpu; \
    p.satd[LUMA_32x64] = x265_pixel_satd_32x64_ ## cpu; \
    p.satd[LUMA_64x48] = x265_pixel_satd_64x48_ ## cpu; \
    p.satd[LUMA_48x64] = x265_pixel_satd_48x64_ ## cpu; \
    p.satd[LUMA_64x16] = x265_pixel_satd_64x16_ ## cpu

#define ASSGN_SSE(cpu) \
    p.sse_pp[LUMA_8x8]   = x265_pixel_ssd_8x8_ ## cpu; \
    p.sse_pp[LUMA_8x4]   = x265_pixel_ssd_8x4_ ## cpu; \
    p.sse_pp[LUMA_16x16] = x265_pixel_ssd_16x16_ ## cpu; \
    p.sse_pp[LUMA_16x4]  = x265_pixel_ssd_16x4_ ## cpu; \
    p.sse_pp[LUMA_16x8]  = x265_pixel_ssd_16x8_ ## cpu; \
    p.sse_pp[LUMA_8x16]  = x265_pixel_ssd_8x16_ ## cpu; \
    p.sse_pp[LUMA_16x12] = x265_pixel_ssd_16x12_ ## cpu; \
    p.sse_pp[LUMA_32x32] = x265_pixel_ssd_32x32_ ## cpu; \
    p.sse_pp[LUMA_32x16] = x265_pixel_ssd_32x16_ ## cpu; \
    p.sse_pp[LUMA_16x32] = x265_pixel_ssd_16x32_ ## cpu; \
    p.sse_pp[LUMA_8x32]  = x265_pixel_ssd_8x32_ ## cpu; \
    p.sse_pp[LUMA_32x8]  = x265_pixel_ssd_32x8_ ## cpu; \
    p.sse_pp[LUMA_32x24] = x265_pixel_ssd_32x24_ ## cpu; \
    p.sse_pp[LUMA_32x64] = x265_pixel_ssd_32x64_ ## cpu; \
    p.sse_pp[LUMA_16x64] = x265_pixel_ssd_16x64_ ## cpu

#define SA8D_INTER_FROM_BLOCK8(cpu) \
    p.sa8d_inter[LUMA_16x8]  = cmp<16, 8, 8, 8, x265_pixel_sa8d_8x8_ ## cpu>; \
    p.sa8d_inter[LUMA_8x16]  = cmp<8, 16, 8, 8, x265_pixel_sa8d_8x8_ ## cpu>; \
    p.sa8d_inter[LUMA_32x24] = cmp<32, 24, 8, 8, x265_pixel_sa8d_8x8_ ## cpu>; \
    p.sa8d_inter[LUMA_24x32] = cmp<24, 32, 8, 8, x265_pixel_sa8d_8x8_ ## cpu>; \
    p.sa8d_inter[LUMA_32x8]  = cmp<32, 8, 8, 8, x265_pixel_sa8d_8x8_ ## cpu>; \
    p.sa8d_inter[LUMA_8x32]  = cmp < 8, 32, 8, 8, x265_pixel_sa8d_8x8_ ## cpu >
#define SA8D_INTER_FROM_BLOCK(cpu) \
    SA8D_INTER_FROM_BLOCK8(cpu); \
    p.sa8d[BLOCK_32x32] = cmp<32, 32, 16, 16, x265_pixel_sa8d_16x16_ ## cpu>; \
    p.sa8d[BLOCK_64x64] = cmp<64, 64, 16, 16, x265_pixel_sa8d_16x16_ ## cpu>; \
    p.sa8d_inter[LUMA_32x32] = cmp<32, 32, 16, 16, x265_pixel_sa8d_16x16_ ## cpu>; \
    p.sa8d_inter[LUMA_32x16] = cmp<32, 16, 16, 16, x265_pixel_sa8d_16x16_ ## cpu>; \
    p.sa8d_inter[LUMA_16x32] = cmp<16, 32, 16, 16, x265_pixel_sa8d_16x16_ ## cpu>; \
    p.sa8d_inter[LUMA_64x64] = cmp<64, 64, 16, 16, x265_pixel_sa8d_16x16_ ## cpu>; \
    p.sa8d_inter[LUMA_64x32] = cmp<64, 32, 16, 16, x265_pixel_sa8d_16x16_ ## cpu>; \
    p.sa8d_inter[LUMA_32x64] = cmp<32, 64, 16, 16, x265_pixel_sa8d_16x16_ ## cpu>; \
    p.sa8d_inter[LUMA_64x48] = cmp<64, 48, 16, 16, x265_pixel_sa8d_16x16_ ## cpu>; \
    p.sa8d_inter[LUMA_48x64] = cmp<48, 64, 16, 16, x265_pixel_sa8d_16x16_ ## cpu>; \
    p.sa8d_inter[LUMA_64x16] = cmp<64, 16, 16, 16, x265_pixel_sa8d_16x16_ ## cpu>; \
    p.sa8d_inter[LUMA_16x64] = cmp < 16, 64, 16, 16, x265_pixel_sa8d_16x16_ ## cpu >

#define PIXEL_AVG(cpu) \
    p.pixelavg_pp[LUMA_64x64] = x265_pixel_avg_64x64_ ## cpu; \
    p.pixelavg_pp[LUMA_64x48] = x265_pixel_avg_64x48_ ## cpu; \
    p.pixelavg_pp[LUMA_64x16] = x265_pixel_avg_64x16_ ## cpu; \
    p.pixelavg_pp[LUMA_48x64] = x265_pixel_avg_48x64_ ## cpu; \
    p.pixelavg_pp[LUMA_32x64] = x265_pixel_avg_32x64_ ## cpu; \
    p.pixelavg_pp[LUMA_32x32] = x265_pixel_avg_32x32_ ## cpu; \
    p.pixelavg_pp[LUMA_32x24] = x265_pixel_avg_32x24_ ## cpu; \
    p.pixelavg_pp[LUMA_32x16] = x265_pixel_avg_32x16_ ## cpu; \
    p.pixelavg_pp[LUMA_32x8 ] = x265_pixel_avg_32x8_ ## cpu; \
    p.pixelavg_pp[LUMA_24x32] = x265_pixel_avg_24x32_ ## cpu; \
    p.pixelavg_pp[LUMA_16x64] = x265_pixel_avg_16x64_ ## cpu; \
    p.pixelavg_pp[LUMA_16x32] = x265_pixel_avg_16x32_ ## cpu; \
    p.pixelavg_pp[LUMA_16x16] = x265_pixel_avg_16x16_ ## cpu; \
    p.pixelavg_pp[LUMA_16x12]  = x265_pixel_avg_16x12_ ## cpu; \
    p.pixelavg_pp[LUMA_16x8]  = x265_pixel_avg_16x8_ ## cpu; \
    p.pixelavg_pp[LUMA_16x4]  = x265_pixel_avg_16x4_ ## cpu; \
    p.pixelavg_pp[LUMA_12x16] = x265_pixel_avg_12x16_ ## cpu; \
    p.pixelavg_pp[LUMA_8x32]  = x265_pixel_avg_8x32_ ## cpu; \
    p.pixelavg_pp[LUMA_8x16]  = x265_pixel_avg_8x16_ ## cpu; \
    p.pixelavg_pp[LUMA_8x8]   = x265_pixel_avg_8x8_ ## cpu; \
    p.pixelavg_pp[LUMA_8x4]   = x265_pixel_avg_8x4_ ## cpu;

#define PIXEL_AVG_W4(cpu) \
    p.pixelavg_pp[LUMA_4x4]  = x265_pixel_avg_4x4_ ## cpu; \
    p.pixelavg_pp[LUMA_4x8]  = x265_pixel_avg_4x8_ ## cpu; \
    p.pixelavg_pp[LUMA_4x16] = x265_pixel_avg_4x16_ ## cpu;

#define SETUP_CHROMA_FUNC_DEF(W, H, cpu) \
    p.chroma_hpp[CHROMA_ ## W ## x ## H] = x265_interp_4tap_horiz_pp_ ## W ## x ## H ## cpu; \
    p.chroma_vpp[CHROMA_ ## W ## x ## H] = x265_interp_4tap_vert_pp_ ## W ## x ## H ## cpu; \
    p.chroma_vps[CHROMA_ ## W ## x ## H] = x265_interp_4tap_vert_ps_ ## W ## x ## H ## cpu; \
    p.chroma_copy_ps[CHROMA_ ## W ## x ## H] = x265_blockcopy_ps_ ## W ## x ## H ## cpu; \
    p.chroma_sub_ps[CHROMA_ ## W ## x ## H] = x265_pixel_sub_ps_ ## W ## x ## H ## cpu;

#define SETUP_CHROMA_SP_FUNC_DEF(W, H, cpu) \
    p.chroma_vsp[CHROMA_ ## W ## x ## H] = x265_interp_4tap_vert_sp_ ## W ## x ## H ## cpu;

#define SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(W, H, cpu) \
    p.chroma_copy_pp[CHROMA_ ## W ## x ## H] = x265_blockcopy_pp_ ## W ## x ## H ## cpu;

#define CHROMA_FILTERS(cpu) \
    SETUP_CHROMA_FUNC_DEF(4, 4, cpu); \
    SETUP_CHROMA_FUNC_DEF(4, 2, cpu); \
    SETUP_CHROMA_FUNC_DEF(2, 4, cpu); \
    SETUP_CHROMA_FUNC_DEF(8, 8, cpu); \
    SETUP_CHROMA_FUNC_DEF(8, 4, cpu); \
    SETUP_CHROMA_FUNC_DEF(4, 8, cpu); \
    SETUP_CHROMA_FUNC_DEF(8, 6, cpu); \
    SETUP_CHROMA_FUNC_DEF(6, 8, cpu); \
    SETUP_CHROMA_FUNC_DEF(8, 2, cpu); \
    SETUP_CHROMA_FUNC_DEF(2, 8, cpu); \
    SETUP_CHROMA_FUNC_DEF(16, 16, cpu); \
    SETUP_CHROMA_FUNC_DEF(16, 8, cpu); \
    SETUP_CHROMA_FUNC_DEF(8, 16, cpu); \
    SETUP_CHROMA_FUNC_DEF(16, 12, cpu); \
    SETUP_CHROMA_FUNC_DEF(12, 16, cpu); \
    SETUP_CHROMA_FUNC_DEF(16, 4, cpu); \
    SETUP_CHROMA_FUNC_DEF(4, 16, cpu); \
    SETUP_CHROMA_FUNC_DEF(32, 32, cpu); \
    SETUP_CHROMA_FUNC_DEF(32, 16, cpu); \
    SETUP_CHROMA_FUNC_DEF(16, 32, cpu); \
    SETUP_CHROMA_FUNC_DEF(32, 24, cpu); \
    SETUP_CHROMA_FUNC_DEF(24, 32, cpu); \
    SETUP_CHROMA_FUNC_DEF(32, 8, cpu); \
    SETUP_CHROMA_FUNC_DEF(8, 32, cpu);

#define CHROMA_SP_FILTERS(cpu) \
    SETUP_CHROMA_SP_FUNC_DEF(4, 4, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(4, 2, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(8, 8, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(8, 4, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(4, 8, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(8, 6, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(8, 2, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(16, 16, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(16, 8, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(8, 16, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(16, 12, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(12, 16, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(16, 4, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(4, 16, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(32, 32, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(32, 16, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(16, 32, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(32, 24, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(24, 32, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(32, 8, cpu); \
    SETUP_CHROMA_SP_FUNC_DEF(8, 32, cpu);

#define CHROMA_BLOCKCOPY(cpu) \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(4, 4, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(4, 2, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(2, 4, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(8, 8, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(8, 4, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(4, 8, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(8, 6, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(6, 8, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(8, 2, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(2, 8, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(16, 16, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(16, 8, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(8, 16, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(16, 12, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(12, 16, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(16, 4, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(4, 16, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(32, 32, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(32, 16, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(16, 32, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(32, 24, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(24, 32, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(32, 8, cpu); \
    SETUP_CHROMA_BLOCKCOPY_FUNC_DEF(8, 32, cpu);

#define SETUP_LUMA_FUNC_DEF(W, H, cpu) \
    p.luma_hpp[LUMA_ ## W ## x ## H] = x265_interp_8tap_horiz_pp_ ## W ## x ## H ## cpu; \
    p.luma_hps[LUMA_ ## W ## x ## H] = x265_interp_8tap_horiz_ps_ ## W ## x ## H ## cpu; \
    p.luma_vpp[LUMA_ ## W ## x ## H] = x265_interp_8tap_vert_pp_ ## W ## x ## H ## cpu; \
    p.luma_vps[LUMA_ ## W ## x ## H] = x265_interp_8tap_vert_ps_ ## W ## x ## H ## cpu; \
    p.luma_copy_ps[LUMA_ ## W ## x ## H] = x265_blockcopy_ps_ ## W ## x ## H ## cpu; \
    p.luma_sub_ps[LUMA_ ## W ## x ## H] = x265_pixel_sub_ps_ ## W ## x ## H ## cpu;

#define SETUP_LUMA_SP_FUNC_DEF(W, H, cpu) \
    p.luma_vsp[LUMA_ ## W ## x ## H] = x265_interp_8tap_vert_sp_ ## W ## x ## H ## cpu;

#define SETUP_LUMA_BLOCKCOPY_FUNC_DEF(W, H, cpu) \
    p.luma_copy_pp[LUMA_ ## W ## x ## H] = x265_blockcopy_pp_ ## W ## x ## H ## cpu;

#define LUMA_FILTERS(cpu) \
    SETUP_LUMA_FUNC_DEF(4,   4, cpu); \
    SETUP_LUMA_FUNC_DEF(8,   8, cpu); \
    SETUP_LUMA_FUNC_DEF(8,   4, cpu); \
    SETUP_LUMA_FUNC_DEF(4,   8, cpu); \
    SETUP_LUMA_FUNC_DEF(16, 16, cpu); \
    SETUP_LUMA_FUNC_DEF(16,  8, cpu); \
    SETUP_LUMA_FUNC_DEF(8,  16, cpu); \
    SETUP_LUMA_FUNC_DEF(16, 12, cpu); \
    SETUP_LUMA_FUNC_DEF(12, 16, cpu); \
    SETUP_LUMA_FUNC_DEF(16,  4, cpu); \
    SETUP_LUMA_FUNC_DEF(4,  16, cpu); \
    SETUP_LUMA_FUNC_DEF(32, 32, cpu); \
    SETUP_LUMA_FUNC_DEF(32, 16, cpu); \
    SETUP_LUMA_FUNC_DEF(16, 32, cpu); \
    SETUP_LUMA_FUNC_DEF(32, 24, cpu); \
    SETUP_LUMA_FUNC_DEF(24, 32, cpu); \
    SETUP_LUMA_FUNC_DEF(32,  8, cpu); \
    SETUP_LUMA_FUNC_DEF(8,  32, cpu); \
    SETUP_LUMA_FUNC_DEF(64, 64, cpu); \
    SETUP_LUMA_FUNC_DEF(64, 32, cpu); \
    SETUP_LUMA_FUNC_DEF(32, 64, cpu); \
    SETUP_LUMA_FUNC_DEF(64, 48, cpu); \
    SETUP_LUMA_FUNC_DEF(48, 64, cpu); \
    SETUP_LUMA_FUNC_DEF(64, 16, cpu); \
    SETUP_LUMA_FUNC_DEF(16, 64, cpu);

#define LUMA_SP_FILTERS(cpu) \
    SETUP_LUMA_SP_FUNC_DEF(4,   4, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(8,   8, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(8,   4, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(4,   8, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(16, 16, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(16,  8, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(8,  16, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(16, 12, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(12, 16, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(16,  4, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(4,  16, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(32, 32, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(32, 16, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(16, 32, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(32, 24, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(24, 32, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(32,  8, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(8,  32, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(64, 64, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(64, 32, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(32, 64, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(64, 48, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(48, 64, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(64, 16, cpu); \
    SETUP_LUMA_SP_FUNC_DEF(16, 64, cpu);

#define LUMA_BLOCKCOPY(cpu) \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(4,   4, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(8,   8, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(8,   4, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(4,   8, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(16, 16, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(16,  8, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(8,  16, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(16, 12, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(12, 16, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(16,  4, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(4,  16, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(32, 32, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(32, 16, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(16, 32, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(32, 24, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(24, 32, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(32,  8, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(8,  32, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(64, 64, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(64, 32, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(32, 64, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(64, 48, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(48, 64, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(64, 16, cpu); \
    SETUP_LUMA_BLOCKCOPY_FUNC_DEF(16, 64, cpu);

using namespace x265;

namespace {
// file private anonymous namespace

/* template for building arbitrary partition sizes from full optimized primitives */
template<int lx, int ly, int dx, int dy, pixelcmp_t compare>
int cmp(pixel * piOrg, intptr_t strideOrg, pixel * piCur, intptr_t strideCur)
{
    int sum = 0;

    for (int row = 0; row < ly; row += dy)
    {
        for (int col = 0; col < lx; col += dx)
        {
            sum += compare(piOrg + row * strideOrg + col, strideOrg,
                           piCur + row * strideCur + col, strideCur);
        }
    }

    return sum;
}
}

namespace x265 {
// private x265 namespace

void Setup_Assembly_Primitives(EncoderPrimitives &p, int cpuMask)
{
#if HIGH_BIT_DEPTH
    if (cpuMask & X265_CPU_SSE2) p.sa8d[0] = p.sa8d[0];
#else
    if (cpuMask & X265_CPU_SSE2)
    {
        INIT8_NAME(sse_pp, ssd, _mmx);
        INIT8(sad, _mmx2);
        INIT8(sad_x3, _mmx2);
        INIT8(sad_x4, _mmx2);
        INIT8(satd, _mmx2);
        p.satd[LUMA_8x32] = x265_pixel_satd_8x32_sse2;
        p.satd[LUMA_12x16] = x265_pixel_satd_12x16_sse2;
        p.satd[LUMA_16x4] = x265_pixel_satd_16x4_sse2;
        p.satd[LUMA_16x12] = x265_pixel_satd_16x12_sse2;
        p.satd[LUMA_16x32] = x265_pixel_satd_16x32_sse2;
        p.satd[LUMA_16x64] = x265_pixel_satd_16x64_sse2;
        p.satd[LUMA_32x8]  = x265_pixel_satd_32x8_sse2;
        p.satd[LUMA_32x16] = x265_pixel_satd_32x16_sse2;
        p.satd[LUMA_32x24] = x265_pixel_satd_32x24_sse2;
        p.sa8d[BLOCK_4x4]  = x265_pixel_satd_4x4_mmx2;
        p.frame_init_lowres_core = x265_frame_init_lowres_core_mmx2;

        PIXEL_AVG(sse2);
        PIXEL_AVG_W4(mmx2);

        p.sad[LUMA_8x32]  = x265_pixel_sad_8x32_sse2;
        p.sad[LUMA_16x4]  = x265_pixel_sad_16x4_sse2;
        p.sad[LUMA_16x12] = x265_pixel_sad_16x12_sse2;
        p.sad[LUMA_16x32] = x265_pixel_sad_16x32_sse2;
        p.sad[LUMA_16x64] = x265_pixel_sad_16x64_sse2;

        p.sad[LUMA_32x8]  = x265_pixel_sad_32x8_sse2;
        p.sad[LUMA_32x16] = x265_pixel_sad_32x16_sse2;
        p.sad[LUMA_32x24] = x265_pixel_sad_32x24_sse2;
        p.sad[LUMA_32x32] = x265_pixel_sad_32x32_sse2;
        p.sad[LUMA_32x64] = x265_pixel_sad_32x64_sse2;

        p.sad[LUMA_64x16] = x265_pixel_sad_64x16_sse2;
        p.sad[LUMA_64x32] = x265_pixel_sad_64x32_sse2;
        p.sad[LUMA_64x48] = x265_pixel_sad_64x48_sse2;
        p.sad[LUMA_64x64] = x265_pixel_sad_64x64_sse2;

        p.sad[LUMA_48x64] = x265_pixel_sad_48x64_sse2;
        p.sad[LUMA_24x32] = x265_pixel_sad_24x32_sse2;
        p.sad[LUMA_12x16] = x265_pixel_sad_12x16_sse2;

        ASSGN_SSE(sse2);
        INIT2(sad, _sse2);
        INIT2(sad_x3, _sse2);
        INIT2(sad_x4, _sse2);
        INIT6(satd, _sse2);
        HEVC_SATD(sse2);

        CHROMA_BLOCKCOPY(_sse2);
        LUMA_BLOCKCOPY(_sse2);

        // This function pointer initialization is temporary will be removed
        // later with macro definitions.  It is used to avoid linker errors
        // until all partitions are coded and commit smaller patches, easier to
        // review.

        p.chroma_copy_sp[CHROMA_4x2] = x265_blockcopy_sp_4x2_sse2;
        p.chroma_copy_sp[CHROMA_4x4] = x265_blockcopy_sp_4x4_sse2;
        p.chroma_copy_sp[CHROMA_4x8] = x265_blockcopy_sp_4x8_sse2;
        p.chroma_copy_sp[CHROMA_4x16] = x265_blockcopy_sp_4x16_sse2;
        p.chroma_copy_sp[CHROMA_8x2] = x265_blockcopy_sp_8x2_sse2;
        p.chroma_copy_sp[CHROMA_8x4] = x265_blockcopy_sp_8x4_sse2;
        p.chroma_copy_sp[CHROMA_8x6] = x265_blockcopy_sp_8x6_sse2;
        p.chroma_copy_sp[CHROMA_8x8] = x265_blockcopy_sp_8x8_sse2;
        p.chroma_copy_sp[CHROMA_8x16] = x265_blockcopy_sp_8x16_sse2;
        p.chroma_copy_sp[CHROMA_12x16] = x265_blockcopy_sp_12x16_sse2;
        p.chroma_copy_sp[CHROMA_16x4] = x265_blockcopy_sp_16x4_sse2;
        p.chroma_copy_sp[CHROMA_16x8] = x265_blockcopy_sp_16x8_sse2;
        p.chroma_copy_sp[CHROMA_16x12] = x265_blockcopy_sp_16x12_sse2;
        p.chroma_copy_sp[CHROMA_16x16] = x265_blockcopy_sp_16x16_sse2;
        p.chroma_copy_sp[CHROMA_16x32] = x265_blockcopy_sp_16x32_sse2;
        p.luma_copy_sp[LUMA_16x64] = x265_blockcopy_sp_16x64_sse2;
        p.chroma_copy_sp[CHROMA_24x32] = x265_blockcopy_sp_24x32_sse2;
        p.chroma_copy_sp[CHROMA_32x8] = x265_blockcopy_sp_32x8_sse2;
        p.chroma_copy_sp[CHROMA_32x16] = x265_blockcopy_sp_32x16_sse2;
        p.chroma_copy_sp[CHROMA_32x24] = x265_blockcopy_sp_32x24_sse2;
        p.chroma_copy_sp[CHROMA_32x32] = x265_blockcopy_sp_32x32_sse2;
        p.luma_copy_sp[LUMA_32x64] = x265_blockcopy_sp_32x64_sse2;
        p.luma_copy_sp[LUMA_48x64] = x265_blockcopy_sp_48x64_sse2;
        p.luma_copy_sp[LUMA_64x16] = x265_blockcopy_sp_64x16_sse2;
        p.luma_copy_sp[LUMA_64x32] = x265_blockcopy_sp_64x32_sse2;
        p.luma_copy_sp[LUMA_64x48] = x265_blockcopy_sp_64x48_sse2;
        p.luma_copy_sp[LUMA_64x64] = x265_blockcopy_sp_64x64_sse2;

        p.blockfill_s[BLOCK_4x4] = x265_blockfill_s_4x4_sse2;
        p.blockfill_s[BLOCK_8x8] = x265_blockfill_s_8x8_sse2;
        p.blockfill_s[BLOCK_16x16] = x265_blockfill_s_16x16_sse2;
        p.blockfill_s[BLOCK_32x32] = x265_blockfill_s_32x32_sse2;

        p.frame_init_lowres_core = x265_frame_init_lowres_core_sse2;
        p.sa8d[BLOCK_8x8]   = x265_pixel_sa8d_8x8_sse2;
        p.sa8d[BLOCK_16x16] = x265_pixel_sa8d_16x16_sse2;
        SA8D_INTER_FROM_BLOCK(sse2);

        p.cvt32to16_shr = x265_cvt32to16_shr_sse2;
        p.ipfilter_ss[FILTER_V_S_S_8] = x265_interp_8tap_v_ss_sse2;
        p.calcrecon[BLOCK_4x4] = x265_calcRecons4_sse2;
        p.calcrecon[BLOCK_8x8] = x265_calcRecons8_sse2;
    }
    if (cpuMask & X265_CPU_SSSE3)
    {
        p.frame_init_lowres_core = x265_frame_init_lowres_core_ssse3;
        p.sa8d[BLOCK_8x8]   = x265_pixel_sa8d_8x8_ssse3;
        p.sa8d[BLOCK_16x16] = x265_pixel_sa8d_16x16_ssse3;
        SA8D_INTER_FROM_BLOCK(ssse3);
        p.sse_pp[LUMA_4x4] = x265_pixel_ssd_4x4_ssse3;
        ASSGN_SSE(ssse3);
        PIXEL_AVG(ssse3);
        PIXEL_AVG_W4(ssse3);

        p.scale1D_128to64 = x265_scale1D_128to64_ssse3;

        p.sad_x4[LUMA_8x4] = x265_pixel_sad_x4_8x4_ssse3;
        p.sad_x4[LUMA_8x8] = x265_pixel_sad_x4_8x8_ssse3;
        p.sad_x3[LUMA_8x16] = x265_pixel_sad_x3_8x16_ssse3;
        p.sad_x4[LUMA_8x16] = x265_pixel_sad_x4_8x16_ssse3;
        p.sad_x3[LUMA_8x32]  = x265_pixel_sad_x3_8x32_ssse3;
        p.sad_x4[LUMA_8x32]  = x265_pixel_sad_x4_8x32_ssse3;

        p.sad_x3[LUMA_12x16] = x265_pixel_sad_x3_12x16_ssse3;
        p.sad_x4[LUMA_12x16] = x265_pixel_sad_x4_12x16_ssse3;
        p.sad_x3[LUMA_16x12] = x265_pixel_sad_x3_16x12_ssse3;
        p.sad_x4[LUMA_16x12] = x265_pixel_sad_x4_16x12_ssse3;
        p.sad_x3[LUMA_16x32] = x265_pixel_sad_x3_16x32_ssse3;
        p.sad_x4[LUMA_16x32] = x265_pixel_sad_x4_16x32_ssse3;
        p.sad_x3[LUMA_16x64] = x265_pixel_sad_x3_16x64_ssse3;
        p.sad_x4[LUMA_16x64] = x265_pixel_sad_x4_16x64_ssse3;
        p.sad_x3[LUMA_24x32] = x265_pixel_sad_x3_24x32_ssse3;
        p.sad_x4[LUMA_24x32] = x265_pixel_sad_x4_24x32_ssse3;
        p.sad_x3[LUMA_32x8] = x265_pixel_sad_x3_32x8_ssse3;
        p.sad_x3[LUMA_32x16] = x265_pixel_sad_x3_32x16_ssse3;
        p.sad_x3[LUMA_32x24] = x265_pixel_sad_x3_32x24_ssse3;
        p.sad_x3[LUMA_32x32] = x265_pixel_sad_x3_32x32_ssse3;
        p.sad_x3[LUMA_32x64] = x265_pixel_sad_x3_32x64_ssse3;
        p.sad_x4[LUMA_32x8] = x265_pixel_sad_x4_32x8_ssse3;
        p.sad_x4[LUMA_32x16] = x265_pixel_sad_x4_32x16_ssse3;
        p.sad_x4[LUMA_32x24] = x265_pixel_sad_x4_32x24_ssse3;
        p.sad_x4[LUMA_32x32] = x265_pixel_sad_x4_32x32_ssse3;
        p.sad_x4[LUMA_32x64] = x265_pixel_sad_x4_32x64_ssse3;
        p.sad_x3[LUMA_48x64] = x265_pixel_sad_x3_48x64_ssse3;
        p.sad_x4[LUMA_48x64] = x265_pixel_sad_x4_48x64_ssse3;
        p.sad_x3[LUMA_64x16] = x265_pixel_sad_x3_64x16_ssse3;
        p.sad_x3[LUMA_64x32] = x265_pixel_sad_x3_64x32_ssse3;
        p.sad_x3[LUMA_64x48] = x265_pixel_sad_x3_64x48_ssse3;
        p.sad_x3[LUMA_64x64] = x265_pixel_sad_x3_64x64_ssse3;
        p.sad_x4[LUMA_64x16] = x265_pixel_sad_x4_64x16_ssse3;
        p.sad_x4[LUMA_64x32] = x265_pixel_sad_x4_64x32_ssse3;
        p.sad_x4[LUMA_64x48] = x265_pixel_sad_x4_64x48_ssse3;
        p.sad_x4[LUMA_64x64] = x265_pixel_sad_x4_64x64_ssse3;

        p.luma_hvpp[LUMA_8x8] = x265_interp_8tap_hv_pp_8x8_ssse3;
        p.luma_p2s = x265_luma_p2s_ssse3;
        p.chroma_p2s = x265_chroma_p2s_ssse3;
        
        CHROMA_SP_FILTERS(_ssse3);
        LUMA_SP_FILTERS(_ssse3);

    }
    if (cpuMask & X265_CPU_SSE4)
    {
        p.satd[LUMA_4x16]   = x265_pixel_satd_4x16_sse4;
        p.satd[LUMA_12x16]  = x265_pixel_satd_12x16_sse4;
        p.satd[LUMA_32x8] = x265_pixel_satd_32x8_sse4;
        p.satd[LUMA_32x16] = x265_pixel_satd_32x16_sse4;
        p.satd[LUMA_32x24] = x265_pixel_satd_32x24_sse4;
        p.sa8d[BLOCK_8x8]   = x265_pixel_sa8d_8x8_sse4;
        p.sa8d[BLOCK_16x16] = x265_pixel_sa8d_16x16_sse4;
        SA8D_INTER_FROM_BLOCK(sse4);

        CHROMA_FILTERS(_sse4);
        LUMA_FILTERS(_sse4);
        HEVC_SATD(sse4);
        p.chroma_copy_sp[CHROMA_2x4] = x265_blockcopy_sp_2x4_sse4;
        p.chroma_copy_sp[CHROMA_2x8] = x265_blockcopy_sp_2x8_sse4;
        p.chroma_copy_sp[CHROMA_6x8] = x265_blockcopy_sp_6x8_sse4;

        p.chroma_vsp[CHROMA_2x4] = x265_interp_4tap_vert_sp_2x4_sse4;
        p.chroma_vsp[CHROMA_2x8] = x265_interp_4tap_vert_sp_2x8_sse4;
        p.chroma_vsp[CHROMA_6x8] = x265_interp_4tap_vert_sp_6x8_sse4;

        p.calcrecon[BLOCK_16x16] = x265_calcRecons16_sse4;
        p.calcrecon[BLOCK_32x32] = x265_calcRecons32_sse4;
        p.calcrecon[BLOCK_64x64] = x265_calcRecons64_sse4;
    }
    if (cpuMask & X265_CPU_AVX)
    {
        p.frame_init_lowres_core = x265_frame_init_lowres_core_avx;
        p.satd[LUMA_4x16]   = x265_pixel_satd_4x16_avx;
        p.satd[LUMA_12x16]  = x265_pixel_satd_12x16_avx;
        p.satd[LUMA_32x8] = x265_pixel_satd_32x8_avx;
        p.satd[LUMA_32x16] = x265_pixel_satd_32x16_avx;
        p.satd[LUMA_32x24] = x265_pixel_satd_32x24_avx;
        p.sa8d[BLOCK_8x8]   = x265_pixel_sa8d_8x8_avx;
        p.sa8d[BLOCK_16x16] = x265_pixel_sa8d_16x16_avx;
        SA8D_INTER_FROM_BLOCK(avx);
        ASSGN_SSE(avx);
        HEVC_SATD(avx);

        p.sad_x3[LUMA_12x16] = x265_pixel_sad_x3_12x16_avx;
        p.sad_x4[LUMA_12x16] = x265_pixel_sad_x4_12x16_avx;
        p.sad_x3[LUMA_16x4]  = x265_pixel_sad_x3_16x4_avx;
        p.sad_x4[LUMA_16x4]  = x265_pixel_sad_x4_16x4_avx;
        p.sad_x3[LUMA_16x12] = x265_pixel_sad_x3_16x12_avx;
        p.sad_x4[LUMA_16x12] = x265_pixel_sad_x4_16x12_avx;
        p.sad_x3[LUMA_16x32] = x265_pixel_sad_x3_16x32_avx;
        p.sad_x4[LUMA_16x32] = x265_pixel_sad_x4_16x32_avx;
        p.sad_x3[LUMA_16x64] = x265_pixel_sad_x3_16x64_avx;
        p.sad_x4[LUMA_16x64] = x265_pixel_sad_x4_16x64_avx;
        p.sad_x3[LUMA_24x32] = x265_pixel_sad_x3_24x32_avx;
        p.sad_x4[LUMA_24x32] = x265_pixel_sad_x4_24x32_avx;

        p.sad_x3[LUMA_32x8]  = x265_pixel_sad_x3_32x8_avx;
        p.sad_x3[LUMA_32x16] = x265_pixel_sad_x3_32x16_avx;
        p.sad_x3[LUMA_32x24] = x265_pixel_sad_x3_32x24_avx;
        p.sad_x3[LUMA_32x32] = x265_pixel_sad_x3_32x32_avx;
        p.sad_x3[LUMA_32x64] = x265_pixel_sad_x3_32x64_avx;
        p.sad_x4[LUMA_32x8]  = x265_pixel_sad_x4_32x8_avx;
        p.sad_x4[LUMA_32x16] = x265_pixel_sad_x4_32x16_avx;
        p.sad_x4[LUMA_32x24] = x265_pixel_sad_x4_32x24_avx;
        p.sad_x4[LUMA_32x32] = x265_pixel_sad_x4_32x32_avx;
        p.sad_x4[LUMA_32x64] = x265_pixel_sad_x4_32x64_avx;
        p.sad_x3[LUMA_48x64] = x265_pixel_sad_x3_48x64_avx;
        p.sad_x4[LUMA_48x64] = x265_pixel_sad_x4_48x64_avx;
        p.sad_x3[LUMA_64x16] = x265_pixel_sad_x3_64x16_avx;
        p.sad_x3[LUMA_64x32] = x265_pixel_sad_x3_64x32_avx;
        p.sad_x3[LUMA_64x48] = x265_pixel_sad_x3_64x48_avx;
        p.sad_x3[LUMA_64x64] = x265_pixel_sad_x3_64x64_avx;
        p.sad_x4[LUMA_64x16] = x265_pixel_sad_x4_64x16_avx;
        p.sad_x4[LUMA_64x32] = x265_pixel_sad_x4_64x32_avx;
        p.sad_x4[LUMA_64x48] = x265_pixel_sad_x4_64x48_avx;
        p.sad_x4[LUMA_64x64] = x265_pixel_sad_x4_64x64_avx;
    }
    if (cpuMask & X265_CPU_XOP)
    {
        p.frame_init_lowres_core = x265_frame_init_lowres_core_xop;
        p.sa8d[BLOCK_8x8]   = x265_pixel_sa8d_8x8_xop;
        p.sa8d[BLOCK_16x16] = x265_pixel_sa8d_16x16_xop;
        SA8D_INTER_FROM_BLOCK(xop);
        INIT7(satd, _xop);
        INIT5_NAME(sse_pp, ssd, _xop);
        HEVC_SATD(xop);
    }
    if (cpuMask & X265_CPU_AVX2)
    {
        INIT2(sad_x4, _avx2);
        INIT4(satd, _avx2);
        INIT2_NAME(sse_pp, ssd, _avx2);
        p.sa8d[BLOCK_8x8] = x265_pixel_sa8d_8x8_avx2;
        SA8D_INTER_FROM_BLOCK8(avx2);
        p.satd[LUMA_32x32] = cmp<32, 32, 16, 16, x265_pixel_satd_16x16_avx2>;
        p.satd[LUMA_24x32] = cmp<24, 32, 8, 16, x265_pixel_satd_8x16_avx2>;
        p.satd[LUMA_64x64] = cmp<64, 64, 16, 16, x265_pixel_satd_16x16_avx2>;
        p.satd[LUMA_64x32] = cmp<64, 32, 16, 16, x265_pixel_satd_16x16_avx2>;
        p.satd[LUMA_32x64] = cmp<32, 64, 16, 16, x265_pixel_satd_16x16_avx2>;
        p.satd[LUMA_64x48] = cmp<64, 48, 16, 16, x265_pixel_satd_16x16_avx2>;
        p.satd[LUMA_48x64] = cmp<48, 64, 16, 16, x265_pixel_satd_16x16_avx2>;
        p.satd[LUMA_64x16] = cmp<64, 16, 16, 16, x265_pixel_satd_16x16_avx2>;

        p.sad_x4[LUMA_16x12] = x265_pixel_sad_x4_16x12_avx2;
        p.sad_x4[LUMA_16x32] = x265_pixel_sad_x4_16x32_avx2;
    }
#endif // if HIGH_BIT_DEPTH
}
}

extern "C" {
#ifdef __INTEL_COMPILER

/* Agner's patch to Intel's CPU dispatcher from pages 131-132 of
 * http://agner.org/optimize/optimizing_cpp.pdf (2011-01-30)
 * adapted to x265's cpu schema. */

// Global variable indicating cpu
int __intel_cpu_indicator = 0;
// CPU dispatcher function
void x265_intel_cpu_indicator_init(void)
{
    unsigned int cpu = cpu_detect();

    if (cpu & X265_CPU_AVX)
        __intel_cpu_indicator = 0x20000;
    else if (cpu & X265_CPU_SSE42)
        __intel_cpu_indicator = 0x8000;
    else if (cpu & X265_CPU_SSE4)
        __intel_cpu_indicator = 0x2000;
    else if (cpu & X265_CPU_SSSE3)
        __intel_cpu_indicator = 0x1000;
    else if (cpu & X265_CPU_SSE3)
        __intel_cpu_indicator = 0x800;
    else if (cpu & X265_CPU_SSE2 && !(cpu & X265_CPU_SSE2_IS_SLOW))
        __intel_cpu_indicator = 0x200;
    else if (cpu & X265_CPU_SSE)
        __intel_cpu_indicator = 0x80;
    else if (cpu & X265_CPU_MMX2)
        __intel_cpu_indicator = 8;
    else
        __intel_cpu_indicator = 1;
}

/* __intel_cpu_indicator_init appears to have a non-standard calling convention that
 * assumes certain registers aren't preserved, so we'll route it through a function
 * that backs up all the registers. */
void __intel_cpu_indicator_init(void)
{
    x265_safe_intel_cpu_indicator_init();
}

#else // ifdef __INTEL_COMPILER
void x265_intel_cpu_indicator_init(void) {}

#endif // ifdef __INTEL_COMPILER
}

#endif
