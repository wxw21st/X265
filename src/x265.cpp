/*****************************************************************************
 * x265.cpp: Example for encode
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
#include "version.h"
#include <time.h>

#define MAX_OUTBUF_SIZE     (1 * 1024 * 1024)
UInt8 buf[MAX_WIDTH * MAX_HEIGHT * 3 / 2];
X265_t h;

int main( int argc, char *argv[] )
{
    UInt8 *pucOutBuf0 = (UInt8 *)MALLOC(MAX_OUTBUF_SIZE);
    UInt8 *pucOutBuf1 = (UInt8 *)MALLOC(MAX_OUTBUF_SIZE);
    assert( pucOutBuf0 != NULL );
    assert( pucOutBuf1 != NULL );
    int i;
    char   *inFile   = NULL;
    char   *outFile  = NULL;
    UInt32   nWidth   = 352;
    UInt32  nHeight  = 288;
    UInt32  nFrames  = 1;
    Int32   iQP      = 32;
    Int32   iMax     = 1;
    UInt32  bWriteRecFlag = FALSE;
    UInt32  bWriteVisCUFlag = FALSE;
    UInt32  bStrongIntraSmoothing = FALSE;
    UInt32  nThreads = 1;

    // print information
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
        fprintf( stderr, "    -w          : Width of Video                      [%4d]\n", nWidth );
        fprintf( stderr, "    -h          : Height of Video                     [%4d]\n", nHeight );
        fprintf( stderr, "    -f frames   : Encode frames                       [%4d]\n", nFrames );
        fprintf( stderr, "    -q QP       : Encode QP                           [%4d]\n", iQP );
        fprintf( stderr, "    -ip num     : Max P Slices between two I Slice    [%4d]\n", iMax );
        fprintf( stderr, "    -sis        : Enable StrongIntraSmoothingFilter   [%4d]\n", bStrongIntraSmoothing );
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
        }
        else if ( !strcmp(argv[i], "-o") ) {
            outFile = argv[++i];
        }
        else if ( !strcmp(argv[i], "-w") ) {
            nWidth = atoi(argv[++i]);
        }
        else if ( !strcmp(argv[i], "-h") ) {
            nHeight = atoi(argv[++i]);
        }
        else if ( !strcmp(argv[i], "-f") ) {
            nFrames = atoi(argv[++i]);
        }
        else if ( !strcmp(argv[i], "-q") ) {
            iQP = atoi(argv[++i]);
        }
        else if ( !strcmp(argv[i], "-ip") ) {
            iMax = atoi(argv[++i]);
        }
        else if ( !strcmp(argv[i], "-sis") ) {
            bStrongIntraSmoothing = TRUE;
        }
        else if ( !strcmp(argv[i], "-rec") ) {
            bWriteRecFlag = TRUE;
        }
        else if ( !strcmp(argv[i], "-vis_cu") ) {
            bWriteVisCUFlag = TRUE;
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
    h.usWidth           = nWidth;
    h.usHeight          = nHeight;
    h.ucMaxCUWidth      = 32;
    h.ucMaxCUDepth      =  3;
    h.nThreads          = nThreads;
    h.bWriteRecFlag     = bWriteRecFlag;
    h.bWriteVisCUFlag   = bWriteVisCUFlag;
    h.bStrongIntraSmoothing = bStrongIntraSmoothing;
    h.eSliceType        = SLICE_I;
    h.iQP               = iQP;
    xCheckParams( &h );
    fprintf( stdout, "Encoder working on %d threads\n\n", h.nThreads );

    CUInt32  nYSize = nWidth*nHeight;
    CUInt32  nImageSize = nYSize*3/2;

    xEncInit( &h );

    FILE *fpi = fopen( inFile, "rb");
    FILE *fpo = fopen( outFile, "wb");
    assert( fpi != NULL );
    assert( fpo != NULL );

    // starting time
    clock_t start = clock();

    for( i=0; i<(int)nFrames; i++ ) {
        xFrame frame;
        frame.pucY = buf;
        frame.pucU = buf + nYSize;
        frame.pucV = buf + nYSize * 5 / 4;
        frame.nStrideY = nWidth;
        frame.nStrideC = nWidth >> 1;
        if ( fread( buf, 1, nImageSize, fpi ) != nImageSize)
            break;

        if ( (i % iMax) == 0 )
            h.eSliceType = SLICE_I;
        else
            h.eSliceType = SLICE_P;

        printf("Encoding Frame[%5d]\n", i);
        Int32 iEncSize = xEncodeFrame( &h, &frame, pucOutBuf1, MAX_OUTBUF_SIZE);
        assert( iEncSize < MAX_OUTBUF_SIZE );
        fwrite( pucOutBuf1, 1, iEncSize, fpo );
        fflush( fpo );
    }

    FREE(pucOutBuf0);
    FREE(pucOutBuf1);
    fclose(fpi);
    fclose(fpo);
    xEncFree(&h);
    printf("\nAll Done!\n");

    // ending time
    double dTime = (double)(clock()-start) / CLOCKS_PER_SEC;
    printf("Time: %.3f sec, (%.2f fps).\n", dTime, (double)nFrames/dTime);
    return 0;
}
