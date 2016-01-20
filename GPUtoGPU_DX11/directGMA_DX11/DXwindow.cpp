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

#include "DXwindow.h"


const int DXwindow::m_width = 1280;
const int DXwindow::m_height = 720;


DXwindow::DXwindow()
{
	m_pSDI = NULL;
	m_hInst = NULL;
	m_hWnd = NULL;
	m_pSwapChain = NULL;
	m_pd3dDevice = NULL;
	m_pImmediateContext = NULL;
	m_pBackBuffer = NULL;
	m_pBackBuffer_RTV = NULL;
	m_pDSV = NULL;
	m_pEventQuery_ = NULL;
}

DXwindow::~DXwindow()
{
	SAFE_RELEASE(m_pDSV);
	SAFE_RELEASE(m_pEventQuery_);
}



LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

		case WM_CLOSE:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

HRESULT DXwindow::InitWindow( HINSTANCE hInstance, int nCmdShow , LPCWSTR className, LPCWSTR wndName, int posX,int posY )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = className;
    wcex.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    m_hInst = hInstance;
    RECT rc = { 0, 0, m_width, m_height };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    m_hWnd = CreateWindow( wcex.lpszClassName, wndName, WS_OVERLAPPEDWINDOW,
							posX,posY,
						   rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !m_hWnd )
        return E_FAIL;

    ShowWindow( m_hWnd, nCmdShow );

    return S_OK;
}


HRESULT DXwindow::InitDevice(IDXGIAdapter* adapter,D3D_DRIVER_TYPE driverType)
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( m_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 2;//1
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL    featureLevel = D3D_FEATURE_LEVEL_11_0;

    hr = D3D11CreateDeviceAndSwapChain( 
		adapter,
		driverType,
		NULL, 
		D3D11_CREATE_DEVICE_SINGLETHREADED, 
		NULL,
		0,
        D3D11_SDK_VERSION, 
		&sd, 
		&m_pSwapChain, 
		&m_pd3dDevice, 
		&featureLevel, 
		&m_pImmediateContext );


    if( FAILED( hr ) )
	{
        return hr;
	}

    // Create a render target view
    m_pBackBuffer = NULL;
    hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&m_pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = m_pd3dDevice->CreateRenderTargetView( m_pBackBuffer, NULL, &m_pBackBuffer_RTV );

    if( FAILED( hr ) )
        return hr;

	// Create depth stencil texture
    ID3D11Texture2D* pDepthStencil = NULL;
    D3D11_TEXTURE2D_DESC descDepth;
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D16_UNORM;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = m_pd3dDevice->CreateTexture2D( &descDepth, NULL, &pDepthStencil );
	CHECK_SUCCESS(hr,"pd3dDevice->CreateTexture2D");

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
	descDSV.Flags = 0;
    hr = m_pd3dDevice->CreateDepthStencilView( pDepthStencil, &descDSV,&m_pDSV );
	CHECK_SUCCESS(hr,"pd3dDevice->CreateDepthStencilView");
    if(pDepthStencil)
		pDepthStencil->Release();
    PASS_TEST( hr );

	m_pImmediateContext->OMSetRenderTargets( 1, &m_pBackBuffer_RTV, m_pDSV); 
	

    // Set up the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pImmediateContext->RSSetViewports( 1, &vp );


	D3D11_QUERY_DESC pQueryDesc;
	pQueryDesc.Query = D3D11_QUERY_EVENT;
	pQueryDesc.MiscFlags = 0;
	PASS_TEST(m_pd3dDevice->CreateQuery(&pQueryDesc, &m_pEventQuery_));


    return S_OK;
}

HRESULT DXwindow::InitSDIForDevice()
{
	IAmdDxExt* pExt = NULL;
    IAmdDxExtSDI* pLocalSDI = NULL;
    HRESULT hr = S_OK;
	m_pSDI = NULL;

	HMODULE hDrvDll = GetModuleHandleW(L"atidxx32.dll");
	if ( hDrvDll == NULL )
	{
		hDrvDll = GetModuleHandleW(L"atidxx64.dll");
	}

	if (hDrvDll != NULL)
	{
		PFNAmdDxExtCreate11 pAmdDxExtCreate = reinterpret_cast<PFNAmdDxExtCreate11>(GetProcAddress(hDrvDll, "AmdDxExtCreate11"));
		if (pAmdDxExtCreate != NULL)
		{			
			hr = pAmdDxExtCreate(m_pd3dDevice, &pExt);
			CHECK_SUCCESS(hr,"pAmdDxExtCreate");
			if (pExt != NULL)
			{		
				pLocalSDI = static_cast<IAmdDxExtSDI*>((pExt)->GetExtInterface(AmdDxExtSDIID));

				if ( pLocalSDI == NULL )
				{
					MessageBoxA(NULL,"Error, you must activate DirectGMA with AMD Control Center -> AMD Firepro -> SDI/DirectGMA","DirectGMA",NULL);
					return E_FAIL;
				}
				
			}
		}
	}

	m_pSDI = pLocalSDI;

	return S_OK;
}




void DXwindow::FlushAndWaitForGPUIdle()
{

    // Add an end marker to the command buffer queue.  
	if(m_pEventQuery_)
	{
		UINT queryData;
		m_pImmediateContext->End(m_pEventQuery_);
		// Force the driver to execute the commands from the command buffer.
		// Empty the command buffer and wait until the GPU is idle.
		while(m_pImmediateContext->GetData(m_pEventQuery_, &queryData, sizeof(queryData), 0) != S_OK) {}
	}
	else
	{
		assert(0);
	}
	
}