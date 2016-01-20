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

#if defined ( WIN32 )
#include <GL/wglew.h>
#endif

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/cl_gl.h>

#include "defines.h"
#include "Thread.h"
#include "CLSink.h"
#include "CLSource.h"
#include "CLCopyApp.h"
#include "GLWindow.h"
#include "ADLtool.h"


#define clGetGLContextInfoKHR        clGetGLContextInfoKHR_proc
clGetGLContextInfoKHR_fn             clGetGLContextInfoKHR           = NULL;
clEnqueueWaitSignalAMD_fn            clEnqueueWaitSignalAMD          = NULL; 
clEnqueueWriteSignalAMD_fn           clEnqueueWriteSignalAMD         = NULL;
clEnqueueMakeBuffersResidentAMD_fn   clEnqueueMakeBuffersResidentAMD = NULL;


CLCopyApp::CLCopyApp()
{
    m_pclCtx[0]      = NULL;
    m_pclCtx[1]      = NULL;

    m_pclCmdQueue[0] = NULL;
    m_pclCmdQueue[1] = NULL;

    m_pclDevId[0]    = NULL;
    m_pclDevId[1]    = NULL;

    m_pWindow = NULL;

    m_pSink   = NULL;
    m_pSource = NULL;

    // Set size of the gl window
    m_uiWindowWidth  = 1280;
    m_uiWindowHeight = 1024;

    m_uiFrameWidth   = 0;
    m_uiFrameHeight  = 0;

    m_uiQuad = 0;

    m_bRunning    = false;
    m_bStarted    = false;
    m_bNeedResize = false;
}


CLCopyApp::~CLCopyApp()
{
    if (m_pclCmdQueue[0])
        clReleaseCommandQueue(m_pclCmdQueue[0]);

    if (m_pclCmdQueue[1])
        clReleaseCommandQueue(m_pclCmdQueue[1]);

    if (m_pclCtx[0])
        clReleaseContext(m_pclCtx[0]);

    if (m_pclCtx[1])
        clReleaseContext(m_pclCtx[1]);

    if (m_pWindow)
	{
        delete m_pWindow; m_pWindow = NULL;
	}

	ADLtool::CleanClass();
}


bool CLCopyApp::init(unsigned int uiWidth, unsigned int uiHeight, const char* cClassName, std::string& strErrorMessage)
{
	strErrorMessage = "";

    m_uiFrameWidth  = uiWidth;
    m_uiFrameHeight = uiHeight;

    m_pWindow = new GLWindow("CL Sink", cClassName);

    if (!ADLtool::InitClass())
    {
		strErrorMessage = "ERROR, Could not init GLWindow";
        return false;
    }

    if (ADLtool::getNumGPUs() < 2)
    {
		strErrorMessage = "ERROR, Only 1 GPU/monitor detected. You will need 2 GPUs to run this demo, and 1 monitor connected on each GPU.";
        return false;
    }

    // get first display on GPU 0
    unsigned int uiDsp = ADLtool::getDisplayOnGPU(0, 0);

    if (!m_pWindow->create(m_uiWindowWidth, m_uiWindowHeight, uiDsp, true))
    {
		strErrorMessage = "ERROR, Could not create GLWindow";
        return false;
    }


    m_bNeedResize = true;

    // Create semaphore that is released as soon as the initialization of the sink is done
    m_SinkReady.create(0, 1);
    // Create semaphore that is released as soon as the initialization of the source is done
    m_SourceReady.create(0, 1);
    // Create semaphore to indicate that the Source has terminated
    m_SourceDone.create(0, 1);

    return true;
}


bool CLCopyApp::start()
{
    if (m_bRunning)
    {
        return false;
    }

    m_bRunning = true;

    // Start sink and source thread
    m_SinkThread.create((THREAD_PROC)SinkThreadFunc,     this);
    m_SourceThread.create((THREAD_PROC)SourceThreadFunc, this);

    m_bStarted = true;

    return true;
}


void CLCopyApp::stop()
{
    if (m_bStarted)
    {
        m_bRunning = false;

        m_SinkThread.join();
        m_SourceThread.join();

        m_bStarted = false;
    }
}


void CLCopyApp::resize(unsigned int uiWidth, unsigned int uiHeight)
{
    m_uiWindowWidth  = uiWidth;
    m_uiWindowHeight = uiHeight;

    m_bNeedResize = true;
}


DWORD CLCopyApp::SinkThreadFunc(void* pArg)
{
    CLCopyApp* pApp = static_cast<CLCopyApp*>(pArg);

    pApp->SinkLoop();

    return 0;
}


