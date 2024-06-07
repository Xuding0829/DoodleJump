#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define N 15

// 全局变量声明
HBITMAP role, hopter;														 // 位图文件 角色  竹蜻蜓
HBITMAP bg, bg2, bg3;														 // 背景， bg3菜单 bg2重开 bg经典地图
HBITMAP map, sky, sea;														 // 地图
HBITMAP brokenBoard, moveBoard, board[N];									 // 一次性板 移动的板 普通板 // 3 一次性 6 移动
HBITMAP monster, monster0, monster1, monster2, monster3, monster4, monster5; // 怪兽
HBITMAP Cmonster0, Cmonster1, Cmonster2, Cmonster3, Cmonster4, Cmonster5;	 // 怪兽
HBITMAP ufo, Cufo;															 // ufo
HBITMAP doodle, Cdoodle;													 // doodle
HBITMAP pay;																 // 收款码

HDC hdc, mdc, bufdc;
HWND hWnd;
DWORD tPre, tNow;
HPEN hpen;
HBRUSH hbrush;
PAINTSTRUCT ps;
TEXTMETRIC tm;

int gameMap = -1;  // 选择地图
int totol;		   // 历史板块
int maxScore, now; // 记录最高得分，当前局踩过的板块
int num;
int X, Y;
int x, y;						// 游戏中
int score;						// 分数
static char sc[100];			// 显示分数
int mousex, mousey;				// 鼠标坐标x,y
int a = 5, v = -10;				// 加速度、速度
int mode;						// mode 1 游戏界面 mode 0 菜单界面 mode 2 选择地图 mode 3 游戏结算界面 4 支付界面
int winX = 350, winY = 720;		// 窗口大小
char *helptxt[6] = {};			// 帮助
int monsterShow;				// show 0 不出现怪兽 show 1 出现怪兽
int zqt, zqtUse, zqtScore, cnt; // 竹蜻蜓
int isBroken;					// 一次性板是否坏了
int movex = 4;					// 板子位移
int height = 30;				// 二段跳高度
int mx, my, mv, no;				// 怪兽坐标 速度
int payShow;					// 显示支付界面
const char filename[] = "data/histroyScore.txt";
const char filename2[] = "data/histroyBoard.txt";
struct point
{
	int x, y;
} pos[20], deadpos[20]; // 板坐标  // 死亡时板块的位置
int deadx, deady;		// 死亡坐标

FILE *fp;

// 全局函数声明
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void MyPaint(HDC hdc);

//****WinMain函数，程序入口点函数***********************
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	MyRegisterClass(hInstance);

	// 初始化
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	GetMessage(&msg, NULL, NULL, NULL); // 初始化msg
	// 游戏循环
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

//****设计一个窗口类****
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

