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
#include <string>

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/cl_gl.h>


#include "defines.h"
#include "BufferQueue.h"
#include "CLTransferBuffer.h"
#include "CLSource.h"



CLSource::CLSource(cl_context clCtx, cl_command_queue clCmdQueue)
{
    m_clCtx      = clCtx;
    m_clCmdQueue = clCmdQueue;
 
    m_clKernel  = NULL;
    m_clProgram = NULL;
    m_clDevId   = NULL;

    // CL buffers that will be transferred
    m_pclSourceBuffer[0] = NULL;
    m_pclSourceBuffer[1] = NULL;
    m_clProcessedBuffer  = NULL;

    m_uiSourceBufferId  = 0;
    m_uiBufferSize      = 0;
    m_uiNumBuffers      = 0;
    m_uiFrameWidth      = 0;
    m_uiFrameHeight     = 0;
    m_uiFrameCount      = 0;

    m_pOutputQueue     = NULL;

    // DirectGMA buffer
    m_pOutputBuffer    = NULL;

    // buffer to store some dummy data
    m_pData = NULL;
}


CLSource::~CLSource()
{
    // release output queue to unblock waiting threads
    release();

    if (m_pclSourceBuffer[0])
        clReleaseMemObject(m_pclSourceBuffer[0]);

    if (m_pclSourceBuffer[1])
        clReleaseMemObject(m_pclSourceBuffer[1]);

    if (m_clProcessedBuffer)
        clReleaseMemObject(m_clProcessedBuffer);

    if (m_clKernel)
        clReleaseKernel(m_clKernel);

    delete m_pOutputBuffer;
    
    delete [] m_pData;
}


bool CLSource::createStream(unsigned int uiNumBuffers, unsigned int uiFrameWidth, unsigned int uiFrameHeight)
{
    cl_int nStatus;

    if (!m_clCtx || !m_clCmdQueue)
    {
        return false;
    }

    if (uiNumBuffers == 0 || uiFrameWidth == 0 || uiFrameHeight == 0)
    {
        return false;
    }
   
    // A stream was already created, cannot create another one
    if (m_pOutputBuffer)
    {
        return false;
    }
    
    m_uiNumBuffers  = uiNumBuffers;
    m_uiFrameWidth  = uiFrameWidth;
    m_uiFrameHeight = uiFrameHeight;

    m_pOutputBuffer = new CLTransferBuffer(m_clCtx, m_clCmdQueue);

    // The buffer will contain RGBA data. Calculate the appropriate buffer size
    m_uiBufferSize = m_uiFrameWidth * m_uiFrameHeight * 4;

    // Create cl buffers that was allocated on the remote device. The physical addresses of this
    // buffer will be assigned later by the function setRemoteMemory
    if (!m_pOutputBuffer->createBuffers(m_uiNumBuffers, m_uiBufferSize, false))
    {
        return false;
    }

    m_pclSourceBuffer[0] = clCreateBuffer(m_clCtx, CL_MEM_READ_WRITE, m_uiBufferSize, NULL, &nStatus);
    CL_CHECK_STATUS(nStatus);

    m_pclSourceBuffer[1] = clCreateBuffer(m_clCtx, CL_MEM_READ_WRITE, m_uiBufferSize, NULL, &nStatus);
    CL_CHECK_STATUS(nStatus);

    // This buffer can be used as destination for the imageCopy kernel. The kernel can either write directly into
    // the bus_addressable memory on the remote GPU or into this buffer. If the kernel writes to this buffer
    // the data will be transferred by using clEnqueueCopyBuffer
    m_clProcessedBuffer = clCreateBuffer(m_clCtx, CL_MEM_READ_WRITE, m_uiBufferSize, NULL, &nStatus);
    CL_CHECK_STATUS(nStatus);

    m_pData = new char[m_uiBufferSize];

    // Create data that will be transferred to the sink GPU.
    createCheckerboard();

    // copy data to cl buffer
    nStatus = clEnqueueWriteBuffer(m_clCmdQueue, m_pclSourceBuffer[0], CL_TRUE, 0, m_uiBufferSize, m_pData, 0, NULL, NULL);
    CL_CHECK_STATUS(nStatus);

    nStatus = clEnqueueWriteBuffer(m_clCmdQueue, m_pclSourceBuffer[1], CL_TRUE, 0, m_uiBufferSize, m_pData, 0, NULL, NULL);
    CL_CHECK_STATUS(nStatus);

    clFlush(m_clCmdQueue);

    if (!setupKernel())
    {
        return false;
    }

    return true;
}


// assigns physical addresses of memory on remote device to the transfer buffer
bool CLSource::setRemoteMemory(unsigned int uiNumBuffers, unsigned long long *pBufferBusAddress, unsigned long long *pMarkerBusAddress)
{
    if (!m_pOutputBuffer || uiNumBuffers != m_uiNumBuffers)
    {
        return false;
    }

    for (unsigned int i = 0; i < m_uiNumBuffers; ++i)
    {       
        if (!m_pOutputBuffer->assignRemoteMemory(i, pBufferBusAddress[i], pMarkerBusAddress[i]))
        {
            return false;
        }
    }

    return true;
}


