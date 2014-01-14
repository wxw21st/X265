/*****************************************************************************
 * Deblock.cpp: Deblock Functions
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Xiangwen Wang <wxw21st@163.com>, Min Chen <chenm003@163.com> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;
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
 * For more information, contact us at wxw21st@163.com.
 *****************************************************************************/

#include "x265.h"
#include "bitstream.h"
#include "utils.h"


//#if DEBLOCK //x64 modify

// line zero
#define P3 pix[-4*xstride]
#define P2 pix[-3*xstride]
#define P1 pix[-2*xstride]
#define P0 pix[-xstride]
#define Q0 pix[0]
#define Q1 pix[xstride]
#define Q2 pix[2*xstride]
#define Q3 pix[3*xstride]

// line three. used only for deblocking decision
#define TP3 pix[-4*xstride+3*ystride]
#define TP2 pix[-3*xstride+3*ystride]
#define TP1 pix[-2*xstride+3*ystride]
#define TP0 pix[-xstride+3*ystride]
#define TQ0 pix[3*ystride]
#define TQ1 pix[xstride+3*ystride]
#define TQ2 pix[2*xstride+3*ystride]
#define TQ3 pix[3*xstride+3*ystride]

#define BIT_DEPTH 8

static UInt8 av_clip_pixel(int a)
{
	if (a&(~0xFF)) return (-a)>>31;
	else           return a;
}


static void hevc_loop_filter_luma(UInt8 *pix, Int xstride, Int ystride,
	int no_p, int no_q, int _beta, int _tc)
{
	int d;
	//pixel *pix = (pixel*)_pix;
	//ptrdiff_t xstride = _xstride/sizeof(pixel);
	//ptrdiff_t ystride = _ystride/sizeof(pixel);
	const int dp0 = abs(P2 - 2 * P1 +  P0);
	const int dq0 = abs(Q2 - 2 * Q1 +  Q0);
	const int dp3 = abs(TP2 - 2 * TP1 + TP0);
	const int dq3 = abs(TQ2 - 2 * TQ1 + TQ0);
	const int d0 = dp0 + dq0;
	const int d3 = dp3 + dq3;

	const int beta = _beta << (BIT_DEPTH - 8);
	const int tc = _tc << (BIT_DEPTH - 8);

	if (d0 + d3 < beta) {
		const int beta_3 = beta >> 3;
		const int beta_2 = beta >> 2;
		const int tc25 = ((tc * 5 + 1) >> 1);

		if(abs( P3 -  P0) + abs( Q3 -  Q0) < beta_3 && abs( P0 -  Q0) < tc25 &&
			abs(TP3 - TP0) + abs(TQ3 - TQ0) < beta_3 && abs(TP0 - TQ0) < tc25 &&
			(d0 << 1) < beta_2 &&      (d3 << 1) < beta_2) {
				// strong filtering
				const int tc2 = tc << 1;
				for(d = 0; d < 4; d++) {
					const int p3 = P3;
					const int p2 = P2;
					const int p1 = P1;
					const int p0 = P0;
					const int q0 = Q0;
					const int q1 = Q1;
					const int q2 = Q2;
					const int q3 = Q3;
					if(!no_p) {
						P0 = av_clip_c(( p2 + 2*p1 + 2*p0 + 2*q0 + q1 + 4 ) >> 3, p0-tc2, p0+tc2);
						P1 = av_clip_c(( p2 + p1 + p0 + q0 + 2 ) >> 2, p1-tc2, p1+tc2);
						P2 = av_clip_c(( 2*p3 + 3*p2 + p1 + p0 + q0 + 4 ) >> 3, p2-tc2, p2+tc2);
					}
					if(!no_q) {
						Q0 = av_clip_c(( p1 + 2*p0 + 2*q0 + 2*q1 + q2 + 4 ) >> 3, q0-tc2, q0+tc2);
						Q1 = av_clip_c(( p0 + q0 + q1 + q2 + 2 ) >> 2, q1-tc2, q1+tc2);
						Q2 = av_clip_c(( 2*q3 + 3*q2 + q1 + q0 + p0 + 4 ) >> 3, q2-tc2, q2+tc2);
					}
					pix += ystride;
				}
		} else { // normal filtering
			int nd_p = 1;
			int nd_q = 1;
			const int tc_2 = tc >> 1;
			if (dp0 + dp3 < ((beta+(beta>>1))>>3))
				nd_p = 2;
			if (dq0 + dq3 < ((beta+(beta>>1))>>3))
				nd_q = 2;

			for(d = 0; d < 4; d++) {
				const int p2 = P2;
				const int p1 = P1;
				const int p0 = P0;
				const int q0 = Q0;
				const int q1 = Q1;
				const int q2 = Q2;
				int delta0 = (9*(q0 - p0) - 3*(q1 - p1) + 8) >> 4;
				if (abs(delta0) < 10 * tc) {
					delta0 = av_clip_c(delta0, -tc, tc);
					if(!no_p)
						P0 = av_clip_pixel(p0 + delta0);
					if(!no_q)
						Q0 = av_clip_pixel(q0 - delta0);
					if(!no_p && nd_p > 1) {
						const int deltap1 = av_clip_c((((p2 + p0 + 1) >> 1) - p1 + delta0) >> 1, -tc_2, tc_2);
						P1 = av_clip_pixel(p1 + deltap1);
					}
					if(!no_q && nd_q > 1) {
						const int deltaq1 = av_clip_c((((q2 + q0 + 1) >> 1) - q1 - delta0) >> 1, -tc_2, tc_2);
						Q1 = av_clip_pixel(q1 + deltaq1);
					}
				}
				pix += ystride;
			}
		}
	}
}

