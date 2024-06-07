#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define N 15

// ȫ�ֱ�������
HBITMAP role, monster, hopter;			  // λͼ�ļ� ��ɫ ���� ������
HBITMAP bg, bg2, bg3;					  // ������ bg3�˵� bg2�ؿ� bg�����ͼ
HBITMAP choose, sky, sea;				  // ��ͼ
HBITMAP brokenBoard, moveBoard, board[N]; // һ���԰� �ƶ��İ� ��ͨ��
// 3 һ���� 6 �ƶ�

HDC hdc, mdc, bufdc;
HWND hWnd;
DWORD tPre, tNow;
HPEN hpen;
HBRUSH hbrush;
PAINTSTRUCT ps;
TEXTMETRIC tm;

int gameMap;					 // ѡ���ͼ
int totol;						 // ��ʷ���
int maxScore, now;				 // ��¼��ߵ÷֣���ǰ�ֲȹ��İ��
int num, dir, walk;				 // num ���ﵱǰ����  dir ���ﳯ��  isWalk ���Ƿ�����
int X, Y;						 // ���ﾲ̬��ͼ����
int x, y;						 // ��Ϸ��
int flag, maxy;					 // flag ���ҽű�־  maxy ��ǰy�ܵ�������߶�
int score;						 // ����
static char sc[100];			 // ��ʾ����
int mousex, mousey;				 // �������x,y
int a = 5, v = -10;				 // ���ٶȡ��ٶ�
int mode;						 // mode 1 ��Ϸ���� mode 0 �˵����� mode 2 ���� mode 3 ��Ϸ�������
int winX = 350, winY = 720;		 // ���ڴ�С
char *helptxt[6] = {};			 // ����
int bullet, bulletx, bullety;	 // �ӵ�
int monsteShow;					 // show 0 �����ֹ��� show 1 ���ֹ���
int zqt, zqtUse, zqtScore, num2; // ������
int isBroken;					 // һ���԰��Ƿ���
int movex = 4;					 // ����λ��
int height = 30;				 // �������߶�
int tmp = 8;					 // ���������ڰ�
const char filename[] = "histroyScore.txt";
const char filename2[] = "histroyBoard.txt";
struct point
{
	int x, y;
} plat[20], deadpos[20]; // ������  // ����ʱ����λ��
int deadx, deady;		  // ��������

FILE *fp;

// ȫ�ֺ�������
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void MyPaint(HDC hdc);
void createboard(HDC hdc, int bany, int n);
void createboard2(HDC hdc, int bany, int n);
void fresh(HDC hdc);

//****WinMain������������ڵ㺯��***********************
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	MyRegisterClass(hInstance);

	// ��ʼ��
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	GetMessage(&msg, NULL, NULL, NULL); // ��ʼ��msg
	// ��Ϸѭ��
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			tNow = GetTickCount();
			if (tNow - tPre >= 40)
				MyPaint(hdc);
		}
	}
	return msg.wParam;
}

//****���һ��������****
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "canvas";
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}

