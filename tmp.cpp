#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define N 15

// 全局变量声明
HBITMAP role, monster, hopter;			  // 位图文件 角色 怪兽 竹蜻蜓
HBITMAP bg, bg2, bg3;					  // 背景， bg3菜单 bg2重开 bg经典地图
HBITMAP choose, sky, sea;				  // 地图
HBITMAP brokenBoard, moveBoard, board[N]; // 一次性板 移动的板 普通板
// 3 一次性 6 移动

HDC hdc, mdc, bufdc;
HWND hWnd;
DWORD tPre, tNow;
HPEN hpen;
HBRUSH hbrush;
PAINTSTRUCT ps;
TEXTMETRIC tm;

int gameMap;					 // 选择地图
int totol;						 // 历史板块
int maxScore, now;				 // 记录最高得分，当前局踩过的板块
int num, dir, walk;				 // num 人物当前动画  dir 人物朝向  isWalk 人是否行走
int X, Y;						 // 人物静态贴图坐标
int x, y;						 // 游戏中
int flag, maxy;					 // flag 左右脚标志  maxy 当前y能到达的最大高度
int score;						 // 分数
static char sc[100];			 // 显示分数
int mousex, mousey;				 // 鼠标坐标x,y
int a = 5, v = -10;				 // 加速度、速度
int mode;						 // mode 1 游戏界面 mode 0 菜单界面 mode 2 帮助 mode 3 游戏结算界面
int winX = 350, winY = 720;		 // 窗口大小
char *helptxt[6] = {};			 // 帮助
int bullet, bulletx, bullety;	 // 子弹
int monsteShow;					 // show 0 不出现怪兽 show 1 出现怪兽
int zqt, zqtUse, zqtScore, num2; // 竹蜻蜓
int isBroken;					 // 一次性板是否坏了
int movex = 4;					 // 板子位移
int height = 30;				 // 二段跳高度
int tmp = 8;					 // 竹蜻蜓所在版
const char filename[] = "histroyScore.txt";
const char filename2[] = "histroyBoard.txt";
struct point
{
	int x, y;
} plat[20], deadpos[20]; // 板坐标  // 死亡时板块的位置
int deadx, deady;		  // 死亡坐标

FILE *fp;

