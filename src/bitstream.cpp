/*****************************************************************************
 * bitstream.cpp: Bitstream Functions
 *****************************************************************************
 * Copyright (C) 2012-2020 x265 project
 *
 * Authors: Min Chen <chenm003@163.com> Xiangwen Wang <wxw21st@163.com>
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

// ***************************************************************************

// ***************************************************************************
#define WRITE_CODE( value, length, name)     xWriteCode ( pBS, value, length )
#define WRITE_UVLC( value,         name)     xWriteUvlc ( pBS, value )
#define WRITE_SVLC( value,         name)     xWriteSvlc ( pBS, value )
#define WRITE_FLAG( value,         name)     xWriteFlag ( pBS, value )


// ***************************************************************************
static void xWriteShortTermRefPicSet( X265_t *h )
{
    xBitStream *pBS = &h->bs;

    // for MultiReference only
    //WRITE_FLAG( 0, "inter_ref_pic_set_prediction_flag" ); // inter_RPS_prediction_flag

    WRITE_UVLC( 1, "num_negative_pics" );
    WRITE_UVLC( 0, "num_positive_pics" );
    WRITE_UVLC( 0, "delta_poc_s0_minus1" );
    WRITE_FLAG( 1, "used_by_curr_pic_s0_flag");
}

static void xCodeProfileTier( X265_t *h )
{
    xBitStream *pBS = &h->bs;
    int i;

    WRITE_CODE( h->ucProfileSpace,  2,      "XXX_profile_space[]" );
    WRITE_FLAG( 0,                          "XXX_tier_flag[]"     );
    WRITE_CODE( h->ucProfileIdc,    5,      "XXX_profile_idc[]"   );   
    for( i=0; i<32; i++ ) {
        WRITE_FLAG( FALSE,  "XXX_profile_compatibility_flag[][j]" );   
    }
    WRITE_CODE( 0 , 16, "XXX_reserved_zero_16bits[]" );  
}

static void xCodePTL( X265_t *h, UInt profilePresentFlag, Int maxNumSubLayersMinus1 )
{
    xBitStream *pBS = &h->bs;

    assert( maxNumSubLayersMinus1 == 0 );
    if(profilePresentFlag) {
        xCodeProfileTier( h );    // general_...
    }
    WRITE_CODE( h->ucProfileIdc, 8, "general_level_idc" );
}

void xWriteSPS( X265_t *h )
{
    xBitStream *pBS = &h->bs;
    UInt i;

    WRITE_CODE( 0,                          4,          "sps_video_parameter_set_id" );
    WRITE_CODE( 1-1,                        3,          "sps_max_sub_layers_minus1" );
    WRITE_FLAG( 0,                                      "temporal_id_nesting_flag" );
    xCodePTL( h, 1, 1 - 1);
    WRITE_UVLC( 0,                                      "sps_seq_parameter_set_id" );
    WRITE_UVLC( CHROMA_420,                             "chroma_format_idc" );

    WRITE_UVLC( h->usWidth,                             "pic_width_in_luma_samples" );
    WRITE_UVLC( h->usHeight,                            "pic_height_in_luma_samples" );
    WRITE_FLAG( FALSE,                                  "conformance_window_flag" );
    WRITE_UVLC( 0,                                      "bit_depth_luma_minus8" );
    WRITE_UVLC( 0,                                      "bit_depth_chroma_minus8" );
    WRITE_UVLC( h->ucBitsForPOC-4,                      "log2_max_pic_order_cnt_lsb_minus4" );
    WRITE_FLAG( FALSE,                                  "sps_sub_layer_ordering_info_present_flag");

    WRITE_UVLC( 1,                                      "sps_max_dec_pic_buffering[i]" );
    WRITE_UVLC( 0,                                      "sps_num_reorder_pics[i]" );
    WRITE_UVLC( 0,                                      "sps_max_latency_increase[i]" );

    UInt32 MinCUSize = h->ucMaxCUWidth >> (h->ucMaxCUDepth - 1);
    UInt32 log2MinCUSize = xLog2(MinCUSize)-1;

    WRITE_UVLC( log2MinCUSize - 3,                                                    "log2_min_coding_block_size_minus3" );
    WRITE_UVLC( h->ucMaxCUDepth - 1,                                                  "log2_diff_max_min_coding_block_size" );
    WRITE_UVLC( h->ucQuadtreeTULog2MinSize - 2,                                       "log2_min_transform_block_size_minus2" );
    WRITE_UVLC( h->ucQuadtreeTULog2MaxSize - h->ucQuadtreeTULog2MinSize,              "log2_diff_max_min_transform_block_size" );
    WRITE_UVLC( h->ucQuadtreeTUMaxDepthInter - 1,                                     "max_transform_hierarchy_depth_inter" );
    WRITE_UVLC( h->ucQuadtreeTUMaxDepthIntra - 1,                                     "max_transform_hierarchy_depth_intra" );
    WRITE_FLAG( 0,                                                                    "scaling_list_enabled_flag" );
    WRITE_FLAG( 1,                                                                    "amp_enabled_flag" );
    WRITE_FLAG( 0,                                                                    "sample_adaptive_offset_enabled_flag");
    WRITE_FLAG( 0,                                                                    "pcm_enabled_flag");

    WRITE_UVLC( 1,                                                                    "num_short_term_ref_pic_sets" );
    xWriteShortTermRefPicSet( h );
    WRITE_FLAG( 0,                                                                    "long_term_ref_pics_present_flag" );
    WRITE_FLAG( 1,                                                                    "sps_temporal_mvp_enable_flag" );
    WRITE_FLAG( h->bStrongIntraSmoothing,                                             "sps_strong_intra_smoothing_enable_flag" );
    WRITE_FLAG( FALSE,                                                                "vui_parameters_present_flag" );

    WRITE_FLAG( 0, "sps_extension_flag" );

    xWriteRBSPTrailingBits(pBS);
}

void xWritePPS( X265_t *h )
{
    xBitStream *pBS = &h->bs;

    WRITE_UVLC( 0,  "pps_pic_parameter_set_id" );
    WRITE_UVLC( 0,  "pps_seq_parameter_set_id" );

    WRITE_FLAG( 0,  "dependent_slice_enabled_flag" );

    WRITE_FLAG( h->bSignHideFlag, "sign_data_hiding_flag" );

    WRITE_FLAG( 1,                                          "cabac_init_present_flag" );

    WRITE_UVLC( h->ucMaxNumRefFrames-1,                     "num_ref_idx_l0_default_active_minus1");
    WRITE_UVLC( 0,                                          "num_ref_idx_l1_default_active_minus1");

    WRITE_SVLC( 0/*h->iQP - 26*/,                           "init_qp_minus26");
    WRITE_FLAG( 0,                                          "constrained_intra_pred_flag" );
    WRITE_FLAG( 0,                                          "transform_skip_enabled_flag" ); 
    WRITE_FLAG( 0,                                          "cu_qp_delta_enabled_flag" );

    WRITE_SVLC( 0,                                          "pps_cb_qp_offset" );
    WRITE_SVLC( 0,                                          "pps_cr_qp_offset" );
    WRITE_FLAG( 0,                                          "pps_slice_chroma_qp_offsets_present_flag" );

    WRITE_FLAG( 0,                                          "weighted_pred_flag" );   // Use of Weighting Prediction (P_SLICE)
    WRITE_FLAG( 0,                                          "weighted_bipred_flag" );  // Use of Weighting Bi-Prediction (B_SLICE)

    WRITE_FLAG( 0,                                          "output_flag_present_flag" );
    WRITE_FLAG( 0,                                          "transquant_bypass_enable_flag" );
    WRITE_FLAG( FALSE,                                      "tiles_enabled_flag"              );
    WRITE_FLAG( FALSE,                                      "entropy_coding_sync_enabled_flag");
    WRITE_FLAG( 0,                                          "seq_loop_filter_across_slices_enabled_flag");
    WRITE_FLAG( 1,                                          "deblocking_filter_control_present_flag");
    WRITE_FLAG( FALSE,                                      "deblocking_filter_override_enabled_flag" ); 
    WRITE_FLAG( TRUE,                                       "pps_disable_deblocking_filter_flag" );
    WRITE_FLAG( 0,                                          "pps_scaling_list_data_present_flag" ); 
    WRITE_FLAG( FALSE,                                      "lists_modification_present_flag" );
    WRITE_UVLC( 0,                                          "log2_parallel_merge_level_minus2");
    WRITE_CODE( 0,          3,                              "num_extra_slice_header_bits");
    WRITE_FLAG( 0,                                          "slice_segment_header_extension_present_flag");
    WRITE_FLAG( 0,                                          "pps_extension_flag" );
    xWriteRBSPTrailingBits(pBS);
}

