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

#include "DXcopyApp.h"

#include "DXSinkWindow.h"
#include "DXSourceWindow.h"
#include "BufferQueue.h"
#include "ADLtool.h"


DXcopyApp::DXcopyApp()
{
	m_SinkApp = NULL;
	m_SourceApp = NULL;

	m_uiTransferId = 0;

	m_SinkRun = false;
	m_SourceRun = false;

	m_pSyncBuffer = NULL;
}

DXcopyApp::~DXcopyApp()
{
	if (m_pSyncBuffer) { delete m_pSyncBuffer; m_pSyncBuffer=NULL; }
}

HRESULT DXcopyApp::Init(HINSTANCE hInstance, int nCmdShow)
{
	HRESULT hr = S_OK;


	//Get desktop positions
	unsigned int gpu0_uiDsp = ADLtool::getDisplayOnGPU(0, 0); 
	ADLtool::DisplayData* gpu0_pDsp = ADLtool::getDisplayData(gpu0_uiDsp);

	unsigned int gpu1_uiDsp = ADLtool::getDisplayOnGPU(1, 0); 
	ADLtool::DisplayData* gpu1_pDsp = ADLtool::getDisplayData(gpu1_uiDsp);

	if ( gpu0_pDsp == NULL || gpu1_pDsp == NULL )
	{
		MessageBoxA(NULL,"Error. For this demo, you must have 2 GPUs, and one monitor connected on each GPU.","demo DGMA",NULL);
		return E_FAIL;
	}


	m_SinkApp = new DXSinkWindow();
	m_SourceApp = new DXSourceWindow();

	if( FAILED(m_SinkApp->InitWindow( hInstance, nCmdShow , L"DX11DGMAWindowClass3_A",  L"Sink", 
		gpu1_pDsp->iOriginX + gpu1_pDsp->uiWidth/2 - DXwindow::m_width/2, 
		gpu1_pDsp->iOriginY
		)) ) 
	{ return E_FAIL; }
	
	if( FAILED(m_SourceApp->InitWindow( hInstance, nCmdShow , L"DX11DGMAWindowClass3_B",  L"Source", 
		gpu0_pDsp->iOriginX + gpu0_pDsp->uiWidth/2 - DXwindow::m_width/2, 
		gpu0_pDsp->iOriginY
		))  ) 
	{ return E_FAIL; }
	
	if( FAILED( m_SinkApp->InitDevice(NULL,D3D_DRIVER_TYPE_HARDWARE) )) { return E_FAIL; }

	   
	IDXGIDevice* pDXGIDevice = NULL;
    hr = m_SinkApp->GetD3D11Device()->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
    PASS_TEST(hr);

    IDXGIAdapter* pDXGIAdapter = NULL;
    hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter);
	PASS_TEST(hr);

    IDXGIFactory* pIDXGIFactory = NULL;
    pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void **)&pIDXGIFactory);

	UINT g_numAdapters = 0;
    IDXGIAdapter* pAdapter = NULL; 
    std::vector <IDXGIAdapter*> vAdapters; 
    while(pIDXGIFactory->EnumAdapters(g_numAdapters, &pAdapter) != DXGI_ERROR_NOT_FOUND) 
    {
	    vAdapters.push_back(pAdapter); 
	    ++g_numAdapters; 
    }

    // Get the device for the 2nd adapter (if one is present)
	IDXGIAdapter* adapter2 = NULL;
    if (2 == g_numAdapters)
    {
        adapter2 = vAdapters.at(1);
    }
	else
	{
		MessageBoxA(NULL,"You should have 2 monitors","demo DGMA",NULL);
	}



	if( FAILED( m_SourceApp->InitDevice(adapter2,D3D_DRIVER_TYPE_UNKNOWN) )) { return E_FAIL; }

	if( FAILED( m_SinkApp->InitSDIForDevice() )) { return E_FAIL; }
	if( FAILED( m_SourceApp->InitSDIForDevice() )) { return E_FAIL; }

	m_SinkApp->InitSDItexture(NUM_BUFFERS);

	m_SourceApp->InitSDItexture(m_SinkApp->GetSurfaceList());


	m_pSyncBuffer = new BufferQueue(NUM_BUFFERS);


	// Allocate memory to store frame data
    FrameData*  m_pFrameData = new FrameData[NUM_BUFFERS];
   
	if ( m_SinkApp->GetSurfaceList().numSurfaces != NUM_BUFFERS )
	{
		PASS_TEST(E_FAIL);
	}

    for (unsigned int i = 0; i < NUM_BUFFERS; i++)
    {
        m_pFrameData[i].uiTransferId        = 0;
        m_pFrameData[i].uiBufferId          = i;
        m_pFrameData[i].ullBufferBusAddress = m_SinkApp->GetSurfaceList().pInfo[i].pInfo->SurfaceBusAddr;
        m_pFrameData[i].ullMarkerBusAddress = m_SinkApp->GetSurfaceList().pInfo[i].pInfo->MarkerBusAddr;

        m_pSyncBuffer->setBufferMemory(i, (void*)(&m_pFrameData[i]));

    }

	return S_OK;
}


bool DXcopyApp::Start()
{

	m_SinkRun = true;
	m_SourceRun = true;


	m_SinkThread.create((THREAD_PROC)SinkThreadFunc,     this);
    m_SourceThread.create((THREAD_PROC)SourceThreadFunc, this);


	while (true)
	{
		
		MSG	 Msg;
		if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
		{
			if (Msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
		Sleep(100); // sleep since we do not want to spend too many cycles in the window message management
	}

	Stop();




	return true;
}


void DXcopyApp::Stop()
{

	m_SinkRun = false;
	m_SinkThread.join();

	m_SourceRun = false;
	m_pSyncBuffer->releaseReadBuffer();//free one buffer in order to be 100% sure that getBufferForWriting() of m_SourceThread will not be blocked
	m_SourceThread.join();


}


DWORD DXcopyApp::SourceThreadFunc(void* pArg)
{

    DXcopyApp* pApp = static_cast<DXcopyApp *>(pArg);

    pApp->SourceLoop();

    return 0;
}


DWORD DXcopyApp::SinkThreadFunc(void *pArg)
{

    DXcopyApp* pApp = static_cast<DXcopyApp *>(pArg);

    pApp->SinkLoop();

    return 0;
}

bool DXcopyApp::SinkLoop()
{  
	while (m_SinkRun)
    {
		m_SinkApp->Render(m_pSyncBuffer);
	}

	return true;

}

bool DXcopyApp::SourceLoop()
{  
	while (m_SourceRun)
    {
		m_uiTransferId++;
		m_SourceApp->Render(m_pSyncBuffer,m_uiTransferId);
	}

	return true;
}











