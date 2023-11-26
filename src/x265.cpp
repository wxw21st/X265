/*****************************************************************************
 * x265.cpp: Example for encode
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Xiangwen Wang <wxw21st@163.com>, Min Chen <chenm003@163.com> 
 *          Yanan Zhao, Zhengyi Luo
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
#include "version.h"
#include <time.h>
#include <math.h> // statistical use, PSNR calculation

#define MAX_OUTBUF_SIZE     (1 * 1024 * 1024 * 8)
UInt8 buf[MAX_WIDTH * MAX_HEIGHT * 3 / 2];

#if 0 //turn this off when testAPI_main is on
UInt8 buf1[MAX_WIDTH * MAX_HEIGHT * 3 / 2];
X265_t h;

int main( int argc, char *argv[] )
{
    UInt8 *pucOutBuf0 = (UInt8 *)MALLOC(MAX_OUTBUF_SIZE);
    UInt8 *pucOutBuf1 = (UInt8 *)MALLOC(MAX_OUTBUF_SIZE);
    assert( pucOutBuf0 != NULL );
    assert( pucOutBuf1 != NULL );

    int i;
    char   *inFile   = NULL;
    char   *outFile  = "output.bin";
    Int16   iWidth   = 352;//x64 modify, change type to Int
    Int16   iHeight  = 288;//x64 modify, change type to Int
    UInt32  nFrames  = 1;
    Int32   iQP      = 32;
    Int32   iMax     = 1;
    UInt32  bWriteRecFlag = FALSE;
    UInt32  bWriteVisCUFlag = FALSE;
    UInt32  bStrongIntraSmoothing = FALSE;
    UInt32  bTiles = FALSE;
    UInt32  nThreads = 1;
		Int bUseSceneChange = SCENE_CHANGE_DETECTION;

		//params for rate control
		UInt8   nFrameRate = 25;
		UInt8   nGOPSize   = 8;
		UInt8   bUseRateCtrl = FALSE;
		Int     targetKbps = 1000;
		
    fprintf( stdout, "\n" );
    fprintf( stdout, "x265 Version [%s] ", X265_VERSION );
    fprintf( stdout, "Built on [%s]", __DATE__ );
    fprintf( stdout, X265_ONOS );
    fprintf( stdout, X265_BITS );
    fprintf( stdout, X265_COMPILEDBY );
    fprintf( stdout, "\n" );

    if ( argc < 2 ) {
        fprintf( stderr, "Usage:\n\t%s -i inYuvFile -o outBinFile [options]\n", argv[0] );
        fprintf( stderr, "\nOptions:\n" );
        fprintf( stderr, "    -w          : Width of Video                      [%4d]\n", iWidth );
        fprintf( stderr, "    -h          : Height of Video                     [%4d]\n", iHeight );
        fprintf( stderr, "    -f frames   : Encode frames                       [%4u]\n", nFrames );//x64 modify, change UInt print specifier to be %u to avoid warning 
        fprintf( stderr, "    -q QP       : Encode QP                           [%4d]\n", iQP );
        fprintf( stderr, "    -ip num     : Max P Slices between two I Slice    [%4d]\n", iMax );
        fprintf( stderr, "    -sis        : Enable StrongIntraSmoothingFilter   [%4u]\n", bStrongIntraSmoothing );
        #ifdef _OPENMP
        fprintf( stderr, "    -t N        : Encode with N threads(Max %2d)       [%4u]\n", MAX_THREADS, nThreads );
        #endif
        fprintf( stderr, "\nOptions[Debug]:\n" );
        fprintf( stderr, "    -rec        : Write Recon Video into OX.YUV\n" );
        #ifdef DEBUG_VIS
        fprintf( stderr, "    -vis_cu     : Write Visual CU Split Video into VIS_CU.YUV\n" );
        #endif
        return 0;
    }

    for( i=1; i<argc; i++ ) {
        if ( !strcmp(argv[i], "-i") ) {
          inFile = argv[++i];
					printf("%s;  ", inFile);
				}
        else if ( !strcmp(argv[i], "-o") ) {
          outFile = argv[++i];
        }
        else if ( !strcmp(argv[i], "-w") ) {
          iWidth = atoi(argv[++i]);
					printf("-w: %d;  ", iWidth);
        }
        else if ( !strcmp(argv[i], "-h") ) {
          iHeight = atoi(argv[++i]);
					printf("-h: %d;  ", iHeight);
				}
        else if ( !strcmp(argv[i], "-f") ) {
          nFrames = atoi(argv[++i]);
					printf("-f: %u;  ", nFrames);
        }
        else if ( !strcmp(argv[i], "-q") ) {
          iQP = atoi(argv[++i]);
					printf("-QP: %d;  ", iQP);
        }
        else if ( !strcmp(argv[i], "-ip") ) {
          iMax = atoi(argv[++i]);
					printf("-IDR: %d;  ", iMax);
        }
				else if ( !strcmp(argv[i], "-fps") ) {
					nFrameRate = atoi(argv[++i]);
				}
				else if ( !strcmp(argv[i], "-rc") ) {
					bUseRateCtrl = atoi(argv[++i]);
				}
				else if ( !strcmp(argv[i], "-kbps") ) {
					targetKbps = atoi(argv[++i]);
				}
        else if ( !strcmp(argv[i], "-sis") ) {
          bStrongIntraSmoothing = TRUE;
					printf("-sis: %u;  ", bStrongIntraSmoothing);
        }
        else if ( !strcmp(argv[i], "-rec") ) {
          bWriteRecFlag = TRUE;
        }
        else if ( !strcmp(argv[i], "-vis_cu") ) {
          bWriteVisCUFlag = TRUE;
        }
        else if ( !strcmp(argv[i], "-tiles") ) {
          bTiles = TRUE;
					printf("-tiles:4   ");
				}
				else if ( !strcmp(argv[i], "-sceneChange") ) {
					bUseSceneChange = atoi(argv[++i]);
				}
				else if ( !strcmp(argv[i], "-t") ) {
					nThreads = atoi(argv[++i]);
				}
				else {
          fprintf( stderr, "Unknown option [%s]\n", argv[i] );
        }
    }
    memset( &h, 0, sizeof(h) );
    xDefaultParams( &h );

		Int iCUWidth  = (iWidth +MAX_CU_SIZE-1)/MAX_CU_SIZE;
		Int iCUHeight = (iHeight+MAX_CU_SIZE-1)/MAX_CU_SIZE;
		Int iWidthAligned  = iCUWidth*MAX_CU_SIZE;
		Int iHeightAligned = iCUHeight*MAX_CU_SIZE;
		Int iSizePaddedX   = iWidthAligned - iWidth;
		Int iSizePaddedY   = iHeightAligned - iHeight;
		bool bFrameAlignment = (iSizePaddedX>0 || iSizePaddedY>0);
		h.bUseSceneChangeDetection = bUseSceneChange;
		h.iSceneChangeIntervalCurr = 0;

		h.iWidth           = (bFrameAlignment? iWidthAligned  : iWidth);
		h.iHeight          = (bFrameAlignment? iHeightAligned : iHeight);
		h.ucMaxCUWidth      = 64; 
		h.ucMaxCUDepth      =  4;
		h.nThreads          = nThreads;
    h.bWriteRecFlag     = bWriteRecFlag;
    h.bWriteVisCUFlag   = bWriteVisCUFlag;
    h.bStrongIntraSmoothing = bStrongIntraSmoothing;
    h.eSliceType        = SLICE_I;
    h.iQP               = iQP;
    h.bTiles            = bTiles;

		h.iCUWidth		 = iCUWidth;
		h.iCUHeight    = iCUHeight;
		h.nIntraPeriod = iMax;
		h.nFrameRate	 = nFrameRate;
		h.nGOPSize     = nGOPSize;
		h.bUseRateCtrl = bUseRateCtrl;
		h.targetKbps   = targetKbps;

#if USE_WPP_YANAN
	h.pOutBuf0 = pucOutBuf0;
	h.pOutBuf1 = pucOutBuf1;
#endif
	
	if(bUseRateCtrl){
		assert(nGOPSize>0);
		xInitRCSeq(&h, nFrames, nFrameRate, nGOPSize, targetKbps*1000);
	}
    xCheckParams( &h );
    fprintf( stdout, "Encoder working on %d threads\n\n", h.nThreads * (bTiles ? 4 : 1) );

    CUInt32  nYSize = iWidth*iHeight;
    CUInt32  nImageSize = nYSize*3/2;
		double totalBits = 0;

    xEncInit( &h );
	
    FILE *fpi = fopen( inFile, "rb");
    FILE *fpo = fopen( outFile, "wb");
    assert( fpi != NULL );
    assert( fpo != NULL );
    
    clock_t start = clock();

		xFrame frame;
		xFrame frameAligned;

		frame.pucY = buf;
		frame.pucU = buf + nYSize;
		frame.pucV = buf + nYSize * 5 / 4;
		frame.iStrideY = iWidth;
		frame.iStrideC = iWidth >> 1;
		frameAligned.pucY = buf1;
		frameAligned.pucU = buf1 + iWidthAligned*iHeightAligned;
		frameAligned.pucV = buf1 + iWidthAligned*iHeightAligned * 5 / 4;
		frameAligned.iStrideY = iWidthAligned;
		frameAligned.iStrideC = iWidthAligned >> 1;

		Int bSceneChanged = 0;
    for( i=0; i<(int)nFrames; i++ ) {
			if ( fread( buf, 1, nImageSize, fpi ) != nImageSize){
				break;
			}

			if (bFrameAlignment) {
				xFrameAligning(&frameAligned, iWidthAligned, iHeightAligned, &frame, iWidth, iHeight);
			}

			if ( h.bUseSceneChangeDetection && (h.iSceneChangeIntervalCurr>SCENE_CHANGE_INTERVAL) )	{
				bSceneChanged = sceneChangeDetection(&h, frameAligned.pucY);//&frameAligned);
			}
			
			if ( (i%iMax==0 || bSceneChanged) && (h.iPoc!=0) ){ // make sure two continuous frames not both be IDR frames
        /*Last POC be 0 means previous frame is an IDR, so current frame should not be an IDR*/
				h.eSliceType = SLICE_I;
				//if (bSceneChanged) {
				//	printf("Scene change detected, frame number %d, interval %d \n", i, h.iSceneChangeIntervalCurr);
				//}
				h.iSceneChangeIntervalCurr = 0; //reset current scene change interval
				bSceneChanged = 0;
			}
			else{
        h.eSliceType = SLICE_P;
				h.iSceneChangeIntervalCurr++; //increase scene change interval
			}

			if (bUseRateCtrl)	{
				if (!i || i%nGOPSize==1){//init RCGOP
					Int numPic = (Int)(i==0? 1: (nFrames-i>nGOPSize? nGOPSize:nFrames-i));//pic number in one GOP
					xInitRCGOP(&h, numPic);
				}

				//init picture level rate control
				Int sliceQP              = h.iQP;
				Int frameLevel = ( (h.eSliceType)==SLICE_I? 0: 1);//0: I frame; 1: ref; 2: non-ref

				xInitRCPic( &h );

				RateCtrl* rc        = &(h.rc);	
				xRCSeq*   rcSeq     = &rc->rcSeq;
				xRCGOP*   rcGOP     = &rc->rcGOP;
				xRCPic*   rcPic     = &rc->rcPic;
				double lambda = 0.0;
				if ( frameLevel == 0 ){   // intra case, but use the model
					if ( h.nIntraPeriod != 1 ){   // do not refine allocated bits for all intra case
						Int64 bits = rcSeq->m_bitsLeft / rcSeq->m_framesLeft;

						//get refine bits for intra
						double bpp = ( (double)bits ) / h.iHeight/h.iWidth;
						bits *= ( bpp>0.2? 5: (bpp>0.1? 7:10) );
						if ( bits < 200 ){
							bits = 200;
						}

						rcPic->m_targetBits =  Int64(bits);
					}

					lambda  = xRCPicEstPicLambda( &h );
					sliceQP = xRCPicEstPicQP( &h, lambda );
				}
				else{    // normal case
					lambda  = xRCPicEstPicLambda( &h );
					sliceQP = xRCPicEstPicQP( &h, lambda );
				}

				if (i==0){
					sliceQP = Clip3( h.iQP-1, h.iQP+1, sliceQP );
				}
				else{
					sliceQP = Clip3( h.iQP-3, h.iQP+3, sliceQP );
				}
				sliceQP = Clip3( 0, MAX_QP, sliceQP );//only appropriate for 8bit YUV 

				h.iQP = sliceQP;
				rcPic->m_picQP = sliceQP;
				rcPic->m_picLambda = lambda;
			}