void xWriteVPS( X265_t *h )
{
    xBitStream *pBS = &h->bs;

    WRITE_CODE( 0,          4,                              "vps_video_parameter_set_id" );
    WRITE_CODE( 3,          2,                              "vps_reserved_zero_2bits" );
    WRITE_CODE( 0,          6,                              "vps_reserved_zero_6bits" );
    WRITE_CODE( 1-1,        3,                              "vps_max_sub_layers_minus1" );
    WRITE_FLAG( TRUE,                                       "vps_temporal_id_nesting_flag" );
    WRITE_CODE( 0xFFFF,    16,                              "vps_reserved_ffff_16bits" );
    xCodePTL( h, TRUE, 1-1 );

    // codeBitratePicRateInfo
    {
        WRITE_FLAG( 0,                                      "bit_rate_info_present_flag[i]" );
        WRITE_FLAG( 0,                                      "pic_rate_info_present_flag[i]" );
    }
    WRITE_FLAG( FALSE,                                      "vps_sub_layer_ordering_info_present_flag");
    {
        WRITE_UVLC( 1,                                      "vps_max_dec_pic_buffering[i]" );
        WRITE_UVLC( 0,                                      "vps_num_reorder_pics[i]" );
        WRITE_UVLC( 0,                                      "vps_max_latency_increase[i]" );
    }
    WRITE_CODE( 0, 6,                                       "vps_max_nuh_reserved_zero_layer_id" );
    WRITE_UVLC( 1 - 1,                                      "vps_max_op_sets_minus1" );
    WRITE_UVLC( 0,                                          "vps_num_hrd_parameters" );

    // hrd_parameters
    WRITE_FLAG( 0,                                          "vps_extension_flag" );

    //future extensions here..

    xWriteRBSPTrailingBits(pBS);
}

