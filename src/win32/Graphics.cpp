#include "stdafx.h"
#include "Graphics.h"

const float MATH_PI = 3.14159f;

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

void Coordinate::Initialize(int x, int y)
{
	X = x;
	Y = y;
}

void FourCoordinates::Initialize(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
	Location[0] = { x0, y0 };
	Location[1] = { x1, y1 };
	Location[2] = { x2, y2 };
	Location[3] = { x3, y3 };
}

void PieceLayout::AddRotation(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
	FourCoordinates f;
	f.Initialize(x0, y0, x1, y1, x2, y2, x3, y3);
	EachRotation.push_back(f);
}

Random::Random()
{
	srand(time(0));
}

int Random::Next(int n)
{
	return rand() % n;
}

void Grid::Initialize(int blockSize, int blocksXCount, int blocksYCount)
{
	m_blockSize = blockSize;
	m_blocksXCount = blocksXCount;
	m_blocksYCount = blocksYCount;

	m_pieceLayouts.resize(7);

	// I
	m_pieceLayouts[0].AddRotation(0, 0, 1, 0, 2, 0, 3, 0);
	m_pieceLayouts[0].AddRotation(2, -2, 2, -1, 2, 0, 2, 1);

	// L
	m_pieceLayouts[1].AddRotation(1, 0, 2, 0, 3, 0, 1, 1);
	m_pieceLayouts[1].AddRotation(2, -1, 2, 0, 2, 1, 3, 1);
	m_pieceLayouts[1].AddRotation(1, 0, 2, 0, 3, 0, 3, -1);
	m_pieceLayouts[1].AddRotation(1, -1, 2, -1, 2, 0, 2, 1);

	// O
	m_pieceLayouts[2].AddRotation(1, 0, 2, 0, 1, 1, 2, 1);

	// R
	m_pieceLayouts[3].AddRotation(1, 0, 2, 0, 3, 0, 3, 1);
	m_pieceLayouts[3].AddRotation(2, -1, 3, -1, 2, 0, 2, 1);
	m_pieceLayouts[3].AddRotation(1, -1, 1, 0, 2, 0, 3, 0);
	m_pieceLayouts[3].AddRotation(2, -1, 2, 0, 1, 1, 2, 1);

	// S
	m_pieceLayouts[4].AddRotation(2, 0, 3, 0, 1, 1, 2, 1);
	m_pieceLayouts[4].AddRotation(2, -1, 2, 0, 3, 0, 3, 1);

	// T
	m_pieceLayouts[5].AddRotation(1, 0, 2, 0, 3, 0, 2, 1);
	m_pieceLayouts[5].AddRotation(2, -1, 2, 0, 3, 0, 2, 1);
	m_pieceLayouts[5].AddRotation(2, -1, 1, 0, 2, 0, 3, 0);
	m_pieceLayouts[5].AddRotation(2, -1, 1, 0, 2, 0, 2, 1);

	// Z
	m_pieceLayouts[6].AddRotation(1, 0, 2, 0, 2, 1, 3, 1);
	m_pieceLayouts[6].AddRotation(3, -1, 2, 0, 3, 0, 2, 1);

	m_pieceColors.resize(7);
	m_pieceColors[0] = Color::Purple;
	m_pieceColors[1] = Color::Red;
	m_pieceColors[2] = Color::Yellow;
	m_pieceColors[3] = Color::Cyan;
	m_pieceColors[4] = Color::Green;
	m_pieceColors[5] = Color::Red;
	m_pieceColors[6] = Color::Pink;

	Reset();
}

void Grid::Reset()
{
	m_data.clear();

	for (int i = 0; i < m_blocksXCount * m_blocksYCount; ++i)
	{
		m_data.push_back(-1);
	}

	m_statistics.resize(7);

	m_score = 0;
	m_lineClearCount = 0;

	m_nextPieceType = m_random.Next(7);
	NewPiece();
}

bool Grid::SetCell(Coordinate location, int value, bool allowOverwrite)
{
	return SetCell(location.X, location.Y, value, allowOverwrite);
}

