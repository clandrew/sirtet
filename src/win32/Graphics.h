#pragma once

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
	Red,
	Blue,
	Green,
	Cyan,
	Magenta,
	Yellow,
	Purple
};

class Random
{
public:
	Random();

	int Next(int n);
};

enum class DropPieceResult { None, PieceLanded, RowsCleared, GameOver };

class Grid
{
	std::vector<int> data;

	std::vector<PieceLayout> pieceLayouts;
	std::vector<Color> pieceColors;
	std::vector<int> statistics;

	int currentPieceType;
	int nextPieceType;
	int currentPieceRotation;
	Coordinate currentPieceLocation;
	int lineClearCount;
	int score;
	int maxForceDropThisPiece;
	int currentForceDropLengthThisPiece;

	std::vector<int> rowsBeingCleared;

	Random random;

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
	int frames = -1;

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

	ComPtr<ID2D1BitmapRenderTarget> m_native;

	ComPtr<ID2D1Bitmap1> m_bg;
	ComPtr<ID2D1Bitmap1> m_ui;
	ComPtr<ID2D1Bitmap1> m_blocks;

	enum class PieceType { I, L, O, R, S, T, Z };

	// Gameplay
	Grid grid;
	int framesPerPieceDrop;
	int framesUntilPieceDrop;
	bool forcingDrop;
	bool gameOver;
	bool loserMode;
	RowClearingAnimation rowClearingAnimation;
	Random random;

	// Camera settings
	float cameraX;
	float cameraY;
	float cameraTargetX;
	float cameraTargetY;
	float cameraRotation;
	float cameraTargetRotation;
	bool showDebuggingAids;

	// Graphics resources
	bool prebakedDrawingValid;
	bool finishedLoadingResources;
	float backgroundScrollX;
	float backgroundScrollY;
	
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
	void DrawBlock(int x, int y);
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