void xWriteSliceHeader( X265_t *h )
{
    xBitStream *pBS = &h->bs;
    const xSliceType eSliceType = h->eSliceType;

    //write slice address
    WRITE_FLAG( 1,              "first_slice_segment_in_pic_flag" );
    if ( eSliceType == SLICE_I ) {
        WRITE_FLAG( 0, "no_output_of_prior_pics_flag" );
    }
    WRITE_UVLC( 0,              "slice_pic_parameter_set_id" );

    // use only slice address is nonzero
    //WRITE_FLAG( 0,              "dependent_slice_flag" );
    //WRITE_FLAG( 0,              "slice_reserved_undetermined_flag[]");

    WRITE_UVLC( eSliceType,     "slice_type" );

    if ( (eSliceType != SLICE_I) || ((eSliceType == SLICE_I) && (h->iPoc != 0)) ) {
        WRITE_CODE( h->iPoc % (1<<h->ucBitsForPOC), h->ucBitsForPOC, "pic_order_cnt_lsb");
        WRITE_FLAG( 1, "short_term_ref_pic_set_pps_flag");
        // (1<<numbits) >= ref_frame_nums
        //WRITE_CODE( 0, 0, "short_term_ref_pic_set_idx" );
        WRITE_FLAG( 0,                                "slice_temporal_mvp_enable_flag" );
    }

    // we always set num_ref_idx_active_override_flag equal to one. this might be done in a more intelligent way
    if ( eSliceType != SLICE_I ) {
        WRITE_FLAG( 1 ,                               "num_ref_idx_active_override_flag");
        WRITE_UVLC( h->ucMaxNumRefFrames - 1,         "num_ref_idx_l0_active_minus1" );
    }

    if ( eSliceType != SLICE_I ) {
        // Use P-Tables now
        WRITE_FLAG( 0, "cabac_init_flag" );
    }

    if ( eSliceType != SLICE_I ) {
        assert( h->ucMaxNumMergeCand <= MRG_MAX_NUM_CANDS );
        WRITE_UVLC( MRG_MAX_NUM_CANDS - h->ucMaxNumMergeCand, "five_minus_max_num_merge_cand" );
    }

    WRITE_SVLC( h->iQP - 26, "slice_qp_delta" );
    //   if( sample_adaptive_offset_enabled_flag )
    //     sao_param()
    //   if( deblocking_filter_control_present_flag )
    //     disable_deblocking_filter_idc

#if BYTE_ALIGNMENT
    xWriteByteAlignment(pBS);   // Slice header byte-alignment
#else
    xWriteAlignOne(pBS);
#endif
}

