/*****************************************************************************
 * fifo.cpp: fifo and test fifo
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Min Chen <chenm003@163.com>, Xiangwen Wang <wxw21st@163.com>, 
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

#include <stdio.h>
#include <stdlib.h>
#include "fifo.h"

#ifdef WIN32
static
void Initialize(fifo_t *pFifo)
{
    // The event object is set to cancel a blocking push call
    // due to queue size upper bound.
    pFifo->m_hCancelEvent =   CreateEvent(NULL,
                                          TRUE,  // manual reset
                                          FALSE, // not signaned.
                                          NULL);

    // The event object is set when queue size is less than
    // upper bound. It's reset when queue size is at the upper
    // bound.
    pFifo->m_hOverflowWaiting =   CreateEvent(NULL,
                                              FALSE,  // auto reset
                                              FALSE,  // not signaled.
                                              NULL);
}

/*! The Activate method activates the FIFO. The FIFO must be
 *  activated to call any Push/Pull function.
 */
static
void Activate(fifo_t *pFifo)
{
	pFifo->m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(pFifo->m_hCompletionPort == NULL){
#ifdef _cplusplus
		throw("Failed to CreateIoCompletionPort");
#else
		abort();
#endif
	}

	ResetEvent(pFifo->m_hCancelEvent);
	ResetEvent(pFifo->m_hCancelEvent);
	pFifo->m_elmCount = 0;            
}

/*! The Inactivate method inactivates the FIFO. When FIFO is
 *  inactivated, Push/Pull calls just return false.
 *
 *  The Inactivate method can be called from any thread to
 *  cancel any blocking operation.
 */
static
void Inactivate(fifo_t *pFifo)
{
    SetEvent(pFifo->m_hCancelEvent);
    CloseHandle(pFifo->m_hCompletionPort);
}


void fifo_init(fifo_t *pFifo, size_t queueSize)
{
    pFifo->m_queueSize = queueSize;
    Initialize(pFifo);
    Activate(pFifo);
}

void fifo_free(fifo_t *pFifo)
{
    Inactivate(pFifo);
    CloseHandle(pFifo->m_hCancelEvent);
    CloseHandle(pFifo->m_hOverflowWaiting);

    pFifo->m_queueSize = 0;
}

/*! The Push method push an element to the queue. The call is
 *  blocked when the queue is full (upper bound).
 */
int fifo_enq(fifo_t *pFifo, const int obj)
{
    if(InterlockedIncrement(&pFifo->m_elmCount) > pFifo->m_queueSize){
        // the queue is full.
        HANDLE waitList[] = {pFifo->m_hOverflowWaiting, pFifo->m_hCancelEvent};
        DWORD wret = WaitForMultipleObjects(2, waitList, FALSE, INFINITE);
        if(wret == WAIT_OBJECT_0 + 1){
            // cancellation
            return FALSE;
        }else if(wret != WAIT_OBJECT_0){
            // some error.
            #ifdef _cplusplus
            throw ("Failed to FIFO Enqueue");
            #else
            abort();
            #endif
            return FALSE;
        }
    }
        
    int ret = PostQueuedCompletionStatus(pFifo->m_hCompletionPort, 0, obj, NULL);
    if(!ret){
        return FALSE;
    }
    return TRUE;
}

/*! The Pull method pulls an element from the queue. The call
 *  is blocked when the queue is empty.
 */
int fifo_deq(fifo_t *pFifo, int *obj)
{
    if(InterlockedDecrement(&pFifo->m_elmCount) == pFifo->m_queueSize){
        // there is an waiting thread in Push.
        if(!SetEvent(pFifo->m_hOverflowWaiting)){
            // Not expected ...
            return FALSE;
        }
    }
        
    DWORD numberOfBytes;
    ULONG_PTR key;
    OVERLAPPED* overlapped = NULL;
    int bResult = GetQueuedCompletionStatus(pFifo->m_hCompletionPort, &numberOfBytes, &key, &overlapped, INFINITE);
    if(!bResult){
        return FALSE;
    }
    *obj = key;
    return TRUE;
}

//#define TEST_MAIN
#ifdef TEST_MAIN
#include <process.h>
#define N   (100)
volatile int done_enq=0, done_deq=0;

static unsigned WINAPI my_enq(void *param)
{
    fifo_t *pFifo = (fifo_t*)param;
    int i;
    for(i=0; i<N; i++) {
        fifo_enq(pFifo, i);
        printf("Enq --> %6d\n", i);
        Sleep(rand() % 80);
    }
    done_enq=1;
    return 0;
}

static unsigned WINAPI my_deq(void *param)
{
    fifo_t *pFifo = (fifo_t*)param;
    int i;
    for(i=0; i<N; i++) {
        int tmp;
        fifo_deq(pFifo, &tmp);
        if (tmp != i) {
            printf("Error at %d\n", i);
            break;
        }
        printf("Deq     %6d -->\n", tmp);
        Sleep(rand() % 100);
    }
    if (i == N)
        printf("All Passed!\n");
    done_deq=1;
    return 0;
}

int main()
{
    fifo_t fifo;

    fifo_init(&fifo, 4);
    LONG hEnq, hDeq;
    UINT enqID, deqID;

    hEnq = _beginthreadex(
                NULL,
                0,
                &my_enq,
                &fifo,
                0,
                &enqID
            );
    hDeq = _beginthreadex(
                NULL,
                0,
                &my_deq,
                &fifo,
                0,
                &deqID
            );
    while(!done_enq || !done_deq)
        Sleep(1000);
    fifo_free(&fifo);
    
    return 0;
}
#endif /* TEST_MAIN */

#endif /* WIN32 */
