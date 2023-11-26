/*****************************************************************************
 * testDll_main.cpp: x265 dll testing wrapper
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Yanan Zhao, Xiangwen Wang, Zhengyi Luo, Li Song
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

#if 1


#include "x265dll.h"
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
 //#include "x265.h"
#include <math.h>
typedef int Int;
typedef unsigned char UInt8;

#define OUT_STATIC_PSNR 1

#define MAX_WIDTH 1024
#define MAX_HEIGHT 1024

#define MAX_OUTBUF_SIZE     (1 * 1024 * 1024 *4)
extern UInt8 buf[MAX_WIDTH * MAX_HEIGHT * 3 / 2];
UInt8 pucOutBuf0[ MAX_OUTBUF_SIZE];
UInt8 pucOutBuf1[ MAX_OUTBUF_SIZE];

#define NEW_API 1//new encoder API, x265_t is created using malloc

int main()
{
	char *cfg = ".\\..\\encoder_config.cfg";

	void *ph;
	x265_encoder_init(&ph, 1, cfg);
	
	Int nFrames = 1000;
	Int nal_num = -1;
	Int nal_len[10] = {-1};
	unsigned char *p_data = NULL;
	unsigned char *p_yuv  = NULL;

	
	char *inFile = "D:\\test\\YUV\\ClassD\\BasketballPass_416x240_50.yuv";
	char *outFile = "bit.265";
	

	FILE *fpi = fopen( inFile, "rb");
	assert( fpi != NULL );
	FILE *fpo = fopen( outFile, "wb");
	assert( fpo != NULL );

	Int  nYSize    = ph->iWidth*ph->iHeight;
	Int  nImageSize = nYSize*3/2;

	
	double totalBits = 0;
	for (Int i=0; i<250; i++){
		nal_num = 0;
		if ( fread( buf, 1, nImageSize, fpi ) != nImageSize){
			printf("Failed when Reading Yuv! \n");
			break;
		}

		//printf("Encode:%d \n",i);
		Int iEncSize =  x265_encode(ph, pucOutBuf1, &nal_num, nal_len, buf );
				
		X265_t *pH = (X265_t *)ph;
  	if (OUT_STATIC_PSNR){  
			visa_encode_static(pH, pH->eSliceType, iEncSize, pH->pFrameCur->pucY, pH->pFrameCur->pucU,pH->pFrameCur->pucV);
		}
		else{
			printf("\n");
		}

		totalBits += (iEncSize<<3);
		//for (Int i=0; i<nal_num; i++)	{
		//	printf("nal_ID:%d, nal_len:%d \n", i, nal_len[i]);
		//}

		fwrite( pucOutBuf1, 1, iEncSize, fpo );
		fflush( fpo );
	}

  if (STATIC_PSNR) {
		visa_close_static  ( (X265_t *)ph );
	}
	//if (h.bUseRateCtrl){
	//	printf("Bitrate: %6.1f Kbps \n", totalBits/1000/((double)h.nFramesToBeEncoded/ph->nFrameRate));
	//}

	x265_encoder_free(ph);

	fclose(fpi);
	fclose(fpo);
}

#endif