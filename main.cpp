#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

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

int gameMap = -1;				// 选择地图
int totol;						// 历史板块
int maxScore, now;				// 记录最高得分，当前局踩过的板块
int num, dir, walk;				// num 人物当前动画  dir 人物朝向  isWalk 人是否行走
int X, Y;						// 人物静态贴图坐标
int x, y;						// 游戏中
int flag, maxy;					// flag 左右脚标志  maxy 当前y能到达的最大高度
int score;						// 分数
static char sc[100];			// 显示分数
int mousex, mousey;				// 鼠标坐标x,y
int a = 5, v = -10;				// 加速度、速度
int mode;						// mode 1 游戏界面 mode 0 菜单界面 mode 2 选择地图 mode 3 游戏结算界面 4 支付界面
int winX = 350, winY = 720;		// 窗口大小
char *helptxt[6] = {};			// 帮助
int bullet, bulletx, bullety;	// 子弹
int monsterShow;				// show 0 不出现怪兽 show 1 出现怪兽
int zqt, zqtUse, zqtScore, cnt; // 竹蜻蜓
int isBroken;					// 一次性板是否坏了
int movex = 4;					// 板子位移
int height = 30;				// 二段跳高度
int tmp = 8;					// 竹蜻蜓所在版
int mx, my, mv;					// 怪兽坐标 速度
int payShow;					// 显示支付界面
const char filename[] = "data/histroyScore.txt";
const char filename2[] = "data/histroyBoard.txt";
struct point
{
	int x, y;
} plat[20], deadplat[20]; // 板坐标  // 死亡时板块的位置
int deadx, deady;		  // 死亡坐标

FILE *fp;

