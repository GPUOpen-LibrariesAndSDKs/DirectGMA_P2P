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

#include "DXSourceWindow.h"

#include "DXcopyApp.h"

#include "BufferQueue.h"

#define D3DCOMPILE_ENABLE_STRICTNESS              (1 << 11)

HRESULT CompileShaderFromFile( WCHAR str[], LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;
	
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( str, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        SAFE_RELEASE( pErrorBlob );
        return hr;
    }
    SAFE_RELEASE( pErrorBlob );

    return S_OK;
}


DXSourceWindow::DXSourceWindow()
{
	m_clearColorR = 0.0f;
	m_clearColorG = 0.5f;
	m_clearColorB = 0.5f;

	m_source_SDItexture.clear();

	m_Cube_angle = 0.0f;
	m_Cube_pcb = NULL;  

	m_Cube_pVertexShader = NULL;
	m_Cube_pPixelShader = NULL;

	m_Cube_pVertexLayout11 = NULL;
	m_Cube_pVertexBuffer = NULL;
	m_Cube_pIndexBuffer = NULL;
	m_Cube_pRasterizerStateSolid = NULL;
	m_Cube_pTextureRV = NULL;
	m_Cube_pSamplerLinear = NULL;

}


DXSourceWindow::~DXSourceWindow()
{

	SAFE_RELEASE(m_Cube_pcb);
	SAFE_RELEASE(m_Cube_pVertexShader);
	SAFE_RELEASE(m_Cube_pPixelShader);
	SAFE_RELEASE(m_Cube_pVertexLayout11);
	SAFE_RELEASE(m_Cube_pVertexBuffer);
	SAFE_RELEASE(m_Cube_pIndexBuffer);
	SAFE_RELEASE(m_Cube_pRasterizerStateSolid);
	SAFE_RELEASE(m_Cube_pTextureRV);
	SAFE_RELEASE(m_Cube_pSamplerLinear);

}