bool Grid::SetCell(int x, int y, int value, bool allowOverwrite)
{
	ValidateBounds(x, y);

	if (y < 0)
		return false;

	if (!allowOverwrite && m_data[y * m_blocksXCount + x] != -1)
		return false;

	m_data[y * m_blocksXCount + x] = value;
	return true;
}

int Grid::GetCell(int x, int y)
{
	ValidateBounds(x, y);

	if (y < 0)
		return -1;

	return m_data[y * m_blocksXCount + x];
}

void Grid::ValidateBounds(int x, int y)
{
	assert(x >= 0 && x < m_blocksXCount);
	assert(y < m_blocksYCount);
}

bool Grid::TryRotatePiece()
{
	int forcastedIndex = (m_currentPieceRotation + 1) % 4;
	int forcastedRotation = forcastedIndex % m_pieceLayouts[m_currentPieceType].EachRotation.size();
	FourCoordinates forcastedCoordinates = GetPieceCoordinates(m_currentPieceLocation, m_currentPieceType, forcastedRotation);
	if (CanPlacePieceAt(forcastedCoordinates))
	{
		m_currentPieceRotation = forcastedIndex;
		return true;
	}
	return false;
}

// Returns whether we could do it.
bool Grid::TryMovePiece(int x, int y)
{
	Coordinate forcastedLocation;
	forcastedLocation.Initialize(m_currentPieceLocation.X + x, m_currentPieceLocation.Y + y);
	FourCoordinates forcastedCoordinates = GetPieceCoordinates(forcastedLocation, m_currentPieceType, m_currentPieceRotation);
	if (CanPlacePieceAt(forcastedCoordinates))
	{
		m_currentPieceLocation = forcastedLocation;
		return true;
	}

	return false;
}

bool Grid::MovePieceLeft()
{
	return TryMovePiece(-1, 0);
}

bool Grid::MovePieceRight()
{
	return TryMovePiece(1, 0);
}

DropPieceResult Grid::DropPiece(bool forcedDrop)
{
	if (forcedDrop)
	{
		m_currentForceDropLengthThisPiece++;
		m_maxForceDropThisPiece = max(m_maxForceDropThisPiece, m_currentForceDropLengthThisPiece);
	}

	if (!TryMovePiece(0, 1))
	{
		if (CommitCurrentPiece())
		{
			ProcessAnyClearedRows();
			NewPiece();
			return m_rowsBeingCleared.size() > 0 ? DropPieceResult::RowsCleared : DropPieceResult::PieceLanded;
		}
		else
		{
			return DropPieceResult::GameOver;
		}
	}
	return DropPieceResult::None;
}

bool Grid::IsRowClear(int y)
{
	for (int x = 0; x < m_blocksXCount; ++x)
	{
		if (GetCell(x, y) == -1)
		{
			return false;
		}
	}
	return true;
}

void Grid::MoveEverythingDown(int yStoppingPoint)
{
	for (int y = yStoppingPoint; y >= 0; --y)
	{
		for (int x = 0; x < m_blocksXCount; ++x)
		{
			int newValue;

			if (y > 0)
				newValue = GetCell(x, y - 1);
			else
				newValue = -1;

			SetCell(x, y, newValue, true);
		}
	}
}

void Grid::ProcessAnyClearedRows()
{
	int rowsClearedThisMove = 0;
	for (int y = 0; y < m_blocksYCount; ++y)
	{
		if (IsRowClear(y))
		{
			m_rowsBeingCleared.push_back(y);
			rowsClearedThisMove++;
		}
	}

	switch (rowsClearedThisMove)
	{
	case 0: break;
	case 1: m_score += 40; break;
	case 2: m_score += 100; break;
	case 3: m_score += 300; break;
	default: m_score += 1200; break;
	}

	m_lineClearCount += rowsClearedThisMove;
}

std::vector<int> Grid::GetRowsBeingCleared()
{
	return m_rowsBeingCleared;
}

void Grid::PurgeClearedRows()
{
	for (int i = 0; i < m_rowsBeingCleared.size(); ++i)
	{
		MoveEverythingDown(m_rowsBeingCleared[i]);

	}
	m_rowsBeingCleared.clear();
}