static void hevc_loop_filter_chroma(UInt8 *pix, Int32 xstride, Int32 ystride, int no_p, int no_q, int _tc)
{
	int d;
	//pixel *pix = (pixel*)_pix;
	//ptrdiff_t xstride = _xstride/sizeof(pixel);
	//ptrdiff_t ystride = _ystride/sizeof(pixel);
	const int tc = _tc << (BIT_DEPTH - 8);

	for(d = 0; d < 4; d++) {
		int delta0;
		const int p1 = P1;
		const int p0 = P0;
		const int q0 = Q0;
		const int q1 = Q1;
		delta0 = av_clip_c((((q0 - p0) << 2) + p1 - q1 + 4) >> 3, -tc, tc);
		if(!no_p)
			P0 = av_clip_pixel(p0 + delta0);
		if(!no_q)
			Q0 = av_clip_pixel(q0 - delta0);
		pix += ystride;
	}
}

#undef P3
#undef P2
#undef P1
#undef P0
#undef Q0
#undef Q1
#undef Q2
#undef Q3

#undef TP3
#undef TP2
#undef TP1
#undef TP0
#undef TQ0
#undef TQ1
#undef TQ2
#undef TQ3

#define LUMA 0
#define CB 1
#define CR 2

#define MAX_QP 51
#define DEFAULT_INTRA_TC_OFFSET 2

static CUInt8 tctable[54] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // QP  0...18
	1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, // QP 19...37
	5, 5, 6, 6, 7, 8, 9,10,11,13,14,16,18,20,22,24           // QP 38...53
};

static CUInt8 betatable[52] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 7, 8, // QP 0...18
	9,10,11,12,13,14,15,16,17,18,20,22,24,26,28,30,32,34,36, // QP 19...37
	38,40,42,44,46,48,50,52,54,56,58,60,62,64                 // QP 38...51
};

static int chroma_tc( X265_t *s, int qp_y, int c_idx, int tc_offset)
{
	static int qp_c[] = { 29, 30, 31, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37 };
	int qp_i, offset;
	int qp;
	int idxt;

	// slice qp offset is not used for deblocking
	if (c_idx == 1)
		offset = s->pps_cb_qp_offset;
	else
		offset = s->pps_cr_qp_offset;

	qp_i = av_clip_c(qp_y + offset, - s->sps->qp_bd_offset, 57);
	if (qp_i < 30)
		qp = qp_i;
	else if (qp_i > 43)
		qp = qp_i - 6;
	else
		qp = qp_c[qp_i - 30];

	qp += s->sps->qp_bd_offset;

	idxt = av_clip_c(qp + DEFAULT_INTRA_TC_OFFSET + tc_offset, 0, 53);
	return tctable[idxt];
}

