#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <memory>
#include <string.h>
#include "raylib.h"

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define ALIVE true
#define DEAD false
#define ALIVE_COLOR RAYWHITE
#define DEAD_COLOR BLACK
#define DELAY_TIME 1
#define FPS 30

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
struct Pattern {
	int x;
	int y;
	unsigned char* pattern;
};


//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 1280;
static const int screenHeight = 800;         

static unsigned int cellMapWidth = 1000;            // # of cells in X direction
static unsigned int cellMapHeight = 1000;           // # of cells in Y direction

static int cellWidth = 10; // screenWidth / cellMapWidth;      // Width of individual cells
static int cellHeight = 10; // screenHeight / cellMapHeight;	// Height of individual cells

unsigned int length_in_bytes = cellMapWidth * cellMapHeight;
unsigned char* cells = new unsigned char[length_in_bytes];       // Create buffer for cell data storage
unsigned char* temp_cells = new unsigned char[length_in_bytes];  // Buffer for use while updating map

static unsigned int generation;
Vector2 worldPos = { 0 };

Camera2D camera = { 0 };
int cameraSpeed;

bool isPaused = false;






//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);			     // Initialize game
static void UpdateGame(void);		     // Update game (one frame)
static void DrawGame(void);				 // Draw game (one frame)
static void UpdateDrawFrame(void);	     // Update and Draw (one frame)
static void RandomCellMap(void);         // Sets cellmap to random values (creates a soup)
static void UpdateCellMap(void);         // Updates the cellmap one generation following the rules of life
static void SetCell(unsigned int x, unsigned int y);
static void ClearCell(unsigned int x, unsigned int y);
struct Pattern InitPattern(int x, int y, char name[]);
static void LoadPattern(Pattern pattern);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
	

	//Pattern pat_glider = InitPattern(3, 3, "patterns/glider.rle");
	//Pattern pat_sparkcoil = InitPattern(8, 5, "patterns/sparkcoil.rle");

	// Initialization
	//-------------------------------------------------
	InitWindow(screenWidth, screenHeight, "Conway's Game of Life");
	InitGame();



	camera.offset = { 0 };
	camera.zoom = 1.0f;
	camera.target = { 0, 0 };
	cameraSpeed = 20;

	

	SetTargetFPS(FPS);
	////-------------------------------------------------

	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		UpdateDrawFrame();
	}


	CloseWindow();	// Close window and OpenGL context
	return 0;
}

struct Pattern InitPattern(int x, int y, char name[])
{
	Pattern pattern = { 0 };
	pattern.x = x;
	pattern.y = y;
	int pattern_length = x * y;
	pattern.pattern = new unsigned char[pattern_length];
	memset(pattern.pattern, 0, pattern_length);


	FILE* fp = fopen(name, "r");
	

	int c;
	unsigned char* ptr = pattern.pattern;
	while ((c = getc(fp)) != EOF)
	{
		if (c == 'o')
		{
			*ptr = 1;
		}
		ptr++;
	}

	
	//ptr = pattern;
	/*while (ptr < (pattern + pattern_length))
	{
		printf("%i ", *ptr);
		ptr++;
	}*/


	return pattern;
}

void LoadPattern(Pattern pattern)
{
	memset(cells, 0, length_in_bytes);

	int x, y;
	int xo = cellMapWidth / 2;
	int yo = cellMapHeight / 2;
	
	unsigned char* ptr = pattern.pattern;

	for (y = 0; y < pattern.y; y++)
	{
		x = 0;
		do
		{
			if (*ptr & 0x01)
			{
				SetCell((x + xo), (y + yo));
			}

			ptr++;

		} while (++x < pattern.y);
	}


}

void InitGame()
{
	RandomCellMap(); // Sets initial cellmap to random soup
}

void RandomCellMap()
{
	memset(cells, 0, length_in_bytes);
	int x;
	int r;
	for (int y = 0; y < cellMapHeight; y++)
	{
		x = 0;
		do
		{

			r = GetRandomValue(1, 100);

			if (r > 75)
			{
				SetCell(x, y);
			}


		} while (++x < cellMapWidth);
	}
}

void UpdateCellMap()
{
	generation++;
	unsigned int x, y, count;
	unsigned char* cell_ptr;

	// Copy to temp map, so we can have an unaltered version from which to work
	memcpy(temp_cells, cells, length_in_bytes);

	// Process all cells in current cell map
	cell_ptr = temp_cells;
	for (y = 0; y < cellMapHeight; y++)
	{
		x = 0;
		do
		{
			// Skips dead cells w/ no neighbors
			while (*cell_ptr == 0)
			{
				cell_ptr++;
				if (++x >= cellMapWidth) goto RowDone;
			}

			// Found cell dead/alive and has neighbors
			// Check if state needs to be changed
			count = *cell_ptr >> 1;
			if (*cell_ptr & 0x01)  // Cel is alive
			{
				// Cell is on; turn off it it doesnt have 2 or 3 neighbors
				if ((count != 2) && (count != 3))
				{
					// Cell dies
					ClearCell(x, y);
				
				}
			}
			else // Cell is dead
			{
				// Cell is off; turn it on it has exactly 3 neighbors
				if (count == 3)
				{
					// Cell comes alive
					SetCell(x, y);
					
				}
			}

			cell_ptr++;

		} while (++x < cellMapWidth);

	RowDone:;

	}
}