void xWriteSliceEnd( X265_t *h )
{
    xBitStream *pBS = &h->bs;

    xWriteByteAlignment(pBS);   // Byte-alignment in slice_data() at end of sub-stream
    xWriteRBSPTrailingBits(pBS);
}

Int32 xPutRBSP(UInt8 *pucDst, UInt8 *pucSrc, UInt32 uiLength)
{
    assert( uiLength > 2 );

    UInt8 *pucDst0 = pucDst;
    UInt i;
    *pucDst++ = *pucSrc++;
    *pucDst++ = *pucSrc++;

    for( i=2; i < uiLength; i++ ) {
        CInt nLeadZero = (pucSrc[-2] | pucSrc[-1]);
        if ( (nLeadZero == 0) && (pucSrc[0] <= 3) ) {
            *pucDst++ = 0x03;
        }
        *pucDst++ = *pucSrc++;
    }
    return (Int32)(pucDst - pucDst0);
}

#define CABAC_ENTER \
    UInt32  uiLow = pCabac->uiLow; \
    UInt32  uiRange = pCabac->uiRange; \
    Int32   iBitsLeft = pCabac->iBitsLeft; \
    UInt8   ucCache = pCabac->ucCache; \
    UInt32  uiNumBytes = pCabac->uiNumBytes;

#define CABAC_LEAVE \
    pCabac->uiLow = uiLow; \
    pCabac->uiRange = uiRange; \
    pCabac->iBitsLeft = iBitsLeft; \
    pCabac->ucCache = ucCache; \
    pCabac->uiNumBytes = uiNumBytes;


// ***************************************************************************
// * Cabac tables
// ***************************************************************************
static CUInt8
INIT_SPLIT_FLAG[3][NUM_SPLIT_FLAG_CTX] = {
    { 107,  139,  126, },
    { 107,  139,  126, },
    { 139,  141,  157, },
};

static CUInt8
INIT_SKIP_FLAG[3][NUM_SKIP_FLAG_CTX] = {
    { 197,  185,  201, },
    { 197,  185,  201, },
    { CNU,  CNU,  CNU, },
};

static CUInt8
INIT_MERGE_FLAG_EXT[3][NUM_MERGE_FLAG_EXT_CTX] = {
    { 154, },
    { 110, },
    { CNU, },
};

static CUInt8
INIT_MERGE_IDX_EXT[3][NUM_MERGE_IDX_EXT_CTX] = {
    { 137, },
    { 122, },
    { CNU, },
};

static CUInt8
INIT_PART_SIZE[3][NUM_PART_SIZE_CTX] = {
    { 154,  139,  CNU,  CNU, },
    { 154,  139,  CNU,  CNU, },
    { 184,  CNU,  CNU,  CNU, },
};

static CUInt8
INIT_CU_AMP_POS[3][NUM_CU_AMP_CTX] = {
    { 154, },
    { 154, },
    { CNU, },
};

static CUInt8
INIT_PRED_MODE[3][NUM_PRED_MODE_CTX] = {
    { 134, },
    { 149, },
    { CNU, },
};

static CUInt8
INIT_INTRA_PRED_MODE[3][NUM_ADI_CTX] = {
    { 183, },
    { 154, },
    { 184, },
};

static CUInt8
INIT_CHROMA_PRED_MODE[3][NUM_CHROMA_PRED_CTX] = {
    { 152,  139, },
    { 152,  139, },
    {  63,  139, },
};

