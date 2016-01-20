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

#include "DXSinkWindow.h"

#include "DXcopyApp.h"

#include "BufferQueue.h"

DXSinkWindow::DXSinkWindow() : DXwindow()
{

	m_sink_pDiffuseTexture.clear();
	memset(&m_surfList_texture,0,sizeof(m_surfList_texture));

}

DXSinkWindow::~DXSinkWindow()
{
	
}

HRESULT DXSinkWindow::InitSDItexture(int nbBuffer)
{
	HRESULT hr = S_OK;

	m_sink_pDiffuseTexture.assign(nbBuffer,NULL);

	for (int i = 0; i < nbBuffer; i++)
    {
		D3D11_TEXTURE2D_DESC desc_texture;	
		memset(&desc_texture, 0 , sizeof(D3D11_TEXTURE2D_DESC));
		D3D11_SUBRESOURCE_DATA InitialData_texture;
		desc_texture.Width = DXwindow::m_width;
		desc_texture.Height = DXwindow::m_height;
		desc_texture.MipLevels = 1;
		desc_texture.ArraySize = 1;
		desc_texture.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
		desc_texture.SampleDesc.Count = 1;
		desc_texture.SampleDesc.Quality = 0;	
		desc_texture.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		InitialData_texture.SysMemPitch = desc_texture.Width;
		InitialData_texture.SysMemSlicePitch = 0;
		desc_texture.Usage = D3D11_USAGE_DEFAULT;
		InitialData_texture.pSysMem = &AmdDxSDIAlloc;
		hr = m_pd3dDevice->CreateTexture2D( &desc_texture, &InitialData_texture, &m_sink_pDiffuseTexture[i] );	
		PASS_TEST(hr);
	}


	memset(&m_surfList_texture,0,sizeof(m_surfList_texture));
	m_surfList_texture.numSurfaces = nbBuffer;
	m_surfList_texture.pInfo = (AmdDxSDIQueryAllocInfo*)malloc( sizeof(AmdDxSDIQueryAllocInfo)  * nbBuffer   ); 		
	for(int i=0; i<nbBuffer; i++)
	{
		m_surfList_texture.pInfo[i].pResource11 = m_sink_pDiffuseTexture[i];
		m_surfList_texture.pInfo[i].pResource10 = NULL;
		m_surfList_texture.pInfo[i].pInfo = (AmdDxSDISurfaceAttributes*)malloc( sizeof(AmdDxSDISurfaceAttributes)   );;
	}
	hr = m_pSDI->MakeResidentSDISurfaces(&m_surfList_texture);
	if( FAILED( hr ) )  {assert(((HRESULT)(hr)) >= 0); return E_FAIL;}
	

	return S_OK;
}


HRESULT DXSinkWindow::Render(BufferQueue* pSyncBuffer)
{

	DXcopyApp::FrameData* pFrame = NULL;
	unsigned int uiBufferIdx = pSyncBuffer->getBufferForReading((void*&)pFrame);


	BOOL WaitMarkerSuccess = m_pSDI->WaitMarker11(m_sink_pDiffuseTexture[uiBufferIdx], pFrame->uiTransferId);
	if ( !WaitMarkerSuccess ) { PASS_TEST(E_FAIL) }
 
	m_pImmediateContext->CopyResource(m_pBackBuffer, m_sink_pDiffuseTexture[uiBufferIdx]);
 
	// This FlushAndWaitForGPUIdle() call is necessary to make sure no transfer buffer being used by both source and sink GPU at same time
	FlushAndWaitForGPUIdle();
	pSyncBuffer->releaseReadBuffer();
	m_pSwapChain->Present(0,0);
	
	return S_OK;
}