static int get_qPy( X265_t *s, int xC, int yC)
{
	int log2_min_cb_size = s->sps->log2_min_coding_block_size;
	int pic_width        = s->sps->pic_width_in_luma_samples>>log2_min_cb_size;
	int x                = xC >> log2_min_cb_size;
	int y                = yC >> log2_min_cb_size;
	return s->qp_y_tab[x + y * pic_width];
}
void ff_hevc_deblocking_filter_CTB( X265_t *s, int x0, int y0)
{
	UInt8 *src;
	int x, y;
	int pixel = 1;// + !!(s->sps->bit_depth - 8); // sizeof(pixel)
	
	//int pic_width_in_min_pu = s->sps->pic_width_in_min_cbs * 4;
	int min_pu_size = 1 << s->sps->log2_min_pu_size;
	int log2_min_pu_size = s->sps->log2_min_pu_size;
	int log2_ctb_size =  s->sps->log2_ctb_size;
	int x_end, y_end;
	int ctb_size = 1<<log2_ctb_size;
	int ctb = (x0 >> log2_ctb_size) + (y0 >> log2_ctb_size) * s->sps->pic_width_in_ctbs;

	int tc_offset = s->deblock[ctb].tc_offset;
	int beta_offset = s->deblock[ctb].beta_offset;
	if (s->deblock[ctb].disable)
		return;

	x_end = x0+ctb_size;
	if (x_end > s->sps->pic_width_in_luma_samples)
		x_end = s->sps->pic_width_in_luma_samples;
	y_end = y0+ctb_size ;
	if (y_end > s->sps->pic_height_in_luma_samples)
		y_end = s->sps->pic_height_in_luma_samples;

	UInt nStrideY = s->pFrameRef->iStrideY;
	UInt nStrideC = s->pFrameRef->iStrideC;

	// vertical filtering
	for (x = x0 ? x0:8; x < x_end; x += 8) {
		for (y = y0; y < y_end; y += 4) {
			int bs = s->vertical_bs[(x >> 3) + (y >> 2) * s->bs_width]; 
			if (bs) {
				int qp = (get_qPy(s, x - 1, y) + get_qPy(s, x, y) + 1) >> 1;
				const int idxb = av_clip_c(qp + ((beta_offset >> 1) << 1), 0, MAX_QP);
				const int beta = betatable[idxb];
				int no_p = 0;
				int no_q = 0;
				const int idxt = av_clip_c(qp + DEFAULT_INTRA_TC_OFFSET * (bs - 1) + ((tc_offset >> 1) << 1), 0, MAX_QP + DEFAULT_INTRA_TC_OFFSET);
				const int tc = tctable[idxt];
				//if((s->sps->pcm_enabled_flag && s->sps->pcm_loop_filter_disable_flag) || s->pps_transquant_bypass_enable_flag) {
				//	int y_pu = y >> log2_min_pu_size;
				//	int xp_pu = (x - 1) / min_pu_size;
				//	int xq_pu = x >> log2_min_pu_size;
				//	if (s->is_pcm[y_pu * pic_width_in_min_pu + xp_pu])
				//		no_p = 1;
				//	if (s->is_pcm[y_pu * pic_width_in_min_pu + xq_pu])
				//		no_q = 1;
				//}
				//src = &s->frame->data[LUMA][y * s->frame->linesize[LUMA] + x];
				src = &s->pFrameRec->pucY[y * nStrideY + x];
				hevc_loop_filter_luma(src, 1, nStrideY, no_p, no_q, beta, tc); // filter four vertical pixels one time

				if ((x & 15) == 0 && (y & 7) == 0 && bs == 2) {
					src = &s->pFrameRec->pucU[(y / 2) * nStrideC + (x / 2)];
					hevc_loop_filter_chroma(src, 1, nStrideC, no_p, no_q, chroma_tc(s, qp, CB, tc_offset));
					src = &s->pFrameRec->pucV[(y / 2) * nStrideC + (x / 2)];
					hevc_loop_filter_chroma(src, 1, nStrideC, no_p, no_q, chroma_tc(s, qp, CR, tc_offset));
				}
			}
		}
	}
	// horizontal filtering
	if (x_end != s->sps->pic_width_in_luma_samples)
		x_end -= 8;
	for (y = y0 ? y0:8; y < y_end; y += 8) {
		for (x = x0 ? x0-8:0; x < x_end; x += 4) {
			int bs = s->horizontal_bs[(x + y * s->bs_width) >> 2];
			if (bs) {
				int qp = (get_qPy(s, x, y - 1) + get_qPy(s, x, y) + 1) >> 1;
				const int idxb = av_clip_c(qp + ((beta_offset >> 1) << 1), 0, MAX_QP);
				const int beta = betatable[idxb];
				int no_p = 0;
				int no_q = 0;
				const int idxt = av_clip_c(qp + DEFAULT_INTRA_TC_OFFSET * (bs - 1) + ((tc_offset >> 1) << 1), 0, MAX_QP + DEFAULT_INTRA_TC_OFFSET);
				const int tc = tctable[idxt];
				//if((s->sps->pcm_enabled_flag && s->sps->pcm_loop_filter_disable_flag) || s->transquant_bypass_enable_flag) {
				//	int yp_pu = (y - 1) / min_pu_size;
				//	int yq_pu = y >> log2_min_pu_size;
				//	int x_pu = x >> log2_min_pu_size;
				//	if (s->is_pcm[yp_pu * pic_width_in_min_pu + x_pu])
				//		no_p = 1;
				//	if (s->is_pcm[yq_pu * pic_width_in_min_pu + x_pu])
				//		no_q = 1;
				//}
				src = &s->pFrameRec->pucY[y * nStrideY + x];
				//hevc_loop_filter_luma(src, s->frame->linesize[LUMA], pixel, no_p, no_q, beta, tc);
				hevc_loop_filter_luma(src, nStrideY, 1, no_p, no_q, beta, tc);   //filter four horizontal pixels one time
				if ((x & 7) == 0 && (y & 15) == 0 && bs == 2) {
					src = &s->pFrameRec->pucU[(y / 2) * nStrideC + (x / 2)];
					hevc_loop_filter_chroma(src, nStrideC, 1, no_p, no_q, chroma_tc(s, qp, CB, tc_offset));
					src = &s->pFrameRec->pucV[(y / 2) * nStrideC + (x / 2)];
					hevc_loop_filter_chroma(src, nStrideC, 1, no_p, no_q, chroma_tc(s, qp, CR, tc_offset));
				}
			}
		}
	}
}