//#if DEBLOCK
			if (DEBLOCK) {
				memset(h.qp_y_tab, h.iQP, h.iWidth/h.ucMaxCUDepth * h.iHeight/h.ucMaxCUDepth);
			}
//#endif

			printf("Encoding Frame[%5d, %c, %d]", i, (h.eSliceType==SLICE_I? 'I':'P'), h.iQP);
			Int32 iEncSize = xEncodeFrame( &h, &(bFrameAlignment? frameAligned:frame), pucOutBuf1, MAX_OUTBUF_SIZE);

      assert( iEncSize < MAX_OUTBUF_SIZE );
      fwrite( pucOutBuf1, 1, iEncSize, fpo );
			fflush( fpo );

			if(STATIC_PSNR){  
				visa_encode_static(&h, h.eSliceType, iEncSize, h.pFrameCur->pucY, h.pFrameCur->pucU,h.pFrameCur->pucV);
			}
			else{
				printf("\n");
			}
			totalBits += (iEncSize<<3);
    }

		if(STATIC_PSNR){
			visa_close_static  ( &h );
		}
	
    FREE(pucOutBuf0);
    FREE(pucOutBuf1);

    fclose(fpi);
    fclose(fpo);
    xEncFree(&h);
    printf("\nAll Done!\n");

    double dTime = (double)(clock()-start) / CLOCKS_PER_SEC;
    printf("Time: %.3f sec, (%.2f fps). ", dTime, (double)nFrames/dTime);

		if (h.bUseRateCtrl){
			printf("Bitrate: %6.1f Kbps \n", totalBits/1000/((double)nFrames/nFrameRate));
		}
		else{
			printf("\n");
		}
    return 0;
}
#endif

