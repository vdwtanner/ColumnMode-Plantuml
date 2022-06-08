#include "pch.h"

#include <math.h>

#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <iostream>
#include <fstream>
#include <Shlwapi.h>
#include <shcore.h>
#include <windowsx.h>

#include "D2DPreviewRenderer.h"

using namespace CMPlantuml;

#pragma region helpers

static D2D1_SIZE_U GetWindowSize(HWND hwnd)
{
	RECT clientRect{};
	VerifyBool(GetClientRect(hwnd, &clientRect));

	UINT32 windowWidth = clientRect.right - clientRect.left;
	UINT32 windowHeight = clientRect.bottom - clientRect.top;

	return D2D1::SizeU(windowWidth, windowHeight);
}


#pragma endregion

D2DPreviewRenderer::D2DPreviewRenderer() :
	m_hwnd(NULL)
{
	CreateDeviceIndependentResources();
}

void D2DPreviewRenderer::Init(HWND hwnd)
{
	ReleaseDeviceDependentResources();
	m_hwnd = hwnd;
	CreateDeviceDependentResources();
}

void D2DPreviewRenderer::CreateDeviceIndependentResources()
{
	// Create a Direct2D factory.
	D2D1_FACTORY_OPTIONS factoryOptions = {};
	factoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	VerifyHR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory6), &factoryOptions, &m_d2dFactory));

	VerifyHR(CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_wicImagingFactory)));
}

void D2DPreviewRenderer::CreateDeviceDependentResources()
{
	auto windowSize = GetWindowSize(m_hwnd);
	m_viewportSize.height = windowSize.height;
	m_viewportSize.width = windowSize.width;

	
	m_paddedViewportSize.width = m_viewportSize.width - m_padding;
	m_paddedViewportSize.height = m_viewportSize.height - m_padding;

	DXGI_SWAP_CHAIN_DESC swapChainDescription = {};
	swapChainDescription.BufferCount = 2;
	swapChainDescription.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDescription.BufferDesc.Width = windowSize.width;
	swapChainDescription.BufferDesc.Height = windowSize.height;
	swapChainDescription.OutputWindow = m_hwnd;
	swapChainDescription.Windowed = TRUE;
	swapChainDescription.SampleDesc.Count = 1;
	swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	ComPtr<ID3D11Device> d3dDevice;

	UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	VerifyHR(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		deviceFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&swapChainDescription,
		&m_swapChain,
		&d3dDevice,
		nullptr,
		nullptr));

	ComPtr<IDXGIDevice> dxgiDevice;
	VerifyHR(d3dDevice.As(&dxgiDevice));

	VerifyHR(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));

	VerifyHR(m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_deviceContext));

	SetTargetToBackBuffer();
}

void D2DPreviewRenderer::ReleaseDeviceDependentResources()
{
	m_imageSize = D2D1::SizeU(0, 0);
	m_imageSource.Reset();
	m_scaledImage.Reset();
	m_outputEffect.Reset();
	m_swapChain.Reset();
	m_d2dDevice.Reset();
	m_deviceContext.Reset();
}

void D2DPreviewRenderer::SetTargetToBackBuffer()
{
	ComPtr<IDXGISurface> swapChainBackBuffer;
	VerifyHR(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainBackBuffer)));

	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));

	ComPtr<ID2D1Bitmap1> image;
	VerifyHR(m_deviceContext->CreateBitmapFromDxgiSurface(swapChainBackBuffer.Get(), bitmapProperties, &image));

	m_deviceContext->SetTarget(image.Get());
}

void D2DPreviewRenderer::Zoom(float deltaZoom)
{
	m_zoom = min(
		m_zoomMax,
		max(m_zoomMin, m_zoom + deltaZoom)
	);
	UpdateZoomState();
}

void D2DPreviewRenderer::SetPosition(D2D1_VECTOR_2F newPos)
{
	m_previewPos = newPos;
}