HRESULT DXSourceWindow::Render(BufferQueue* pSyncBuffer,unsigned int uiTransferId)
{

	DXcopyApp::FrameData* pFrame = NULL;
    unsigned int uiBufferIdx = pSyncBuffer->getBufferForWriting((void*&)pFrame);

	//change the fill color
	float clearColorSource[4] = { m_clearColorR, m_clearColorG, m_clearColorB, 1.0f }; //red,green,blue,alpha

	//fill the backbuffer of the Source with 'clearColorR'
	m_pImmediateContext->ClearRenderTargetView( m_pBackBuffer_RTV, clearColorSource );
	m_pImmediateContext->ClearDepthStencilView( m_pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 ); 


	D3DXMATRIX mWorldViewProjection;
	D3DXMATRIX mWorldView;
    D3DXMATRIX mWorld;
    D3DXMATRIX mView;
    D3DXMATRIX mProj;
    
    // Set the per object constant data
	D3DXMatrixRotationAxis( &mWorld, &D3DXVECTOR3(1,1,1), m_Cube_angle  );
	
	m_Cube_angle += 0.04f * 0.01745f;
	
	if (m_Cube_angle >= 2.0f*3.1415926f) { m_Cube_angle = 0.0f; }

	D3DXVECTOR3 vEye( 0.0f, 0.0f, 2.0f );
	D3DXVECTOR3 vLook( 0.0f,0.0f,0.0f ); 
	D3DXVECTOR3 vUp( 0.0f,1.0f,0.0f );
	D3DXMatrixLookAtLH( &mView, &vEye, &vLook, &vUp );
	
	D3DXMatrixPerspectiveFovLH(&mProj, 3.1415926f/4.0f,  (float)DXwindow::m_width/(float)DXwindow::m_height,    0.1f, 10.0f);
	
	mWorldViewProjection = mWorld * mView * mProj;
	mWorldView			 = mWorld * mView;

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	m_pImmediateContext->Map( m_Cube_pcb, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	CUBE_CB* pCB = ( CUBE_CB* )MappedResource.pData;
	D3DXMatrixTranspose( &pCB->f4x4WorldViewProjection, &mWorldViewProjection );
	D3DXMatrixTranspose( &pCB->f4x4WorldView, &mWorldView );
	m_pImmediateContext->Unmap( m_Cube_pcb, 0 );

	m_pImmediateContext->VSSetConstantBuffers( 0, 1, &m_Cube_pcb );
	m_pImmediateContext->PSSetConstantBuffers( 0, 1, &m_Cube_pcb );

	m_pImmediateContext->VSSetShader( m_Cube_pVertexShader, NULL, 0 );
	m_pImmediateContext->PSSetShader( m_Cube_pPixelShader, NULL, 0 );

	m_pImmediateContext->IASetInputLayout( m_Cube_pVertexLayout11 );
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT Stride = sizeof( CUBE_Vertex1 );
	UINT Offset = 0;
	m_pImmediateContext->IASetVertexBuffers( 0, 1, &m_Cube_pVertexBuffer, &Stride, &Offset );
	m_pImmediateContext->IASetIndexBuffer( m_Cube_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );
	m_pImmediateContext->PSSetShaderResources( 0, 1, &m_Cube_pTextureRV );
	m_pImmediateContext->PSSetSamplers( 0, 1, &m_Cube_pSamplerLinear );
	m_pImmediateContext->DrawIndexed( 36, 0, 0 );


	// update frame data
	pFrame->uiTransferId = uiTransferId;

	m_pImmediateContext->CopyResource(m_source_SDItexture[uiBufferIdx], m_pBackBuffer );
	AmdDxMarkerInfo   m_pInfo;
	m_pInfo.markerValue = pFrame->uiTransferId;
	m_pInfo.markerOffset = pFrame->ullMarkerBusAddress & 0xfff;
 
	BOOL WriteMarkerSuccess = m_pSDI->WriteMarker11(m_source_SDItexture[uiBufferIdx], &m_pInfo);
	if ( !WriteMarkerSuccess ) { PASS_TEST(E_FAIL) }
 
	// This Flush() call is not necessary but with this one, the performance will be much better.
	m_pImmediateContext->Flush();
	pSyncBuffer->releaseWriteBuffer();
	m_pSwapChain->Present(0,0);


	return S_OK;
}

HRESULT DXSourceWindow::InitSDItexture(const AmdDxLocalSDISurfaceList& m_surfList_texture)
{
	HRESULT hr = S_OK;


	m_source_SDItexture.assign(m_surfList_texture.numSurfaces,NULL);

	for(UINT i=0; i<m_surfList_texture.numSurfaces; i++)
	{
		D3D11_TEXTURE2D_DESC desc_rt;	
		memset(&desc_rt, 0 , sizeof(D3D11_TEXTURE2D_DESC));
		desc_rt.Width = DXwindow::m_width;
		desc_rt.Height = DXwindow::m_height;
		desc_rt.MipLevels = 1;
		desc_rt.ArraySize = 1;
		desc_rt.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
		desc_rt.SampleDesc.Count = 1;
		desc_rt.SampleDesc.Quality = 0;	
		desc_rt.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; //  def:D3D11_BIND_SHADER_RESOURCE 
		desc_rt.Usage =  D3D11_USAGE_DEFAULT;
		desc_rt.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;  // def:0

		D3D11_SUBRESOURCE_DATA InitialData_renderTarg;
		memset(&InitialData_renderTarg, 0 , sizeof(InitialData_renderTarg));
		InitialData_renderTarg.pSysMem = &AmdDxRemoteSDIAlloc;
		InitialData_renderTarg.SysMemSlicePitch = 0;
		InitialData_renderTarg.SysMemPitch = DXwindow::m_width;

	
		hr = m_pd3dDevice->CreateTexture2D( &desc_rt, &InitialData_renderTarg, &m_source_SDItexture[i] );
		PASS_TEST(hr);
	}

	AmdDxSDIAdapterSurfaceInfo* pAdapterSurfInfo = (AmdDxSDIAdapterSurfaceInfo*)malloc(sizeof(AmdDxSDIAdapterSurfaceInfo) * m_surfList_texture.numSurfaces);
	
	for(UINT i=0; i<m_surfList_texture.numSurfaces; i++)
	{
		memset(&pAdapterSurfInfo[i],0,sizeof(AmdDxSDIAdapterSurfaceInfo));

		pAdapterSurfInfo[i].surfAttrib.MarkerBusAddr = m_surfList_texture.pInfo[i].pInfo->MarkerBusAddr    & ~0xfff; // & ~0xfff: make sure the marker address is aligned.
		pAdapterSurfInfo[i].surfAttrib.SurfaceBusAddr = m_surfList_texture.pInfo[i].pInfo->SurfaceBusAddr;
		pAdapterSurfInfo[i].pResource11 = m_source_SDItexture[i];
		pAdapterSurfInfo[i].pResource10 = NULL;
		pAdapterSurfInfo[i].sizeOfSurface = DXwindow::m_width*DXwindow::m_height*4;
		pAdapterSurfInfo[i].markerSize = 0x1000;

	}

	for(UINT i=0; i<m_surfList_texture.numSurfaces; i++)
	{
		AmdDxRemoteSDISurfaceList surfList_renderTarg;
		memset(&surfList_renderTarg,0,sizeof(surfList_renderTarg));
		surfList_renderTarg.numSurfaces = 1;
		surfList_renderTarg.pInfo = &(pAdapterSurfInfo[i]);

		//Create remote surfaces
		hr = m_pSDI->CreateSDIAdapterSurfaces(&surfList_renderTarg);
		PASS_TEST(hr);
	}

	hr = CreateShaderResources();
	PASS_TEST(hr);

	return S_OK;

}

HRESULT DXSourceWindow::CreateShaderResources()
{
	HRESULT hr = S_OK;
		
	// Set up constant buffer
	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;    
	Desc.ByteWidth = sizeof( CUBE_CB );
	hr = m_pd3dDevice->CreateBuffer( &Desc, NULL, &m_Cube_pcb );
	PASS_TEST( hr );

	// create vertex buffer
	const float pVertices[] = { -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
                                 0.5f, -0.5f,  0.5f,   0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,   0.5f,  0.5f,  0.5f,
                                -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,
                                -0.5f, -0.5f,  0.5f,  -0.5f, -0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,  -0.5f,  0.5f,  0.5f,
                                -0.5f,  0.5f,  0.5f,   0.5f,  0.5f,  0.5f,   0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,
                                -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f, -0.5f, -0.5f,  -0.5f, -0.5f, -0.5f };

	const float pNormal[] = {   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
                                1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,
                                -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
                                0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f };

	const float pTexture[] =  { 1.0f,0.0f,   1.0f,1.0f,    0.0f,1.0f,    0.0f,0.0f,
                                1.0f,0.0f,   1.0f,1.0f,    0.0f,1.0f,    0.0f,0.0f,
                                1.0f,0.0f,   1.0f,1.0f,    0.0f,1.0f,    0.0f,0.0f,
                                1.0f,0.0f,   1.0f,1.0f,    0.0f,1.0f,    0.0f,0.0f,
                                1.0f,0.0f,   1.0f,1.0f,    0.0f,1.0f,    0.0f,0.0f,
                                1.0f,0.0f,   1.0f,1.0f,    0.0f,1.0f,    0.0f,0.0f};

    const float pColors[]   = { 1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
                                1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
                                0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,
                                1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f };



	CUBE_Vertex1 verts[24];
	for (int i=0; i<24; i++)
	{
		verts[i].pos = D3DXVECTOR3(pVertices[3*i], pVertices[3*i+1], pVertices[3*i+2]);
		verts[i].texture = D3DXVECTOR2(pTexture[2*i], pTexture[2*i+1] );
		verts[i].normal = D3DXVECTOR3(pNormal[3*i], pNormal[3*i+1], pNormal[3*i+2] );
		verts[i].color = D3DXVECTOR3(pColors[3*i], pColors[3*i+1], pColors[3*i+2]);
	}

	D3D11_BUFFER_DESC vbDesc    = {0};
	vbDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER; // Type of resource
	vbDesc.ByteWidth            = 24*sizeof(CUBE_Vertex1);       // size in bytes
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vbData = {0};
	vbData.pSysMem = verts;

	hr = m_pd3dDevice->CreateBuffer( &vbDesc, &vbData, &m_Cube_pVertexBuffer);
    PASS_TEST(hr);

	// create index buffer
	int indices[36] = {-1};
	for (int i=0; i<6; i++)
	{
		indices[6*i]=4*i; indices[6*i+1]=4*i+1; indices[6*i+2]=4*i+3;
		indices[6*i+3]=4*i+1; indices[6*i+4]=4*i+2; indices[6*i+5]=4*i+3;
	}
	vbDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    vbDesc.ByteWidth            = 36*sizeof(unsigned int);
	vbData.pSysMem = indices;
	hr = m_pd3dDevice->CreateBuffer( &vbDesc, &vbData, &m_Cube_pIndexBuffer );
	PASS_TEST(hr);


	// Compile the shaders using the lowest possible profile for broadest feature level support
	ID3DBlob* pVertexShaderBuffer = NULL;
	hr = CompileShaderFromFile(L"DrawCube.hlsl", "VSMain", "vs_4_0_level_9_1", &pVertexShaderBuffer );
	if ( hr == D3D11_ERROR_FILE_NOT_FOUND ) 
	{ 
		MessageBoxA(NULL,"ERROR, file DrawCube.hlsl not found.","DirectGMA",NULL);
		return D3D11_ERROR_FILE_NOT_FOUND;
	}
	PASS_TEST(hr);
	hr = m_pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(), pVertexShaderBuffer->GetBufferSize(), NULL, &m_Cube_pVertexShader );
	PASS_TEST(hr);

	ID3DBlob* pPixelShaderBuffer = NULL;
	hr = CompileShaderFromFile( L"DrawCube.hlsl", "PSMain", "ps_4_0_level_9_1", &pPixelShaderBuffer );
	PASS_TEST(hr);
	hr = m_pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(), pPixelShaderBuffer->GetBufferSize(), NULL, &m_Cube_pPixelShader );
	PASS_TEST(hr);

	// Create our vertex input layout
    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE",  0,  DXGI_FORMAT_R32G32_FLOAT,      0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",  0,   DXGI_FORMAT_R32G32B32_FLOAT,   0, 20,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",  0,    DXGI_FORMAT_R32G32B32_FLOAT,   0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = m_pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pVertexShaderBuffer->GetBufferPointer(), pVertexShaderBuffer->GetBufferSize(), &m_Cube_pVertexLayout11 );
	PASS_TEST(hr);
    SAFE_RELEASE( pVertexShaderBuffer );
    SAFE_RELEASE( pPixelShaderBuffer );

	
    // Create a sampler state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = m_pd3dDevice->CreateSamplerState( &sampDesc, &m_Cube_pSamplerLinear );
	PASS_TEST(hr);


	// Set the raster state
	D3D11_RASTERIZER_DESC RasterizerDesc;
	RasterizerDesc.FillMode = D3D11_FILL_SOLID;
	RasterizerDesc.CullMode = D3D11_CULL_NONE;
	RasterizerDesc.FrontCounterClockwise = TRUE;
	RasterizerDesc.DepthBias = 0;
	RasterizerDesc.DepthBiasClamp = 0.0f;
	RasterizerDesc.SlopeScaledDepthBias = 0.0f;
	RasterizerDesc.DepthClipEnable = TRUE;
	RasterizerDesc.ScissorEnable = FALSE;
	RasterizerDesc.MultisampleEnable = FALSE;
	RasterizerDesc.AntialiasedLineEnable = FALSE;

	hr = m_pd3dDevice->CreateRasterizerState( &RasterizerDesc, &m_Cube_pRasterizerStateSolid );
	PASS_TEST(hr);

	// Set state Objects
	m_pImmediateContext->RSSetState( m_Cube_pRasterizerStateSolid );


	hr = D3DX11CreateShaderResourceViewFromFile( m_pd3dDevice, L"amdlogo.dds", NULL, NULL, &m_Cube_pTextureRV, NULL );
	if ( hr == D3D11_ERROR_FILE_NOT_FOUND ) 
	{ 
		MessageBoxA(NULL,"ERROR, file amdlogo.dds not found.","DirectGMA",NULL);
		return D3D11_ERROR_FILE_NOT_FOUND;
	}
	PASS_TEST(hr);


	return S_OK;
}