static CUInt8
INIT_INTER_DIR[3][NUM_INTER_DIR_CTX] = {
    {  95,   79,   63,   31, },
    {  95,   79,   63,   31, },
    { CNU,  CNU,  CNU,  CNU, },
};

static CUInt8
INIT_MVD[3][NUM_MV_RES_CTX] = {
    { 169,  198, },
    { 140,  198, },
    { CNU,  CNU, },
};

static CUInt8
INIT_REF_PIC[3][NUM_REF_NO_CTX] = {
    { 153,  153 }, 
    { 153,  153 }, 
    { CNU,  CNU }, 
};

static CUInt8
INIT_DQP[3][NUM_DELTA_QP_CTX] = {
    { 154,  154,  154, },
    { 154,  154,  154, },
    { 154,  154,  154, },
};

static CUInt8
INIT_QT_CBF[3][2*NUM_QT_CBF_CTX] = {
    { 153,  111,  CNU,  CNU,  CNU,  149,   92,  167,  CNU,  CNU, },
    { 153,  111,  CNU,  CNU,  CNU,  149,  107,  167,  CNU,  CNU, },
    { 111,  141,  CNU,  CNU,  CNU,   94,  138,  182,  CNU,  CNU, },
};

static CUInt8
INIT_QT_ROOT_CBF[3][NUM_QT_ROOT_CBF_CTX] = {
    {  79, },
    {  79, },
    { CNU, },
};

static CUInt8
INIT_LAST[3][2*NUM_LAST_FLAG_XY_CTX] = {
    {
        125,  110,  124,  110,   95,   94,  125,  111,  111,   79,  125,  126,  111,  111,   79,
        108,  123,   93,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,
    },
    {
        125,  110,   94,  110,   95,   79,  125,  111,  110,   78,  110,  111,  111,   95,   94,
        108,  123,  108,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,
    },
    {
        110,  110,  124,  125,  140,  153,  125,  127,  140,  109,  111,  143,  127,  111,   79,
        108,  123,   63,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,
    },
};

static CUInt8
INIT_SIG_CG_FLAG[3][2 * NUM_SIG_CG_FLAG_CTX] = {
    {
        121,  140,
         61,  154,
    },
    {
        121,  140,
         61,  154,
    },
    {
         91,  171,
        134,  141,
    },
};

static CUInt8
INIT_SIG_FLAG[3][NUM_SIG_FLAG_CTX] = {
    { 170,  154,  139,  153,  139,  123,  123,   63,  124,  166,  183,  140,  136,  153,  154,  166,  183,  140,  136,  153,  154,  166,  183,  140,  136,  153,  154,  170,  153,  138,  138,  122,  121,  122,  121,  167,  151,  183,  140,  151,  183,  140,  }, 
    { 155,  154,  139,  153,  139,  123,  123,   63,  153,  166,  183,  140,  136,  153,  154,  166,  183,  140,  136,  153,  154,  166,  183,  140,  136,  153,  154,  170,  153,  123,  123,  107,  121,  107,  121,  167,  151,  183,  140,  151,  183,  140,  }, 
    { 111,  111,  125,  110,  110,   94,  124,  108,  124,  107,  125,  141,  179,  153,  125,  107,  125,  141,  179,  153,  125,  107,  125,  141,  179,  153,  125,  140,  139,  182,  182,  152,  136,  152,  136,  153,  136,  139,  111,  136,  139,  111,  }, 
};

static CUInt8
INIT_ONE_FLAG[3][NUM_ONE_FLAG_CTX] = {
    { 154,  196,  167,  167,  154,  152,  167,  182,  182,  134,  149,  136,  153,  121,  136,  122,  169,  208,  166,  167,  154,  152,  167,  182, },
    { 154,  196,  196,  167,  154,  152,  167,  182,  182,  134,  149,  136,  153,  121,  136,  137,  169,  194,  166,  167,  154,  167,  137,  182, },
    { 140,   92,  137,  138,  140,  152,  138,  139,  153,   74,  149,   92,  139,  107,  122,  152,  140,  179,  166,  182,  140,  227,  122,  197, },
};