//****��ʼ������****
// ����λͼ���趨���ֳ�ʼֵ
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hWnd = CreateWindow("canvas", "Doodle Jump", WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}

	MoveWindow(hWnd, 100, 10, winX, winY, true);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	srand((unsigned)time(NULL));

	////////////////////////////////////////
	hdc = GetDC(hWnd);
	mdc = CreateCompatibleDC(hdc);
	bufdc = CreateCompatibleDC(hdc);

	// �����յ�λͼ������mdc��
	HBITMAP fullmap;
	fullmap = CreateCompatibleBitmap(hdc, winX, winY);
	SelectObject(mdc, fullmap);

	// �趨������ͼ��ʼλ�ú��ƶ�����
	// ����������ƶ�λͼ������ͼ
	role = (HBITMAP)LoadImage(NULL, "role.bmp", IMAGE_BITMAP, 564, 190, LR_LOADFROMFILE);
	monster = (HBITMAP)LoadImage(NULL, "monster.bmp", IMAGE_BITMAP, 100, 110, LR_LOADFROMFILE);
	hopter = (HBITMAP)LoadImage(NULL, "hopter.bmp", IMAGE_BITMAP, 200, 100, LR_LOADFROMFILE);

	// bg2 �����ؿ� bg3 ���ڿ�ʼ�˵� bg ������Ϸ����
	bg = (HBITMAP)LoadImage(NULL, "bg.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	bg2 = (HBITMAP)LoadImage(NULL, "bg2.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	bg3 = (HBITMAP)LoadImage(NULL, "bg3.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);

	// ��ͼ
	choose = (HBITMAP)LoadImage(NULL, "select.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	sky = (HBITMAP)LoadImage(NULL, "sky.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	sea = (HBITMAP)LoadImage(NULL, "sea.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);

	for (int i = 0; i < N; i++)
	{
		if (i == 3)
			board[i] = (HBITMAP)LoadImage(NULL, "board2.bmp", IMAGE_BITMAP, 80, 15, LR_LOADFROMFILE);
		else if (i == 6)
		{
			board[i] = (HBITMAP)LoadImage(NULL, "moveBoard.bmp", IMAGE_BITMAP, 80, 15, LR_LOADFROMFILE);
		}
		else
			board[i] = (HBITMAP)LoadImage(NULL, "board.bmp", IMAGE_BITMAP, 80, 15, LR_LOADFROMFILE);
	}
	moveBoard = (HBITMAP)LoadImage(NULL, "moveBoard.bmp", IMAGE_BITMAP, 80, 15, LR_LOADFROMFILE);
	brokenBoard = (HBITMAP)LoadImage(NULL, "brokenBoard.bmp", IMAGE_BITMAP, 80, 60, LR_LOADFROMFILE);

	num = 0;
	x = 180; // ������ͼ��ʼX����
	y = 330; // ������ͼ��ʼY����
	maxy = y;
	X = 50;	 // ��������ͼ����
	Y = 400; // ��������ͼ����

	// ��ʼ��������
	srand((unsigned)time(NULL));
	for (int i = 0; i < N; i++)
	{
		pos[i].x = rand() % 5;
		pos[i].y = 550 - 80 * i;
	}
	// ����һ���Ϊ�ƶ���
	pos[6].x = 100;
	pos[8].x = 3000;
	pos[8].y = 5000;
	MyPaint(hdc);

	return TRUE;
}

//****�Զ����ͼ����****
// ������ͼ����������������ͼ
void MyPaint(HDC hdc)
{
	if (mode == 0)
	{ // �˵�����
		// ����mdc�����ϱ���ͼ
		SelectObject(bufdc, bg3);
		BitBlt(mdc, 0, 0, winX, winY, bufdc, 0, 0, SRCCOPY);

		// ��
		SelectObject(bufdc, board[0]);
		BitBlt(mdc, 40, 440 + 95, 80, 15, bufdc, 0, 0, SRCAND);
		BitBlt(mdc, 40, 440 + 95, 80, 15, bufdc, 0, 0, SRCPAINT);

		// �������ͼ
		SelectObject(bufdc, role);
		BitBlt(mdc, X, Y, 47, 95, bufdc, num * 47, 95, SRCAND);
		BitBlt(mdc, X, Y, 47, 95, bufdc, num * 47, 0, SRCPAINT);

		// ������
		SelectObject(bufdc, hopter);
		BitBlt(mdc, X, Y - 25, 50, 50, bufdc, num2 * 50, 50, SRCAND);
		BitBlt(mdc, X, Y - 25, 50, 50, bufdc, num2 * 50, 0, SRCPAINT);
		num2++;
		if (num2 == 4)
			num2 = 0;

		// // ����
		// SelectObject(bufdc, monster);
		// BitBlt(mdc, 60, 440 + 95, 100, 55, bufdc, 0, 0, SRCAND);
		// BitBlt(mdc, 60, 440 + 95, 100, 55, bufdc, 0, 0, SRCPAINT);

		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);
		tPre = GetTickCount(); // ��¼�˴λ�ͼʱ��

		// ��������
		v = v + a;
		Y = Y + v;

		x = 180; // ��ͼ��ʼX����
		y = 330; // ��ͼ��ʼY����

		score = 0;
		if (Y >= 440 && Y <= 440 + 20 && v >= 0)
		{ // ����͵�
			v = -40;
		}

		// �����ʷ�÷�
		// ���Դ��ļ��Զ�ȡ
		fp = fopen(filename, "r");
		if (fp == NULL)
		{
			// ����ļ������ڣ��򴴽��ļ�
			fp = fopen(filename, "w");
			if (fp != NULL)
			{
				fclose(fp);
			}
		}
		else
		{
			// ��ȡ��߷�
			if (fscanf(fp, "%d", &maxScore) != 1)
			{
				// ��ȡʧ�ܣ�����һ��Ĭ��ֵ���ߴ������
				maxScore = 0;
			}
			fclose(fp);
		}
		sprintf(sc, "����ʷ��ߵ÷�%d", maxScore);
		TextOut(hdc, 10, 10, sc, lstrlen(sc));

		// �����ʷ�ܰ��
		// ���Դ��ļ��Զ�ȡ
		fp = fopen(filename2, "r");
		if (fp == NULL)
		{
			// ����ļ������ڣ��򴴽��ļ�
			fp = fopen(filename2, "w");
			if (fp != NULL)
			{
				fclose(fp);
			}
		}
		else
		{
			// ��ȡ�ܰ���
			if (fscanf(fp, "%d", &totol) != 1)
			{
				// ��ȡʧ�ܣ�����һ��Ĭ��ֵ���ߴ������
				totol = 0;
			}
			fclose(fp);
		}
		sprintf(sc, "���ۼƲȹ�%d��ľ��", totol);
		TextOut(hdc, 10, 30, sc, lstrlen(sc));
	}
	else if (mode == 1)
	{
		// ��Ϸ����
		// ��mdc�����ϱ���ͼ
		switch (gameMap)
		{
		case 0:
			SelectObject(bufdc, bg);
			break;
		case 1:
			SelectObject(bufdc, sky);
			break;
		case 2:
			SelectObject(bufdc, sea);
			break;
		}
		BitBlt(mdc, 0, 0, winX, winY, bufdc, 0, 0, SRCCOPY);

		if (dir == 1)
		{
			if (flag % 6 == 1)
				num = 1;
			else if (flag % 6 == 5)
				num = 2;
			if (walk == 0)
				num = 0;
		}
		else if (dir == 2)
		{
			if (flag % 6 == 1)
				num = 4;
			else if (flag % 6 == 5)
				num = 5;
			if (walk == 0)
				num = 3;
		}
		else if (dir == 3)
		{
			if (flag % 6 == 1)
				num = 6;
			else if (flag % 6 == 5)
				num = 7;
			if (walk == 0)
				num = 8;
		}
		else if (dir == 4)
		{
			if (flag % 6 == 1)
				num = 10;
			else if (flag % 6 == 5)
				num = 11;
			if (walk == 0)
				num = 9;
		}

		// ��������
		v = v + a;
		y = y + v;

		// �ƶ���
		pos[6].x += movex;
		if (pos[6].x >= 250)
			movex = -movex;
		if (pos[6].x <= 0)
			movex = -movex;

		// ��Խ
		if (x + 40 > 350)
			x = 0;
		if (x + 40 < 0)
			x = 320;

		fresh(hdc);
	}
	else if (mode == 2)
	{
		// ��ͼѡ�����
		mode = 3;
		SelectObject(bufdc, choose);
		BitBlt(mdc, 0, 0, winX, winY, bufdc, 0, 0, SRCCOPY);
	}
	else if (mode == 3)
	{ // ��Ϸ��������ʾ�÷�
		// ��mdc�����ϱ���ͼ
		SelectObject(bufdc, bg2);
		BitBlt(mdc, 0, 0, winX, winY, bufdc, 0, 0, SRCCOPY);
		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);

		// ��
		SelectObject(bufdc, board[0]);
		BitBlt(mdc, 120, 400, 80, 15, bufdc, 0, 0, SRCAND);
		BitBlt(mdc, 120, 400, 80, 15, bufdc, 0, 0, SRCPAINT);

		// �������ͼ
		SelectObject(bufdc, role);
		BitBlt(mdc, 135, 300, 47, 95, bufdc, num * 47, 95, SRCAND);
		BitBlt(mdc, 135, 300, 47, 95, bufdc, num * 47, 0, SRCPAINT);

		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);

		// ��ʾ�����ط�

		// ��ʾ�÷�
		sprintf(sc, "�����ĵ÷�Ϊ%d��", score);
		TextOut(hdc, 90, 440, sc, lstrlen(sc)); // ����ַ���
		tPre = GetTickCount();					// ��¼�˴λ�ͼʱ��
		num++;									// ��ֹҲ�ڶ�
		if (num == 4)
			num = 0;

		// �޸���ߵ÷�
		fp = fopen(filename, "r+"); // ���ļ����ڶ�д������ļ��������򴴽���
		if (fp == NULL)
		{
			// ����ļ���ʧ�ܣ�������д��ģʽ��
			fp = fopen(filename, "w");
			if (fp == NULL)
			{
			}
		}

		// ��ȡ�ļ��е�������ߵ÷�
		if (fscanf(fp, "%d", &maxScore) == 1)
		{
			// �����ǰ�÷ָ�����ߵ÷֣��������ߵ÷�
			if (score > maxScore)
			{
				maxScore = score;
				// ���ļ�ָ���ƻ��ļ���ʼ��
				fseek(fp, 0, SEEK_SET);
				// д���µ���ߵ÷�
				fprintf(fp, "%d", maxScore);
			}
		}
		else
		{
			// ����ļ�Ϊ�ջ��ʽ����ȷ��д�뵱ǰ�÷�
			fseek(fp, 0, SEEK_SET);
			fprintf(fp, "%d", score);
		}

		// �ر��ļ�
		fclose(fp);

		// �޸��ܰ��
		fp = fopen(filename2, "r+"); // ���ļ����ڶ�д������ļ��������򴴽���
		if (fp == NULL)
		{
			// ����ļ���ʧ�ܣ�������д��ģʽ��
			fp = fopen(filename2, "w");
			if (fp == NULL)
			{
			}
		}

		// ��ȡ�ļ��е�������ߵ÷�
		int tmp;
		if (fscanf(fp, "%d", &tmp) == 1)
		{
			fseek(fp, 0, SEEK_SET);
			// д���µ���ߵ÷�
			fprintf(fp, "%d", totol);
		}
		else
		{
			// ����ļ�Ϊ�ջ��ʽ����ȷ��д�뵱ǰ�÷�
			fseek(fp, 0, SEEK_SET);
			fprintf(fp, "%d", totol);
		}

		// �ر��ļ�
		fclose(fp);
	}
}