void SetCell(unsigned int x, unsigned int y)
{
	int xoleft, xoright, yoabove, yobelow;
	unsigned char* cell_ptr = cells + (y * cellMapWidth) + x;

	//printf("(%i, %i)\n", x, y);

	/* Calculate offsets to eight neighboring cells, accounting for
	   wrapping around at edges of cell map */
	if (x == 0)
	{
		xoleft = cellMapWidth - 1; // if cell is at left edge, left neighbor is wrapped to right edge
		//printf("xoleft: %i\n", xoleft);
	}
	else
	{
		xoleft = -1;
		//printf("xoleft: %i\n", xoleft);
	}
	if (y == 0) // if cell is at top edge, top neighbor is wrapped to bottom edge
	{
		yoabove = length_in_bytes - cellMapWidth;
		//printf("yoabove: %i\n", yoabove);
	}
	else
	{
		yoabove = -cellMapWidth;
		//printf("yoabove: %i\n", yoabove);
	}
	if (x == (cellMapWidth - 1)) // if cell is at right edge, right edge is wrapped to left edge
	{
		xoright = -(cellMapWidth - 1);
		//printf("xoright: %i\n", xoright);
	}
	else
	{
		xoright = 1;
		//printf("xoright: %i\n", xoright);
	}
	if (y == (cellMapHeight - 1)) // if cell is at bottom edge, bottom neighbor is wrapped to top edge
	{
		yobelow = -(length_in_bytes - cellMapWidth);
		//printf("yobelow: %i\n", yobelow);
	}
	else
	{
		yobelow = cellMapWidth;
		//printf("yobelow: %i\n", yobelow);
	}

	*(cell_ptr) |= 0x01;                  // turns cell on with bitmask
	*(cell_ptr + yoabove + xoleft) += 2;  // increments top left
	*(cell_ptr + yoabove) += 2;			  // increments bottom
	*(cell_ptr + yoabove + xoright) += 2; // increments top right
	*(cell_ptr + xoleft) += 2;            // increments left
	*(cell_ptr + xoright) += 2;           // increments right
	*(cell_ptr + yobelow + xoleft) += 2;  // increments bottom left
	*(cell_ptr + yobelow) += 2;           // increments bottom
	*(cell_ptr + yobelow + xoright) += 2; // increments bottom right

}

void ClearCell(unsigned int x, unsigned int y)
{
	int xoleft, xoright, yoabove, yobelow;
	unsigned char* cell_ptr = cells + (y * cellMapWidth) + x;

	/* Calculate offsets to eight neighboring cells, accounting for
	   wrapping around at edges of cell map */
	if (x == 0)
	{
		xoleft = cellMapWidth - 1; // if cell is at left edge, left neighbor is wrapped to right edge
		//printf("xoleft: %i\n", xoleft);
	}
	else
	{
		xoleft = -1;
		//printf("xoleft: %i\n", xoleft);
	}
	if (y == 0) // if cell is at top edge, top neighbor is wrapped to bottom edge
	{
		yoabove = length_in_bytes - cellMapWidth;
		//printf("yoabove: %i\n", yoabove);
	}
	else
	{
		yoabove = -cellMapWidth;
		//printf("yoabove: %i\n", yoabove);
	}
	if (x == (cellMapWidth - 1)) // if cell is at right edge, right edge is wrapped to left edge
	{
		xoright = -(cellMapWidth - 1);
		//printf("xoright: %i\n", xoright);
	}
	else
	{
		xoright = 1;
		//printf("xoright: %i\n", xoright);
	}
	if (y == (cellMapHeight - 1)) // if cell is at bottom edge, bottom neighbor is wrapped to top edge
	{
		yobelow = -(length_in_bytes - cellMapWidth);
		//printf("yobelow: %i\n", yobelow);
	}
	else
	{
		yobelow = cellMapWidth;
		//printf("yobelow: %i\n", yobelow);
	}

	*(cell_ptr) &= ~0x01;                  // turns cell on with bitmask
	*(cell_ptr + yoabove + xoleft) -= 2;  // increments top left
	*(cell_ptr + yoabove) -= 2;			  // increments bottom
	*(cell_ptr + yoabove + xoright) -= 2; // increments top right
	*(cell_ptr + xoleft) -= 2;            // increments left
	*(cell_ptr + xoright) -= 2;           // increments right
	*(cell_ptr + yobelow + xoleft) -= 2;  // increments bottom left
	*(cell_ptr + yobelow) -= 2;           // increments bottom
	*(cell_ptr + yobelow + xoright) -= 2; // increments bottom right
}

void DrawGame()
{
	BeginDrawing();

		ClearBackground(BLACK);


		BeginMode2D(camera);

			unsigned int x, y;
			unsigned char* cell_ptr = cells;
			for (y = 0; y < cellMapHeight; y++)
			{
				x = 0;
				do
				{
					if (*cell_ptr & 0x01)
					{
						DrawRectangle(((x * cellWidth) - (cellMapWidth / 2)), ((y * cellHeight) - (cellMapHeight / 2)), cellWidth, cellHeight, ALIVE_COLOR);
					}
					cell_ptr++;
				} while (++x < cellMapWidth);
			}

			

		EndMode2D();

	

		DrawFPS(0, 0);
		DrawText(FormatText("Generation: %i", generation), 0, 50, 20, GREEN);
		


	EndDrawing();
}

void UpdateGame()
{
	
	// Camera zoom controls
	camera.zoom += (GetMouseWheelMove() * 0.05f);
	if (IsKeyDown(KEY_D)) camera.target.x += cameraSpeed;
	else if (IsKeyDown(KEY_A)) camera.target.x -= cameraSpeed;
	if (IsKeyDown(KEY_S)) camera.target.y += cameraSpeed;
	else if (IsKeyDown(KEY_W)) camera.target.y -= cameraSpeed;

	if (IsKeyPressed(KEY_P)) isPaused = !isPaused;
	



	// Updates cells to next generation following rules of life
	if (!isPaused)
	{
		UpdateCellMap();
	}

	

}

void UpdateDrawFrame()	
{
	UpdateGame();
	DrawGame();
}
