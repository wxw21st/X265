/*****************************************************************************
 * fifo.h: 
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

#ifndef __FIFO_H__
#define __FIFO_H__

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct {
    // The IO Completion port is used as a FIFO object.
    HANDLE m_hCompletionPort;

    // The event object is set when queue size is less than upper
    // bound. It's reset when queue size is at the upper bound.
    HANDLE m_hOverflowWaiting;

    // The event object is set to cancel a blocking push call due
    // to queue size upper bound.
    HANDLE m_hCancelEvent;

    // The queue size upper bound.
    LONG m_queueSize;

    // The current queue size.
    LONG m_elmCount;
} fifo_t;

void fifo_init(fifo_t *pFifo, size_t queueSize);
void fifo_free(fifo_t *pFifo);
int fifo_enq(fifo_t *pFifo, const int obj);
int fifo_deq(fifo_t *pFifo, int *obj);
#endif /* WIN32 */

#endif /* __FIFO_H__ */
