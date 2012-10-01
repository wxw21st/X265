/*****************************************************************************
 * params.cpp: Params config and check
 *****************************************************************************
 * Copyright (C) 2012-2020 x265 project
 *
 * Authors:  Xiangwen Wang <wxw21st@163.com> Min Chen <chenm003@163.com>
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

void xDefaultParams(X265_t *h)
{
    memset(h, 0, sizeof(*h));

    h->ucProfileSpace               =  0;
    h->ucProfileIdc                 =  0;
    h->ucLevelIdc                   =  0;
    h->eSliceType                   = SLICE_I;

    // Params
    h->usWidth                      =  0;
    h->usHeight                     =  0;
    h->ucMaxCUWidth                 = 32;
    h->ucMaxCUDepth                 =  3;
    h->ucQuadtreeTULog2MinSize      =  2;
    h->ucQuadtreeTULog2MaxSize      =  5;
    h->ucQuadtreeTUMaxDepthInter    =  1;
    h->ucQuadtreeTUMaxDepthIntra    =  1;
    h->ucMaxNumRefFrames            =  1;
    h->ucBitsForPOC                 =  8;
    h->ucMaxNumMergeCand            = MRG_MAX_NUM_CANDS;
    h->ucTSIG                       =  5;
    h->nThreads                     = 1;

    // Feature
    h->bUseNewRefSetting            = FALSE;
    h->bUseSAO                      = FALSE;
    h->bMRG                         = FALSE;
    h->bLoopFilterDisable           = TRUE;
    h->bSignHideFlag                = FALSE;
    h->bStrongIntraSmoothing        = TRUE;
}

int confirmPara(int bflag, const char* message)
{
    if (!bflag)
        return FALSE;

    printf("Error: %s\n",message);
    return TRUE;
}

int xCheckParams( X265_t *h )
{
    int check_failed = FALSE; /* abort if there is a fatal configuration problem */

    h->nThreads = 1;
    if ( h->nThreads > MAX_THREADS ) {
        printf( "Warning: Encode Threads must been less or equal %d\n", MAX_THREADS );
        h->nThreads = MAX_THREADS;
    }
#define xConfirmPara(a,b) check_failed |= confirmPara(a,b)
    xConfirmPara( (h->ucMaxCUWidth >> (h->ucMaxCUDepth+1)) == (1 << h->ucQuadtreeTULog2MaxSize), "Assume (QuadtreeTULog2MinSize >= MinCUWidth) fail" );
    xConfirmPara( h->usWidth  % h->ucMaxCUWidth, "Frame width should be multiple of minimum CU size");
    xConfirmPara( h->usHeight % h->ucMaxCUWidth, "Frame height should be multiple of minimum CU size");
    xConfirmPara( h->ucMaxNumRefFrames > MAX_REF_NUM, "Currently, x265 can not support so many reference");
    xConfirmPara( h->ucMaxCUWidth < 16, "Maximum partition width size should be larger than or equal to 16");
    xConfirmPara( h->ucQuadtreeTULog2MaxSize != 5, "Maximum transform width size should be equal to 32" );
    xConfirmPara( h->usWidth > MAX_WIDTH, "The video width larger than MAX_WIDTH, please increment it and rebuild" );
    xConfirmPara( h->usHeight > MAX_HEIGHT, "The video height larger than MAX_HEIGHT, please increment it and rebuild" );

#undef xConfirmPara
    if (check_failed)
    {
        exit(EXIT_FAILURE);
    }
    return 0;
}