void Grid::NewPiece()
{
	m_currentPieceType = m_nextPieceType;
	m_nextPieceType = m_random.Next(7);
	m_currentPieceRotation = 0;
	m_currentPieceLocation.Initialize(3, 0);
	m_maxForceDropThisPiece = 0;
	m_currentForceDropLengthThisPiece = 0;

	m_statistics[m_currentPieceType]++;
}

bool Grid::CommitCurrentPiece()
{
	FourCoordinates currentPieceCoordinates = GetCurrentPieceCoordinates();

	for (int i = 0; i < 4; i++)
	{
		if (!SetCell(currentPieceCoordinates.Location[i], m_currentPieceType, false))
			return false;
	}

	m_score += m_maxForceDropThisPiece;

	return true;
}

bool Grid::CanPlacePieceAt(FourCoordinates coordinates)
{
	for (int i = 0; i < 4; i++)
	{
		Coordinate l = coordinates.Location[i];

		// No comparison for y < 0, since pieces are allowed to hang off the top.
		if (l.X < 0 || l.X >= m_blocksXCount || l.Y >= m_blocksYCount)
			return false;

		int cell = GetCell(l.X, l.Y);
		if (cell != -1)
			return false;
	}

	return true;
}

FourCoordinates Grid::GetPieceCoordinates(Coordinate pieceLocation, int pieceType, int pieceRotation)
{
	int effectiveRotation = pieceRotation % m_pieceLayouts[pieceType].EachRotation.size();

	FourCoordinates r = m_pieceLayouts[pieceType].EachRotation[effectiveRotation];

	for (int i = 0; i < 4; ++i)
	{
		r.Location[i].X += pieceLocation.X;
		r.Location[i].Y += pieceLocation.Y;
	}

	return r;
}

FourCoordinates Grid::GetPieceCoordinates(int pieceType)
{
	Coordinate c;
	c.Initialize(0, 3);
	return GetPieceCoordinates(c, pieceType, 0);
}

FourCoordinates Grid::GetCurrentPieceCoordinates()
{
	return GetPieceCoordinates(m_currentPieceLocation, m_currentPieceType, m_currentPieceRotation);
}

FourCoordinates Grid::GetNextPieceCoordinates()
{
	Coordinate c;

	if (m_nextPieceType == (int)PieceType::I)
	{
		c.Initialize(-1, 2);
		return GetPieceCoordinates(c, m_nextPieceType, 1);
	}
	else
	{
		c.Initialize(-1, 1);
		return GetPieceCoordinates(c, m_nextPieceType, 0);
	}
}

int Grid::GetLinesCleared()
{
	return m_lineClearCount;
}

int Grid::GetScore()
{
	return m_score;
}

int Grid::GetNextPieceType()
{
	return m_nextPieceType;
}

Coordinate Grid::GetCurrentPieceLocation()
{
	return m_currentPieceLocation;
}

int Grid::GetCurrentPieceRotation()
{
	return m_currentPieceRotation;
}

Color Grid::GetColor(int index)
{
	assert(index >= 0 && index < 7);
	return m_pieceColors[index];
}

int Grid::GetCurrentPieceType()
{
	return m_currentPieceType;
}

int Grid::GetStatistic(int pieceType)
{
	assert(pieceType >= 0 && pieceType < 7);
	return m_statistics[pieceType];
}

void RowClearingAnimation::Start()
{
	m_frames = 5;
}

bool RowClearingAnimation::IsAnimating()
{
	return m_frames != -1;
}

void RowClearingAnimation::Stop()
{
	m_frames = -1;
}

void RowClearingAnimation::Update()
{
	m_frames--;
}

struct CameraTransforms
{
	D2D1_MATRIX_3X2_F Prebaked;
	D2D1_MATRIX_3X2_F Final;
};

