#pragma once

class Graphics
{
	ComPtr<IWICImagingFactory> m_wicImagingFactory;
	ComPtr<ID2D1Factory7> m_d2dFactory;
	ComPtr<ID2D1HwndRenderTarget> m_renderTarget;

	ComPtr<ID2D1BitmapRenderTarget> m_native;

	ComPtr<ID2D1Bitmap1> m_bg;
	ComPtr<ID2D1Bitmap1> m_ui;
	ComPtr<ID2D1Bitmap1> m_blocks;

	enum class PieceType { I, L, O, R, S, T, Z };

public:
	void Initialize(HWND hwnd);
	void Draw();
	void Resize(HWND hwnd);

private:
	void EnsureWicImagingFactory();
	ComPtr<ID2D1Bitmap1> LoadImageFile(std::wstring fileName);
};