DWORD CLCopyApp::SourceThreadFunc(void* pArg)
{
    CLCopyApp* pApp = static_cast<CLCopyApp*>(pArg);

    pApp->SourceLoop();

    return 0;
}





bool CLCopyApp::SinkLoop()
{
    if (!m_pWindow)
    {
        return false;
    }

    // Create GL context
    if (!m_pWindow->createContext())
    {
        return false;
    }

    // create cl context and command queues for sink and source. Since the sink cl context is shared
    // with GL it can only be created after a valid GL context exists. The remaining device is used 
    // to create the source CL context
    if (!setupCL())
    {
        return false;
    }

    m_pWindow->open();
    m_pWindow->makeCurrent();

    setupGL();

    // Create CL Sink and use context and cmd queue 0 for sink
    m_pSink = new CLSink(m_pclCtx[0], m_pclCmdQueue[0]);

    if (m_pSink->createStream(NUM_BUFFERS, m_uiFrameWidth, m_uiFrameHeight))
    {
        // Indicate the source thread that the sink is ready and that now all 
        // CL contexts and command queues are ready to be used
        m_SinkReady.release();

        // Wait until Source thread is initialized
        if (!m_SourceReady.waitForObject(THREAD_WAIT_TIMEOUT))
        {
            m_bRunning = false;
        }

        while (m_bRunning)
        {
            // Receive frame from source and copy it into a cl_image
            // that is shared withGL
            m_pSink->processFrame();

            if (m_bNeedResize)
            {
                glViewport(0, 0, m_uiWindowWidth, m_uiWindowHeight); 

                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();

                gluPerspective(60.0f, (float)m_uiWindowWidth/(float)m_uiWindowHeight, 0.1f, 10.0f);
    
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
            }

            // Display received data
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glPushMatrix();

            // Adapt the aspect ratio of teh quad to the one of the texture
            glScalef(1.0f, (float)m_uiFrameHeight/(float)m_uiFrameWidth, 1.0f);
            glTranslatef(0.0f, 0.0f, -1.0f);

            glBindBuffer(GL_ARRAY_BUFFER, m_uiQuad);

            glBindTexture(GL_TEXTURE_2D, m_pSink->getTexture());

            glDrawArrays(GL_QUADS, 0, 4);
   
            glBindTexture(GL_TEXTURE_2D, 0);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glPopMatrix();

            m_pWindow->swapBuffers();
			
			processEvents(m_pWindow);
        }
    }

    m_pSink->release();

    if (m_SourceDone.waitForObject(THREAD_WAIT_TIMEOUT))
    {
        delete m_pSink; m_pSink=NULL;
    }

    glDeleteBuffers(1, &m_uiQuad);
    m_uiQuad = 0;

    m_pWindow->deleteContext();

    return true;
}



bool CLCopyApp::SourceLoop()
{
    // Wait until initialization is done
    if (!m_SinkReady.waitForObject(THREAD_WAIT_TIMEOUT))
    {
        m_SourceDone.release();

        // Init of Sink failed and we run into a timeout
        m_bRunning = false;

        return false;
    }

    // The Sink is done, now init source
    m_pSource = new CLSource(m_pclCtx[1], m_pclCmdQueue[1]);

    // Connect source and think through a consumer/producer queue
    m_pSource->setOutputQueue(m_pSink->getInputQueue());

    if (m_pSource->createStream(NUM_BUFFERS, m_uiFrameWidth, m_uiFrameHeight))
    {
        // Get bus addresses of the buffer that was allocated on teh sink GPU
        unsigned long long pSurfaceAddress[NUM_BUFFERS];
        unsigned long long pMarkerBusAddress[NUM_BUFFERS];

        for (unsigned int i = 0; i < NUM_BUFFERS; ++i)
        {
            pSurfaceAddress[i]   = m_pSink->getBufferBusAddress(i);
            pMarkerBusAddress[i] = m_pSink->getMarkerBusAddress(i);
        }

        // Pass bus addresses of buffer on sink GPU to the source, so that the source can write to this buffer
        if (!m_pSource->setRemoteMemory(NUM_BUFFERS, pSurfaceAddress, pMarkerBusAddress))
        {
            m_bRunning = false;
        }

        m_SourceReady.release();

        while (m_bRunning)
        {
            // transfer frame to the sink GPU
            m_pSource->processFrame();
        }
    }

    m_pSource->release();

    delete m_pSource;

    m_SourceDone.release();

    return true;
}