void D2DPreviewRenderer::UpdateZoomState()
{
	// When using ID2D1ImageSource, the recommend method of scaling is to use
	// ID2D1TransformedImageSource. It is inexpensive to recreate this object.
	D2D1_TRANSFORMED_IMAGE_SOURCE_PROPERTIES props =
	{
		D2D1_ORIENTATION_DEFAULT,
		m_zoom,
		m_zoom,
		D2D1_INTERPOLATION_MODE_LINEAR, // This is ignored when using DrawImage.
		D2D1_TRANSFORMED_IMAGE_SOURCE_OPTIONS_NONE
	};

	VerifyHR(m_deviceContext->CreateTransformedImageSource(
		m_imageSource.Get(),
		&props,
		&m_scaledImage));
}

void D2DPreviewRenderer::ResetPreview()
{
	if (m_imageSize.width > 0 && m_imageSize.width > 0)
	{
		m_zoom = min(
			min(m_paddedViewportSize.width / (float)m_imageSize.width, m_paddedViewportSize.height / (float)m_imageSize.height),
			m_zoomMax);

		m_previewPos.x = .5f * (m_viewportSize.width - (m_zoom * m_imageSize.width));
		m_previewPos.y = .5f * (m_viewportSize.height - (m_zoom * m_imageSize.height));

		UpdateZoomState();
	}
}

bool D2DPreviewRenderer::LoadPreviewImage(std::filesystem::path path, bool needToReset)
{
	std::scoped_lock lock(m_imageMutex);
	// Let WIC handle file loading and handling the various formats	
	ComPtr<IWICBitmapDecoder> decoder;
	VerifyHR(m_wicImagingFactory->CreateDecoderFromFilename(
		path.c_str(),
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&decoder));
	
	ComPtr<IWICBitmapFrameDecode> frame;
	VerifyHR(decoder->GetFrame(0, &frame));

	//Convert to D2D format image
	ComPtr<IWICFormatConverter> formatConverter;
	VerifyHR(m_wicImagingFactory->CreateFormatConverter(&formatConverter));
	VerifyHR(formatConverter->Initialize(
		frame.Get(),
		GUID_WICPixelFormat32bppPBGRA, //guarenteed to be supported on all hardware
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeCustom));

	VerifyHR(formatConverter->GetSize(&m_imageSize.width, &m_imageSize.height));

	// Attempt to read the embedded color profile from the image; use sRGB if the data doesn't exist.
	UINT actualCount;
	VerifyHR(m_wicImagingFactory->CreateColorContext(&m_wicColorContext));
	VerifyHR(frame->GetColorContexts(1, m_wicColorContext.GetAddressOf(), &actualCount));

	if (actualCount == 0)
	{
		m_wicColorContext->InitializeFromExifColorSpace(1); // 1 = sRGB.
	}

	m_deviceContext->CreateImageSourceFromWic(formatConverter.Get(),D2D1_IMAGE_SOURCE_LOADING_OPTIONS_RELEASE_SOURCE, &m_imageSource);
	
	if (needToReset)
	{
		ResetPreview();
	}

	UpdateZoomState();

	return true;
}

void D2DPreviewRenderer::Draw()
{
	m_deviceContext->BeginDraw();

	m_deviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightGray));
	//SetPos
	D2D1_MATRIX_3X2_F transform = D2D1::Matrix3x2F::Translation(m_previewPos.x, m_previewPos.y);

	//SetScale
	transform = transform * D2D1::Matrix3x2F::Scale(1, 1);

	m_deviceContext->SetTransform(transform);

	std::unique_lock<std::mutex> lock(m_imageMutex, std::try_to_lock);
	if (m_imageSize.width > 0 && lock)
	{
		m_deviceContext->DrawImage(m_scaledImage.Get(), D2D1_INTERPOLATION_MODE::D2D1_INTERPOLATION_MODE_LINEAR);
	}

	// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
	// is lost. It will be handled during the next call to Present.
	HRESULT hr = m_deviceContext->EndDraw();
	if (hr != D2DERR_RECREATE_TARGET)
	{
		VerifyHR(hr);
	}

	m_swapChain->Present(1,0);
}

void D2DPreviewRenderer::OnResize()
{
	m_deviceContext->SetTarget(nullptr);

	auto windowSize = GetWindowSize(m_hwnd);
	m_viewportSize.height = windowSize.height;
	m_viewportSize.width = windowSize.width;

	VerifyHR(m_swapChain->ResizeBuffers(2, windowSize.width, windowSize.height, DXGI_FORMAT_B8G8R8A8_UNORM, 0));

	SetTargetToBackBuffer();
}