void fresh(HDC hdc)
{
	// ���������������Ĵ���
	if (y < 250)
	{
		for (int i = 0; i < N; i++)
		{
			y = 250;
			pos[i].y = pos[i].y - v;
			pos[8].y = pos[0].y;
			if (pos[i].y > 540)
			{
				if (i == 0)
					zqt = 0;
				if (i == 5)
					monsteShow = 0;
				score++;
				if (i == 3)
					isBroken = 0;
				pos[i].y = -40;
				pos[i].x = rand() % 5;
			}
		}
	}
	for (int i = 0; i < N; i++)
	{
		if (i == 3)
		{
			if (isBroken == 1)
			{
				continue;
			}
			else if (isBroken == 0)
			{
				createboard2(hdc, pos[i].y, pos[i].x);
			}
		}
		else if (i == 6)
		{
			if (y >= pos[i].y - 20 && y <= pos[i].y + 20 && x >= pos[i].x && x <= pos[i].x + 80 && v >= 0)
			{
				// ˵�����������
				v = -40;
				y = pos[i].y;
			}
		}
		else if (i == 5)
		{
			if (monsteShow == 1 && y >= pos[i].y - 20 && y <= pos[i].y + 20 && x >= pos[i].x && x <= pos[i].x + 80 && v >= 0)
			{
				Sleep(300);
				mode = 0;
				X = 50;
				Y = 400;
				v = -10;
				isBroken = 0;
				mode = 3;
				for (int i = 0; i < N; i++)
				{
					srand((unsigned)time(NULL) * i);

					pos[i].x = rand() % 5;
					pos[i].y = 500 - 80 * i;
				}
				pos[6].x = 100;
			}
		}
		// ���������ѵ�ʹ��
		if (zqt == 1 && y >= pos[8].y - 25 && y <= pos[8].y + 25 && x >= 350 / 6 * pos[8].x + 25 && x <= 350 / 6 * pos[8].x + 25 + 80 && v >= 0)
		{
			if (zqt == 1)
			{
				zqtUse = 1;
				zqt = 0;
				zqtScore = score;
			}
		}
		createboard(hdc, pos[i].y, pos[i].x);
	}
}

