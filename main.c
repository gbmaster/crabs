#include <windows.h>
#include <time.h>

#define N_CRABS	32
#define MAXVEL 8
#define MILLISEC_SLEEP 2000

typedef struct _CRAB
{
	int x;
	int y;
	int vx;
	int vy;
} CRAB;

char crab_up[64] = {	0, 1, 1, 0, 1, 1, 1, 0,
						1, 1, 1, 1, 0, 1, 1, 1,
						1, 0, 0, 0, 0, 0, 0, 1,
						1, 0, 1, 1, 1, 1, 0, 1,
						0, 1, 1, 1, 1, 1, 1, 0,
						0, 1, 1, 1, 1, 1, 1, 0,
						1, 0, 1, 1, 1, 1, 0, 1,
						0, 1, 0 ,0, 0, 0, 1, 0 };

char crab_down[64] = {	0, 1, 0 ,0, 0, 0, 1, 0,
						1, 0, 1, 1, 1, 1, 0, 1,
						0, 1, 1, 1, 1, 1, 1, 0,
						0, 1, 1, 1, 1, 1, 1, 0,
						1, 0, 1, 1, 1, 1, 0, 1,
						1, 0, 0, 0, 0, 0, 0, 1,
						1, 1, 1, 1, 0, 1, 1, 1,
						0, 1, 1, 0, 1, 1, 1, 0 };

char crab_left[64] = {	0, 1, 1, 1, 0, 0, 1, 0,
						1, 1, 0, 0, 1, 1, 0, 1,
						1, 1, 0, 1, 1, 1, 1, 0,
						1, 0, 0, 1, 1, 1, 1, 0,
						0, 1, 0, 1, 1, 1, 1, 0,
						1, 1, 0, 1, 1, 1, 1, 0,
						1, 1, 0, 0, 1, 1, 0, 1,
						0, 1, 1, 1, 0, 0, 1, 0 };

char crab_right[64] = {	0, 1, 0, 0, 1, 1, 1, 0,
						1, 0, 1, 1, 0, 0, 1, 1,
						0, 1, 1, 1, 1, 0, 1, 1,
						0, 1, 1, 1, 1, 0, 0, 1,
						0, 1, 1, 1, 1, 0, 1, 0,
						0, 1, 1, 1, 1, 0, 1, 1,
						1, 0, 1, 1, 0, 0, 1, 1,
						0, 1, 0, 0, 1, 1, 1, 0 };

HBITMAP hCrabUpBitmap, hCrabDownBitmap, hCrabLeftBitmap, hCrabRightBitmap;
HDC hDesktopDC, hFolderViewMem;

void create_crab(char *crab_map, HDC *hCrabDC, HBITMAP *hCrabBitmap)
{
	int i, j;

	for(i = 0; i < 8; i++)
		for(j = 0; j < 8; j++)
			if(crab_map[j + 8 * i] == 1)
				SetPixel(*hCrabDC, j, i, 0x00010101);	// due to transparency problems
			else
				SetPixel(*hCrabDC, j, i, 0x00000000);	// due to transparency problems
}

void EraseCrab(CRAB crab)
{
	char *crab_map;
	int i, j;
	COLORREF color;

	if(abs(crab.vx) >= abs(crab.vy))
		if(crab.vx > 0)
			crab_map = crab_up;
		else
			crab_map = crab_down;
	else
		if(crab.vy > 0)
			crab_map = crab_right;
		else
			crab_map = crab_left;

	for(i = 0; i < 8; i++)
		for(j = 0; j < 8; j++)
			if(crab_map[j + 8 * i] == 1)
			{
				// I know this is really a slow method, but it's the only one (at least for now)
				color = GetPixel(hFolderViewMem, crab.x + j, crab.y + i);
				SetPixel(hDesktopDC, crab.x + j, crab.y + i, color);
			}
}

