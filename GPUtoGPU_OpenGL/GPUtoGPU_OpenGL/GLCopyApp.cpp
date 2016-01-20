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

#include "defines.h"
#include "GLSource.h"
#include "GLSink.h"
#include "GLCopyApp.h"
#include "ADLtool.h"


PFNGLWAITMARKERAMDPROC          glWaitMarkerAMD;
PFNGLWRITEMARKERAMDPROC         glWriteMarkerAMD;
PFNGLMAKEBUFFERSRESIDENTAMDPROC glMakeBuffersResidentAMD;
PFNGLBUFFERBUSADDRESSAMDPROC    glBufferBusAddressAMD;


using namespace std;


GLCopyApp::GLCopyApp() :   

    m_bRunning(false),  m_bStarted(false), m_bResizeSource(false), m_bResizeSink(false), m_bUseP2P(false), m_bCanDoP2P(false),
m_uiBufferWidth(0), m_uiBufferHeight(0),    m_pSourceWin(NULL), m_pSinkWin(NULL), 
m_pSink(NULL), m_pSource(NULL)

{

}


GLCopyApp::~GLCopyApp()
{
    if (m_pSourceWin)
	{
        delete m_pSourceWin; m_pSourceWin=NULL;
	}

    if (m_pSinkWin)
	{
        delete m_pSinkWin; m_pSinkWin=NULL;
	}

	ADLtool::CleanClass();
}


bool GLCopyApp::init(unsigned int uiWidth, unsigned int uiHeight, const char *pClassName, std::string& strErrorMessage)
{
	strErrorMessage = "";

    // Create a window for source and sink
    m_pSourceWin = new GLWindow("GL Source", pClassName);
    m_pSinkWin   = new GLWindow("GL Sink",   pClassName);

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

    if ((ADLtool::getNumDisplaysOnGPU(0) == 0) || ADLtool::getNumDisplaysOnGPU(1) == 0)
    {
        strErrorMessage = "ERROR, Not all GPUs have a Display mapped!";
        return false;
    } 

    // Create window on first Display of GPU 0
    unsigned int uiDsp = ADLtool::getDisplayOnGPU(0, 0);
    
	if (! m_pSourceWin->create(uiWidth, uiHeight, uiDsp, false) )
	{
		strErrorMessage = "ERROR, Could not create GLWindow";
        return false;
	}
    m_pSourceWin->open();

    // create dummy context to load extensions
    m_pSourceWin->createContext();
    m_pSourceWin->makeCurrent();

    if (glewInit() != GLEW_OK)
    {
        strErrorMessage = "ERROR, glew init failed";
        return false;
    }

    // get number of extensions
    int i = 0;
    int nNumExtensions = 0;

    glGetIntegerv(GL_NUM_EXTENSIONS, &nNumExtensions);

    for (i = 0; i < nNumExtensions; ++i)
    {
        std::string strExt = (char*)glGetStringi(GL_EXTENSIONS, i);

		std::string strExt_lowerCase;
		for(unsigned int j=0; j<strExt.length(); j++)
		{
			strExt_lowerCase.push_back(  (strExt[j] >= 'A' && strExt[j] <= 'Z') ? (strExt[j]-'A'+'a') : (strExt[j])   );
		}

        if (strExt_lowerCase == "gl_amd_bus_addressable_memory")
        {
            break;
        }
    }

    if (i < nNumExtensions)
    {
        // failed to find extension
        m_bCanDoP2P = true;
    }
	else
	{
		const char warningMessage[] = "WARNING, This demo will run but it will not use DirectGMA. You should activate DirectGMA, read README.txt of this SDK.";
		
		#if defined (WIN32)
		MessageBoxA(NULL,warningMessage,"DirectGMA",NULL);
		#else
		printf("%s\n",warningMessage);
		#endif

	}

#if defined (WIN32)
    // Load functions of GL_AMD_bus_addressable_memory
    if (!glMakeBuffersResidentAMD)
        glMakeBuffersResidentAMD = (PFNGLMAKEBUFFERSRESIDENTAMDPROC) wglGetProcAddress("glMakeBuffersResidentAMD");

    if (!glBufferBusAddressAMD)
        glBufferBusAddressAMD = (PFNGLBUFFERBUSADDRESSAMDPROC) wglGetProcAddress("glBufferBusAddressAMD");

    if (!glWaitMarkerAMD)
        glWaitMarkerAMD = (PFNGLWAITMARKERAMDPROC) wglGetProcAddress("glWaitMarkerAMD");

    if (!glWriteMarkerAMD)
        glWriteMarkerAMD = (PFNGLWRITEMARKERAMDPROC) wglGetProcAddress("glWriteMarkerAMD");
#elif defined (LINUX)

    if (!glMakeBuffersResidentAMD)
        glMakeBuffersResidentAMD = (PFNGLMAKEBUFFERSRESIDENTAMDPROC) glXGetProcAddress((GLubyte*)"glMakeBuffersResidentAMD");

    if (!glBufferBusAddressAMD)
        glBufferBusAddressAMD = (PFNGLBUFFERBUSADDRESSAMDPROC) glXGetProcAddress((GLubyte*)"glBufferBusAddressAMD");

    if (!glWaitMarkerAMD)
        glWaitMarkerAMD = (PFNGLWAITMARKERAMDPROC) glXGetProcAddress((GLubyte*)"glWaitMarkerAMD");

    if (!glWriteMarkerAMD)
        glWriteMarkerAMD = (PFNGLWRITEMARKERAMDPROC) glXGetProcAddress((GLubyte*)"glWriteMarkerAMD");

#endif

    if (!(glMakeBuffersResidentAMD && glWaitMarkerAMD && glWriteMarkerAMD && glBufferBusAddressAMD))
    {
        strErrorMessage = "ERROR, Could not load Bus addressable memory functions!";
        return false;
    }

	//delete dummy context
    m_pSourceWin->deleteContext();

    // Create window on first Display of GPU 1
    uiDsp = ADLtool::getDisplayOnGPU(1, 0);

    m_uiBufferWidth  = uiWidth;
    m_uiBufferHeight = uiHeight;

    m_pSinkWin->create(uiWidth, uiHeight, uiDsp, false);
    m_pSinkWin->open();
   
    m_bRunning      = false;
    m_bResizeSink   = false;
    m_bResizeSource = false;

    m_pSource       = NULL;
    m_pSink         = NULL;

    // Create semaphore to indicate that the init of the Sink is ready
    m_SinkReady.create(0, 1); //    CreateSemaphore(NULL, 0, 1, NULL);
    // Create semaphore to indicate that the init of the source is done
    m_SourceReady.create(0, 1); // CreateSemaphore(NULL, 0, 1, NULL);
    // Create semaphore to indicate that the Source has terminated
    m_SourceDone.create(0, 1); // CreateSemaphore(NULL, 0, 1, NULL);

    return true;
}


