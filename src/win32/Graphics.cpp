#include "stdafx.h"
#include "Graphics.h"

const int cellSize = 25;
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

	pieceLayouts.resize(7);

	// I
	pieceLayouts[0].AddRotation(0, 0, 1, 0, 2, 0, 3, 0);
	pieceLayouts[0].AddRotation(2, -2, 2, -1, 2, 0, 2, 1);

	// L
	pieceLayouts[1].AddRotation(1, 0, 2, 0, 3, 0, 1, 1);
	pieceLayouts[1].AddRotation(2, -1, 2, 0, 2, 1, 3, 1);
	pieceLayouts[1].AddRotation(1, 0, 2, 0, 3, 0, 3, -1);
	pieceLayouts[1].AddRotation(1, -1, 2, -1, 2, 0, 2, 1);

	// O
	pieceLayouts[2].AddRotation(1, 0, 2, 0, 1, 1, 2, 1);

	// R
	pieceLayouts[3].AddRotation(1, 0, 2, 0, 3, 0, 3, 1);
	pieceLayouts[3].AddRotation(2, -1, 3, -1, 2, 0, 2, 1);
	pieceLayouts[3].AddRotation(1, -1, 1, 0, 2, 0, 3, 0);
	pieceLayouts[3].AddRotation(2, -1, 2, 0, 1, 1, 2, 1);

	// S
	pieceLayouts[4].AddRotation(2, 0, 3, 0, 1, 1, 2, 1);
	pieceLayouts[4].AddRotation(2, -1, 2, 0, 3, 0, 3, 1);

	// T
	pieceLayouts[5].AddRotation(1, 0, 2, 0, 3, 0, 2, 1);
	pieceLayouts[5].AddRotation(2, -1, 2, 0, 3, 0, 2, 1);
	pieceLayouts[5].AddRotation(2, -1, 1, 0, 2, 0, 3, 0);
	pieceLayouts[5].AddRotation(2, -1, 1, 0, 2, 0, 2, 1);

	// Z
	pieceLayouts[6].AddRotation(1, 0, 2, 0, 2, 1, 3, 1);
	pieceLayouts[6].AddRotation(3, -1, 2, 0, 3, 0, 2, 1);

	pieceColors.resize(7);
	pieceColors[0] = Color::Purple;
	pieceColors[1] = Color::Red;
	pieceColors[2] = Color::Yellow;
	pieceColors[3] = Color::Cyan;
	pieceColors[4] = Color::Green;
	pieceColors[5] = Color::Red;
	pieceColors[6] = Color::Pink;

	Reset();
}

void Grid::Reset()
{
	for (int i = 0; i < m_blocksXCount * m_blocksYCount; ++i)
	{
		data.push_back(-1);
	}

	statistics.resize(7);

	score = 0;
	lineClearCount = 0;

	nextPieceType = random.Next(7);
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

	if (!allowOverwrite && data[y * m_blocksXCount + x] != -1)
		return false;

	data[y * m_blocksXCount + x] = value;
	return true;
}

int Grid::GetCell(int x, int y)
{
	ValidateBounds(x, y);

	if (y < 0)
		return -1;

	return data[y * m_blocksXCount + x];
}

void Grid::ValidateBounds(int x, int y)
{
	assert(x >= 0 && x < m_blocksXCount);
	assert(y < m_blocksYCount);
}

bool Grid::TryRotatePiece()
{
	int forcastedIndex = (currentPieceRotation + 1) % 4;
	int forcastedRotation = forcastedIndex % pieceLayouts[currentPieceType].EachRotation.size();
	FourCoordinates forcastedCoordinates = GetPieceCoordinates(currentPieceLocation, currentPieceType, forcastedRotation);
	if (CanPlacePieceAt(forcastedCoordinates))
	{
		currentPieceRotation = forcastedIndex;
		return true;
	}
	return false;
}

