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

#include <GL/glew.h>

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/cl_gl.h>

#include "BufferQueue.h"
#include "CLTransferBuffer.h"
#include "CLSink.h"


CLSink::CLSink(cl_context clCtx, cl_command_queue clCmdQueue) 
{
    m_clCtx      = clCtx;
    m_clCmdQueue = clCmdQueue;

    m_uiBufferSize  = 0;
    m_uiFrameWidth  = 0;
    m_uiFrameHeight = 0;

    m_glTexture    = NULL;
    m_clImage      = NULL;
    m_pInputBuffer = NULL;
    m_pInputQueue  = NULL;
    m_pFrameData   = NULL;
}


CLSink::~CLSink(void)
{
    // make sure now other thread is blocked
    release();
    
    if (m_glTexture)
    {
        glDeleteTextures(1, &m_glTexture);
    }

    if (m_clImage)
    {
        clReleaseMemObject(m_clImage);
    }

    delete m_pInputBuffer;
    delete m_pInputQueue;

    if (m_pFrameData)
    {
        delete [] m_pFrameData;
    }
   
}


bool CLSink::createStream(unsigned int uiNumBuffers, unsigned int w, unsigned int h)
{
    int nStatus;

    if (!m_clCtx || !m_clCmdQueue)
    {
        return false;
    }
    
    if (uiNumBuffers == 0 || w == 0 || h == 0)
    {
        return false;
    }

    // if the input buffer already exists, no new one can be created.
    if (m_pInputBuffer)
    {
        return false;
    }

    m_uiFrameWidth  = w;
    m_uiFrameHeight = h;

    // The buffer will contain RGBA data. Calculate the appropriate buffer size
    m_uiBufferSize = m_uiFrameWidth * m_uiFrameHeight * 4;

    m_pInputBuffer = new CLTransferBuffer(m_clCtx, m_clCmdQueue);

    // Create cl buffers in local GPU memory that will be used to receive the data. 
    if (!m_pInputBuffer->createBuffers(uiNumBuffers, m_uiBufferSize, true))
    {
        return false;
    }

    glGenTextures(1, &m_glTexture);

    glBindTexture(GL_TEXTURE_2D, m_glTexture);

    // This texture is shared with CL to display the data that was received
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_uiFrameWidth, m_uiFrameHeight, 0,  GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
        
    m_clImage = clCreateFromGLTexture(m_clCtx, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, m_glTexture, &nStatus);
    CL_CHECK_STATUS(nStatus);

    m_pInputQueue = new BufferQueue(uiNumBuffers);

    m_pFrameData = new FrameData[uiNumBuffers];

    for (unsigned int n = 0; n < uiNumBuffers; ++n)
    {
        cl_bus_address_amd* pAddr = m_pInputBuffer->getBufferBusAddress(n);

        m_pFrameData[n].ullBufferBusAddress = pAddr->surface_bus_address;
        m_pFrameData[n].ullMarkerBusAddress = pAddr->marker_bus_address;

        // Store the id of the CLTransferBuffer.
        m_pFrameData[n].uiBufferId = n;

        m_pFrameData[n].uiTransferId = 0;

        m_pInputQueue->setBufferMemory(n, (char*)&(m_pFrameData[n]));
    }

    return true;
}



void CLSink::processFrame()
{
    int            nStatus;
    FrameData*     pFrameData = NULL;

    // Wait until a frame was queued for each input.
    m_pInputQueue->getBufferForReading((void*&)pFrameData);
    // Wait for transfer to complete
    m_pInputBuffer->waitMarker(pFrameData->uiBufferId, pFrameData->uiTransferId);

    nStatus = clEnqueueAcquireGLObjects(m_clCmdQueue, 1, &m_clImage, 0, 0, 0);

    cl_mem clInputBuffer = m_pInputBuffer->getBuffer(pFrameData->uiBufferId);

    // copy DriectGMA input buffer into GL texture for rendering
    size_t dst_origin[3] = { 0, 0, 0 };
    size_t region[3]     = { m_uiFrameWidth, m_uiFrameHeight, 1 };

    clEnqueueCopyBufferToImage(m_clCmdQueue, clInputBuffer, m_clImage, 0, dst_origin, region, 0, NULL, NULL);

    nStatus = clEnqueueReleaseGLObjects(m_clCmdQueue, 1, &m_clImage, 0, 0, 0);
    clFinish(m_clCmdQueue);

    m_pInputQueue->releaseReadBuffer();
}


unsigned long long CLSink::getBufferBusAddress(unsigned int uiId)
{
    cl_bus_address_amd* pAddress = NULL;

    if (m_pInputBuffer)
    {
        pAddress = m_pInputBuffer->getBufferBusAddress(uiId);

        if (pAddress)
        {
            return pAddress->surface_bus_address;
        }
    }

    return 0;
}


unsigned long long CLSink::getMarkerBusAddress(unsigned int uiId)
{
    cl_bus_address_amd* pAddress = NULL;

    if (m_pInputBuffer)
    {
        pAddress = m_pInputBuffer->getBufferBusAddress(uiId);

        if (pAddress)
        {
            return pAddress->marker_bus_address;
        }
    }

    return 0;
}


void CLSink::release()
{
    if (m_pInputQueue)
        m_pInputQueue->releaseReadBuffer();
}