static CUInt8
INIT_ABS_FLAG[3][NUM_ABS_FLAG_CTX] = {
    { 107,  167,   91,  107,  107,  167, },
    { 107,  167,   91,  122,  107,  167, },
    { 138,  153,  136,  167,  152,  152, },
};

static CUInt8
INIT_MVP_IDX[3][NUM_MVP_IDX_CTX] = {
    { 168,  CNU, },
    { 168,  CNU, },
    { CNU,  CNU, },
};

static CUInt8
INIT_TRANS_SUBDIV_FLAG[3][NUM_TRANS_SUBDIV_FLAG_CTX] = {
    { 224,  167,  122, },
    { 124,  138,   94, },
    { 153,  138,  138, },
};

static CUInt8
INIT_TRANSFORMSKIP_FLAG[3][2*NUM_TRANSFORMSKIP_FLAG_CTX] = {
    { 139,  139},
    { 139,  139},
    { 139,  139},
};

// ***************************************************************************
// * Entropy Functions
// ***************************************************************************
void xCabacInitEntry( UInt items, CInt qp, UInt8 *pucState, CUInt8 *pInitValue )
{
    UInt i;
    assert( (qp >= 0) && (qp <= 51) );

    for( i=0; i<items; i++ ) {
        Int initValue = pInitValue[i];
        // [9.2.1.1]
        Int32  slopeIdx   = ( initValue >> 4);
        Int32  intersecIdx= ( initValue & 15 );
        Int32  m          = slopeIdx * 5 - 45;
        Int32  n          = ( intersecIdx << 3 ) - 16;
        Int32  initState  =  Clip3( 1, 126, ( ( ( m * qp ) >> 4 ) + n ) );
        UInt32 valMPS     = (initState >= 64 );
        pucState[i]       = ( (valMPS ? (initState - 64) : (63 - initState)) <<1) + valMPS;
    }
}