void CLCopyApp::setupGL()
{
    const float pVertices [] = { -0.5f, -0.5f, 0.0f,   0.5f, -0.5f, 0.0f,   0.5f, 0.5f, 0.0f,   -0.5f, 0.5f, 0.0f };
    const float pTexCoord [] = {  0.0f, 0.0f,          1.0f,  0.0f,         1.0f, 1.0f,          0.0f, 1.0f       };

     // GL init
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glShadeModel(GL_SMOOTH);

    glPolygonMode(GL_FRONT, GL_FILL);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    // Create quad to display the received texture data
    glGenBuffers(1, &m_uiQuad);

    glBindBuffer(GL_ARRAY_BUFFER, m_uiQuad);

    glBufferData(GL_ARRAY_BUFFER,    20 * sizeof(float), pVertices, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 12 * sizeof(float), 8 * sizeof(float),  pTexCoord);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, 0);
    glTexCoordPointer(2, GL_FLOAT, 0, ((char*)NULL + 12 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60.0f, (float)m_uiWindowWidth/(float)m_uiWindowHeight, 0.1f, 10.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



bool CLCopyApp::CheckSupportExtension(cl_device_id deviceID, const char* extensionAsked)
{
	size_t extensionSize;
	clGetDeviceInfo(deviceID, CL_DEVICE_EXTENSIONS,0,NULL, &extensionSize);

	char* extensions_pChar = (char*)malloc(extensionSize+1); //+1 for end null character
	clGetDeviceInfo(deviceID, CL_DEVICE_EXTENSIONS,extensionSize,extensions_pChar, &extensionSize);
	extensions_pChar[extensionSize] = '\0';
	std::string extensions_string = std::string(extensions_pChar);
	free(extensions_pChar); extensions_pChar=NULL;

	//convert to lower case
	std::string extensions_string_lowerCase;
	for(size_t j=0; j<extensions_string.length(); j++)
	{
		extensions_string_lowerCase.push_back(  (extensions_string[j] >= 'A' && extensions_string[j] <= 'Z') ? (extensions_string[j]-'A'+'a') : (extensions_string[j])   );
	}

	std::size_t pos = extensions_string_lowerCase.find(extensionAsked);
	if ( pos == std::string::npos )
	{
		return false;
	}
	return true; 

}


bool CLCopyApp::setupCL()
{
    cl_int          nStatus;
    cl_uint         uiNumPlatforms;
    cl_platform_id* pPlatforms;
    cl_platform_id  SelectedPlatform;

    ////////////////////////////////////////////
    // get Platform IDs, and search AMD platform

    nStatus = clGetPlatformIDs(0, NULL, &uiNumPlatforms);
    CL_CHECK_STATUS(nStatus);

    pPlatforms = new cl_platform_id[uiNumPlatforms];

    nStatus = clGetPlatformIDs(uiNumPlatforms, pPlatforms, NULL);
    CL_CHECK_STATUS(nStatus);

    // Loop through platforms and check if the vendor is equal to AMD
    for (unsigned int i = 0; i < uiNumPlatforms; ++i)
    {
        char pBuffer[128];

        nStatus = clGetPlatformInfo(pPlatforms[i], CL_PLATFORM_VENDOR, 128, pBuffer, NULL);
        CL_CHECK_STATUS(nStatus);

        if (strcmp(pBuffer, "Advanced Micro Devices, Inc.") == 0)
        {
            SelectedPlatform = pPlatforms[i];
            break;
        }
    }

    delete [] pPlatforms;

    if (SelectedPlatform == NULL)
    {
        return false;
    }

    /////////////////////////////////////////////////
    // create cl context that shares resources with GL

    // load required extensions
    clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn)clGetExtensionFunctionAddressForPlatform(SelectedPlatform, "clGetGLContextInfoKHR");
       
    clEnqueueWaitSignalAMD =          (clEnqueueWaitSignalAMD_fn)clGetExtensionFunctionAddressForPlatform(SelectedPlatform, "clEnqueueWaitSignalAMD");
    clEnqueueWriteSignalAMD =         (clEnqueueWriteSignalAMD_fn)clGetExtensionFunctionAddressForPlatform(SelectedPlatform, "clEnqueueWriteSignalAMD");
    clEnqueueMakeBuffersResidentAMD = (clEnqueueMakeBuffersResidentAMD_fn)clGetExtensionFunctionAddressForPlatform(SelectedPlatform, "clEnqueueMakeBuffersResidentAMD");

    if (!clGetGLContextInfoKHR || !clEnqueueWaitSignalAMD || !clEnqueueWriteSignalAMD || !clEnqueueMakeBuffersResidentAMD) 
    {
        return false;
    }

    cl_device_id *  pclDeviceList   = 0;
	cl_uint         nNumDevices     = 0;

	nStatus = clGetDeviceIDs(SelectedPlatform, CL_DEVICE_TYPE_GPU, 0, 0, &nNumDevices);
    CL_CHECK_STATUS(nStatus);

    if (nNumDevices < 2)
    {
        return false;
    }

    pclDeviceList = new cl_device_id[nNumDevices];

    nStatus = clGetDeviceIDs(SelectedPlatform, CL_DEVICE_TYPE_GPU, nNumDevices, pclDeviceList, &nNumDevices);
    CL_CHECK_STATUS(nStatus);

	for(cl_uint iDevice=0; iDevice<nNumDevices; iDevice++)
	{
		if ( !CheckSupportExtension(pclDeviceList[iDevice],"cl_amd_bus_addressable_memory") )
		{

			const char warningMessage[] = "ERROR, This demo will not run. You must activate DirectGMA. Read README.txt of this SDK";

			#if defined (WIN32)
			MessageBoxA(NULL,warningMessage,"DirectGMA",NULL);
			#else
			printf("%s\n",warningMessage);
			#endif

			return false;
		}
	}

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Create CL context for Sink, on the sink the context is shared with GL to display the result

    #if defined (WIN32)

    DC    hDC   = wglGetCurrentDC();
    GLCTX hGLRC = wglGetCurrentContext();

    cl_context_properties pProperties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties) SelectedPlatform,
                                            CL_GL_CONTEXT_KHR,   (cl_context_properties) hGLRC,
                                            CL_WGL_HDC_KHR,      (cl_context_properties) hDC,
                                            0 };