//****初始化函数****
// 加载位图并设定各种初始值
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

	hdc = GetDC(hWnd);
	mdc = CreateCompatibleDC(hdc);
	bufdc = CreateCompatibleDC(hdc);

	// 建立空的位图并置入mdc中
	HBITMAP fullmap;
	fullmap = CreateCompatibleBitmap(hdc, winX, winY);
	SelectObject(mdc, fullmap);

	// 设定人物贴图初始位置和移动方向
	// 载入各连续移动位图及背景图
	role = (HBITMAP)LoadImage(NULL, "res/role.bmp", IMAGE_BITMAP, 564, 190, LR_LOADFROMFILE);
	monster = (HBITMAP)LoadImage(NULL, "res/monster.bmp", IMAGE_BITMAP, 100, 110, LR_LOADFROMFILE);
	hopter = (HBITMAP)LoadImage(NULL, "res/hopter.bmp", IMAGE_BITMAP, 200, 100, LR_LOADFROMFILE);
	monster0 = (HBITMAP)LoadImage(NULL, "res/monster0.bmp", IMAGE_BITMAP, 195, 97, LR_LOADFROMFILE);
	monster1 = (HBITMAP)LoadImage(NULL, "res/monster1.bmp", IMAGE_BITMAP, 43, 55, LR_LOADFROMFILE);
	monster2 = (HBITMAP)LoadImage(NULL, "res/monster2.bmp", IMAGE_BITMAP, 78, 30, LR_LOADFROMFILE);
	monster3 = (HBITMAP)LoadImage(NULL, "res/monster3.bmp", IMAGE_BITMAP, 71, 48, LR_LOADFROMFILE);
	monster4 = (HBITMAP)LoadImage(NULL, "res/monster4.bmp", IMAGE_BITMAP, 80, 55, LR_LOADFROMFILE);
	monster5 = (HBITMAP)LoadImage(NULL, "res/monster5.bmp", IMAGE_BITMAP, 44, 55, LR_LOADFROMFILE);
	Cmonster0 = (HBITMAP)LoadImage(NULL, "res/Cmonster0.bmp", IMAGE_BITMAP, 195, 97, LR_LOADFROMFILE);
	Cmonster1 = (HBITMAP)LoadImage(NULL, "res/Cmonster1.bmp", IMAGE_BITMAP, 43, 55, LR_LOADFROMFILE);
	Cmonster2 = (HBITMAP)LoadImage(NULL, "res/Cmonster2.bmp", IMAGE_BITMAP, 78, 30, LR_LOADFROMFILE);
	Cmonster3 = (HBITMAP)LoadImage(NULL, "res/Cmonster3.bmp", IMAGE_BITMAP, 71, 48, LR_LOADFROMFILE);
	Cmonster4 = (HBITMAP)LoadImage(NULL, "res/Cmonster4.bmp", IMAGE_BITMAP, 80, 55, LR_LOADFROMFILE);
	Cmonster5 = (HBITMAP)LoadImage(NULL, "res/Cmonster5.bmp", IMAGE_BITMAP, 44, 55, LR_LOADFROMFILE);
	ufo = (HBITMAP)LoadImage(NULL, "res/ufo.bmp", IMAGE_BITMAP, 84, 126, LR_LOADFROMFILE);
	Cufo = (HBITMAP)LoadImage(NULL, "res/Cufo.bmp", IMAGE_BITMAP, 84, 126, LR_LOADFROMFILE);
	doodle = (HBITMAP)LoadImage(NULL, "res/doodle.bmp", IMAGE_BITMAP, 85, 85, LR_LOADFROMFILE);
	Cdoodle = (HBITMAP)LoadImage(NULL, "res/Cdoodle.bmp", IMAGE_BITMAP, 85, 85, LR_LOADFROMFILE);
	// bg2 用于重开 bg3 用于开始菜单 bg 用于游戏界面
	bg = (HBITMAP)LoadImage(NULL, "res/bg.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	bg2 = (HBITMAP)LoadImage(NULL, "res/bg2.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	bg3 = (HBITMAP)LoadImage(NULL, "res/bg3.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	// 地图
	map = (HBITMAP)LoadImage(NULL, "res/map.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	sky = (HBITMAP)LoadImage(NULL, "res/sky.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	sea = (HBITMAP)LoadImage(NULL, "res/sea.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	for (int i = 0; i < N; i++)
	{
		if (i == 3)
			board[i] = (HBITMAP)LoadImage(NULL, "res/board2.bmp", IMAGE_BITMAP, 80, 15, LR_LOADFROMFILE);
		else if (i == 6)
		{
			board[i] = (HBITMAP)LoadImage(NULL, "res/moveBoard.bmp", IMAGE_BITMAP, 80, 15, LR_LOADFROMFILE);
		}
		else
			board[i] = (HBITMAP)LoadImage(NULL, "res/board.bmp", IMAGE_BITMAP, 80, 15, LR_LOADFROMFILE);
	}
	moveBoard = (HBITMAP)LoadImage(NULL, "res/moveBoard.bmp", IMAGE_BITMAP, 80, 15, LR_LOADFROMFILE);
	brokenBoard = (HBITMAP)LoadImage(NULL, "res/brokenBoard.bmp", IMAGE_BITMAP, 80, 60, LR_LOADFROMFILE);
	// 支付
	pay = (HBITMAP)LoadImage(NULL, "res/pay.bmp", IMAGE_BITMAP, 206, 152, LR_LOADFROMFILE);

	// 结算界面的怪兽
	mx = 125;
	my = 350;
	mv = -40;

	// 初始化菜单中doodle坐标
	X = 55;
	Y = 399;

	no = rand() % 7;

	// 初始化板坐标
	srand((unsigned)time(NULL));
	for (int i = 0; i < N; i++)
	{
		pos[i].x = rand() % 5;
		pos[i].y = 550 - 80 * i;
	}

	x = 350 / 6 * pos[0].x + 5;
	y = pos[0].y;

	MyPaint(hdc);

	return TRUE;
}

//****自定义绘图函数****
// 人物贴图坐标修正及窗口贴图
void MyPaint(HDC hdc)
{
	if (mode == 0)
	{ // 菜单界面
		// 先在mdc中贴上背景图
		SelectObject(bufdc, bg3);
		BitBlt(mdc, 0, 0, winX, winY, bufdc, 0, 0, SRCCOPY);

		// 板
		SelectObject(bufdc, board[0]);
		BitBlt(mdc, 60, 450 + 95, 80, 15, bufdc, 0, 0, SRCAND);
		BitBlt(mdc, 60, 450 + 95, 80, 15, bufdc, 0, 0, SRCPAINT);

		// doodle
		SelectObject(bufdc, Cdoodle);
		BitBlt(mdc, X, Y, 85, 85, bufdc, 0, 0, SRCAND);
		SelectObject(bufdc, doodle);
		BitBlt(mdc, X, Y, 85, 85, bufdc, 0, 0, SRCPAINT);

		// 竹蜻蜓
		SelectObject(bufdc, hopter);
		BitBlt(mdc, X + 15, Y - 15, 50, 50, bufdc, cnt * 50, 50, SRCAND);
		BitBlt(mdc, X + 15, Y - 15, 50, 50, bufdc, cnt * 50, 0, SRCPAINT);
		cnt++;
		if (cnt == 4)
			cnt = 0;

		// ufo
		SelectObject(bufdc, Cufo);
		BitBlt(mdc, 215, 390, 84, 126, bufdc, 0, 0, SRCAND);
		SelectObject(bufdc, ufo);
		BitBlt(mdc, 215, 390, 84, 126, bufdc, 0, 0, SRCPAINT);

		// 怪兽
		SelectObject(bufdc, monster);
		BitBlt(mdc, 210, 470, 100, 55, bufdc, 0, 55, SRCAND);
		BitBlt(mdc, 210, 470, 100, 55, bufdc, 0, 0, SRCPAINT);

		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);
		tPre = GetTickCount();
		// 记录此次绘图时间

		v = v + a;
		Y = Y + v;

		x = 180; // 贴图起始X坐标
		y = 330; // 贴图起始Y坐标

		score = 0;
		if (Y >= 440 && Y <= 440 + 20 && v >= 0)
		{
			v = -40; // 到最低点
		}

		// 输出历史得分
		// 尝试打开文件以读取
		fp = fopen(filename, "r");
		if (fp == NULL)
		{
			// 如果文件不存在，则创建文件
			fp = fopen(filename, "w");
			if (fp != NULL)
			{
				fclose(fp);
			}
		}
		else
		{
			// 读取最高分
			if (fscanf(fp, "%d", &maxScore) != 1)
			{
				// 读取失败，设置一个默认值或者处理错误
				maxScore = 0;
			}
			fclose(fp);
		}
		sprintf(sc, "您历史最高得分%d", maxScore);
		TextOut(hdc, 10, 10, sc, lstrlen(sc));

		// 输出历史总板块
		// 尝试打开文件以读取
		fp = fopen(filename2, "r");
		if (fp == NULL)
		{
			// 如果文件不存在，则创建文件
			fp = fopen(filename2, "w");
			if (fp != NULL)
			{
				fclose(fp);
			}
		}
		else
		{
			// 读取总板数
			if (fscanf(fp, "%d", &totol) != 1)
			{
				// 读取失败，设置一个默认值或者处理错误
				totol = 0;
			}
			fclose(fp);
		}
		sprintf(sc, "您累计踩过%d个木板", totol);
		TextOut(hdc, 10, 30, sc, lstrlen(sc));
	}
	else if (mode == 1)
	{
		// 游戏界面
		// 在mdc中贴上背景图

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

		// 板块贴图
		// 3 一次性 6 移动
		for (int i = 0; i < N; i++)
		{
			// 移动版贴图
			if (i == 6)
			{
				SelectObject(bufdc, moveBoard);
				BitBlt(mdc, pos[i].x + 25, pos[i].y + 95, 80, 15, bufdc, 0, 0, SRCAND);
				BitBlt(mdc, pos[i].x + 25, pos[i].y + 95, 80, 15, bufdc, 0, 0, SRCPAINT);
				continue;
			}

			// 一次性板贴图
			if (i == 3 && isBroken == 1)
			{
				SelectObject(bufdc, brokenBoard);
				BitBlt(mdc, 350 / 6 * pos[i].x + 25, pos[i].y + 95, 80, 30, bufdc, 0, 30, SRCAND);
				BitBlt(mdc, 350 / 6 * pos[i].x + 25, pos[i].y + 95, 80, 30, bufdc, 0, 0, SRCPAINT);
				continue;
			}

			SelectObject(bufdc, board[i]); // 普通板
			BitBlt(mdc, 350 / 6 * pos[i].x + 25, pos[i].y + 85, 80, 15, bufdc, 0, 0, SRCAND);
			BitBlt(mdc, 350 / 6 * pos[i].x + 25, pos[i].y + 85, 80, 15, bufdc, 0, 0, SRCPAINT);
		}

		// doodle
		SelectObject(bufdc, Cdoodle);
		BitBlt(mdc, x + 5, y, 85, 85, bufdc, 0, 0, SRCAND);
		SelectObject(bufdc, doodle);
		BitBlt(mdc, x + 5, y, 85, 85, bufdc, 0, 0, SRCPAINT);

		// 竹蜻蜓
		if (zqt)
		{
			SelectObject(bufdc, hopter);
			BitBlt(mdc, 350 / 6 * pos[0].x + 25, pos[0].y + 40, 50, 50, bufdc, 50, 50, SRCAND);
			BitBlt(mdc, 350 / 6 * pos[0].x + 25, pos[0].y + 40, 50, 50, bufdc, 50, 0, SRCPAINT);
		}

		if (zqtUse)
		{
			SelectObject(bufdc, hopter);
			BitBlt(mdc, x + 20, y - 15, 50, 50, bufdc, cnt * 50, 50, SRCAND);
			BitBlt(mdc, x + 20, y - 15, 50, 50, bufdc, cnt * 50, 0, SRCPAINT);
			cnt++;
			if (cnt == 4)
				cnt = 0;
		}

		// 怪兽
		if (monsterShow)
		{
			// 随机怪兽的形状
			switch (no)
			{
			case 0:
				SelectObject(bufdc, Cmonster0);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 195, 97, bufdc, 0, 0, SRCAND);
				SelectObject(bufdc, monster0);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 195, 97, bufdc, 0, 0, SRCPAINT);
				break;
			case 1:
				SelectObject(bufdc, Cmonster1);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 43, 55, bufdc, 0, 0, SRCAND);
				SelectObject(bufdc, monster1);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 43, 55, bufdc, 0, 0, SRCPAINT);
				break;
			case 2:
				SelectObject(bufdc, Cmonster2);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 78, 30, bufdc, 0, 0, SRCAND);
				SelectObject(bufdc, monster);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 78, 30, bufdc, 0, 0, SRCPAINT);
				break;
			case 3:
				SelectObject(bufdc, Cmonster3);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 71, 48, bufdc, 0, 0, SRCAND);
				SelectObject(bufdc, monster3);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 71, 48, bufdc, 0, 0, SRCPAINT);
				break;
			case 4:
				SelectObject(bufdc, Cmonster4);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 80, 55, bufdc, 0, 0, SRCAND);
				SelectObject(bufdc, monster4);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 80, 55, bufdc, 0, 0, SRCPAINT);
				break;
			case 5:
				SelectObject(bufdc, Cmonster5);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 44, 55, bufdc, 0, 0, SRCAND);
				SelectObject(bufdc, monster5);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 44, 55, bufdc, 0, 0, SRCPAINT);
				break;
			case 6:
				SelectObject(bufdc, monster);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 100, 55, bufdc, 0, 55, SRCAND);
				BitBlt(mdc, 350 / 6 * pos[5].x + 25, pos[5].y + 35, 100, 55, bufdc, 0, 0, SRCPAINT);
				break;
			}
		}

		// 贴图完成，显示
		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);

		tPre = GetTickCount(); // 记录此次绘图时间

		// 移动板
		pos[6].x += movex;
		if (pos[6].x >= 250)
			movex = -movex;
		if (pos[6].x <= 0)
			movex = -movex;

		v = v + a;
		y = y + v;

		if (x + 40 > 350)
			x = 0;
		if (x + 40 < 0)
			x = 320;

		// 动态效果
		if (y < 250)
		{
			for (int i = 0; i < N; i++)
			{
				y = 250;
				pos[i].y = pos[i].y - v;
				if (pos[i].y > 540)
				{
					if (i == 0)
						zqt = 0;
					else if (i == 3)
						isBroken = 0;
					else if (i == 5)
						monsterShow = 0;

					score++;
					pos[i].x = rand() % 5;
					pos[i].y = -40;
				}
			}
		}

		for (int i = 0; i < N; i++)
		{
			if (i == 0)
			{
				if (zqt == 1 && y >= pos[i].y - 25 && y <= pos[i].y + 25 && x >= 350 / 6 * pos[i].x && x <= 350 / 6 * pos[i].x + 25 + 80 && v >= 0)
				{
					if (zqt == 1)
					{
						zqtUse = 1;
						zqt = 0;
						zqtScore = score;
					}
				}
			}
			else if (i == 3)
			{
				if (isBroken == 1)
				{
					continue;
				}
				else
				{
					if (y >= pos[i].y - 20 && y <= pos[i].y + 20 && x >= 350 / 6 * pos[i].x && x <= 350 / 6 * pos[i].x + 80 && v >= 0)
					{
						isBroken = 1;
						v = -40;
						y = pos[i].y;
					}
				}
			}
			else if (i == 5)
			{
				if (monsterShow == 1 && y >= pos[i].y - 20 && y <= pos[i].y + 20 && x >= 350 / 6 * pos[i].x && x <= 350 / 6 * pos[i].x + 20 && v >= 0)
				{
					Sleep(300);
					mode = 0;
					X = 50;
					Y = 400;
					v = -10;
					isBroken = 0;
					mode = 3;
					for (int j = 0; j < N; j++)
					{
						srand((unsigned)time(NULL));
						pos[j].x = rand() % 5;
						pos[j].y = 550 - 80 * j;
					}
					pos[6].x = 100;
				}
			}
			else if (i == 6)
			{
				if (y >= pos[i].y - 20 && y <= pos[i].y + 20 && x >= pos[i].x && x <= pos[i].x + 80 && v >= 0)
				{
					v = -40;
					y = pos[i].y;
				}
			}
			if (y >= pos[i].y - 20 && y <= pos[i].y + 20 && x >= 350 / 6 * pos[i].x && x <= 350 / 6 * pos[i].x + 80 && v >= 0)
			{
				v = -40;
				y = pos[i].y;
				now++;
				totol++;
			}
		}

		// 生成竹蜻蜓
		if (score % 6 == 0 && zqtUse == 0)
		{
			zqt = 1;
		}

		// 使用竹蜻蜓
		if (zqtUse == 1)
		{
			v = -50;
			if (score >= zqtScore + 50)
			{
				zqtUse = 0;
				zqt = 0;
			}
		}

		// 生成怪兽
		if (score % 10 == 0 && score != 0)
		{
			monsterShow = 1;
		}

		// 死亡
		if (y >= 720)
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
				pos[i].x = rand() % 5;
				pos[i].y = 550 - 80 * i;
			}
			pos[6].x = 100;
		}

		sprintf(sc, "得分：%d", score);
		TextOut(hdc, 10, 10, sc, lstrlen(sc));
	}
	else if (mode == 2)
	{
		// 地图选择界面
		Sleep(300);
		SelectObject(bufdc, map);
		BitBlt(mdc, 0, 0, winX, winY, bufdc, 0, 0, SRCCOPY);

		SelectObject(bufdc, role);
		BitBlt(mdc, 50, 520, 47, 95, bufdc, num * 47, 95, SRCAND);
		BitBlt(mdc, 50, 520, 47, 95, bufdc, num * 47, 0, SRCPAINT);
		num++;
		if (num == 8)
			num = 0;

		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);
	}
	else if (mode == 3)
	{ // 游戏结束，显示得分
		// 在mdc中贴上背景图
		SelectObject(bufdc, bg2);
		BitBlt(mdc, 0, 0, winX, winY, bufdc, 0, 0, SRCCOPY);
		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);

		// 板
		SelectObject(bufdc, board[0]);
		BitBlt(mdc, 120, 400, 80, 15, bufdc, 0, 0, SRCAND);
		BitBlt(mdc, 120, 400, 80, 15, bufdc, 0, 0, SRCPAINT);

		// 怪兽的贴图
		SelectObject(bufdc, Cmonster3);
		BitBlt(mdc, mx, my, 71, 48, bufdc, 0, 0, SRCAND);
		SelectObject(bufdc, monster3);
		BitBlt(mdc, mx, my, 71, 48, bufdc, 0, 0, SRCPAINT);

		// 显示收款码
		if (payShow)
		{
			SelectObject(bufdc, pay);
			BitBlt(mdc, 70, 250, 206, 152, bufdc, 0, 0, SRCCOPY);
		}

		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);
		tPre = GetTickCount();

		mv = mv + a;
		my = my + mv;

		if (my >= 330 && my <= 350 && mv >= 0)
		{ // 到最低点
			mv = -40;
		}

		// 显示得分
		sprintf(sc, "您最后的得分为%d分", score);
		TextOut(hdc, 90, 440, sc, lstrlen(sc)); // 输出字符串
		tPre = GetTickCount();					// 记录此次绘图时间
		num++;									// 静止也在动
		if (num == 4)
			num = 0;

		// 修改最高得分
		fp = fopen(filename, "r+"); // 打开文件用于读写，如果文件不存在则创建它
		if (fp == NULL)
		{
			// 如果文件打开失败，尝试以写入模式打开
			fp = fopen(filename, "w");
			if (fp == NULL)
			{
			}
		}

		// 读取文件中的现有最高得分
		if (fscanf(fp, "%d", &maxScore) == 1)
		{
			// 如果当前得分高于最高得分，则更新最高得分
			if (score > maxScore)
			{
				maxScore = score;
				// 将文件指针移回文件开始处
				fseek(fp, 0, SEEK_SET);
				// 写入新的最高得分
				fprintf(fp, "%d", maxScore);
			}
		}
		else
		{
			// 如果文件为空或格式不正确，写入当前得分
			fseek(fp, 0, SEEK_SET);
			fprintf(fp, "%d", score);
		}

		// 关闭文件
		fclose(fp);

		// 修改总板块
		fp = fopen(filename2, "r+"); // 打开文件用于读写，如果文件不存在则创建它
		if (fp == NULL)
		{
			// 如果文件打开失败，尝试以写入模式打开
			fp = fopen(filename2, "w");
			if (fp == NULL)
			{
			}
		}

		// 读取文件中的现有最高得分
		int tmp;
		if (fscanf(fp, "%d", &tmp) == 1)
		{
			fseek(fp, 0, SEEK_SET);
			// 写入新的最高得分
			fprintf(fp, "%d", totol);
		}
		else
		{
			// 如果文件为空或格式不正确，写入当前得分
			fseek(fp, 0, SEEK_SET);
			fprintf(fp, "%d", totol);
		}

		// 关闭文件
		fclose(fp);
	}
}