// 全局函数声明
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void MyPaint(HDC hdc);
void createboard(HDC hdc, int bany, int n);
void createboard2(HDC hdc, int bany, int n);
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

	////////////////////////////////////////
	hdc = GetDC(hWnd);
	mdc = CreateCompatibleDC(hdc);
	bufdc = CreateCompatibleDC(hdc);

	// 建立空的位图并置入mdc中
	HBITMAP fullmap;
	fullmap = CreateCompatibleBitmap(hdc, winX, winY);
	SelectObject(mdc, fullmap);

	// 设定人物贴图初始位置和移动方向
	// 载入各连续移动位图及背景图
	role = (HBITMAP)LoadImage(NULL, "role.bmp", IMAGE_BITMAP, 564, 190, LR_LOADFROMFILE);
	monster = (HBITMAP)LoadImage(NULL, "monster.bmp", IMAGE_BITMAP, 100, 110, LR_LOADFROMFILE);
	hopter = (HBITMAP)LoadImage(NULL, "hopter.bmp", IMAGE_BITMAP, 200, 100, LR_LOADFROMFILE);

	// bg2 用于重开 bg3 用于开始菜单 bg 用于游戏界面
	bg = (HBITMAP)LoadImage(NULL, "bg.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	bg2 = (HBITMAP)LoadImage(NULL, "bg2.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);
	bg3 = (HBITMAP)LoadImage(NULL, "bg3.bmp", IMAGE_BITMAP, winX, winY, LR_LOADFROMFILE);

	// 地图
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
	x = 180; // 人物贴图起始X坐标
	y = 330; // 人物贴图起始Y坐标
	maxy = y;
	X = 50;	 // 竹蜻蜓贴图坐标
	Y = 400; // 竹蜻蜓贴图坐标

	// 初始化板坐标
	srand((unsigned)time(NULL));
	for (int i = 0; i < N; i++)
	{
		pos[i].x = rand() % 5;
		pos[i].y = 550 - 80 * i;
	}
	// 设置一块板为移动版
	pos[6].x = 100;
	pos[8].x = 3000;
	pos[8].y = 5000;
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
		BitBlt(mdc, 40, 440 + 95, 80, 15, bufdc, 0, 0, SRCAND);
		BitBlt(mdc, 40, 440 + 95, 80, 15, bufdc, 0, 0, SRCPAINT);

		// 人物的贴图
		SelectObject(bufdc, role);
		BitBlt(mdc, X, Y, 47, 95, bufdc, num * 47, 95, SRCAND);
		BitBlt(mdc, X, Y, 47, 95, bufdc, num * 47, 0, SRCPAINT);

		// 竹蜻蜓
		SelectObject(bufdc, hopter);
		BitBlt(mdc, X, Y - 25, 50, 50, bufdc, num2 * 50, 50, SRCAND);
		BitBlt(mdc, X, Y - 25, 50, 50, bufdc, num2 * 50, 0, SRCPAINT);
		num2++;
		if (num2 == 4)
			num2 = 0;

		// // 怪兽
		// SelectObject(bufdc, monster);
		// BitBlt(mdc, 60, 440 + 95, 100, 55, bufdc, 0, 0, SRCAND);
		// BitBlt(mdc, 60, 440 + 95, 100, 55, bufdc, 0, 0, SRCPAINT);

		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);
		tPre = GetTickCount(); // 记录此次绘图时间

		// 重力代码
		v = v + a;
		Y = Y + v;

		x = 180; // 贴图起始X坐标
		y = 330; // 贴图起始Y坐标

		score = 0;
		if (Y >= 440 && Y <= 440 + 20 && v >= 0)
		{ // 到最低点
			v = -40;
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

		// 重力代码
		v = v + a;
		y = y + v;

		// 移动板
		pos[6].x += movex;
		if (pos[6].x >= 250)
			movex = -movex;
		if (pos[6].x <= 0)
			movex = -movex;

		// 穿越
		if (x + 40 > 350)
			x = 0;
		if (x + 40 < 0)
			x = 320;

		fresh(hdc);
	}
	else if (mode == 2)
	{
		// 地图选择界面
		mode = 3;
		SelectObject(bufdc, choose);
		BitBlt(mdc, 0, 0, winX, winY, bufdc, 0, 0, SRCCOPY);
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

		// 人物的贴图
		SelectObject(bufdc, role);
		BitBlt(mdc, 135, 300, 47, 95, bufdc, num * 47, 95, SRCAND);
		BitBlt(mdc, 135, 300, 47, 95, bufdc, num * 47, 0, SRCPAINT);

		BitBlt(hdc, 0, 0, winX, winY, mdc, 0, 0, SRCCOPY);

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

void fresh(HDC hdc)
{
	// 让纵向连接起来的代码
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
				// 说明碰到板板了
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
		// 触发竹蜻蜓的使用
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
	hpen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0)); // 创新画笔.
	SelectObject(hdc, hpen);					   // 选入画笔.
	if (y >= bany - 20 && y <= bany + 20 && x >= banx && x <= banx + 80 && v >= 0)
	{
		// 说明碰到平台
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
	hpen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0)); // 创新画笔.
	SelectObject(hdc, hpen);

	if (y >= bany - 25 && y <= bany + 25 && x >= banx && x <= banx + 80 && v >= 0)
	{
		// 碰到板板了
		isBroken = 1;
		v = -40;
		y = bany;
	}
	DeleteObject(hpen);
}

//****消息处理函数***********************************
// 1.按下【Esc】键结束程序
// 2.按下方向键重设贴图坐标
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		MessageBox(NULL, "← →控制移动，空格跳跃。", "游戏说明", MB_OK);
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
			gameMap = 0; // 普通地图
		}
		else if (mode == 2 && mousex >= 100 && mousex <= 250 && mousey >= 313 && mousey <= 360)
		{
			gameMap = 1; // 天空地图
		}
		else if (mode == 2 && mousex >= 100 && mousex <= 250 && mousey >= 378 && mousey <= 427)
		{
			gameMap = 2; // 海洋地图
		}
		else if (mode == 2 && mousex >= 100 && mousex <= 250 && mousey >= 248 && mousey <= 296)
		{
			PostQuitMessage(0); // 结束程序
		}

		if (mode == 3 && mousex >= 132 && mousex <= 218 && mousey >= 443 && mousey <= 491)
		{
			mode = 0;
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
		case VK_DOWN: // 按下【↓】键
			walk = 1;
			a = 0;
			v = 0;
			y++;
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
		DeleteObject(brokenBoard);
		DeleteObject(moveBoard);
		for (int i = 0; i < N; i++)
			DeleteObject(board[i]);
		ReleaseDC(hWnd, hdc);
		PostQuitMessage(0);
		break;
	default: // 其他消息
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
