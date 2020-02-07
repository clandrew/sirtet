#pragma once

void VerifyAssert(bool cond);

struct Coordinate
{
	void Initialize(int x, int y);

	int X;
	int Y;
};

struct FourCoordinates
{
	void Initialize(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);

	Coordinate Location[4];
};

struct PieceLayout
{
	void AddRotation(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);

	std::vector<FourCoordinates> EachRotation;
};

enum Color
{
	Purple,
	Orange,
	Yellow,
	Cyan,
	Green,
	Red,
	Pink
};

enum class PieceType { I, L, O, R, S, T, Z };

class Random
{
public:
	Random();

	int Next(int n);
};

enum class DropPieceResult { None, PieceLanded, RowsCleared, GameOver };

class Grid
{
	std::vector<int> m_data;

	std::vector<PieceLayout> m_pieceLayouts;
	std::vector<Color> m_pieceColors;
	std::vector<int> m_statistics;

	int m_currentPieceType;
	int m_nextPieceType;
	int m_currentPieceRotation;
	Coordinate m_currentPieceLocation;
	int m_lineClearCount;
	int m_score;
	int m_maxForceDropThisPiece;
	int m_currentForceDropLengthThisPiece;

	std::vector<int> m_rowsBeingCleared;

	Random m_random;

	int m_blockSize;
	int m_blocksXCount;
	int m_blocksYCount;
	
public:
	void Initialize(int blockSize, int blocksXCount, int blocksYCount);

	void Reset();

	bool SetCell(Coordinate location, int value, bool allowOverwrite);

	bool SetCell(int x, int y, int value, bool allowOverwrite);

	int GetCell(int x, int y);

	void ValidateBounds(int x, int y);

	bool TryRotatePiece();

	// Returns whether we could do it.
	bool TryMovePiece(int x, int y);

	bool MovePieceLeft();

	bool MovePieceRight();

	DropPieceResult DropPiece(bool forcedDrop);

	bool IsRowClear(int y);

	void MoveEverythingDown(int yStoppingPoint);

	void ProcessAnyClearedRows();

	std::vector<int> GetRowsBeingCleared();

	void PurgeClearedRows();

	void NewPiece();

	bool CommitCurrentPiece();

	bool CanPlacePieceAt(FourCoordinates coordinates);

	FourCoordinates GetPieceCoordinates(Coordinate pieceLocation, int pieceType, int pieceRotation);

	FourCoordinates GetPieceCoordinates(int pieceType);

	FourCoordinates GetCurrentPieceCoordinates();

	FourCoordinates GetNextPieceCoordinates();

	int GetLinesCleared();

	int GetScore();

	int GetNextPieceType();

	Coordinate GetCurrentPieceLocation();

	int GetCurrentPieceRotation();

	Color GetColor(int index);

	int GetCurrentPieceType();

	int GetStatistic(int pieceType);
};

class RowClearingAnimation
{
	int m_frames = -1;

public:
	void Start();

	bool IsAnimating();

	void Stop();

	void Update();
};

class Graphics
{
	ComPtr<IWICImagingFactory> m_wicImagingFactory;
	ComPtr<ID2D1Factory7> m_d2dFactory;
	ComPtr<ID2D1HwndRenderTarget> m_renderTarget;
	ComPtr<ID2D1SolidColorBrush> m_whiteBrush;
	ComPtr<ID2D1SolidColorBrush> m_magentaBrush;

	ComPtr<ID2D1BitmapRenderTarget> m_native;

	ComPtr<ID2D1Bitmap1> m_bg;
	ComPtr<ID2D1Bitmap1> m_ui;
	ComPtr<ID2D1Bitmap1> m_blocks;
	ComPtr<ID2D1Bitmap1> m_numbers;

	// Gameplay
	Grid m_grid;
	int m_framesPerPieceDrop;
	int m_framesUntilPieceDrop;
	bool m_forcingDrop;
	bool m_gameOver;
	bool m_loserMode;
	RowClearingAnimation m_rowClearingAnimation;
	Random m_random;

	// Camera settings
	float m_cameraX;
	float m_cameraY;
	float m_cameraTargetX;
	float m_cameraTargetY;

	float m_currentCameraRotation;
	float m_targetCameraRotation;

	bool m_showDebuggingAids;

	// Graphics resources
	bool m_prebakedDrawingValid;
	bool m_finishedLoadingResources;
	float m_backgroundScrollX;
	float m_backgroundScrollY;

	// Layout
	D2D1_POINT_2U m_gridExteriorOrigin;
	D2D1_POINT_2U m_gridInteriorOrigin;
	
public:
	void Initialize(HWND hwnd);
	void Draw();
	void Resize(HWND hwnd);
	void OnTimerTick();
	void OnKeyDown(WPARAM key);
	void OnKeyUp(WPARAM key);

private:
	void EnsureWicImagingFactory();
	ComPtr<ID2D1Bitmap1> LoadImageFile(std::wstring fileName);
	void DrawBlock(D2D1_POINT_2U origin, int x, int y, Color c);
	void NewGame();
	void SetCameraTargetXY();
	void SetCameraTargetRotation();
	void UpdateCamera();
	void UpdateTimedDrop();
	void UpdateForcedDrop();
	void UpdateBackgroundScrolling();
	void UpdateWeirdBackgroundScrolling();

	int m_blockSize;
	int m_blocksXCount;
	int m_blocksYCount;
};