// Returns whether we could do it.
bool Grid::TryMovePiece(int x, int y)
{
	Coordinate forcastedLocation;
	forcastedLocation.Initialize(currentPieceLocation.X + x, currentPieceLocation.Y + y);
	FourCoordinates forcastedCoordinates = GetPieceCoordinates(forcastedLocation, currentPieceType, currentPieceRotation);
	if (CanPlacePieceAt(forcastedCoordinates))
	{
		currentPieceLocation = forcastedLocation;
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
		currentForceDropLengthThisPiece++;
		maxForceDropThisPiece = max(maxForceDropThisPiece, currentForceDropLengthThisPiece);
	}

	if (!TryMovePiece(0, 1))
	{
		if (CommitCurrentPiece())
		{
			ProcessAnyClearedRows();
			NewPiece();
			return rowsBeingCleared.size() > 0 ? DropPieceResult::RowsCleared : DropPieceResult::PieceLanded;
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
			rowsBeingCleared.push_back(y);
			rowsClearedThisMove++;
		}
	}

	switch (rowsClearedThisMove)
	{
	case 0: break;
	case 1: score += 40; break;
	case 2: score += 100; break;
	case 3: score += 300; break;
	default: score += 1200; break;
	}

	lineClearCount += rowsClearedThisMove;
}

std::vector<int> Grid::GetRowsBeingCleared()
{
	return rowsBeingCleared;
}

void Grid::PurgeClearedRows()
{
	for (int i = 0; i < rowsBeingCleared.size(); ++i)
	{
		MoveEverythingDown(rowsBeingCleared[i]);

	}
	rowsBeingCleared.clear();
}

void Grid::NewPiece()
{
	currentPieceType = nextPieceType;
	nextPieceType = random.Next(7);
	currentPieceRotation = 0;
	currentPieceLocation.Initialize(3, 0);
	maxForceDropThisPiece = 0;
	currentForceDropLengthThisPiece = 0;

	statistics[currentPieceType]++;
}

bool Grid::CommitCurrentPiece()
{
	FourCoordinates currentPieceCoordinates = GetCurrentPieceCoordinates();

	for (int i = 0; i < 4; i++)
	{
		if (!SetCell(currentPieceCoordinates.Location[i], currentPieceType, false))
			return false;
	}

	score += maxForceDropThisPiece;

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
	int effectiveRotation = pieceRotation % pieceLayouts[pieceType].EachRotation.size();

	FourCoordinates r = pieceLayouts[pieceType].EachRotation[effectiveRotation];

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
	return GetPieceCoordinates(currentPieceLocation, currentPieceType, currentPieceRotation);
}

FourCoordinates Grid::GetNextPieceCoordinates()
{
	Coordinate c;

	if (nextPieceType == (int)PieceType::I)
	{
		c.Initialize(-1, 2);
		return GetPieceCoordinates(c, nextPieceType, 1);
	}
	else
	{
		c.Initialize(-1, 1);
		return GetPieceCoordinates(c, nextPieceType, 0);
	}
}

int Grid::GetLinesCleared()
{
	return lineClearCount;
}

int Grid::GetScore()
{
	return score;
}

int Grid::GetNextPieceType()
{
	return nextPieceType;
}

Coordinate Grid::GetCurrentPieceLocation()
{
	return currentPieceLocation;
}

int Grid::GetCurrentPieceRotation()
{
	return currentPieceRotation;
}

Color Grid::GetColor(int index)
{
	assert(index >= 0 && index < 7);
	return pieceColors[index];
}

int Grid::GetCurrentPieceType()
{
	return currentPieceType;
}

int Grid::GetStatistic(int pieceType)
{
	assert(pieceType >= 0 && pieceType < 7);
	return statistics[pieceType];
}

void RowClearingAnimation::Start()
{
	frames = 5;
}

bool RowClearingAnimation::IsAnimating()
{
	return frames != -1;
}

void RowClearingAnimation::Stop()
{
	frames = -1;
}

void RowClearingAnimation::Update()
{
	frames--;
}

float RotationIndexToRadians(int rotation)
{
	switch (rotation)
	{
	case 0: return 0;
	case 1: return (float)MATH_PI / 2;
	case 2: return (float)MATH_PI;
	case 3: return 3 * (float)MATH_PI / 2;
	default:
		assert(false);
		return 0;
	}
}

