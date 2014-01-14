/*****************************************************************************
 * Copyright (C) 2013 x265 project
 *
 * Authors: Steve Borho <steve@borho.org>
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

#ifndef X265_IPFILTER8_H
#define X265_IPFILTER8_H

#define SETUP_CHROMA_FUNC_DEF(W, H, cpu) \
    void x265_interp_4tap_horiz_pp_ ## W ## x ## H ## cpu(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx); \
    void x265_interp_4tap_vert_pp_ ## W ## x ## H ## cpu(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx); \
    void x265_interp_4tap_vert_ps_ ## W ## x ## H ## cpu(pixel * src, intptr_t srcStride, int16_t * dst, intptr_t dstStride, int coeffIdx);

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
    SETUP_CHROMA_FUNC_DEF(8, 32, cpu)

#define SETUP_LUMA_FUNC_DEF(W, H, cpu) \
    void x265_interp_8tap_horiz_pp_ ## W ## x ## H ## cpu(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx); \
    void x265_interp_8tap_horiz_ps_ ## W ## x ## H ## cpu(pixel * src, intptr_t srcStride, int16_t * dst, intptr_t dstStride, int coeffIdx); \
    void x265_interp_8tap_vert_pp_ ## W ## x ## H ## cpu(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx); \
    void x265_interp_8tap_vert_ps_ ## W ## x ## H ## cpu(pixel * src, intptr_t srcStride, int16_t * dst, intptr_t dstStride, int coeffIdx);

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
    SETUP_LUMA_FUNC_DEF(16, 64, cpu)

#define SETUP_LUMA_SP_FUNC_DEF(W, H, cpu) \
    void x265_interp_8tap_vert_sp_ ## W ## x ## H ## cpu(int16_t * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);

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

  #define SETUP_CHROMA_SP_FUNC_DEF(W, H, cpu) \
    void x265_interp_4tap_vert_sp_ ## W ## x ## H ## cpu(int16_t * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);

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


CHROMA_FILTERS(_sse4);
CHROMA_SP_FILTERS(_ssse3);
LUMA_FILTERS(_sse4);
LUMA_SP_FILTERS(_ssse3);

void x265_interp_8tap_hv_pp_8x8_ssse3(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int idxX, int idxY);
void x265_interp_8tap_v_ss_sse2(int16_t *src, intptr_t srcStride, int16_t *dst, intptr_t dstStride, int width, int height, const int coefIdx);
void x265_luma_p2s_ssse3(pixel *src, intptr_t srcStride, int16_t *dst, int width, int height);
void x265_chroma_p2s_ssse3(pixel *src, intptr_t srcStride, int16_t *dst, int width, int height);
void x265_interp_4tap_vert_sp_2x4_sse4(int16_t * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
void x265_interp_4tap_vert_sp_2x8_sse4(int16_t * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
void x265_interp_4tap_vert_sp_6x8_sse4(int16_t * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);

#undef SETUP_CHROMA_FUNC_DEF
#undef SETUP_CHROMA_SP_FUNC_DEF
#undef SETUP_LUMA_FUNC_DEF
#undef SETUP_LUMA_SP_FUNC_DEF
#undef CHROMA_FILTERS
#undef CHROMA_SP_FILTERS
#undef LUMA_FILTERS
#undef LUMA_SP_FILTERS

#endif // ifndef X265_MC_H