void Graphics::Initialize(HWND hwnd)
{
	m_showDebuggingAids = false;

	m_blockSize = 6;
	m_blocksXCount = 10;
	m_blocksYCount = 16;

	m_gridExteriorOrigin = D2D1::Point2U(30, 3);
	m_gridInteriorOrigin = D2D1::Point2U(m_gridExteriorOrigin.x + 5, m_gridExteriorOrigin.y + 5);

	m_grid.Initialize(m_blockSize, m_blocksXCount, m_blocksYCount);

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

	VerifyHR(m_native->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.8f), &m_whiteBrush));
	VerifyHR(m_native->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Magenta), &m_magentaBrush));

	m_bg = LoadImageFile(L"Images\\testbg.png");
	m_ui = LoadImageFile(L"Images\\ui.png");
	m_blocks = LoadImageFile(L"Images\\blocks.png");
	m_numbers = LoadImageFile(L"Images\\numbers.png");
	
	m_framesPerPieceDrop = 10;

	m_loserMode = true;

	NewGame();
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

void Graphics::OnTimerTick()
{
	if (m_gameOver)
	{
		UpdateWeirdBackgroundScrolling();
		return; // No animating
	}
	
	if (m_rowClearingAnimation.IsAnimating())
	{
		m_rowClearingAnimation.Update();

		if (!m_rowClearingAnimation.IsAnimating())
		{
			m_grid.PurgeClearedRows();
		}
	}
	else
	{
		UpdateTimedDrop();

		UpdateForcedDrop();

		if (!m_rowClearingAnimation.IsAnimating())
		{
			UpdateCamera();
		}
	}
}

void Graphics::DrawBlock(D2D1_POINT_2U origin, int x, int y, Color c)
{
	// The current piece might have an edge which goes outside the grid. We just ignore those.
	if (x < 0 ||y < 0 || x >= m_blocksXCount || y >= m_blocksYCount) 
		return;

	D2D1_RECT_F srcRect{};
	srcRect.bottom = m_blockSize;
	srcRect.left = (int)c * m_blockSize;
	srcRect.right = srcRect.left + m_blockSize;

	D2D1_RECT_F dstRect{};
	dstRect.left = x * m_blockSize + origin.x;
	dstRect.right = dstRect.left + m_blockSize;
	dstRect.top = y * m_blockSize + origin.y;
	dstRect.bottom = dstRect.top + m_blockSize;
	m_native->DrawBitmap(m_blocks.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);
}