struct CameraTransforms
{
	D2D1_MATRIX_3X2_F Prebaked;
	D2D1_MATRIX_3X2_F Final;
};

void Graphics::Initialize(HWND hwnd)
{
	m_blockSize = 6;
	m_blocksXCount = 10;
	m_blocksYCount = 16;

	grid.Initialize(m_blockSize, m_blocksXCount, m_blocksYCount);

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

	framesPerPieceDrop = 10;

	loserMode = false;

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
	if (gameOver)
	{
		UpdateWeirdBackgroundScrolling();
		return; // No animating
	}

	//UpdateBackgroundScrolling();

	if (rowClearingAnimation.IsAnimating())
	{
		rowClearingAnimation.Update();

		if (!rowClearingAnimation.IsAnimating())
		{
			grid.PurgeClearedRows();
		}
	}
	else
	{
		UpdateTimedDrop();

		UpdateForcedDrop();

		if (!rowClearingAnimation.IsAnimating())
		{
			UpdateCamera();
		}
	}
}

void Graphics::DrawBlock(int x, int y, Color c)
{
	// The current piece might have an edge which goes outside the grid. We just ignore those.
	if (x < 0 ||y < 0 || x >= m_blocksXCount || y >= m_blocksYCount) 
		return;

	D2D1_RECT_F srcRect{};
	srcRect.bottom = m_blockSize;
	srcRect.left = (int)c * m_blockSize;
	srcRect.right = srcRect.left + m_blockSize;

	D2D1_RECT_F dstRect{};
	dstRect.left = x * m_blockSize;
	dstRect.right = dstRect.left + m_blockSize;
	dstRect.top = y * m_blockSize;
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
		
		auto currentPieceLocation = grid.GetCurrentPieceLocation();
		int currentPieceXPixelsInGrid = currentPieceLocation.X * 6;
		int currentPieceYPixelsInGrid = currentPieceLocation.Y * 6;
		
		auto rotate = D2D1::Matrix3x2F::Rotation(0.0f);

		auto gridTransform = D2D1::Matrix3x2F::Translation(52 - currentPieceXPixelsInGrid, 51 - currentPieceYPixelsInGrid);
		auto invGridTransform = D2D1::Matrix3x2F::Translation(-(52 - currentPieceXPixelsInGrid), -(51 - currentPieceYPixelsInGrid));

		int uiBorderThickness = 5;
		auto uiTransform = D2D1::Matrix3x2F::Translation(- uiBorderThickness, -uiBorderThickness);

		auto nextPieceTransform = D2D1::Matrix3x2F::Translation(52 + 72 - 6 - currentPieceXPixelsInGrid, 51 + 15 - 6 - currentPieceYPixelsInGrid);
		
		m_native->SetTransform(rotate * gridTransform);

		// Draw the UI
		m_native->SetTransform(uiTransform * rotate * gridTransform);
		{
			// m_ui
			D2D1_RECT_F srcRect = D2D1::RectF(0, 0, 98, 107);
			int x = 0;
			int y = 0;
			D2D1_RECT_F dstRect = D2D1::RectF(x, y, x + 98, y + 107);
			m_native->DrawBitmap(m_ui.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);
		}

		// Draw the current piece
		auto currentPiece = grid.GetCurrentPieceCoordinates();
		for (int i = 0; i < 4; ++i)
		{
			DrawBlock(currentPiece.Location[i].X, currentPiece.Location[i].Y, (Color)grid.GetCurrentPieceType());
		}

		// Draw the grid
		/*for (int cellY = 0; cellY < m_blocksYCount; ++cellY)
		{
			for (int cellX = 0; cellX < m_blocksXCount; ++cellX)
			{
				int cell = grid.GetCell(cellX, cellY);

				if (cell == -1)
				{
					// No color.
				}
				else
				{
					DrawBlock(cellX, cellY, (Color)cell);
				}
			}
		}

		// Row-clearing animation
		if (rowClearingAnimation.IsAnimating())
		{
			auto rowsBeingCleared = grid.GetRowsBeingCleared();


			for(int i=0; i<rowsBeingCleared.size(); ++i)
			{
				D2D1_RECT_F fillRect;
				fillRect.left = 0;
				fillRect.right = m_blockSize * m_blocksXCount;
				fillRect.top = rowsBeingCleared[i] * m_blockSize;
				fillRect.bottom = fillRect.top + m_blockSize;

				m_native->FillRectangle(&fillRect, m_whiteBrush.Get());
			}
		}


		D2D1_MATRIX_3X2_F nextPieceOrigin = D2D1::Matrix3x2F::Translation(101, 17);
		m_native->SetTransform(invNextPieceTransform * rotate * nextPieceTransform);
		auto nextPiece = grid.GetNextPieceCoordinates();
		for (int i = 0; i < 4; ++i)
		{
			DrawBlock(nextPiece.Location[i].X, nextPiece.Location[i].Y, (Color)grid.GetNextPieceType());
		}*/

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

		m_renderTarget->DrawLine(D2D1::Point2F(targetWidth/2, 0), D2D1::Point2F(targetWidth / 2, targetHeight), m_magentaBrush.Get(), 5.0f);
		m_renderTarget->DrawLine(D2D1::Point2F(0, targetHeight/2), D2D1::Point2F(targetWidth, targetHeight / 2), m_magentaBrush.Get(), 5.0f);


		VerifyHR(m_renderTarget->EndDraw());
	}
}