void xCabacInit( X265_t *h )
{
    xCabac *pCabac = &h->cabac;
    CUInt  nSlice  = h->eSliceType;
    CInt   iQp     = h->iQP;
    UInt8 *pucState= pCabac->contextModels;
    UInt   nOffset = 0;

#define INIT_CABAC( n, m, v ) \
    xCabacInitEntry( (m)*(n), iQp, pucState, (v)[nSlice] ); \
    pucState += (n)*(m); \
    nOffset  += (n)*(m);

    assert( nOffset == OFF_SPLIT_FLAG_CTX );
    INIT_CABAC( 1, NUM_SPLIT_FLAG_CTX,          INIT_SPLIT_FLAG         );
    assert( nOffset == OFF_SKIP_FLAG_CTX );
    INIT_CABAC( 1, NUM_SKIP_FLAG_CTX,           INIT_SKIP_FLAG          );
    assert( nOffset == OFF_MERGE_FLAG_EXT_CTX );
    INIT_CABAC( 1, NUM_MERGE_FLAG_EXT_CTX,      INIT_MERGE_FLAG_EXT     );
    assert( nOffset == OFF_MERGE_IDX_EXT_CTX );
    INIT_CABAC( 1, NUM_MERGE_IDX_EXT_CTX,       INIT_MERGE_IDX_EXT      );
    assert( nOffset == OFF_PART_SIZE_CTX );
    INIT_CABAC( 1, NUM_PART_SIZE_CTX,           INIT_PART_SIZE          );
    assert( nOffset == OFF_CU_AMP_CTX );
    INIT_CABAC( 1, NUM_CU_AMP_CTX,              INIT_CU_AMP_POS         );
    assert( nOffset == OFF_PRED_MODE_CTX );
    INIT_CABAC( 1, NUM_PRED_MODE_CTX,           INIT_PRED_MODE          );
    assert( nOffset == OFF_INTRA_PRED_CTX );
    INIT_CABAC( 1, NUM_ADI_CTX,                 INIT_INTRA_PRED_MODE    );
    assert( nOffset == OFF_CHROMA_PRED_CTX );
    INIT_CABAC( 1, NUM_CHROMA_PRED_CTX,         INIT_CHROMA_PRED_MODE   );
    assert( nOffset == OFF_INTER_DIR_CTX );
    INIT_CABAC( 1, NUM_INTER_DIR_CTX,           INIT_INTER_DIR          );
    assert( nOffset == OFF_MVD_CTX );
    INIT_CABAC( 1, NUM_MV_RES_CTX,              INIT_MVD                );
    assert( nOffset == OFF_REF_PIC_CTX );
    INIT_CABAC( 1, NUM_REF_NO_CTX,              INIT_REF_PIC            );
    assert( nOffset == OFF_DELTA_QP_CTX );
    INIT_CABAC( 1, NUM_DELTA_QP_CTX,            INIT_DQP                );
    assert( nOffset == OFF_QT_CBF_CTX );
    INIT_CABAC( 2, NUM_QT_CBF_CTX,              INIT_QT_CBF             );
    assert( nOffset == OFF_QT_ROOT_CBF_CTX );
    INIT_CABAC( 1, NUM_QT_ROOT_CBF_CTX,         INIT_QT_ROOT_CBF        );
    assert( nOffset == OFF_SIG_CG_FLAG_CTX );
    INIT_CABAC( 2, NUM_SIG_CG_FLAG_CTX,         INIT_SIG_CG_FLAG        );
    assert( nOffset == OFF_SIG_FLAG_CTX );
    INIT_CABAC( 1, NUM_SIG_FLAG_CTX,            INIT_SIG_FLAG           );
    assert( nOffset == OFF_LAST_X_CTX );
    INIT_CABAC( 2, NUM_LAST_FLAG_XY_CTX,        INIT_LAST               );
    assert( nOffset == OFF_LAST_Y_CTX );
    INIT_CABAC( 2, NUM_LAST_FLAG_XY_CTX,        INIT_LAST               );
    assert( nOffset == OFF_ONE_FLAG_CTX );
    INIT_CABAC( 1, NUM_ONE_FLAG_CTX,            INIT_ONE_FLAG           );
    assert( nOffset == OFF_ABS_FLAG_CTX );
    INIT_CABAC( 1, NUM_ABS_FLAG_CTX,            INIT_ABS_FLAG           );
    assert( nOffset == OFF_MVP_IDX_CTX );
    INIT_CABAC( 1, NUM_MVP_IDX_CTX,             INIT_MVP_IDX            );
    assert( nOffset == OFF_TRANS_SUBDIV_FLAG_CTX );
    INIT_CABAC( 1, NUM_TRANS_SUBDIV_FLAG_CTX,   INIT_TRANS_SUBDIV_FLAG  );
    assert( nOffset == OFF_TS_FLAG_CTX );
    INIT_CABAC( 1, NUM_TRANSFORMSKIP_FLAG_CTX,  INIT_TRANSFORMSKIP_FLAG );

#undef INIT_CABAC

    assert( nOffset < MAX_NUM_CTX_MOD );
}

void xCabacReset( xCabac *pCabac )
{
    pCabac->uiLow         = 0;
    pCabac->uiRange       = 510;
    pCabac->iBitsLeft     = 23;
    pCabac->ucCache       = 0xFF;
    pCabac->uiNumBytes    = 0;
}

void xCabacFlush( xCabac *pCabac, xBitStream *pBS )
{
    CABAC_ENTER;
    if ( uiLow >> (32 - iBitsLeft) ) {
        //assert( uiNumBytes > 0 );
        //assert( ucCache != 0xff );
        WRITE_CODE( ucCache + 1, 8, "xCabacFlush0" );
        while( uiNumBytes > 1 ) {
            WRITE_CODE( 0x00, 8, "xCabacFlush1" );
            uiNumBytes--;
        }
        uiLow -= 1 << ( 32 - iBitsLeft );
    }
    else  {
        if ( uiNumBytes > 0 ) {
            WRITE_CODE( ucCache, 8, "xCabacFlush2" );
        }
        while ( uiNumBytes > 1 ) {
            WRITE_CODE( 0xFF, 8, "xCabacFlush3" );
            uiNumBytes--;
        }
    }
    WRITE_CODE( uiLow >> 8, 24 - iBitsLeft, "xCabacFlush4" );
    CABAC_LEAVE;
}