void Graphics::Draw()
{
	float srcWidth = 128;
	float srcHeight = 112;

	{
		m_native->BeginDraw();
		m_native->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		m_native->Clear(D2D1::ColorF(D2D1::ColorF::Magenta)); // Debug color; should be covered

		m_native->SetTransform(D2D1::Matrix3x2F::Identity());

		// Draw BG
		{
			D2D1_RECT_F srcRect = D2D1::RectF(0, 0, 128, 112);
			D2D1_RECT_F dstRect = D2D1::RectF(0, 0, 128, 112);
			m_native->DrawBitmap(m_bg.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);
		}
		
		auto currentPieceLocation = m_grid.GetCurrentPieceLocation();
		int screenX = (currentPieceLocation.X + 2) * 6 + m_gridInteriorOrigin.x;
		int screenY = (currentPieceLocation.Y + 1) * 6 + m_gridInteriorOrigin.y;
				
		if (!m_loserMode)
		{
			auto translate1 = D2D1::Matrix3x2F::Translation(
				-m_cameraX,
				-m_cameraY
			);

			auto rotate = D2D1::Matrix3x2F::Rotation(m_currentCameraRotation);

			auto translate2 = D2D1::Matrix3x2F::Translation(
				srcWidth / 2,
				srcHeight / 2
			);

			m_native->SetTransform(translate1 * rotate * translate2);
		}


		// Draw the UI
		{
			D2D1_RECT_F srcRect = D2D1::RectF(0, 0, 98, 107);
			D2D1_RECT_F dstRect = D2D1::RectF(m_gridExteriorOrigin.x, m_gridExteriorOrigin.y, m_gridExteriorOrigin.x + 98, m_gridExteriorOrigin.y + 107);
			m_native->DrawBitmap(m_ui.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);
		}

		// Draw the current piece
		auto currentPieceCoords = m_grid.GetCurrentPieceCoordinates();
		for (int i = 0; i < 4; ++i)
		{
			DrawBlock(m_gridInteriorOrigin, currentPieceCoords.Location[i].X, currentPieceCoords.Location[i].Y, (Color)m_grid.GetCurrentPieceType());
		}

		// Draw the grid
		for (int cellY = 0; cellY < m_blocksYCount; ++cellY)
		{
			for (int cellX = 0; cellX < m_blocksXCount; ++cellX)
			{
				int cell = m_grid.GetCell(cellX, cellY);

				if (cell == -1)
				{
					// No color.
				}
				else
				{
					DrawBlock(m_gridInteriorOrigin, cellX, cellY, (Color)cell);
				}
			}
		}

		// Row-clearing animation
		if (m_rowClearingAnimation.IsAnimating())
		{
			auto rowsBeingCleared = m_grid.GetRowsBeingCleared();


			for (int i = 0; i < rowsBeingCleared.size(); ++i)
			{
				D2D1_RECT_F fillRect;
				fillRect.left = m_gridInteriorOrigin.x;
				fillRect.right = m_blockSize * m_blocksXCount + m_gridInteriorOrigin.x;
				fillRect.top = rowsBeingCleared[i] * m_blockSize + m_gridInteriorOrigin.y;
				fillRect.bottom = fillRect.top + m_blockSize;

				m_native->FillRectangle(&fillRect, m_whiteBrush.Get());
			}
		}

		D2D1_POINT_2U nextPieceOrigin = D2D1::Point2U(101, 17);
		auto nextPiece = m_grid.GetNextPieceCoordinates();
		for (int i = 0; i < 4; ++i)
		{
			DrawBlock(nextPieceOrigin, nextPiece.Location[i].X, nextPiece.Location[i].Y, (Color)m_grid.GetNextPieceType());
		}

		// m_numbers
		D2D1_POINT_2U scoreOrigin = D2D1::Point2U(100, 47);
		{
			int score = m_grid.GetScore();

			for (int i = 0; i < 6; ++i)
			{
				int placeValue = score % 10;

				D2D1_RECT_F srcRect;
				srcRect.left = placeValue * 4;
				srcRect.right = srcRect.left + 3;
				srcRect.top = 0;
				srcRect.bottom = srcRect.top + 5;

				D2D1_RECT_F dstRect;
				dstRect.left = scoreOrigin.x + ((6 - 1 - i) * 4);
				dstRect.right = dstRect.left + 3;
				dstRect.top = scoreOrigin.y;
				dstRect.bottom = dstRect.top + 5;
				m_native->DrawBitmap(m_numbers.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);

				score /= 10;
			}
		}

		VerifyHR(m_native->EndDraw());
	}
	{
		m_renderTarget->BeginDraw();
		m_renderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		m_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

		D2D1_RECT_F srcRect = D2D1::RectF(0, 0, srcWidth, srcHeight);
		float goalRatio = srcWidth / srcHeight;

		D2D1_SIZE_F rtSize = m_renderTarget->GetSize();
		float targetWidth = rtSize.width;
		float targetHeight = rtSize.height;
		float targetRatio = targetWidth / targetHeight;

		D2D1_RECT_F dstRect;
		if (targetRatio > goalRatio)
		{
			// Letterbox
			float heightScale = targetHeight / srcHeight;
			float proportionateDstWidth = srcWidth * heightScale;
			float centerX = (targetWidth - proportionateDstWidth) / 2;

			dstRect = D2D1::RectF(centerX, 0, proportionateDstWidth + centerX, rtSize.height);

		}
		else
		{
			// Pillarbox
			float widthScale = targetWidth / srcWidth;
			float proportionalDstHeight = srcHeight * widthScale;
			float centerY = (targetHeight - proportionalDstHeight) / 2;

			dstRect = D2D1::RectF(0, centerY, targetWidth, proportionalDstHeight + centerY);
		}

		ComPtr<ID2D1Bitmap> bitmapRenderTarget;
		VerifyHR(m_native->GetBitmap(&bitmapRenderTarget));
		m_renderTarget->DrawBitmap(bitmapRenderTarget.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);

		if (m_showDebuggingAids)
		{
			m_renderTarget->DrawLine(D2D1::Point2F(targetWidth / 2, 0), D2D1::Point2F(targetWidth / 2, targetHeight), m_magentaBrush.Get(), 5.0f);
			m_renderTarget->DrawLine(D2D1::Point2F(0, targetHeight / 2), D2D1::Point2F(targetWidth, targetHeight / 2), m_magentaBrush.Get(), 5.0f);
		}

		VerifyHR(m_renderTarget->EndDraw());
	}
}