bool GLCopyApp::start(bool bUseP2P)
{
    if (m_bRunning)
        return false;

    if (!m_pSourceWin || !m_pSinkWin)
        return false;

    m_bRunning = true;

    if (bUseP2P && m_bCanDoP2P)
        m_bUseP2P = true;

    m_SinkThread.create((THREAD_PROC)SinkThreadFunc,     this);
    m_SourceThread.create((THREAD_PROC)SourceThreadFunc, this);

    m_bStarted = true;

    return true;
}


void GLCopyApp::stop()
{
    if (m_bStarted)
    {
        m_bRunning = false;

        m_SinkThread.join();
        m_SourceThread.join();

        m_bStarted = false;
    }
}


DWORD GLCopyApp::SourceThreadFunc(void* pArg)
{
    GLCopyApp* pApp = static_cast<GLCopyApp *>(pArg);

    pApp->SourceLoop();

    return 0;
}


DWORD GLCopyApp::SinkThreadFunc(void *pArg)
{
    GLCopyApp* pApp = static_cast<GLCopyApp *>(pArg);

    pApp->SinkLoop();

    return 0;
}


bool GLCopyApp::SourceLoop()
{
    // The GLSink class that is created in SinkThread will allocate the buffers
    // the source needs to wait until the Sink is created and initialized.
    if (!m_SinkReady.waitForObject(THREAD_WAIT_TIMEOUT))
    {
        m_SourceDone.release();
        m_bRunning = false;
        
        return false;
    }

    // Get input buffer of Sink, this buffer will be used to synchronize
    // Sink and source
    BufferQueue* pBuffer = m_pSink->getInputBuffer();

    if (!pBuffer)
    {
        m_bRunning = false;
        
        return false;
    }

    // Create OpenGL context
    m_pSourceWin->createContext();
    m_pSourceWin->makeCurrent();

    m_pSource = new GLSource;

    m_pSource->initGL();
    m_pSource->resize(m_uiBufferWidth, m_uiBufferHeight);

    // get pinned memory data store allocated by Sink. In case of p2p this is NULL
    AlignedMem* pPinnedMemSinkBuffer = m_pSink->getPinnedMemory();

    // Create an external bus addressable memory buffer. Later memory allocated on the GPU on which
    // GLSink is running will be assigned to this buffer.
    if (m_pSource->createUpStream(m_uiBufferWidth, m_uiBufferHeight, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, m_bUseP2P, pPinnedMemSinkBuffer))
    {
        if (m_bUseP2P)
        {
            // Now the buffers on sink GPU are created and the addresses can be obtained
            unsigned long long* pBufferBusAddress = m_pSink->getBufferBusAddress();
            unsigned long long* pMarkerBusAddress = m_pSink->getMarkerBusAddress();

            // Pass the bus addresses of the buffer on the remote device to my local data source
            if (!m_pSource->setRemoteMemory(pBufferBusAddress, pMarkerBusAddress))
            {
                m_bRunning = false;
            }
        }

        // Set the input buffer of the Sink as output buffer of the Source
        // This is just a synchronization object to control when a given buffer object
        // can be reused
        m_pSource->setSyncBuffer(pBuffer);

        m_SourceReady.release(); 

        while (m_bRunning)
        {
            // The GLSource is always rendering into a FBO of a fixed size (g_uiWidth x g_uiHeight)
            // The FBO is mapped as texture into the viewport. A resize will only change the viewport
            // not the FBO.
            if (m_bResizeSource)
            {
                m_pSource->resize(m_pSourceWin->getWidth(), m_pSourceWin->getHeight());
                m_bResizeSource = false;
            }

            // Produces a frame and starts transfer to 
            // remote device on which GLSink is running
            m_pSource->draw();
            m_pSourceWin->swapBuffers();

            processEvents(m_pSourceWin);
        }    
    }

    m_bRunning = false;

    m_pSource->release();

    delete m_pSource; m_pSource=NULL;

    m_SourceDone.release();

    return true;
}