void Graphics::NewGame()
{
	framesUntilPieceDrop = framesPerPieceDrop;

	SetCameraTargetXY();
	cameraX = cameraTargetX;
	cameraY = cameraTargetY;
	cameraRotation = cameraTargetRotation;

	backgroundScrollX = 0;
	backgroundScrollY = 0;
}

void Graphics::OnKeyDown(WPARAM key)
{
	if (gameOver)
	{
		return;
	}

	if (rowClearingAnimation.IsAnimating())
	{
		return;
	}

	if (key == 40)
	{
		bool forcedDrop = true;
		auto dropResult = grid.DropPiece(forcedDrop);

		if (dropResult == DropPieceResult::None || dropResult == DropPieceResult::PieceLanded)
		{
			forcingDrop = true;
			SetCameraTargetXY();
			SetCameraTargetRotation();
		}
		else if (dropResult == DropPieceResult::GameOver)
		{
			gameOver = true;
		}
		else
		{
			assert(dropResult == DropPieceResult::RowsCleared);
			rowClearingAnimation.Start();
		}
	}
}

void Graphics::OnKeyUp(WPARAM key)
{
	if (gameOver)
	{
		grid.Reset();
		NewGame();
		gameOver = false;
		return;
	}

	if (rowClearingAnimation.IsAnimating())
	{
		return;
	}

	if (key == 38)
	{
		if (grid.TryRotatePiece())
		{
			SetCameraTargetRotation();
		}
	}
	else if (key == 37)
	{
		if (grid.MovePieceLeft())
		{
			SetCameraTargetXY();
		}
	}
	else if (key == 39)
	{
		if (grid.MovePieceRight())
		{
			SetCameraTargetXY();
		}
	}
	else if (key == 40)
	{
		forcingDrop = false;
	}
#if DEBUG
	else if (args.VirtualKey == Windows.System.VirtualKey.Number1)
	{
		showDebuggingAids = !showDebuggingAids;
	}
	else if (args.VirtualKey == Windows.System.VirtualKey.Number2)
	{
		loserMode = !loserMode;
	}
#endif
}

void Graphics::SetCameraTargetXY()
{
	assert(!rowClearingAnimation.IsAnimating());

	auto location = grid.GetCurrentPieceLocation();
	int screenX = location.X * cellSize;
	int screenY = location.Y * cellSize;

	// Ensure camera is centered on the piece itself
	cameraTargetX = screenX + (cellSize * 2);
	cameraTargetY = screenY + (cellSize * 1);
}

void Graphics::SetCameraTargetRotation()
{
	cameraTargetRotation = RotationIndexToRadians(grid.GetCurrentPieceRotation());
}

