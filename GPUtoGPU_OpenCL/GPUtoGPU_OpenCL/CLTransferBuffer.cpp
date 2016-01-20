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
#include <fstream>

#include <CL/cl.h>
#include <CL/cl_ext.h>

#include "defines.h"
#include "CLTransferBuffer.h"


extern clEnqueueWaitSignalAMD_fn            clEnqueueWaitSignalAMD; 
extern clEnqueueWriteSignalAMD_fn           clEnqueueWriteSignalAMD;
extern clEnqueueMakeBuffersResidentAMD_fn   clEnqueueMakeBuffersResidentAMD;

using namespace std;

CLTransferBuffer::CLTransferBuffer(cl_context clCtx, cl_command_queue clQueue)
{
    m_clCtx             = clCtx;
    m_clCmdQueue        = clQueue;
    
    m_bBufferReady  = false;
    m_bLocalMemory  = true;

    m_uiNumBuffers  = 0;
    m_uiBufferSize  = 0;
    m_uiBufferIdx   = 0;

    m_pBuffer        = NULL;
    m_pBusAddresses  = NULL;
    m_pMarkerOffsets = NULL;
}


CLTransferBuffer::~CLTransferBuffer()
{
    for (unsigned int i = 0; i < m_uiNumBuffers; ++i)
    {
        if (m_pBuffer[i])
        {
            clReleaseMemObject(m_pBuffer[i]);
        }
    }

    delete [] m_pBusAddresses;
    delete [] m_pMarkerOffsets;
    delete [] m_pBuffer;
}


bool CLTransferBuffer::createBuffers(unsigned int uiNumBuffers, unsigned int uiBufferSize, bool bLocal)
{
    int nStatus;

    if (uiBufferSize == 0 || uiNumBuffers == 0)
    {
        return false;
    }

    // Delete buffers if they already exist
    for (unsigned int i = 0; i < m_uiNumBuffers; ++i)
    {
        nStatus = clReleaseMemObject(m_pBuffer[i]);

        CL_CHECK_STATUS(nStatus);
    }

    if (m_pBuffer)
    {
        delete m_pBuffer;
    }

    // get device id
    cl_uint uiNumDev;
    nStatus = clGetContextInfo(m_clCtx, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint), &uiNumDev, NULL);
    CL_CHECK_STATUS(nStatus);

    if (uiNumDev == 0)
    {
        return false;
    }

    cl_device_id* clDevices = new  cl_device_id[uiNumDev];

    nStatus = clGetContextInfo(m_clCtx, CL_CONTEXT_DEVICES, uiNumDev*sizeof(cl_device_id), clDevices, NULL);
    CL_CHECK_STATUS(nStatus);

    m_clDevId = clDevices[0];

    delete [] clDevices;

    m_uiNumBuffers = uiNumBuffers;

    m_pBuffer       = new cl_mem[m_uiNumBuffers];
    m_pBusAddresses = new cl_bus_address_amd[m_uiNumBuffers];

    memset(m_pBuffer,       0, m_uiNumBuffers * sizeof(cl_mem));
    memset(m_pBusAddresses, 0, m_uiNumBuffers * sizeof(cl_bus_address_amd));

    m_uiBufferSize = uiBufferSize;

    m_bLocalMemory = bLocal;

    // m_bLocalMemory indicates if the buffer is allocated on the local GPU memory or if the buffer 
    // is allocated on a remote device.In the case of remote memory nothing is allocated here but the
    // addresses of the remote memory will be assigned later by calling assignRemoteMemory
    if (m_bLocalMemory)
    {
        for (unsigned int i = 0; i < m_uiNumBuffers; ++i)
        {
            m_pBuffer[i] = clCreateBuffer(m_clCtx, CL_MEM_BUS_ADDRESSABLE_AMD, m_uiBufferSize, NULL, &nStatus);
            CL_CHECK_STATUS(nStatus);
        }

        nStatus = clEnqueueMakeBuffersResidentAMD(m_clCmdQueue, m_uiNumBuffers, m_pBuffer, true, m_pBusAddresses, 0, 0, 0);
        CL_CHECK_STATUS(nStatus);

        clFlush(m_clCmdQueue);
    }
    else
    {
        // In case of remote memory we need to store the offsets of the
        // marker addresses.
        m_pMarkerOffsets = new cl_ulong[m_uiNumBuffers];
        memset(m_pMarkerOffsets, 0, m_uiNumBuffers*sizeof(cl_ulong));
    }

    return true;
}