//****消息处理函数***********************************
// 1.按下【Esc】键结束程序
// 2.按下方向键重设贴图坐标
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		MessageBox(NULL, "← →控制移动，空格跳跃。\n 开始你的游戏吧~^v^~", "游戏说明", MB_OK);
		break;
	case WM_LBUTTONDOWN:
		mousex = LOWORD(lParam);
		mousey = HIWORD(lParam);
		if (mode == 0 && mousex >= 175 && mousex <= 328 && mousey >= 293 && mousey <= 342)
		{
			mode = 2; // 点击play，挑选地图
		}

		// 地图选择
		if (mode == 2 && mousex >= 100 && mousex <= 250 && mousey >= 248 && mousey <= 296)
		{
			gameMap = 0; // 普通地图
			mode = 1;
		}
		else if (mode == 2 && mousex >= 100 && mousex <= 250 && mousey >= 313 && mousey <= 360)
		{
			gameMap = 1; // 天空地图
			mode = 1;
		}
		else if (mode == 2 && mousex >= 100 && mousex <= 250 && mousey >= 378 && mousey <= 427)
		{
			gameMap = 2; // 海洋地图
			mode = 1;
		}
		else if (mode == 2 && mousex >= 132 && mousex <= 218 && mousey >= 443 && mousey <= 491)
		{
			PostQuitMessage(0);
			mode = 0;
		}

		// 重玩
		if (mode == 3 && mousex >= 74 && mousex <= 238 && mousey >= 463 && mousey <= 522)
		{
			mode = 0;
		}
		// 支付
		else if (mode == 3 && mousex >= 63 && mousex <= 267 && mousey >= 542 && mousey <= 580)
		{
			payShow = 1;
		}
		else if (mode == 3 && mousex >= 70 && mousex <= 280 && mousey >= 250 && mousey <= 410)
		{
			payShow = 0;
		}
		break;
	case WM_KEYDOWN: // 按下键盘消息
		// 判断按键的虚拟键码
		switch (wParam)
		{
		case VK_ESCAPE:			// 按下【Esc】键
			PostQuitMessage(0); // 结束程序
			break;
		case VK_LEFT: // 按下【←】键
			if (mode == 1)
			{
				x -= 30;
			}
			break;
		case VK_RIGHT: // 按下【→】键
			if (mode == 1)
			{
				x += 30;
			}
			break;
		case VK_SPACE: // 按下空格键-二段跳
			if (mode == 1)
			{
				y -= height;
			}
			break;
		}
		break;
	case WM_DESTROY: // 窗口结束消息
		DeleteDC(mdc);
		DeleteDC(bufdc);
		DeleteObject(role);
		DeleteObject(monster);
		DeleteObject(hopter);
		DeleteObject(bg3);
		DeleteObject(bg2);
		DeleteObject(bg);
		DeleteObject(pay);
		DeleteObject(doodle);
		DeleteObject(Cdoodle);
		DeleteObject(ufo);
		DeleteObject(Cufo);
		DeleteObject(map);
		DeleteObject(sky);
		DeleteObject(sea);
		DeleteObject(brokenBoard);
		DeleteObject(moveBoard);
		DeleteObject(monster0);
		DeleteObject(monster1);
		DeleteObject(monster2);
		DeleteObject(monster3);
		DeleteObject(monster4);
		DeleteObject(monster5);
		DeleteObject(Cmonster0);
		DeleteObject(Cmonster1);
		DeleteObject(Cmonster2);
		DeleteObject(Cmonster3);
		DeleteObject(Cmonster4);
		DeleteObject(Cmonster5);
		DeleteObject(pay);
		for (int i = 0; i < N; i++)
			DeleteObject(board[i]);
		ReleaseDC(hWnd, hdc);
		// PlaySound(NULL, NULL, NULL); // 停止播放
		// mciSendString("CLOSE MUSIC", NULL, 0, NULL); // 关闭
		PostQuitMessage(0);
		break;
	default: // 其他消息
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