void Graphics::UpdateWeirdBackgroundScrolling()
{
	float backgroundScrollInc = 0.5f;

	if (random.Next(2) == 0)
		backgroundScrollInc = -backgroundScrollInc;

	backgroundScrollX += backgroundScrollInc;

	if (random.Next(2) == 0)
		backgroundScrollInc = -backgroundScrollInc;

	backgroundScrollY += backgroundScrollInc;
}

void Graphics::UpdateBackgroundScrolling()
{
	float backgroundScrollInc = 0.5f;
	backgroundScrollX += backgroundScrollInc;
	if (backgroundScrollX > 400)
		backgroundScrollX = 0;

	backgroundScrollY += backgroundScrollInc;
	if (backgroundScrollY > 431)
		backgroundScrollY = 0;
}

void Graphics::UpdateForcedDrop()
{
	if (!forcingDrop)
		return;

	if (rowClearingAnimation.IsAnimating())
	{
		forcingDrop = false;
		return;
	}

	auto dropResult = grid.DropPiece(true);

	if (dropResult == DropPieceResult::None)
	{
		SetCameraTargetXY();
		SetCameraTargetRotation();
	}
	else if (dropResult == DropPieceResult::PieceLanded)
	{
		SetCameraTargetXY();
		SetCameraTargetRotation();
		forcingDrop = false;
	}
	else if (dropResult == DropPieceResult::GameOver)
	{
		gameOver = true;
	}
	else if (dropResult == DropPieceResult::RowsCleared)
	{
		rowClearingAnimation.Start();
		forcingDrop = false;
	}

}

void Graphics::UpdateTimedDrop()
{
	if (framesUntilPieceDrop <= 0)
	{
		auto dropResult = grid.DropPiece(false);

		if (dropResult == DropPieceResult::None || dropResult == DropPieceResult::PieceLanded)
		{
			framesUntilPieceDrop = framesPerPieceDrop;

			SetCameraTargetXY();
			SetCameraTargetRotation();
		}
		else if (dropResult == DropPieceResult::GameOver)
		{
			gameOver = true;
		}
		else if (dropResult == DropPieceResult::RowsCleared)
		{
			framesUntilPieceDrop = framesPerPieceDrop;

			rowClearingAnimation.Start();
		}
	}
	else
	{
		framesUntilPieceDrop--;
	}
}

void Graphics::UpdateCamera()
{
	assert(!rowClearingAnimation.IsAnimating());

	float cameraIncXAmt = 4;
	float cameraIncYAmt = cameraIncXAmt;

	float cameraDispX = cameraTargetX - cameraX;
	float cameraDispY = cameraTargetY - cameraY;

	if (abs(cameraDispX) > cellSize)
		cameraIncXAmt = cellSize * 3;

	if (abs(cameraDispY) > cellSize)
		cameraIncYAmt = cellSize * 3;

	if (cameraX < cameraTargetX)
	{
		cameraX += cameraIncXAmt;
		cameraX = min(cameraX, cameraTargetX);
	}
	if (cameraX > cameraTargetX)
	{
		cameraX -= cameraIncXAmt;
		cameraX = max(cameraX, cameraTargetX);
	}
	if (cameraY < cameraTargetY)
	{
		cameraY += cameraIncYAmt;
		cameraY = min(cameraY, cameraTargetY);
	}
	if (cameraY > cameraTargetY)
	{
		cameraY -= cameraIncYAmt;
		cameraY = max(cameraY, cameraTargetY);
	}

	float cameraIncRotationAmt = 0.5f;
	if (cameraRotation < cameraTargetRotation)
	{
		cameraRotation += cameraIncRotationAmt;
		cameraRotation = min(cameraRotation, cameraTargetRotation);
	}
	if (cameraRotation > cameraTargetRotation)
	{
		cameraRotation += cameraIncRotationAmt;
		cameraRotation = min(cameraRotation, 2 * MATH_PI);
	}
	if (cameraRotation >= 2 * MATH_PI)
	{
		cameraRotation = 0;
	}
}