void Graphics::NewGame()
{
	m_framesUntilPieceDrop = m_framesPerPieceDrop;

	SetCameraTargetXY();
	m_cameraX = m_cameraTargetX;
	m_cameraY = m_cameraTargetY;
	m_currentCameraRotation = 0;
	m_targetCameraRotation = 0;

	m_backgroundScrollX = 0;
	m_backgroundScrollY = 0;
}

void Graphics::OnKeyDown(WPARAM key)
{
	if (m_gameOver)
	{
		return;
	}

	if (m_rowClearingAnimation.IsAnimating())
	{
		return;
	}

	if (key == 40)
	{
		bool forcedDrop = true;
		auto dropResult = m_grid.DropPiece(forcedDrop);

		if (dropResult == DropPieceResult::None || dropResult == DropPieceResult::PieceLanded)
		{
			m_forcingDrop = true;
			SetCameraTargetXY();
			SetCameraTargetRotation();
		}
		else if (dropResult == DropPieceResult::GameOver)
		{
			m_gameOver = true;
		}
		else
		{
			assert(dropResult == DropPieceResult::RowsCleared);
			m_rowClearingAnimation.Start();
		}
	}
}

void Graphics::OnKeyUp(WPARAM key)
{
	if (m_gameOver)
	{
		m_grid.Reset();
		NewGame();
		m_gameOver = false;
		return;
	}

	if (m_rowClearingAnimation.IsAnimating())
	{
		return;
	}

	if (key == 38)
	{
		if (m_grid.TryRotatePiece())
		{
			SetCameraTargetRotation();
		}
	}
	else if (key == 37)
	{
		if (m_grid.MovePieceLeft())
		{
			SetCameraTargetXY();
		}
	}
	else if (key == 39)
	{
		if (m_grid.MovePieceRight())
		{
			SetCameraTargetXY();
		}
	}
	else if (key == 40)
	{
		m_forcingDrop = false;
	}
#if _DEBUG
	else if (key == 192) // tilde
	{
		m_showDebuggingAids = !m_showDebuggingAids;
	}
	else if (key == 49) // numerical 1
	{
		m_loserMode = !m_loserMode;
	}
#endif
}

void Graphics::SetCameraTargetXY()
{
	assert(!m_rowClearingAnimation.IsAnimating());

	auto location = m_grid.GetCurrentPieceLocation();
	int screenX = location.X * m_blockSize;
	int screenY = location.Y * m_blockSize;

	// Ensure camera is centered on the piece itself
	m_cameraTargetX = screenX + (m_blockSize * 2) + m_gridInteriorOrigin.x;
	m_cameraTargetY = screenY + (m_blockSize * 1) + m_gridInteriorOrigin.y;
}

float RotationIndexToDegrees(int rotation)
{
	switch (rotation)
	{
	case 0: return 0;
	case 1: return 90.0f;
	case 2: return 180.0f;
	case 3: return 270.0f;
	default:
		assert(false);
		return 0;
	}
}

void Graphics::SetCameraTargetRotation()
{
	m_targetCameraRotation = RotationIndexToDegrees(m_grid.GetCurrentPieceRotation());
}

void Graphics::UpdateWeirdBackgroundScrolling()
{
	float backgroundScrollInc = 0.5f;

	if (m_random.Next(2) == 0)
		backgroundScrollInc = -backgroundScrollInc;

	m_backgroundScrollX += backgroundScrollInc;

	if (m_random.Next(2) == 0)
		backgroundScrollInc = -backgroundScrollInc;

	m_backgroundScrollY += backgroundScrollInc;
}

void Graphics::UpdateBackgroundScrolling()
{
	float backgroundScrollInc = 0.5f;
	m_backgroundScrollX += backgroundScrollInc;
	if (m_backgroundScrollX > 400)
		m_backgroundScrollX = 0;

	m_backgroundScrollY += backgroundScrollInc;
	if (m_backgroundScrollY > 431)
		m_backgroundScrollY = 0;
}

