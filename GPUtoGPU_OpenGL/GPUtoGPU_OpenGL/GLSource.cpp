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


#include "BufferQueue.h"
#include "GLTransferBuffers.h"
#include "FormatInfo.h"
#include "RenderTarget.h"
#include "defines.h"

#include "GLSource.h"

#include "rgbimage.h"


GLSource::GLSource()
{
    m_bUseP2P        = false;

	m_cubeRendered = NULL;



    m_uiWindowWidth  = 0;
    m_uiWindowHeight = 0;

    m_uiBufferWidth  = 0;
    m_uiBufferHeight = 0;

    m_nIntFormat = GL_RGB8;
    m_nExtFormat = GL_RGB;
    m_nType      = GL_UNSIGNED_BYTE;

    m_fRotationAngle = 0.0f;

    m_pRenderTarget[0] = NULL;
    m_pRenderTarget[1] = NULL;

    m_pOutputBuffer = NULL;
    m_pSyncBuffer   = NULL;

    for (unsigned int i = 0; i < NUM_BUFFERS; ++i)
    {
        m_pRenderTarget[i] = NULL;
    }
}


GLSource::~GLSource()
{
    release();

    if (m_pOutputBuffer)
        delete m_pOutputBuffer;

    for (unsigned int i = 0; i < NUM_BUFFERS; ++i)
    {
        if (m_pRenderTarget[i])
            delete m_pRenderTarget[i];
    }
}



void GLSource::initGL()
{
    glClearColor(0.0f, 0.2f, 0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);

	m_cubeRendered = new FireCube();
	m_cubeRendered->init();

}



// Resize only the window. Since we are rendering into a FBO the
// projection matrix does not change on a window resize
void GLSource::resize(unsigned int w, unsigned int h)
{
    m_uiWindowWidth  = w;
    m_uiWindowHeight = h;
}


// Create a FBO that will be used as render target and a synchronized PACK buffer to transfer
// FBO content to the other GPU
bool GLSource::createUpStream(unsigned int w, unsigned int h, int nIntFormat, int nExtFormat, int nType, bool bUseP2P, AlignedMem* pMem)
{
    unsigned int uiBufferSize = 0;

    m_uiBufferWidth  = w;
    m_uiBufferHeight = h;

    m_nIntFormat = nIntFormat;
    m_nExtFormat = nExtFormat;
    m_nType      = nType;

    uiBufferSize = FormatInfo::getInternalFormatSize(m_nIntFormat) * m_uiBufferWidth * m_uiBufferHeight;

    if (uiBufferSize == 0)
        return false;

    // check if external format is supported
    if (FormatInfo::getExternalFormatSize(m_nExtFormat, m_nType) == 0)
        return false;

    // Create a FBO for each output buffer. If a buffer was displayed on the sink it can be reused for rendering.
    for (unsigned int i = 0; i < NUM_BUFFERS; ++i)
    {
        m_pRenderTarget[i] = new RenderTarget;
        m_pRenderTarget[i]->createBuffer(m_uiBufferWidth, m_uiBufferHeight, m_nIntFormat, m_nExtFormat, m_nType);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60.0f, (float)m_uiBufferWidth/(float)m_uiBufferHeight, 0.1f, 1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Create output Buffer
    m_pOutputBuffer = new GLTransferBuffers;

    if (!m_pOutputBuffer->createBuffers(NUM_BUFFERS, uiBufferSize, GL_PIXEL_PACK_BUFFER, bUseP2P, pMem))
    {
        return false;
    }

    glPixelStorei(GL_PACK_ALIGNMENT, FormatInfo::getAlignment(m_uiBufferWidth, m_nExtFormat, m_nType));

    m_bUseP2P = bUseP2P;

    return true;
}


bool GLSource::setRemoteMemory(unsigned long long* pBufferBusAddress, unsigned long long* pMarkerBusAddress)
{
    // check if we received enough addresses
    if (!m_pOutputBuffer ||!pBufferBusAddress || !pMarkerBusAddress)
        return false;

    // assign remote buffer to local buffer objects
    if (!m_pOutputBuffer->assignRemoteMemory(NUM_BUFFERS, pBufferBusAddress, pMarkerBusAddress))
    {
        return false;
    }

    return true;    
}




void GLSource::draw()
{
    FrameData*      pFrame = NULL;
    unsigned int    uiBufferIdx;

    // Wait at the beginning of each frame until we receive an empty buffer
    // Each buffer has its corresponding FBO into which we will render
    uiBufferIdx = m_pSyncBuffer->getBufferForWriting((void*&)pFrame);

    // Draw a rotating cube to FBO
    m_pRenderTarget[uiBufferIdx]->bind();

    // Set viewport of FBO
    glViewport(0, 0, m_uiBufferWidth, m_uiBufferHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




    // Draw rotating cube
    glPushMatrix();

    glTranslatef(0.0f, 0.0f, -1.5f);
    glRotatef(m_fRotationAngle, 1.0f, 0.0f, 1.0f);

	m_cubeRendered->draw();

    glPopMatrix();

    m_fRotationAngle += 0.04f;





    // update frame data
    ++pFrame->uiTransferId;

    m_pOutputBuffer->bindBuffer(pFrame->uiBufferId);

    // Copy local buffer into remote buffer
    glReadPixels(0, 0, m_uiBufferWidth, m_uiBufferHeight, m_nExtFormat, m_nType, NULL);
	
    if (m_bUseP2P)
    {
        // Write marker pFrame->uiTransferId into memory on remote gpu. The GLSink will
        // wait until the marker has the value indicated by pFrame before using the buffer
        m_pOutputBuffer->writeMarker(pFrame->ullBufferBusAddress, pFrame->ullMarkerBusAddress, pFrame->uiTransferId);
    }

    // Mark the buffer as ready to be consumed. No need to wait for the transfer to terminate
    // since the consumer will call waitMarker that will block the GPU until the transfer is done.
    m_pSyncBuffer->releaseWriteBuffer();

    m_pRenderTarget[uiBufferIdx]->unbind();

    // Set viewport for window
    glViewport(0, 0, m_uiWindowWidth, m_uiWindowHeight);

    // Copy FBO to window
    m_pRenderTarget[uiBufferIdx]->draw();
}
    


void GLSource::setSyncBuffer(BufferQueue* pSyncBuffer)
{
    if (pSyncBuffer)
    {
        m_pSyncBuffer = pSyncBuffer;
    }
}


void GLSource::release()
{
    if (m_pSyncBuffer)
	{
        m_pSyncBuffer->releaseWriteBuffer();
	}

	if ( m_cubeRendered )
	{
		delete m_cubeRendered;
		m_cubeRendered = NULL;
	}
}