int64_t visa_ssd_wxh( UInt8 * pix1, int i_pix1, UInt8 * pix2, int i_pix2, int lx, int ly )
{
	int i_sum = 0;                                  
	int x, y;                                      
	for( y = 0; y < ly; y++ )	{                                               
		for( x = 0; x < lx; x++ )	{                                           
			int d = pix1[x] - pix2[x];              
			i_sum += d*d;                           
		}                                           
		pix1 += i_pix1;                      
		pix2 += i_pix2;                      
	}                                               
	return i_sum;                                   
}

float visa_psnr( int64_t i_sqe, int64_t i_size )
{
	double f_mse = (double)i_sqe / ((double)65025.0 * (double)i_size);
	if( f_mse <= 0.00000001 ) {
		return 100;
	}

	return (float)(-10.0 * log( f_mse ) / log( 10.0 ));
}

void visa_encode_static(X265_t *h,int i_slice_type,int i_frame_size, UInt8 *Src_y,UInt8 *Src_u,UInt8 *Src_v)
{
	h->stat.i_slice_count[i_slice_type]++;
	h->stat.i_slice_size[i_slice_type] += i_frame_size ;

	{
		int64_t i_sqe_y, i_sqe_u, i_sqe_v;
		xFrame   *frame_psnr = &(h->refn[0]);
		/* PSNR */
		i_sqe_y = visa_ssd_wxh( frame_psnr->pucY, frame_psnr->iStrideY, Src_y, h->iWidth, 	 h->iWidth, h->iHeight );
		i_sqe_u = visa_ssd_wxh( frame_psnr->pucU, frame_psnr->iStrideC, Src_u, h->iWidth/2,  h->iWidth/2, h->iHeight/2);
		i_sqe_v = visa_ssd_wxh( frame_psnr->pucV, frame_psnr->iStrideC, Src_v, h->iWidth/2,  h->iWidth/2, h->iHeight/2);

		h->stat.i_sqe_global[i_slice_type] += i_sqe_y + i_sqe_u + i_sqe_v;
		h->stat.f_psnr_average[i_slice_type] += visa_psnr( i_sqe_y + i_sqe_u + i_sqe_v, 3 * h->iWidth * h->iHeight / 2 );
		h->stat.f_psnr_mean_y[i_slice_type]  += visa_psnr( i_sqe_y, h->iWidth * h->iHeight );
		h->stat.f_psnr_mean_u[i_slice_type]  += visa_psnr( i_sqe_u, h->iWidth * h->iHeight / 4 );
		h->stat.f_psnr_mean_v[i_slice_type]  += visa_psnr( i_sqe_v, h->iWidth * h->iHeight / 4 );
#ifndef BATCH
		printf( " PSNR Y:%2.2f U:%2.2f V:%2.2f  bits:%d\n",
			visa_psnr( i_sqe_y, h->iWidth * h->iHeight ),
			visa_psnr( i_sqe_u, h->iWidth * h->iHeight / 4),
			visa_psnr( i_sqe_v, h->iWidth * h->iHeight / 4),
			i_frame_size*8
			);
#endif
	}
} 

