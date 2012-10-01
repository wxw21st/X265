/*****************************************************************************
 * interface.cpp: interface APIs
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


#include <assert.h>
#include "x265.h"

#ifdef __cplusplus
extern "C" {
#endif

X265_t* xIEncAlloc()
{
    X265_t *h = (X265_t*)malloc( sizeof(X265_t) );
    assert( h );
    memset( h, 0, sizeof(X265_t) );
    xDefaultParams( h );

    return h;
}

void xIEncCheckParams( X265_t *h )
{
    assert( h );
    xCheckParams( h );
}

void xIEncInit( X265_t *h )
{
    assert( h );
    xEncInit( h );
}

int xIEncFrame( X265_t *h, xFrame *frame, unsigned char *outbuf, unsigned bufsize )
{
    //printf("w=%d, h=%d, t=%d, q=%d, rec=%d, vis=%d\n", h->usWidth, h->usHeight, h->nThreads, h->iQP, h->bWriteRecFlag, h->bWriteVisCUFlag);
    int nSize = xEncodeFrame( h, frame, outbuf, bufsize );
    return( nSize );
}

void xIEncFree( X265_t *h )
{
    assert( h );
    xEncFree( h );
}

int xIEncSetParamInt( X265_t *h, const char *name, const int val )
{
    if ( !strcmp(name, "w") ) {
        h->usWidth = val;
    }
    else if ( !strcmp(name, "h") ) {
        h->usHeight = val;
    }
    else if ( !strcmp(name, "q") ) {
        h->iQP = val;
    }
    else if ( !strcmp(name, "rec") ) {
        h->bWriteRecFlag = val ? TRUE : FALSE;
    }
    else if ( !strcmp(name, "vis_cu") ) {
        h->bWriteVisCUFlag = val ? TRUE : FALSE;
    }
    else if ( !strcmp(name, "t") ) {
        h->nThreads = val;
    }
    else if ( !strcmp(name, "st") ) {
        h->eSliceType = (xSliceType)val;
    }
    else if ( !strcmp(name, "sis") ) {
        h->bStrongIntraSmoothing = val ? TRUE : FALSE;
    }
    else {
        fprintf( stderr, "Unknown option [%s]\n", name );
        return -1;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif
