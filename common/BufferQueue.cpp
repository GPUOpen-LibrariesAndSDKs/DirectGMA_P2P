//--------------------------------------------------------------------------------------
//
//
// Copyright 2014 ADVANCED MICRO DEVICES, INC.  All Rights Reserved.
//
// AMD is granting you permission to use this software and documentation (if
// any) (collectively, the "Materials") pursuant to the terms and conditions
// of the Software License Agreement included with the Materials.  If you do
// not have a copy of the Software License Agreement, contact your AMD
// representative for a copy.
// You agree that you will not reverse engineer or decompile the Materials,
// in whole or in part, except as allowed by applicable law.
//
// WARRANTY DISCLAIMER: THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND.  AMD DISCLAIMS ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
// INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE, NON-INFRINGEMENT, THAT THE SOFTWARE
// WILL RUN UNINTERRUPTED OR ERROR-FREE OR WARRANTIES ARISING FROM CUSTOM OF
// TRADE OR COURSE OF USAGE.  THE ENTIRE RISK ASSOCIATED WITH THE USE OF THE
// SOFTWARE IS ASSUMED BY YOU.
// Some jurisdictions do not allow the exclusion of implied warranties, so
// the above exclusion may not apply to You.
//
// LIMITATION OF LIABILITY AND INDEMNIFICATION:  AMD AND ITS LICENSORS WILL
// NOT, UNDER ANY CIRCUMSTANCES BE LIABLE TO YOU FOR ANY PUNITIVE, DIRECT,
// INCIDENTAL, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING FROM USE OF
// THE SOFTWARE OR THIS AGREEMENT EVEN IF AMD AND ITS LICENSORS HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
// In no event shall AMD's total liability to You for all damages, losses,
// and causes of action (whether in contract, tort (including negligence) or
// otherwise) exceed the amount of $100 USD.  You agree to defend, indemnify
// and hold harmless AMD and its licensors, and any of their directors,
// officers, employees, affiliates or agents from and against any and all
// loss, damage, liability and other expenses (including reasonable attorneys'
// fees), resulting from Your use of the Software or violation of the terms and
// conditions of this Agreement. 
//
// U.S. GOVERNMENT RESTRICTED RIGHTS: The Materials are provided with "RESTRICTED
// RIGHTS." Use, duplication, or disclosure by the Government is subject to the
// restrictions as set forth in FAR 52.227-14 and DFAR252.227-7013, et seq., or
// its successor.  Use of the Materials by the Government constitutes
// acknowledgement of AMD's proprietary rights in them.
//
// EXPORT RESTRICTIONS: The Materials may be subject to export restrictions as
// stated in the Software License Agreement.
//
//--------------------------------------------------------------------------------------





#include "os_include.h"

#include "BufferQueue.h"


BufferQueue::BufferQueue(unsigned int uiSize) : m_pBuffer(NULL), m_uiSize(uiSize), m_uiHead(0), m_uiTail(0), m_uiNumFullElements(0)
{
    // Create Semaphore to monitor the number of full elements
    // initial value is 0, the max value is uiSize
    m_NumFull.create(0, m_uiSize);

    // Create Semaphore to monitor the number of empty elements
    // initial value is uiSize, the max value is uiSize
    m_NumEmpty.create(uiSize, m_uiSize);

    m_pBuffer = new BufferElement[m_uiSize];

    for (unsigned int i = 0; i < m_uiSize; i++)
    {
        m_pBuffer[i].Mutex.create(1, 1);
        m_pBuffer[i].pData      = NULL;
    }
}


BufferQueue::~BufferQueue()
{
    m_NumFull.destroy();
    m_NumEmpty.destroy();

    if (m_pBuffer)
    {
        // Release all semaphores
        for (unsigned int i = 0; i < m_uiSize; i++)
        {
            m_pBuffer[i].Mutex.destroy();
        }

        delete [] m_pBuffer;
    }
}


// assigns memory to this buffer.
void BufferQueue::setBufferMemory(unsigned int uiId, void* pData)
{
    if (m_pBuffer && uiId < m_uiSize)
    {
        m_pBuffer[uiId].Mutex.waitForObject();
        m_pBuffer[uiId].pData  = pData;

        m_pBuffer[uiId].Mutex.release();
    }
}

// Produces an entry
// get a buffer for writing. Buffer will be filled
unsigned int BufferQueue::getBufferForWriting(void* &pBuffer)
{
    // Wait until an empty slot is available
    m_NumEmpty.waitForObject();

    // Enter critical section
    m_pBuffer[m_uiHead].Mutex.waitForObject();

    pBuffer = m_pBuffer[m_uiHead].pData;

    return m_uiHead;
}


// Mark buffer as full, ready to be consumed
void BufferQueue::releaseWriteBuffer()
{
    // Leave critical section
    m_pBuffer[m_uiHead].Mutex.release();

    // Increment the number of full buffers
    m_NumFull.release();

    ++m_uiNumFullElements;

    // switch to next buffer
    m_uiHead = (m_uiHead + 1) % m_uiSize;
}


// get a buffer for reading
unsigned int BufferQueue::getBufferForReading(void* &pBuffer)
{
    // Wait until the buffer is available
    m_NumFull.waitForObject();

    // Block buffer
    m_pBuffer[m_uiTail].Mutex.waitForObject();

    pBuffer = m_pBuffer[m_uiTail].pData;

    return m_uiTail;
}   


// Check if a full buffer is ready but do not block in case no buffer is available
bool BufferQueue::getBufferForReadingIfAvailable(void* &pBuffer, unsigned int &uiIdx)
{
    bool bStatus = m_NumFull.waitForObject(0);

    if (bStatus)
    {
        // Block buffer
        m_pBuffer[m_uiTail].Mutex.waitForObject();

        pBuffer = m_pBuffer[m_uiTail].pData;

        uiIdx = m_uiTail;

        return true;
    }

    return false;
}



// Mark buffer as empty, ready to be filled
void BufferQueue::releaseReadBuffer()
{
    // Release buffer
    m_pBuffer[m_uiTail].Mutex.release();

    // Increase number of empty buffers
    m_NumEmpty.release();

    --m_uiNumFullElements;

    // switch to next buffer
    m_uiTail = (m_uiTail + 1) % m_uiSize;
}