bool GLCopyApp::SinkLoop()
{  
    m_pSinkWin->createContext();
    m_pSinkWin->makeCurrent();

    m_pSink = new GLSink;

    m_pSink->initGL();
    m_pSink->resize(m_uiBufferWidth, m_uiBufferHeight);

    // Create bus addressable buffer on the GPU that the GLSink is running. This buffer will
    // be used as destination buffer for the frames produced by GLSource
    if (m_pSink->createDownStream(m_uiBufferWidth, m_uiBufferHeight, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, m_bUseP2P))
    {
         m_SinkReady.release();

        // Wait until Source thread is initialized
        if (!m_SourceReady.waitForObject(THREAD_WAIT_TIMEOUT))
        {
            m_bRunning = false;
        }
        
        while (m_bRunning)
        {
            // Resize only affects the window and viewport size. The received frames
            // are not affected they are still g_uiWidth x g_uiHeight.
            if (m_bResizeSink)
            {
                m_pSink->resize(m_pSinkWin->getWidth(), m_pSinkWin->getHeight());
                m_bResizeSink = false;
            }

            // Copies the data out of the bus addressable memory buffer into a 
            // texture object and draws a quad with this texture mapped
            m_pSink->draw();
            m_pSinkWin->swapBuffers();

            processEvents(m_pSinkWin);
        }   
    }

    m_bRunning = false;

    m_pSink->release();

    if (m_SourceDone.waitForObject(THREAD_WAIT_TIMEOUT))
    {
        delete m_pSink;
    }

    return true;
}




void GLCopyApp::processEvents(GLWindow *pWin)
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
