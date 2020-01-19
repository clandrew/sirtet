#include "stdafx.h"
#include "Graphics.h"

void VerifyHR(HRESULT hr)
{
	if (FAILED(hr))
		__debugbreak();
}

void VerifyBool(BOOL b)
{
	if (!b)
		__debugbreak();
}

void Graphics::Initialize(HWND hwnd)
{
	D2D1_FACTORY_OPTIONS factoryOptions = {};
	factoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	VerifyHR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &factoryOptions, &m_d2dFactory));

	RECT clientRect;
	VerifyBool(GetClientRect(hwnd, &clientRect));

	D2D1_HWND_RENDER_TARGET_PROPERTIES hwndRenderTargetProperties = D2D1::HwndRenderTargetProperties(
		hwnd, D2D1::SizeU(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top));

	D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), 0, 0);

	VerifyHR(m_d2dFactory->CreateHwndRenderTarget(&renderTargetProperties, &hwndRenderTargetProperties, &m_renderTarget));

	VerifyHR(m_renderTarget->CreateCompatibleRenderTarget(D2D1::SizeF(128, 112), &m_native));

	m_bg = LoadImageFile(L"Images\\bg.png");
	m_blocks = LoadImageFile(L"Images\\blocks.png");
}

void Graphics::EnsureWicImagingFactory()
{
	if (m_wicImagingFactory)
		return;

	VerifyHR(CoInitialize(nullptr));

	VerifyHR(CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory,
		(LPVOID*)& m_wicImagingFactory));
}


ComPtr<ID2D1Bitmap1> Graphics::LoadImageFile(std::wstring fileName)
{
	EnsureWicImagingFactory();

	ComPtr<IWICBitmapDecoder> spDecoder;
	if (FAILED(m_wicImagingFactory->CreateDecoderFromFilename(
		fileName.c_str(),
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad, &spDecoder)))
	{
		return nullptr;
	}

	ComPtr<IWICBitmapFrameDecode> spSource;
	if (FAILED(spDecoder->GetFrame(0, &spSource)))
	{
		return nullptr;
	}

	// Convert the image format to 32bppPBGRA, equiv to DXGI_FORMAT_B8G8R8A8_UNORM
	ComPtr<IWICFormatConverter> spConverter;
	if (FAILED(m_wicImagingFactory->CreateFormatConverter(&spConverter)))
	{
		return nullptr;
	}

	if (FAILED(spConverter->Initialize(
		spSource.Get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.f,
		WICBitmapPaletteTypeMedianCut)))
	{
		return nullptr;
	}

	UINT width, height;
	if (FAILED(spConverter->GetSize(&width, &height)))
	{
		return nullptr;
	}

	std::vector<UINT> buffer;
	buffer.resize(width * height);
	if (FAILED(spConverter->CopyPixels(
		NULL,
		width * sizeof(UINT),
		buffer.size() * sizeof(UINT),
		reinterpret_cast<BYTE*>(buffer.data()))))
	{
		return nullptr;
	}

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = 
		D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
	
	ComPtr<ID2D1DeviceContext> deviceContext;
	if (FAILED(m_renderTarget.As(&deviceContext)))
	{
		return nullptr; // Maybe check here for OS ver
	}

	ComPtr<ID2D1Bitmap1> result;
	if (FAILED(deviceContext->CreateBitmap(
		D2D1::SizeU(width, height),
		buffer.data(),
		width * sizeof(UINT),
		bitmapProperties,
		&result)))
	{
		return nullptr;
	}

	return result;
}

void Graphics::Resize(HWND hwnd)
{
	RECT clientRect;
	VerifyBool(GetClientRect(hwnd, &clientRect));

	D2D1_SIZE_U newSize{};
	newSize.width = clientRect.right - clientRect.left;
	newSize.height = clientRect.bottom - clientRect.top;

	VerifyHR(m_renderTarget->Resize(newSize));
}

void Graphics::Draw()
{
	{
		m_native->BeginDraw();
		m_native->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		m_native->Clear(D2D1::ColorF(D2D1::ColorF::Magenta)); // Debug color; should be covered

		D2D1_MATRIX_3X2_F rotate = D2D1::Matrix3x2F::Rotation(0);
		m_native->SetTransform(rotate);

		// Draw BG
		{
			D2D1_RECT_F srcRect = D2D1::RectF(0, 0, 128, 112);
			D2D1_RECT_F dstRect = D2D1::RectF(0, 0, 128, 112);
			m_native->DrawBitmap(m_bg.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);
		}

		// Draw a block
		D2D1_MATRIX_3X2_F gridOrigin = D2D1::Matrix3x2F::Translation(35, 8);
		m_native->SetTransform(gridOrigin * rotate);
		{
			D2D1_RECT_F srcRect = D2D1::RectF(0, 0, 6, 6);
			D2D1_RECT_F dstRect = D2D1::RectF(0, 0, 6, 6);
			m_native->DrawBitmap(m_blocks.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);
		}

		{
			int x = 2;
			int y = 9;
			D2D1_RECT_F srcRect = D2D1::RectF(0, 0, 6, 6);
			D2D1_RECT_F dstRect{};
			dstRect.left = x * 6;
			dstRect.right = dstRect.left + 6;
			dstRect.top = y * 6;
			dstRect.bottom = dstRect.top + 6;
			m_native->DrawBitmap(m_blocks.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);
		}

		VerifyHR(m_native->EndDraw());
	}
	{
		m_renderTarget->BeginDraw();
		m_renderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		m_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

		D2D1_RECT_F srcRect = D2D1::RectF(0, 0, 128, 112);

		D2D1_SIZE_F rtSize = m_renderTarget->GetSize();

		// Letterbox m_native to the final target
		float srcHeight = static_cast<float>(srcRect.bottom - srcRect.top);
		float heightScale = static_cast<float>(rtSize.height) / srcHeight;
		float proportionateDstWidth = static_cast<float>(srcRect.right - srcRect.left) * heightScale;
		float centerX = (rtSize.width - proportionateDstWidth) / 2;

		D2D1_RECT_F dstRect = D2D1::RectF(centerX, 0, proportionateDstWidth + centerX, rtSize.height);

		ComPtr<ID2D1Bitmap> bitmapRenderTarget;
		VerifyHR(m_native->GetBitmap(&bitmapRenderTarget));
		m_renderTarget->DrawBitmap(bitmapRenderTarget.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);

		VerifyHR(m_renderTarget->EndDraw());
	}
}