void Graphics::UpdateForcedDrop()
{
	if (!m_forcingDrop)
		return;

	if (m_rowClearingAnimation.IsAnimating())
	{
		m_forcingDrop = false;
		return;
	}

	auto dropResult = m_grid.DropPiece(true);

	if (dropResult == DropPieceResult::None)
	{
		SetCameraTargetXY();
		SetCameraTargetRotation();
	}
	else if (dropResult == DropPieceResult::PieceLanded)
	{
		SetCameraTargetXY();
		SetCameraTargetRotation();
		m_forcingDrop = false;
	}
	else if (dropResult == DropPieceResult::GameOver)
	{
		m_gameOver = true;
	}
	else if (dropResult == DropPieceResult::RowsCleared)
	{
		m_rowClearingAnimation.Start();
		m_forcingDrop = false;
	}

}

void Graphics::UpdateTimedDrop()
{
	if (m_framesUntilPieceDrop <= 0)
	{
		auto dropResult = m_grid.DropPiece(false);

		if (dropResult == DropPieceResult::None || dropResult == DropPieceResult::PieceLanded)
		{
			m_framesUntilPieceDrop = m_framesPerPieceDrop;

			SetCameraTargetXY();
			SetCameraTargetRotation();
		}
		else if (dropResult == DropPieceResult::GameOver)
		{
			m_gameOver = true;
		}
		else if (dropResult == DropPieceResult::RowsCleared)
		{
			m_framesUntilPieceDrop = m_framesPerPieceDrop;

			m_rowClearingAnimation.Start();
		}
	}
	else
	{
		m_framesUntilPieceDrop--;
	}
}

void Graphics::UpdateCamera()
{
	assert(!m_rowClearingAnimation.IsAnimating());

	float cameraIncXAmt = 4;
	float cameraIncYAmt = cameraIncXAmt;

	float cameraDispX = m_cameraTargetX - m_cameraX;
	float cameraDispY = m_cameraTargetY - m_cameraY;

	if (abs(cameraDispX) > m_blockSize)
		cameraIncXAmt = m_blockSize * 3;

	if (abs(cameraDispY) > m_blockSize)
		cameraIncYAmt = m_blockSize * 3;

	if (m_cameraX < m_cameraTargetX)
	{
		m_cameraX += cameraIncXAmt;
		m_cameraX = min(m_cameraX, m_cameraTargetX);
	}
	if (m_cameraX > m_cameraTargetX)
	{
		m_cameraX -= cameraIncXAmt;
		m_cameraX = max(m_cameraX, m_cameraTargetX);
	}
	if (m_cameraY < m_cameraTargetY)
	{
		m_cameraY += cameraIncYAmt;
		m_cameraY = min(m_cameraY, m_cameraTargetY);
	}
	if (m_cameraY > m_cameraTargetY)
	{
		m_cameraY -= cameraIncYAmt;
		m_cameraY = max(m_cameraY, m_cameraTargetY);
	}

	float vec = abs(m_currentCameraRotation - m_targetCameraRotation);

	float newTarget;
	if (vec <= 180)
	{
		// Straightforward
		newTarget = m_targetCameraRotation;
	}
	else
	{
		// Choose a shorter path instead
		newTarget = m_targetCameraRotation - 360;
		vec = abs(m_currentCameraRotation - newTarget);

		if (vec <= 180)
		{
			;
		}
		else
		{
			newTarget = m_targetCameraRotation + 360;
			vec = abs(m_currentCameraRotation - newTarget);
			VerifyAssert(vec <= 180);
		}
	}

	float inc = 20.0f;
	if (m_currentCameraRotation < newTarget)
	{
		m_currentCameraRotation += inc;
		m_currentCameraRotation = min(m_currentCameraRotation, newTarget);
	}
	else if (m_currentCameraRotation > newTarget)
	{
		m_currentCameraRotation -= inc;
		m_currentCameraRotation = max(m_currentCameraRotation, newTarget);
	}
	   
	while (m_currentCameraRotation > 360)
	{
		m_currentCameraRotation -= 360;
	}
}

void VerifyAssert(bool cond)
{
	if (!cond)
	{
		__debugbreak();
	}
}