static int boundary_strength(xMV *curr_mv, UInt8 curr_cbf_luma, xMV *neigh_mv, UInt8 neigh_cbf_luma, xSliceType eSliceType, int tu_border)
{
	if (tu_border) {

		//if (curr->is_intra || neigh->is_intra)
		if(eSliceType == SLICE_I)  //not support Intra prediction in Inter frame
			return 2;
		if (curr_cbf_luma || neigh_cbf_luma)
			return 1;
	}

	if (eSliceType == SLICE_P) {
		//if (abs(neigh->mv[0].x - curr->mv[0].x) >= 4 || abs(neigh->mv[0].y - curr->mv[0].y) >= 4 ||
		//	s->ref->refPicList[0].list[neigh->ref_idx[0]] != s->ref->refPicList[0].list[curr->ref_idx[0]])
		if (abs(neigh_mv->x - curr_mv->x) >= 4 || abs(neigh_mv->y - curr_mv->y) >= 4 
			//|| s->ref->refPicList[0].list[neigh->ref_idx[0]] != s->ref->refPicList[0].list[curr->ref_idx[0]] //Only support one reference
			)
			return 1;
		else
			return 0;
	}
	return 0;
}


void ff_hevc_deblocking_boundary_strengths( 
	UInt8	*horizontal_bs, 
	UInt8	*vertical_bs, 
	CUInt16 nWidth, 
	int		x0, 
	int		y0, 
	int		log2_trafo_size, 
	int  	b64x64Flag, 
	xMV		*pasMV, 
	UInt8	*puclModeYInfo1, 
	xSliceType eSliceType)
{
	b64x64Flag = false;//x64 modify, avoid warning, this parameter is not used
	
	//UInt    bSkipped        = !!(puclModeYInfo1[0] & FLAG_MASK_SKIP);
	//UInt    ePUMode			=   (puslModeYInfo0[0] >> FLAG_OFF_PUPART) & 3;  //--WXW
	//UInt    nCbfY           = !!(puclModeYInfo1[0] & FLAG_MASK_CBFY);
	UInt	bs_width = nWidth>>3;
	int log2_min_pu_size = xLog2(MIN_CU_SIZE-1);//s->sps->log2_min_pu_size;
	int min_pu_size = MIN_CU_SIZE;//1 << s->sps->log2_min_pu_size;
	//int pic_width_in_min_pu = s->sps->pic_width_in_min_cbs * 4;
	int i, j;
	int bs;
	//xMV *tab_mvf = s->xCache;
	if ((y0 & 7) == 0) {    // the first line of the ctb
		//int yp_pu = (y0 - 1) / min_pu_size; //up min_cu line
		//int yq_pu = y0 >> log2_min_pu_size; //curr min_cu line
		for (i = 0; i < (1<<log2_trafo_size); i+=4) {
			//int x_pu = (x0 + i) >> log2_min_pu_size;
			//MvField *top  = &tab_mvf[yp_pu * pic_width_in_min_pu + x_pu];
			//MvField *curr = &tab_mvf[yq_pu * pic_width_in_min_pu + x_pu];
//			xMV *top  = &tab_mvf[yp_pu * (MAX_MV_XY + 2) + x_pu];
//			xMV *curr = &tab_mvf[yq_pu * (MAX_MV_XY + 2) + x_pu];
			xMV *top_mv  = &pasMV[ -(MAX_MV_XY + 2) + i/MIN_MV_SIZE ];
			xMV *curr_mv = &pasMV[ 0 ];
			//UInt8 top_cbf_luma  = s->cbf_luma[yp_pu * pic_width_in_min_pu + x_pu];
			//UInt8 curr_cbf_luma = s->cbf_luma[yq_pu * pic_width_in_min_pu + x_pu];
			//UInt8 top_cbf_luma  = cbf_luma[yp_pu * (MAX_PU_XY+1) + x_pu];
			//UInt8 curr_cbf_luma = cbf_luma[yq_pu * (MAX_PU_XY+1) + x_pu];
			UInt8 top_cbfY  = !!(puclModeYInfo1[-(MAX_PU_XY+1) + i/MIN_CU_SIZE ] & FLAG_MASK_CBFY);//cbf_luma[ -(MAX_PU_XY+1) ];
			UInt8 curr_cbfY = !!(puclModeYInfo1[0] & FLAG_MASK_CBFY);;
			bs = boundary_strength(curr_mv, curr_cbfY, top_mv, top_cbfY, eSliceType, 1);
			//if (s->sh.slice_loop_filter_across_slices_enabled_flag == 0 && (y0 % (1 << s->sps->log2_ctb_size)) == 0 && !s->ctb_up_flag[entry])
			//if ((y0 % (MAX_CU_SIZE)) == 0 && !s->ctb_up_flag[entry])
			if(y0 == 0)
				bs = 0;
			//if (s->sh.disable_deblocking_filter_flag == 1)
			//	bs = 0;
			if (bs)
			{
				horizontal_bs[((x0 + i) + y0 * bs_width) >> 2] = bs;
			}
		}
	}
	// bs for TU internal horizontal PU boundaries
	if (log2_trafo_size > log2_min_pu_size && eSliceType != SLICE_I)
	{
		for (j = 8; j < (1<<log2_trafo_size); j += 8) {
			//int yp_pu = (y0 + j - 1) >> log2_min_pu_size;
			//int yq_pu = (y0 + j) >> log2_min_pu_size;
			for (i = 0; i < (1<<log2_trafo_size); i += 4) {
				//int x_pu = (x0 + i) >> log2_min_pu_size;
				xMV *top_mv =  &pasMV[(MAX_MV_XY + 2)*(j/8-1) + i/8 ];
				xMV *curr_mv = &pasMV[0];
				//UInt8 top_cbf_luma  = s->cbf_luma[yp_pu * pic_width_in_min_pu + x_pu];
				//UInt8 curr_cbf_luma = s->cbf_luma[yq_pu * pic_width_in_min_pu + x_pu];
				UInt8 top_cbfY  = !!(puclModeYInfo1[(MAX_PU_XY+1)*(j/4-1)  + i/4 ] & FLAG_MASK_CBFY);//cbf_luma[ -(MAX_PU_XY+1) ];
				UInt8 curr_cbfY = !!(puclModeYInfo1[0] & FLAG_MASK_CBFY);
				//bs = boundary_strength(s, curr, curr_cbf_luma, top, top_cbf_luma, 0);
				bs = boundary_strength(curr_mv, curr_cbfY, top_mv, top_cbfY, eSliceType, 0);
				//if (s->sh.disable_deblocking_filter_flag == 1)
				//	bs = 0;
				if (bs)
				{
					horizontal_bs[((x0 + i) + (y0 + j) * bs_width) >> 2] = bs;
				}
			}
		}
	}
		// bs for vertical TU boundaries
		if ((x0 & 7) == 0) {
			//int xp_pu = (x0 - 1) / min_pu_size;
			//int xq_pu = x0 >> log2_min_pu_size;
			for (i = 0; i < (1<<log2_trafo_size); i+=4) {
				//int y_pu = (y0 + i) >> log2_min_pu_size;
				//MvField *left = &tab_mvf[y_pu * pic_width_in_min_pu + xp_pu];
				//MvField *curr = &tab_mvf[y_pu * pic_width_in_min_pu + xq_pu];
				//UInt8 left_cbf_luma = s->cbf_luma[y_pu * pic_width_in_min_pu + xp_pu];
				//UInt8 curr_cbf_luma = s->cbf_luma[y_pu * pic_width_in_min_pu + xq_pu];
				xMV *left_mv =  &pasMV[-1 + i/8*(MAX_MV_XY+2)];
				xMV *curr_mv = &pasMV[0];
				UInt8 left_cbfY  = !!(puclModeYInfo1[-1 + i/4*(MAX_PU_XY+1)] & FLAG_MASK_CBFY);//cbf_luma[ -(MAX_PU_XY+1) ];
				UInt8 curr_cbfY  = !!(puclModeYInfo1[0] & FLAG_MASK_CBFY);
				//bs = boundary_strength(s, curr, curr_cbf_luma, left, left_cbf_luma, 1);
				bs = boundary_strength(curr_mv, curr_cbfY, left_mv, left_cbfY, eSliceType, 1);
				//if (s->sh.slice_loop_filter_across_slices_enabled_flag == 0 && (x0 % (1 << s->sps->log2_ctb_size)) == 0 && !s->ctb_left_flag[entry])
				if (x0==0)
					bs = 0;
				//if (s->sh.disable_deblocking_filter_flag == 1)
				//	bs = 0;
				if (bs)
				{
					vertical_bs[(x0 >> 3) + ((y0 + i) >> 2) * bs_width] = bs;
				}
			}
		}
		// bs for TU internal vertical PU boundaries
		if (log2_trafo_size > log2_min_pu_size && eSliceType != SLICE_I)
		{
			for (j = 0; j < (1<<log2_trafo_size); j += 4) {
				//int y_pu = (y0 + j) >> log2_min_pu_size;
				for (i = 8; i < (1<<log2_trafo_size); i += 8) {
					//int xp_pu = (x0 + i - 1) >> log2_min_pu_size;
					//int xq_pu = (x0 + i) >> log2_min_pu_size;
					//MvField *left = &tab_mvf[y_pu * pic_width_in_min_pu + xp_pu];
					//MvField *curr = &tab_mvf[y_pu * pic_width_in_min_pu + xq_pu];
					//UInt8 left_cbf_luma = s->cbf_luma[y_pu * pic_width_in_min_pu + xp_pu];
					//UInt8 curr_cbf_luma = s->cbf_luma[y_pu * pic_width_in_min_pu + xq_pu];
					//bs = boundary_strength(s, curr, curr_cbf_luma, left, left_cbf_luma, 0);
					xMV *left_mv =  &pasMV[-1 + j/8*(MAX_MV_XY+2)+i/8];
					xMV *curr_mv =  &pasMV[0];
					UInt8 left_cbfY  = !!(puclModeYInfo1[-1 + j/4*(MAX_PU_XY+1)+i/4] & FLAG_MASK_CBFY);//cbf_luma[ -(MAX_PU_XY+1) ];
					UInt8 curr_cbfY  = !!(puclModeYInfo1[0] & FLAG_MASK_CBFY);
					bs = boundary_strength(curr_mv, curr_cbfY, left_mv, left_cbfY, eSliceType, 0);
					//if (s->sh.disable_deblocking_filter_flag == 1)
					//	bs = 0;
					if (bs)
					{
						vertical_bs[((x0 + i) >> 3) + ((y0 + j) >> 2) * bs_width] = bs;
					}
				}
			}
		}
}
#undef LUMA
#undef CB
#undef CR