#elif defined (LINUX)

    DC     hDC   = glXGetCurrentDisplay();
    GLCTX  hGLRC = glXGetCurrentContext();
    
    cl_context_properties pProperties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties) SelectedPlatform,
                                            CL_GL_CONTEXT_KHR,   (cl_context_properties) hGLRC,
                                            CL_GLX_DISPLAY_KHR,  (cl_context_properties) hDC,
                                            0 };    
#endif


    if (!hDC || !hGLRC)
    {
        return false;
    }

    
    size_t uiNumDevices = 0;

    // Check if a cl device is available for the current GL context
    nStatus = clGetGLContextInfoKHR(pProperties, 
                                    CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
                                    0, 
                                    NULL, 
                                    &uiNumDevices);

    CL_CHECK_STATUS(nStatus);

    if (uiNumDevices == 0)
    {
        return false;
    }

    // get cl device associated with current GL context
    nStatus = clGetGLContextInfoKHR(pProperties, 
                                    CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
                                    sizeof(cl_device_id),
                                    &(m_pclDevId[0]),
                                    NULL);

    CL_CHECK_STATUS(nStatus);


    m_pclCtx[0] = clCreateContext(pProperties, 1, &(m_pclDevId[0]), NULL, NULL, &nStatus);
    CL_CHECK_STATUS(nStatus);


    m_pclCmdQueue[0] = clCreateCommandQueue(m_pclCtx[0], m_pclDevId[0], 0, &nStatus);
    CL_CHECK_STATUS(nStatus);


    //-------------------------------------------------------------------------------------------------
    // Create cl context for source. 
    //-------------------------------------------------------------------------------------------------
    for (unsigned int i = 0; i < nNumDevices; ++i)
    {
        if (pclDeviceList[i] != m_pclDevId[0])
        {
            // Get first device that is not used by sink context
            m_pclDevId[1] = pclDeviceList[i];
            break;
        }
    }

    cl_context_properties pSourceProperties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties) SelectedPlatform, 0};
    
    m_pclCtx[1] = clCreateContext(pSourceProperties, 1, &(m_pclDevId[1]), NULL, NULL, &nStatus);
    CL_CHECK_STATUS(nStatus);

    m_pclCmdQueue[1] = clCreateCommandQueue(m_pclCtx[1], m_pclDevId[1], 0, &nStatus);
    CL_CHECK_STATUS(nStatus);

    return true;

}   


void CLCopyApp::processEvents(GLWindow *pWin)
{
#if defined (LINUX)
    if (XPending(pWin->getDC()) > 0)
    {

        XEvent event;
        XNextEvent(pWin->getDC(), &event);

        switch (event.type)
        {
           case Expose:
             break;

			case ConfigureNotify:
                resize(event.xconfigure.width, event.xconfigure.height);
                break;
				
            case KeyPress:
            {
                 char buffer[10];
                 XLookupString(&event.xkey, buffer, sizeof(buffer), NULL, NULL);

                 // ESC key
                 if (buffer[0] == 27)
                     m_bRunning = false;

                 break;
            }
         }
     }
#endif
}