void CLSource::processFrame()
{
    cl_int      nStatus;
    cl_int2     vShift              = { 0, 0 };
    cl_int2     vDim                = { m_uiFrameWidth, m_uiFrameHeight };
    size_t      uiGlobalWorkSize[2] = { m_uiFrameWidth, m_uiFrameHeight };
    size_t      uiLoaclWorkSize[2]  = { 16, 16 };
    FrameData*  pFrameData          = NULL;

    // Make sure global size is a multiplier of local size
    uiGlobalWorkSize[0] = (uiGlobalWorkSize[0] + uiLoaclWorkSize[0]-1) & ~(uiLoaclWorkSize[0]-1);
    uiGlobalWorkSize[1] = (uiGlobalWorkSize[1] + uiLoaclWorkSize[1]-1) & ~(uiLoaclWorkSize[1]-1);

    // Wait until an empty element exists in the queue
    m_pOutputQueue->getBufferForWriting((void *&)pFrameData);

    // inc the transfer id
    ++pFrameData->uiTransferId;

    // Get information at which address the SDI board is expecting the next frame
    cl_mem clRemoteBuffer = m_pOutputBuffer->getBuffer(pFrameData->uiBufferId);

    // Argument 0: Input buffer
    nStatus = clSetKernelArg(m_clKernel, 0, sizeof(cl_mem), (void*)&m_pclSourceBuffer[m_uiSourceBufferId]);
    
    // Argument 1: Output buffer. This can be either a buffer on the local GPU, then a call to clEnqueueCopyBuffer
    // is required to transfer the data or this can be a buffer on the remote device. Usually performance is better
    // if the kernel writes first into a local buffer and than the buffer ins transferred to the remote GPU.
    nStatus = clSetKernelArg(m_clKernel, 1, sizeof(cl_mem), (void*)&m_clProcessedBuffer);
    
    ++m_uiFrameCount;
    vShift.s[0] = (m_uiFrameCount % m_uiFrameWidth);

    // Argument 2: shift vector
    nStatus = clSetKernelArg(m_clKernel, 2, sizeof(cl_int2), (cl_int2*)&vShift);

    // Argument 3: Dimension of buffer
    nStatus = clSetKernelArg(m_clKernel, 3, sizeof(cl_int2), (cl_int2*)&vDim);

    nStatus = clEnqueueNDRangeKernel(m_clCmdQueue, m_clKernel, 2, NULL, uiGlobalWorkSize, uiLoaclWorkSize, 0, NULL, NULL);

	clFinish(m_clCmdQueue);
	
    // Transfer processed buffer to remote GPU
    clEnqueueCopyBuffer(m_clCmdQueue, m_clProcessedBuffer, clRemoteBuffer, 0, 0, m_uiBufferSize, 0, 0, 0);
    
    // write marker to indicate end of transfer
    m_pOutputBuffer->writeMarker(pFrameData->uiBufferId, pFrameData->uiTransferId);

    m_pOutputQueue->releaseWriteBuffer();

    m_uiSourceBufferId = 1 - m_uiSourceBufferId;
}


bool CLSource::setupKernel()
{
    cl_int          nStatus;
    std::string     strKernelSource;

    // Determine Device Id
    nStatus = clGetContextInfo(m_clCtx, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &m_clDevId, NULL);
    CL_CHECK_STATUS(nStatus);

    const char* pKernelSrc = "__kernel void copyImage(__global uchar *rgbaIn, __global uchar *rgbaOut, int2 vShift, int2 vDim)          \n \
                              {                                                                                                         \n \
                                    uint uiGlobalId_X = get_global_id(0);                                                               \n \
                                    uint uiGlobalId_Y = get_global_id(1);                                                               \n \
                                                                                                                                        \n \
                                    uint uiBufferOffset = (uiGlobalId_X + (uiGlobalId_Y * vDim.x )) * 4;                                \n \
                                    uint uiSourceOffset = (((uiGlobalId_X + vShift.x) % vDim.x) +  (((uiGlobalId_Y + vShift.y) % vDim.y) * vDim.x )) * 4; \n \
                                                                                                                                        \n \
                                    rgbaOut[uiBufferOffset]     = rgbaIn[uiSourceOffset];                                               \n \
                                    rgbaOut[uiBufferOffset + 1] = rgbaIn[uiSourceOffset + 1];                                           \n \
                                    rgbaOut[uiBufferOffset + 2] = rgbaIn[uiSourceOffset + 2];                                           \n \
                                    rgbaOut[uiBufferOffset + 3] = rgbaIn[uiSourceOffset + 3];                                           \n \
                                } ";                                  

    m_clProgram = clCreateProgramWithSource(m_clCtx, 1, &pKernelSrc, NULL, &nStatus);
    CL_CHECK_STATUS(nStatus);

    nStatus = clBuildProgram(m_clProgram, 1, &m_clDevId, NULL, NULL, NULL);
    CL_CHECK_STATUS(nStatus);

    m_clKernel = clCreateKernel(m_clProgram, "copyImage", &nStatus);
    CL_CHECK_STATUS(nStatus);
   
    return true;
}


void CLSource::setOutputQueue(BufferQueue* pOutputQueue)
{
    if (pOutputQueue)
    {
        m_pOutputQueue = pOutputQueue;
    }
}


void CLSource::release()
{
    if (m_pOutputQueue)
        m_pOutputQueue->releaseWriteBuffer();
}



void CLSource::createCheckerboard()
{
    char* pTexPtr = m_pData;

    for (unsigned int i=0; i < m_uiFrameHeight; ++i)
    {
        for (unsigned int j=0; j < m_uiFrameWidth; ++j)
        {
            char c = ((!(i&0x40)) ^ (!(j&0x40)))*255;

            pTexPtr[0] = 0;             // Red
            pTexPtr[1] = 0;             // Green
            pTexPtr[2] = c;             // Blue
            pTexPtr[3] = (char)0xff;    // Alpha
            pTexPtr += 4;
        }
    }
}