void hls_filter(X265_t *s, int x, int y)
{
	ff_hevc_deblocking_filter_CTB(s, x, y);
}

void hls_filters( X265_t *s, int x_ctb, int y_ctb, int ctb_size)
{
	if(y_ctb && x_ctb) {
		ff_hevc_deblocking_filter_CTB(s, x_ctb-ctb_size, y_ctb-ctb_size);
		if(x_ctb >= (s->sps->pic_width_in_luma_samples - ctb_size))
			ff_hevc_deblocking_filter_CTB(s, x_ctb, y_ctb-ctb_size);
		if(y_ctb >= (s->sps->pic_height_in_luma_samples - ctb_size))
			ff_hevc_deblocking_filter_CTB(s, x_ctb-ctb_size, y_ctb);
	}
}


void xDeblockFrame( 
	X265_t *h, 
	CUInt32 nCUWidth, 
	CUInt32 nCUHeight, 
	Int32 offX, //tiles off //x64 modify, remove const attribute
	Int32 offY, //tiles off //x64 modify, remove const attribute
	Int32 bsOff, //x64 modify, remove const attribute
	Int32 idxTiles )//x64 modify, remove const attribute
{
	offX = 0; //x64 modify, avoid warning, this parameter is not used currently
	offY = 0; //x64 modify, avoid warning, this parameter is not used currently
	bsOff = 0;//x64 modify, avoid warning, this parameter is not used currently
	idxTiles = 0;//x64 modify, avoid warning, this parameter is not used currently

	CInt        iCUSize     = h->ucMaxCUWidth;//x64 modify, convert to Int
	CUInt        nLog2CUSize = xLog2(iCUSize-1);
	CUInt32      bTiles      = h->bTiles;
	Int			 x,y;

	CUInt32 nStrideY    = h->pFrameRef->iStrideY;
	CUInt32 nStrideC    = h->pFrameRef->iStrideC;
	const xSliceType eSliceType = h->eSliceType;

	for( y=0; y < (int)nCUHeight; y++ ) {
		UInt32      uiOffset    = 0;
		for( x=0; x < (int)nCUWidth; x++ ) {
			hls_filters( h, x*iCUSize, y*iCUSize, iCUSize);
			//hls_filters( h, uiOffset + offX*nCUSize, (offY+y)*nCUSize, nCUSize);
			//uiOffset += nCUSize;
		}
	}
	if (x*iCUSize  >= h->sps->pic_width_in_luma_samples && y*iCUSize >= h->sps->pic_height_in_luma_samples)
	{
		hls_filter(h, x*iCUSize-iCUSize, y*iCUSize-iCUSize);
	}
}

//#endif //endif for DEBLOCK