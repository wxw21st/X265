/*****************************************************************************
 * config.h: Common config
 *****************************************************************************
 * Copyright (C) 2012-2020 x265 project
 *
 * Authors: Xiangwen Wang <wxw21st@163.com> Min Chen <chenm003@163.com> 
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

////////////////////////////
// x265 start
////////////////////////////
#define MAX_CU_DEPTH                        (4)
#define MAX_REF_NUM                         (1)
#define MAX_WIDTH                           (4096)
#define MAX_HEIGHT                          (4096)
#define MIN_CU_SIZE                         ( 4)
#define MAX_CU_SIZE                         (32)
#define PAD_YSIZE                           (MAX_CU_SIZE + 16)
#define PAD_CSIZE                           (PAD_YSIZE / 2)
#define MAX_PU_XY                           (MAX_CU_SIZE / MIN_CU_SIZE)
#define MIN_MV_SIZE                         ( 8)
#define MAX_MV_XY                           (MAX_CU_SIZE / MIN_MV_SIZE)
#define MAX_PART_NUM                        (MAX_PU_XY * MAX_PU_XY)
#define MAX_THREADS                         (12)

#define NUM_INTRA_MODE                      (36)
#define NUM_CHROMA_MODE                     ( 5)    // total number of chroma modes
#define CHROMA_MODE_DM                      (NUM_CHROMA_MODE - 1)
#define PLANAR_IDX                          ( 0)
#define VER_IDX                             (26)    // index for intra VERTICAL   mode
#define HOR_IDX                             (10)    // index for intra HORIZONTAL mode
#define DC_IDX                              ( 1)
#define DM_CHROMA_IDX                       (36)    // chroma mode index for derived from luma intra mode
#define SHIFT_INV_1ST                       ( 7)    // Shift after first inverse transform stage
#define SHIFT_INV_2ND                       (12)    // Shift after second inverse transform stage
#define MAX_SAD                             ( 1 << 30 )

#define ASM_NONE                            (0)
#define ASM_MMX                             (1)
#define ASM_SSE2                            (2)
#define ASM_SSE4                            (3)
#define ASM_AVX2                            (4)

#ifndef USE_ASM
    #define USE_ASM                         ASM_NONE
#endif

#define ALWAYS_INLINE                       __inline
////////////////////////////
// x265 end
////////////////////////////

////////////////////////////
// JCT-VC start
////////////////////////////
#define QUANT_IQUANT_SHIFT              20 // Q(QP%6) * IQ(QP%6) = 2^20
#define QUANT_SHIFT                     14 // Q(4) = 2^14
#define SCALE_BITS                      15 // Inherited from TMuC, pressumably for fractional bit estimates in RDOQ
#define MAX_TR_DYNAMIC_RANGE            15 // Maximum transform dynamic range (excluding sign bit)
#define COEF_REMAIN_BIN_REDUCTION        3 ///< indicates the level at which the VLC 
                                           ///< transitions from Golomb-Rice to TU+EG(k)
// AMVP: advanced motion vector prediction
#define AMVP_MAX_NUM_CANDS               2 ///< max number of final candidates
// MERGE
#define MRG_MAX_NUM_CANDS                5
#define MRG_CANDS_START                 (AMVP_MAX_NUM_CANDS)
// Interpolate
#define NTAPS_LUMA                       8 ///< Number of taps for luma
#define NTAPS_CHROMA                     4 ///< Number of taps for chroma

////////////////////////////
// JCT-VC end
////////////////////////////

////////////////////////////
// JCT-VC H start
////////////////////////////
////////////////////////////
// JCT-VC H end
////////////////////////////

////////////////////////////
// JCT-VC G start
////////////////////////////
#define MRG_MAX_NUM_CANDS               5  ///< MERGE
#define MLS_GRP_NUM                    64  ///< G644: Max number of coefficient groups, max(16, 64)
#define MLS_CG_SIZE                     4  ///< G644: Coefficient group size of 4x4
#define SCAN_SET_SIZE                  16
#define LOG2_SCAN_SET_SIZE              4
#define C1FLAG_NUMBER                   8  ///< maximum number of largerThan1 flag coded in one chunk :  16 in HM5
#define C2FLAG_NUMBER                   1  ///< maximum number of largerThan2 flag coded in one chunk:  16 in HM5
////////////////////////////
// JCT-VC G end
////////////////////////////


/////////////////////////////////
// AHG SLICES defines section start
/////////////////////////////////

/////////////////////////////////
// AHG SLICES defines section end
/////////////////////////////////
#define DEPENDENT_SLICES                    1 ///< I0229:
#define BYTE_ALIGNMENT                      1 ///< I0330: Add byte_alignment() procedure to end of slice header
#define RESTRICT_INTRA_BOUNDARY_SMOOTHING   1 ///< K0380, K0186 
#define LINEBUF_CLEANUP                     1 ///< K0101
#define REMOVE_ENTROPY_SLICES               1 ///< K0288:
#define MAX_VPS_NUM_HRD_PARAMETERS                1
#define MAX_VPS_NUM_HRD_PARAMETERS_ALLOWED_PLUS1  1024
#define MAX_VPS_NUH_RESERVED_ZERO_LAYER_ID_PLUS1  1
#define FIX712                              1 ///< Fix for issue #712: CABAC init tables
#define MOVE_VPS_TEMPORAL_ID_NESTING_FLAG           1  ///< K0137: Move vps_temporal_id_nesting_flag

typedef unsigned char       UInt8;
typedef   signed char        Int8;
typedef unsigned short      UInt16;
typedef          short       Int16;

#ifndef ARCH_X64
typedef unsigned int        UInt32;
typedef          int         Int32;
#else
typedef unsigned long       UInt32;
typedef          long        Int32;
#endif

#ifdef _MSC_VER
typedef unsigned __int64    UInt64;
typedef          __int64     Int64;
#else
typedef unsigned long long  UInt64;
typedef          long long   Int64;
#endif

// Fast integer depends on platform
typedef Int32    Int;
typedef UInt32  UInt;

typedef const  Int8      CInt8;
typedef const UInt8     CUInt8;
typedef const  Int16     CInt16;
typedef const UInt16    CUInt16;
typedef const  Int32     CInt32;
typedef const UInt32    CUInt32;
typedef const  Int64     CInt64;
typedef const UInt64    CUInt64;
typedef const  Int       CInt;
typedef const UInt      CUInt;

#define FALSE   (0)
#define TRUE    (1)

#endif /* __CONFIG_H__ */