void createboard(HDC hdc, int bany, int n)
{
	//
	int banx = 350 / 6 * n;
	hpen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0)); // ���»���.
	SelectObject(hdc, hpen);					   // ѡ�뻭��.
	if (y >= bany - 20 && y <= bany + 20 && x >= banx && x <= banx + 80 && v >= 0)
	{
		// ˵������ƽ̨
		v = -40;
		y = bany;
		now++;
		totol++;
	}
	DeleteObject(hpen);
}

void createboard2(HDC hdc, int bany, int n)
{ // broken
	//
	int banx = 350 / 6 * n;
	hpen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0)); // ���»���.
	SelectObject(hdc, hpen);

	if (y >= bany - 25 && y <= bany + 25 && x >= banx && x <= banx + 80 && v >= 0)
	{
		// ���������
		isBroken = 1;
		v = -40;
		y = bany;
	}
	DeleteObject(hpen);
}

//****��Ϣ������***********************************
// 1.���¡�Esc������������
// 2.���·����������ͼ����
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		MessageBox(NULL, "�� �������ƶ����ո���Ծ��", "��Ϸ˵��", MB_OK);
		break;
	case WM_KEYUP:
		walk = 0;
		break;
	case WM_LBUTTONDOWN:
		mousex = LOWORD(lParam);
		mousey = HIWORD(lParam);
		if (mode == 0 && mousex >= 175 && mousex <= 328 && mousey >= 293 && mousey <= 342)
		{
			mode = 1;
		}

		if (mode == 2 && mousex >= 100 && mousex <= 250 && mousey >= 248 && mousey <= 296)
		{
			gameMap = 0; // ��ͨ��ͼ
		}
		else if (mode == 2 && mousex >= 100 && mousex <= 250 && mousey >= 313 && mousey <= 360)
		{
			gameMap = 1; // ��յ�ͼ
		}
		else if (mode == 2 && mousex >= 100 && mousex <= 250 && mousey >= 378 && mousey <= 427)
		{
			gameMap = 2; // �����ͼ
		}
		else if (mode == 2 && mousex >= 100 && mousex <= 250 && mousey >= 248 && mousey <= 296)
		{
			PostQuitMessage(0); // ��������
		}

		if (mode == 3 && mousex >= 132 && mousex <= 218 && mousey >= 443 && mousey <= 491)
		{
			mode = 0;
		}

		break;
	case WM_KEYDOWN: // ���¼�����Ϣ
		// �жϰ������������
		switch (wParam)
		{
		case VK_ESCAPE:			// ���¡�Esc����
			PostQuitMessage(0); // ��������
			break;
		case VK_UP: // ���¡�������
			// �Ȱ���Ŀǰ���ƶ�������������ͼ�������������������������ƶ�������ÿ�ΰ���һ�ΰ����ƶ�10����λ����������������ͼ�����X��Yֵ�������ж������Ƿ񳬳����������������ٴ�����
			if (mode == 3)
				mode = 0;
			if (mode == 2)
				mode = 1;
			if (mode == 1 && bullet == 0)
			{
				bullet = 1;
				bulletx = x + 35;
				bullety = y - 55;
			}
			walk = 1;
			break;
		case VK_DOWN: // ���¡�������
			walk = 1;
			a = 0;
			v = 0;
			y++;
			break;
		case VK_LEFT: // ���¡�������
			if (mode == 1)
			{
				walk = 1;
				dir = 3;
				x -= 30;
			}
			break;
		case VK_RIGHT: // ���¡�������
			if (mode == 1)
			{
				walk = 1;
				dir = 4;
				x += 30;
			}
			break;
		case VK_SPACE: // ���¿ո��-������
			if (mode == 1)
			{
				y -= height;
			}
			break;
		}
		break;

	case WM_DESTROY: // ���ڽ�����Ϣ

		DeleteDC(mdc);
		DeleteDC(bufdc);
		DeleteObject(role);
		DeleteObject(monster);
		DeleteObject(hopter);
		DeleteObject(bg3);
		DeleteObject(bg2);
		DeleteObject(bg);
		DeleteObject(brokenBoard);
		DeleteObject(moveBoard);
		for (int i = 0; i < N; i++)
			DeleteObject(board[i]);
		ReleaseDC(hWnd, hdc);
		PostQuitMessage(0);
		break;
	default: // ������Ϣ
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