void visa_close_static  ( X265_t *h )
{
	int64_t i_yuv_size = 3 * h->iWidth * h->iHeight / 2;
	int i;
	for( i=0; i<3; i++ ) {
		static const int slice_order[] = { SLICE_I, SLICE_P, SLICE_B };
		static const char *slice_name[] = { "B", "P", "I" };
		int i_slice = slice_order[i];

		if( h->stat.i_slice_count[i_slice] > 0 ) {
			const int i_count = h->stat.i_slice_count[i_slice];
#ifndef BATCH
			printf( "%s:%-1d size:%5.0f PSNR Y:%5.2f U:%5.2f V:%5.2f Avg:%5.2f Global:%5.2f\n",
				slice_name[i_slice],
				i_count,
				(double)h->stat.i_slice_size[i_slice] / i_count,
				h->stat.f_psnr_mean_y[i_slice] / i_count, h->stat.f_psnr_mean_u[i_slice] / i_count, h->stat.f_psnr_mean_v[i_slice] / i_count,
				h->stat.f_psnr_average[i_slice] / i_count,
				visa_psnr( h->stat.i_sqe_global[i_slice], i_count * i_yuv_size ) );
#endif
#if		CU_TYPE_STAT
			printf("CU CODED 64x64..4x4: ");
			for(Int ii=xLog2(MAX_CU_SIZE-1)-1; ii>0; ii--) {
				printf("%7lld,", h->stat.iCuTypeCount[i_slice][ii*CU_TYPE_INTERVAL]);//x64 modify, print 64bit integer using %lld to avoid warning 
			}

			printf("\nCU MERGE 64x64..4x4: ");
			for(Int ii=xLog2(MAX_CU_SIZE-1)-1; ii>0; ii--) {
				printf("%6.1f%%,", 100.0*h->stat.iCuTypeCount[i_slice][ii*CU_TYPE_INTERVAL+CU_TYPE_MERGE_OFF]/(h->stat.iCuTypeCount[i_slice][ii*CU_TYPE_INTERVAL]+1));
			}
			printf("\nCU SKIP  64x64..4x4: ");
			for(Int ii=xLog2(MAX_CU_SIZE-1)-1; ii>0; ii--) {
				printf("%6.1f%%,", 100.0*h->stat.iCuTypeCount[i_slice][ii*CU_TYPE_INTERVAL+CU_TYPE_SKIP_OFF]/(h->stat.iCuTypeCount[i_slice][ii*CU_TYPE_INTERVAL]+1));
			}
			printf("\nCU CBFY  64x64..4x4: ");
			for(Int ii=xLog2(MAX_CU_SIZE-1)-1; ii>0; ii--) {
				printf("%6.1f%%,", 100.0*h->stat.iCuTypeCount[i_slice][ii*CU_TYPE_INTERVAL+CU_TYPE_CBF_ZERO]/(h->stat.iCuTypeCount[i_slice][ii*CU_TYPE_INTERVAL]+1));
			}
			printf("\nCU FULL  64x64..4x4: ");
			for(Int ii=xLog2(MAX_CU_SIZE-1)-1; ii>0; ii--) {
				printf("%6.1f%%,", 100.0*(h->stat.iCuTypeCount[i_slice][ii*CU_TYPE_INTERVAL+CU_TYPE_FULL_OFF] 
							- h->stat.iCuTypeCount[i_slice][ii*CU_TYPE_INTERVAL+CU_TYPE_CBF_ZERO])/(h->stat.iCuTypeCount[i_slice][ii*CU_TYPE_INTERVAL]+1));
			}
			printf("\n");
#endif
		}
	}

	if( h->stat.i_slice_count[SLICE_TYPE_I] + h->stat.i_slice_count[SLICE_TYPE_P] > 0 ) {
		const int i_count = h->stat.i_slice_count[SLICE_TYPE_I] + h->stat.i_slice_count[SLICE_TYPE_P] ;
		float fps = 25;
#define SUM3(p) (p[SLICE_TYPE_I] + p[SLICE_TYPE_P] )
#define SUM3b(p,o) (p[SLICE_TYPE_I][o] + p[SLICE_TYPE_P][o] )
		float f_bitrate = fps * SUM3(h->stat.i_slice_size) / i_count / 125;

#ifndef BATCH
		printf(	"PSNR Mean Y:%6.3f U:%6.3f V:%6.3f Avg:%6.3f Global:%6.3f kb/s:%.2f\n",
			SUM3( h->stat.f_psnr_mean_y ) / i_count,
			SUM3( h->stat.f_psnr_mean_u ) / i_count,
			SUM3( h->stat.f_psnr_mean_v ) / i_count,
			SUM3( h->stat.f_psnr_average ) / i_count,
			visa_psnr( SUM3( h->stat.i_sqe_global ), i_count * i_yuv_size ),
			f_bitrate );
#else
		printf(	"QP: %3d PSNR Global:%6.3f kb/s:%8.2f ",
			h->iQP,
			visa_psnr( SUM3( h->stat.i_sqe_global ), i_count * i_yuv_size ),
			f_bitrate );
#endif
	}
}