void DrawCrab(CRAB crab)
{
	HDC hImageDC, hMaskDC;
	HBITMAP hCrabBitmap, hMaskBitmap;

	if(abs(crab.vx) >= abs(crab.vy))
		if(crab.vx > 0)
			hCrabBitmap = hCrabUpBitmap;
		else
			hCrabBitmap = hCrabDownBitmap;
	else
		if(crab.vy > 0)
			hCrabBitmap = hCrabRightBitmap;
		else
			hCrabBitmap = hCrabLeftBitmap;

	hMaskBitmap = CreateBitmap(8, 8, 1, 1, NULL);

	hImageDC = CreateCompatibleDC(0);
	hMaskDC = CreateCompatibleDC(0);

	SelectObject(hImageDC, hCrabBitmap);
	SelectObject(hMaskDC, hMaskBitmap);

	SetBkColor(hImageDC, 0x00000000);

	BitBlt(hMaskDC, 0, 0, 8, 8, hImageDC, 0, 0, SRCCOPY);
	BitBlt(hImageDC, 0, 0, 8, 8, hMaskDC, 0, 0, SRCINVERT);

	BitBlt(hDesktopDC, crab.x, crab.y, 8, 8, hMaskDC, 0, 0, SRCAND);
	BitBlt(hDesktopDC, crab.x, crab.y, 8, 8, hImageDC, 0, 0, SRCINVERT);

 	DeleteObject(hMaskBitmap);
	DeleteDC(hMaskDC);
	DeleteDC(hImageDC);
}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
	const UINT cchType = 255;
	LPSTR pszType = (LPSTR)malloc(cchType * sizeof(char));

	UINT chars = RealGetWindowClassA(hwnd, pszType, cchType);

	if(!strcmp(pszType, "SysListView32"))
	{
		*((HWND *)lParam) = hwnd;

		free(pszType);
		return FALSE;
	}

	free(pszType);
	return TRUE;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int i, new_x, new_y, max_x, max_y, d;
	CRAB crab[N_CRABS];
	HDC hCrabDC, hFolderViewDC;
	HWND hParentFolderView, hFolderView;
	HBITMAP hFolderViewBitmap;

	// We have to get the SysListView32 window to know what's hidden behind our crabs
	if(!(hParentFolderView = FindWindowExA(0, 0, "Progman", "Program Manager")))
		return 1;

	EnumChildWindows(hParentFolderView, EnumChildProc, (LPARAM)&hFolderView);
	if(hFolderView == NULL)
		return 1;

	hDesktopDC = GetDC(NULL);
	hCrabDC = CreateCompatibleDC(NULL);

	hCrabUpBitmap = CreateCompatibleBitmap(hDesktopDC, 8, 8);
	SelectObject(hCrabDC, hCrabUpBitmap);
	create_crab(crab_up, &hCrabDC, &hCrabUpBitmap);

	hCrabDownBitmap = CreateCompatibleBitmap(hDesktopDC, 8, 8);
	SelectObject(hCrabDC, hCrabDownBitmap);
	create_crab(crab_down, &hCrabDC, &hCrabDownBitmap);

	hCrabLeftBitmap = CreateCompatibleBitmap(hDesktopDC, 8, 8);
	SelectObject(hCrabDC, hCrabLeftBitmap);
	create_crab(crab_left, &hCrabDC, &hCrabLeftBitmap);
	
	hCrabRightBitmap = CreateCompatibleBitmap(hDesktopDC, 8, 8);
	SelectObject(hCrabDC, hCrabRightBitmap);
	create_crab(crab_right, &hCrabDC, &hCrabRightBitmap);

	DeleteDC(hCrabDC);

	max_x = GetSystemMetrics(SM_CXSCREEN);
	max_y = GetSystemMetrics(SM_CYSCREEN);

	srand((unsigned int)time(NULL));
	for(i = 0; i < N_CRABS; i++)
	{
		crab[i].x = rand() % GetSystemMetrics(SM_CXSCREEN);
		crab[i].y = 0;
		crab[i].vx = (rand() % (2 * MAXVEL - 1)) - MAXVEL;
		crab[i].vy = (rand() % (2 * MAXVEL - 1)) - MAXVEL;
	}

	while(1)
	{
		// For some reasons, right-clicking stops our kids.
		// That's why we get another DC everytime
		ReleaseDC(GetDesktopWindow(), hDesktopDC);
		hDesktopDC = GetDC(NULL);

		// Unluckily we can't get the pixel value of a hidden window with GetPixel
		// To circumvent this issue, we paint the whole desktop background in a memory HDC
		hFolderViewDC = GetWindowDC(hFolderView);
		if(hFolderViewDC)
		{
			hFolderViewMem = CreateCompatibleDC(hFolderViewDC);
			if(hFolderViewMem)
			{
				hFolderViewBitmap = CreateCompatibleBitmap(hFolderViewDC, max_x, max_y);
				if(hFolderViewBitmap)
				{
					SelectObject(hFolderViewMem, hFolderViewBitmap);
					PrintWindow(hFolderView, hFolderViewMem, 0);
					DeleteObject(hFolderViewBitmap);
				}
			}
		}

		for(i = 0; i < N_CRABS; i++)
		{
			// Delete the crab, please
			EraseCrab(crab[i]);

			while(1)
			{
				new_x = crab[i].x + crab[i].vx;
				new_y = crab[i].y + crab[i].vy;

				if(new_x >= 0 && new_x < max_x - 8 && new_y >= 0 && new_y < max_y - 8)
					break;

				crab[i].vx = (rand() % (2 * MAXVEL - 1)) - MAXVEL;
				crab[i].vy = (rand() % (2 * MAXVEL - 1)) - MAXVEL;
			}

			crab[i].x = new_x;
			crab[i].y = new_y;

			if(crab[i].vx >= MAXVEL - 1)
				d = (rand() % 2) - 1;
			else if(crab[i].vx <= 1 - MAXVEL)
				d = (rand() % 2);
			else
				d = (rand() % 3) - 1;
			crab[i].vx += d;

			if(crab[i].vy >= MAXVEL - 1)
				d = (rand() % 2) - 1;
			else if(crab[i].vy <= 1 - MAXVEL)
				d = (rand() % 2);
			else
				d = (rand() % 3) - 1;
			crab[i].vy += d;

			DrawCrab(crab[i]);
		}

		// Release some stuff and begin again
		DeleteObject(hFolderViewMem);
		ReleaseDC(hFolderView, hFolderViewDC);
		Sleep(MILLISEC_SLEEP);
	}
	
	DeleteObject(hCrabRightBitmap);
	DeleteObject(hCrabLeftBitmap);
	DeleteObject(hCrabDownBitmap);
	DeleteObject(hCrabUpBitmap);

	return 0;
}