// 全局函数声明
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void MyPaint(HDC hdc);
void fresh(HDC hdc);

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

	// 导入音频资源
	// PlaySound(MAKEINTRESOURCE(IDR_DENG), NULL, SND_RESOURCE | SND_ASYNC | SND_LOOP);

	num = 0;
	x = 180;
	y = 330;
	maxy = y;

	// 结算界面的怪兽
	mx = 125;
	my = 350;
	mv = -40;

	// 初始化菜单中doodle坐标
	X = 55;
	Y = 399;

	// 初始化板坐标
	srand((unsigned)time(NULL));
	for (int i = 0; i < N; i++)
	{
		plat[i].x = rand() % 5;
		plat[i].y = 550 - 80 * i;
	}
	plat[6].x = 100;

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
				BitBlt(mdc, plat[i].x + 25, plat[i].y + 95, 80, 15, bufdc, 0, 0, SRCAND);
				BitBlt(mdc, plat[i].x + 25, plat[i].y + 95, 80, 15, bufdc, 0, 0, SRCPAINT);
				continue;
			}

			// 一次性板贴图
			if (i == 3 && isBroken == 1)
			{
				SelectObject(bufdc, brokenBoard);
				BitBlt(mdc, 350 / 6 * plat[i].x + 25, plat[i].y + 95, 80, 30, bufdc, 0, 30, SRCAND);
				BitBlt(mdc, 350 / 6 * plat[i].x + 25, plat[i].y + 95, 80, 30, bufdc, 0, 0, SRCPAINT);
				continue;
			}

			SelectObject(bufdc, board[i]); // 普通板
			BitBlt(mdc, 350 / 6 * plat[i].x + 25, plat[i].y + 85, 80, 15, bufdc, 0, 0, SRCAND);
			BitBlt(mdc, 350 / 6 * plat[i].x + 25, plat[i].y + 85, 80, 15, bufdc, 0, 0, SRCPAINT);
		}

		// 移动板
		plat[6].x += movex;
		if (plat[6].x >= 250)
			movex = -movex;
		if (plat[6].x <= 0)
			movex = -movex;

		// 人
		SelectObject(bufdc, role);
		BitBlt(mdc, x, y, 47, 95, bufdc, num * 47, 95, SRCAND);
		BitBlt(mdc, x, y, 47, 95, bufdc, num * 47, 0, SRCPAINT);

		// 竹蜻蜓
		if (zqt)
		{
			SelectObject(bufdc, hopter);
			BitBlt(mdc, 350 / 6 * plat[tmp].x + 25, plat[tmp].y + 40, 50, 50, bufdc, 50, 50, SRCAND);
			BitBlt(mdc, 350 / 6 * plat[tmp].x + 25, plat[tmp].y + 40, 50, 50, bufdc, 50, 0, SRCPAINT);
		}

		if (zqtUse)
		{
			SelectObject(bufdc, hopter);
			BitBlt(mdc, x, y - 25, 50, 50, bufdc, cnt * 50, 50, SRCAND);
			BitBlt(mdc, x, y - 25, 50, 50, bufdc, cnt * 50, 0, SRCPAINT);
			cnt++;
			if (cnt == 4)
				cnt = 0;
		}

		// 怪兽
		if (monsterShow)
		{
			// 随机怪兽的形状
			int no = rand() % 6;
			switch (no)
			{
			case 0:
				SelectObject(bufdc, Cmonster0);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 195, 97, bufdc, 0, 55, SRCAND);
				SelectObject(bufdc, monster0);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 195, 97, bufdc, 0, 55, SRCAND);
				break;
			case 1:
				SelectObject(bufdc, Cmonster1);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 43, 55, bufdc, 0, 55, SRCAND);
				SelectObject(bufdc, monster1);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 43, 55, bufdc, 0, 55, SRCAND);
				break;
			case 2:
				SelectObject(bufdc, Cmonster2);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 78, 30, bufdc, 0, 55, SRCAND);
				SelectObject(bufdc, monster);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 78, 30, bufdc, 0, 55, SRCAND);
				break;
			case 3:
				SelectObject(bufdc, Cmonster3);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 71, 48, bufdc, 0, 55, SRCAND);
				SelectObject(bufdc, monster3);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 71, 48, bufdc, 0, 55, SRCAND);
				break;
			case 4:
				SelectObject(bufdc, Cmonster4);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 80, 55, bufdc, 0, 55, SRCAND);
				SelectObject(bufdc, monster4);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 80, 55, bufdc, 0, 55, SRCAND);
				break;
			case 5:
				SelectObject(bufdc, Cmonster5);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 44, 55, bufdc, 0, 55, SRCAND);
				SelectObject(bufdc, monster5);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 44, 55, bufdc, 0, 55, SRCAND);
				break;
			case 6:
				SelectObject(bufdc, monster);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 100, 55, bufdc, 0, 55, SRCAND);
				BitBlt(mdc, 350 / 6 * plat[10].x + 25, plat[10].y + 35, 100, 55, bufdc, 0, 0, SRCPAINT);
			}
		}

		// 贴图完成，显示
		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);

		tPre = GetTickCount(); // 记录此次绘图时间

		flag++; // 表示左右脚
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

		v = v + a;
		y = y + v;

		// 禁止穿越
		if (x + 40 > 350)
			x = 300;
		if (x + 40 < 0)
			x = 0;

		// 动态效果
		if (y < 250)
		{
			for (int i = 0; i < N; i++)
			{
				y = 250;
				plat[i].y = plat[i].y - v;
				plat[8].y = plat[0].y;
				if (plat[i].y > 540)
				{
					if (i == 0)
						zqt = 0;
					if (i == 5)
						monsterShow = 0;
					score++;
					if (i == 3)
						isBroken = 0;
					plat[i].y = -40;
					plat[i].x = rand() % 5;
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
					int banx = 350 / 6 * plat[i].x;
					hpen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
					SelectObject(hdc, hpen);
					if (y >= plat[i].y - 25 && y <= plat[i].y + 25 && x >= plat[i].y && x <= plat[i].y + 80 && v >= 0)
					{
						isBroken = 1;
						v = -40;
						y = plat[i].y;
					}
					DeleteObject(hpen);
				}
			}
			else if (i == 6)
			{
				if (y >= plat[i].y - 20 && y <= plat[i].y + 20 && x >= plat[i].x && x <= plat[i].x + 80 && v >= 0)
				{
					// 说明碰到板板了
					v = -40;
					y = plat[i].y;
				}
			}
			else if (i == 5)
			{
				if (monsterShow == 1 && y >= plat[i].y - 20 && y <= plat[i].y + 20 && x >= plat[i].x && x <= plat[i].x + 80 && v >= 0)
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

						plat[i].x = rand() % 5;
						plat[i].y = 500 - 80 * i;
					}
					plat[6].x = 100;
				}
			}
			// 触发竹蜻蜓的使用
			if (zqt == 1 && y >= plat[8].y - 25 && y <= plat[8].y + 25 && x >= 350 / 6 * plat[8].x + 25 && x <= 350 / 6 * plat[8].x + 25 + 80 && v >= 0)
			{
				if (zqt == 1)
				{
					zqtUse = 1;
					zqt = 0;
					zqtScore = score;
				}
			}

			int banx = 350 / 6 * plat[i].x;
			hpen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
			SelectObject(hdc, hpen);
			if (y >= plat[i].y - 20 && y <= plat[i].y + 20 && x >= plat[i].y && x <= plat[i].y + 80 && v >= 0)
			{
				v = -40;
				y = plat[i].y;
				now++;
				totol++;
			}
			DeleteObject(hpen);
		}

		// 生成竹蜻蜓
		if (score % 6 == 0 && zqtUse == 0)
		{
			zqt = 1;
			plat[tmp].x = plat[0].x;
			plat[tmp].y = plat[0].y;
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
		if (score % 10 == 0)
		{
			monsterShow = 1;
		}

		// 子弹
		if (bullet == 1)
		{
			hpen = CreatePen(PS_SOLID, 16, RGB(0, 0, 0));
			SelectObject(hdc, hpen);
			bullety -= 15;
			MoveToEx(hdc, bulletx, bullety, NULL);
			LineTo(hdc, bulletx, bullety);
			if (bullety <= 0)
			{
				bullet = 0;
			}
			if (monsterShow == 1)
			{
				if (bulletx <= 350 / 6 * plat[10].x + 25 + 100 && bulletx >= 350 / 6 * plat[10].x + 25 && bullety <= plat[10].y + 35 + 25 && bullety >= plat[10].y + 35 - 25)
				{ // 命中
					bullet = 0;
					monsterShow = 0;
					score += 5; // +5
				}
			}
			DeleteObject(hpen);
		}

		if (y >= 720)
		{
			// 记录死亡位置
			deadx = x;
			deady = 720 - 95;
			for (int i = 0; i < 20; i++)
			{
				deadplat[i].x = plat[i].x;
				deadplat[i].y = plat[i].y;
			}

			Sleep(300);
			mode = 0;
			X = 50;
			Y = 400;
			v = -10;
			isBroken = 0;
			mode = 3;
			for (int i = 0; i < N; i++)
			{
				plat[i].x = rand() % 5;
				plat[i].y = 550 - 80 * i;
			}
			plat[6].x = 100;
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
		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);

		SelectObject(bufdc, role);
		BitBlt(mdc, 0, 0, 47, 95, bufdc, num * 47, 95, SRCAND);
		BitBlt(mdc, 0, 0, 47, 95, bufdc, num * 47, 95, SRCAND);

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

		// 显示死亡回放

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

		// 直接播放.wav文件，_T表示宽字符串，SND_ASYNC代表异步播放，SND_LOOP代表循环播放
		// PlaySound("res/deal.wav", NULL, SND_ASYNC | SND_LOOP);

		// mciSendString("OPEN res/deal.wav ALIAS MUSIC", NULL, 0, 0); // 打开文件
		// mciSendString("PLAY MUSIC FROM 0", NULL, 0, NULL);			// 播放

		break;
	case WM_KEYUP:
		walk = 0;
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
		case VK_UP: // 按下【↑】键
			// 先按照目前的移动方向来进行贴图坐标修正，并加入人物往上移动的量（每次按下一次按键移动10个单位），来决定人物贴图坐标的X与Y值，接着判断坐标是否超出窗口区域，若有则再次修正
			if (mode == 1 && bullet == 0)
			{
				bullet = 1;
				bulletx = x + 35;
				bullety = y - 55;
			}
			walk = 1;
			break;
		case VK_LEFT: // 按下【←】键
			if (mode == 1)
			{
				walk = 1;
				dir = 3;
				x -= 30;
			}
			break;
		case VK_RIGHT: // 按下【→】键
			if (mode == 1)
			{
				walk = 1;
				dir = 4;
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