UInt xCabacGetNumWrittenBits( xCabac *pCabac, xBitStream *pBS )
{
    Int32 iLen = xBitFlush(pBS);
    return iLen + 8 * pCabac->uiNumBytes + 23 - pCabac->iBitsLeft;
}

#define GetMPS( state )     ( (UInt)(state) &  1 )
#define GetState( state )   ( (state) >> 1 )
#define UpdateLPS( state )  ( (state) = xg_aucNextStateLPS[ (state) ] )
#define UpdateMPS( state )  ( (state) = xg_aucNextStateMPS[ (state) ] )

void testAndWriteOut( xCabac *pCabac, xBitStream *pBS )
{
    CABAC_ENTER;
    if( iBitsLeft < 12 ) {
        UInt32 leadByte = uiLow >> (24 - iBitsLeft);
        iBitsLeft += 8;
        uiLow     &= 0xFFFFFFFF >> iBitsLeft;

        if ( leadByte == 0xff ) {
            uiNumBytes++;
        }
        else {
            if ( uiNumBytes > 0 ) {
                UInt carry = leadByte >> 8;
                UInt byte = ucCache + carry;
                ucCache = leadByte & 0xff;
                WRITE_CODE( byte, 8, "testAndWriteOut0" );

                byte = ( 0xff + carry ) & 0xff;
                while ( uiNumBytes > 1 ) {
                    WRITE_CODE( byte, 8, "testAndWriteOut0" );
                    uiNumBytes--;
                }
            }
            else {
                uiNumBytes = 1;
                ucCache = leadByte;
            }
        }
    }
    CABAC_LEAVE;
}

void xCabacEncodeBin( xCabac *pCabac, xBitStream *pBS, UInt binValue, UInt nCtxState )
{
    CABAC_ENTER;
    UInt8 ucState = pCabac->contextModels[nCtxState];
    UInt  uiLPS   = xg_aucLPSTable[ GetState( ucState ) ][ ( uiRange >> 6 ) & 3 ];
    uiRange    -= uiLPS;

    if( binValue != GetMPS(ucState) ) {
        Int numBits = xg_aucRenormTable[ uiLPS >> 3 ];
        uiLow     = ( uiLow + uiRange ) << numBits;
        uiRange   = uiLPS << numBits;
        UpdateLPS( ucState );

        iBitsLeft -= numBits;
    }
    else {
        UpdateMPS( ucState );
        if ( uiRange < 256 ) {
            uiLow <<= 1;
            uiRange <<= 1;
            iBitsLeft--;
        }
    }

    pCabac->contextModels[nCtxState] = ucState;
    CABAC_LEAVE;
}

void xCabacEncodeBinEP( xCabac *pCabac, xBitStream *pBS, UInt binValue )
{
    CABAC_ENTER;

    uiLow <<= 1;
    if( binValue ) {
        uiLow += uiRange;
    }
    iBitsLeft--;
    CABAC_LEAVE;
    testAndWriteOut( pCabac, pBS );
}

void xCabacEncodeBinsEP( xCabac *pCabac, xBitStream *pBS, UInt binValues, Int numBins )
{
    assert( binValues < (UInt)(1 << numBins) );

    while ( numBins > 8 ) {
        numBins -= 8;
        UInt pattern = binValues >> numBins;
        pCabac->uiLow <<= 8;
        pCabac->uiLow += pCabac->uiRange * pattern;
        binValues -= pattern << numBins;
        pCabac->iBitsLeft -= 8;

        testAndWriteOut( pCabac, pBS );
    }

    pCabac->uiLow <<= numBins;
    pCabac->uiLow  += pCabac->uiRange * binValues;
    pCabac->iBitsLeft -= numBins;
}

void xCabacEncodeTerminatingBit( xCabac *pCabac, xBitStream *pBS, UInt binValue )
{
    CABAC_ENTER;

    uiRange -= 2;
    if( binValue ) {
        uiLow  += uiRange;
        uiLow <<= 7;
        uiRange = 2 << 7;
        iBitsLeft -= 7;
    }
    else if ( uiRange < 256 ) {
        uiLow   <<= 1;
        uiRange <<= 1;
        iBitsLeft--;
    }

    CABAC_LEAVE;
}