bool CLTransferBuffer::assignRemoteMemory(unsigned int uiBufferId, cl_ulong ulSurfaceAddr, cl_ulong ulMarkerAddr)
{
    cl_int nStatus;

    // check if 
    if (m_bLocalMemory)
    {
        return false;
    }
    if (!(uiBufferId < m_uiNumBuffers) || !m_pBuffer)
    {
        return false;
    }

    // Make sure that the marker address is page aligned (4K)
    // The marker address needs to be page aligned, since this is not always the case
    // the offset from the page boundary to the actual marker is stored. When writing
    // the marker this offset can be used.
    m_pMarkerOffsets[uiBufferId]                    = ulMarkerAddr &  0xfff;
    m_pBusAddresses[uiBufferId].marker_bus_address  = ulMarkerAddr & ~0xfff;
    m_pBusAddresses[uiBufferId].surface_bus_address = ulSurfaceAddr;

    m_pBuffer[uiBufferId] = clCreateBuffer(m_clCtx, CL_MEM_EXTERNAL_PHYSICAL_AMD, m_uiBufferSize, &(m_pBusAddresses[uiBufferId]), &nStatus);
    CL_CHECK_STATUS(nStatus);

    // make memory object resident on device. Needs to be called explicitly before using this 
    // external physical mem object!!
    nStatus = clEnqueueMigrateMemObjects(m_clCmdQueue, 1, &m_pBuffer[uiBufferId], 0, 0, 0, 0);
    CL_CHECK_STATUS(nStatus);

    clFlush(m_clCmdQueue);

    return true;
}


cl_mem CLTransferBuffer::getBuffer(unsigned int uiIdx)
{
    if (uiIdx < m_uiNumBuffers)
    {
        return m_pBuffer[uiIdx];
    }

    return 0;
}


cl_bus_address_amd* CLTransferBuffer::getBufferBusAddress(unsigned int uiIdx)
{
    if (m_pBusAddresses && uiIdx < m_uiNumBuffers)
    {
        return &(m_pBusAddresses[uiIdx]);
    }

    return NULL;
}


void CLTransferBuffer::waitMarker(unsigned int uiBufferId, unsigned int uiMarkerValue)
{
    if (uiBufferId < m_uiNumBuffers)
    {
        clEnqueueWaitSignalAMD(m_clCmdQueue, m_pBuffer[uiBufferId], uiMarkerValue, 0, 0, 0);
        clFlush(m_clCmdQueue);
    }
}


void CLTransferBuffer::writeMarker(unsigned int uiBufferId, unsigned int uiMarkerValue)
{
    if (uiBufferId < m_uiNumBuffers)
    {
        clEnqueueWriteSignalAMD(m_clCmdQueue, m_pBuffer[uiBufferId], uiMarkerValue, m_pMarkerOffsets[uiBufferId], 0, 0, 0);
        clFlush(m_clCmdQueue);
    }
}


void CLTransferBuffer::writeMarker(unsigned long long ulRemoteAddress, unsigned int uiMarkerValue)
{
    // Find the buffer that corresponds to the ulRemoteAddress
    for (unsigned int i = 0; i < m_uiNumBuffers; ++i)
    {
        if (ulRemoteAddress == m_pBusAddresses[i].surface_bus_address)
        {
            clEnqueueWriteSignalAMD(m_clCmdQueue, m_pBuffer[i], uiMarkerValue, m_pMarkerOffsets[i], 0, 0, 0);
            clFlush(m_clCmdQueue);

            break;
        }
    }
}

