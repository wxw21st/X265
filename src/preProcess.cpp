/*****************************************************************************
 * preProcess.cpp: Pre-processing functions
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Min Chen <chenm003@163.com> , Xiangwen Wang <wxw21st@163.com>
 *          Yanan Zhao
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

#define HIGH_MOTION_BLOCK_THRESHOLD (320*16)
#define SCENE_CHANGE_MOTION_RATIO	  (0.85f)
#define PESN		   (1e-6)	// desired float precision



void xFrameAligning(xFrame *frameAligned, Int iWidthAligned, Int iHeightAligned, xFrame *frame, Int iWidth, Int iHeight)
{
	if (iWidth==iWidthAligned && iHeight==iHeightAligned) {
		return;
	}

	UInt8 *pDstY = frameAligned->pucY;
	UInt8 *pDstU = frameAligned->pucU;
	UInt8 *pDstV = frameAligned->pucV;
	UInt8 *pSrcY = frame->pucY;
	UInt8 *pSrcU = frame->pucU;
	UInt8 *pSrcV = frame->pucV;
	Int nSizePaddedX    = iWidthAligned - iWidth;
	Int nSizePaddedY    = iHeightAligned - iHeight;
	Int iWidthC				 = iWidth/2;
	Int iHeightC				 = iHeight/2;
	Int iWidthCAligned  = iWidthAligned/2;
	Int iHeightCAligned = iHeightAligned/2;
	Int nOffsetDst			 = 0;
	Int nOffsetSrc			 = 0;

	//Padding Horizontal
	{
		//Padding Y
		for (Int j=0; j<iHeight; j++)	{
			nOffsetDst = j*iWidthAligned;
			nOffsetSrc = j*iWidth;
			memcpy(pDstY+nOffsetDst, pSrcY+nOffsetSrc, iWidth*sizeof(UInt8));
			if (nSizePaddedX>0)	{
				memset(pDstY+nOffsetDst+iWidth, pSrcY[nOffsetSrc+iWidth-1], iWidthAligned-iWidth);
			}
		}

		//Padding U
		for (Int j=0; j<iHeightC; j++)	{
			nOffsetDst = j*iWidthCAligned;
			nOffsetSrc = j*iWidthC;
			memcpy(pDstU+nOffsetDst, pSrcU+nOffsetSrc, iWidth*sizeof(UInt8));
			if (nSizePaddedX>0)	{
				memset(pDstU+nOffsetDst+iWidthC, pSrcU[nOffsetSrc+iWidthC-1], iWidthCAligned-iWidthC);
			}
		}

		//Padding V
		for (Int j=0; j<iHeightC; j++)	{
			nOffsetDst = j*iWidthCAligned;
			nOffsetSrc = j*iWidthC;
			memcpy(pDstV+nOffsetDst, pSrcV+nOffsetSrc, iWidth*sizeof(UInt8));
			if (nSizePaddedX>0)	{
				memset(pDstV+nOffsetDst+iWidthC, pSrcV[nOffsetSrc+iWidthC-1], iWidthCAligned-iWidthC);
			}
		}
	}

	if (nSizePaddedY>0)	{
		nOffsetSrc = (iHeight-1)*iWidth;

		for (Int j=iHeight; j<iHeightAligned; j++) {
			nOffsetDst = j*iWidthAligned;
			memcpy(pDstY+nOffsetDst, pSrcY+nOffsetSrc, iWidthAligned*sizeof(UInt8));
		}

		nOffsetSrc = (iHeightC-1)*iWidthC;
		for (Int j=iHeightC; j<iHeightCAligned; j++) {
			nOffsetDst = j*iWidthCAligned;
			memcpy(pDstU+nOffsetDst, pSrcU+nOffsetSrc, iWidthCAligned*sizeof(UInt8));
			memcpy(pDstV+nOffsetDst, pSrcV+nOffsetSrc, iWidthCAligned*sizeof(UInt8));
		}
	}
}

Int sceneChangeDetection(X265_t *h, UInt8 *pYCurr)//xFrame *frameCurr) //x64 modify
{
	Int iFrameIndex = h->uiFrames;
	if (h->uiFrames<=0) {
		return 1;
	}
	if ( h->nIntraPeriod>1 && h->uiFrames % h->nIntraPeriod==0) { //avoid two sequential frames to be both IDR
		return 0;
	}
	
	xFrame *framePrev = h->pFrameRec;
	UInt8 *pYPrev = framePrev->pucY;
	Int iStridePrev = h->pFrameRec->iStrideY;
	Int iStrideCurr = h->iWidth;//h->pFrameCur->iStrideY;
	
	Int iPicWidthInCtu	= h->iCUWidth;
	Int iPicHeightInCtu	= h->iCUHeight;
	Int iNumCTUs	  = iPicWidthInCtu * iPicHeightInCtu * 4; // one CTU contains 4 32x32, assumes CTU to be 64x64 here

	Int iSceneChangeThreshold = Int(SCENE_CHANGE_MOTION_RATIO * iNumCTUs + 0.5f + PESN);

	Int iSadBlock = 0;
	Int iNumMotionBlocks = 0; //In unit of 32x32, Yanan Zhao, 2014-01-16
	Int bSceneChangeFlag = 0;

	
	for (Int j=0; j<iPicHeightInCtu; j++) {
		for (Int i=0; i<iPicWidthInCtu; i++) {
			iSadBlock = xSadN[3] (pYCurr, iStrideCurr, pYPrev, iStridePrev); //xSadN[3]==SAD32x32
			iNumMotionBlocks += Int(iSadBlock > HIGH_MOTION_BLOCK_THRESHOLD);
			
			iSadBlock = xSadN[3] (pYCurr+32, iStrideCurr, pYPrev+32, iStridePrev); //xSadN[3]==SAD32x32
			iNumMotionBlocks += Int(iSadBlock > HIGH_MOTION_BLOCK_THRESHOLD);

			iSadBlock = xSadN[3] (pYCurr+32*iStrideCurr, iStrideCurr, pYPrev+32*iStridePrev, iStridePrev); //xSadN[3]==SAD32x32
			iNumMotionBlocks += Int(iSadBlock > HIGH_MOTION_BLOCK_THRESHOLD);

			iSadBlock = xSadN[3] (pYCurr+32*iStrideCurr+32, iStrideCurr, pYPrev+32*iStridePrev+32, iStridePrev); //xSadN[3]==SAD32x32
			iNumMotionBlocks += Int(iSadBlock > HIGH_MOTION_BLOCK_THRESHOLD);
		}
	}

	if (iNumMotionBlocks >= iSceneChangeThreshold) {
		bSceneChangeFlag = 1;
	}

	//printf("scene change detection passed!\n");
	return bSceneChangeFlag;
}