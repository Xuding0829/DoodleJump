#include <windows.h>
#include <stdio.h>

//定义结构体
struct BULLET       //bullet结构体代表飞机子弹
{
	int x,y;        //子弹坐标
	bool exist;     //子弹是否存在
};

//全局变量声明
HINSTANCE hInst;
HBITMAP bg,ship,bullet;     //存储背景图，飞机图，子弹图
HDC		hdc,mdc,bufdc;
HWND	hWnd;
DWORD	tPre,tNow;
int		x,y,nowX,nowY;    //x，y代表鼠标光标所在位置，nowX，nowY代表飞机坐标，也是贴图的位置
int		w=0,bcount;       //w为滚动背景所要裁剪的区域宽度，bcount记录飞机现有子弹数目
BULLET  b[30];           //声明一个“bullet”类型的数组，用来存储飞机发出的子弹

//全局函数声明
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void				MyPaint(HDC hdc);

//****WinMain函数，程序入口点函数**************************************
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;

	MyRegisterClass(hInstance);

	//初始化
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	//循环
	 GetMessage(&msg,NULL,NULL,NULL);            //初始化msg    
    while( msg.message!=WM_QUIT )
    {
        if( PeekMessage( &msg, NULL, 0,0 ,PM_REMOVE) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
		else
		{
			tNow = GetTickCount();
			if(tNow-tPre >= 40)
				MyPaint(hdc);
		}
    }

	return msg.wParam;
}

//****设计一个窗口类，类似填空题，使用窗口结构体*********************
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "canvas";
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

//****初始化函数*************************************
// 1.设定飞机初始位置
// 2.设定鼠标光标位置及隐藏
// 3.限制鼠标光标移动区域
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hWnd = CreateWindow("canvas", "绘图窗口" , WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	MoveWindow(hWnd,10,10,640,480,true);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	/////////////////////////////////////////////////////////
	HBITMAP bmp;
	POINT pt,lt,rb;
	RECT rect;
	
	hInst = hInstance;

	hdc = GetDC(hWnd);
	mdc = CreateCompatibleDC(hdc);
	bufdc = CreateCompatibleDC(hdc);

	bmp = CreateCompatibleBitmap(hdc,640,480);
	SelectObject(mdc,bmp);

	bg = (HBITMAP)LoadImage(NULL,"bg.bmp",IMAGE_BITMAP,648,480,LR_LOADFROMFILE);
	ship = (HBITMAP)LoadImage(NULL,"ship.bmp",IMAGE_BITMAP,100,148,LR_LOADFROMFILE);
	bullet = (HBITMAP)LoadImage(NULL,"bullet.bmp",IMAGE_BITMAP,10,20,LR_LOADFROMFILE);
	
	//设定鼠标光标的x，y值，并设定飞机贴图坐标的“nowX”和“nowY”的值为（300，300）
	x = 300;
	y = 300;
	nowX = 300;
	nowY = 300;

	//设定光标位置
	pt.x = 300;
	pt.y = 300;
	ClientToScreen(hWnd,&pt);
	SetCursorPos(pt.x,pt.y);
	
	//ShowCursor(false);		//隐藏鼠标光标

	//限制鼠标光标移动区域
	GetClientRect(hWnd,&rect);  //取得窗口内部矩形
	//将矩形左上点坐标存入lt中
	lt.x = rect.left;
	lt.y = rect.top;
	//将矩形右下坐标存入rb中
	rb.x = rect.right;
	rb.y = rect.bottom;
	//将lt和rb的窗口坐标转换为屏幕坐标
	ClientToScreen(hWnd,&lt);
	ClientToScreen(hWnd,&rb);
	//以屏幕坐标重新设定矩形区域
	rect.left = lt.x;
	rect.top = lt.y;
	rect.right = rb.x;
	rect.bottom = rb.y;
	//限制鼠标光标移动区域
	//ClipCursor(&rect);

	MyPaint(hdc);

	return TRUE;
}

//****自定义绘图函数*********************************
// 1.设定飞机坐标并进行贴图
// 2.设定所有子弹坐标并进行贴图
// 3.显示真正的鼠标光标所在坐标
void MyPaint(HDC hdc)
{
	char str[20] = "";
	int i;

	//贴上背景图
	SelectObject(bufdc,bg);
	BitBlt(mdc,0,0,w,480,bufdc,640-w,0,SRCCOPY);
	BitBlt(mdc,w,0,640-w,480,bufdc,0,0,SRCCOPY);
	
	//计算飞机的贴图坐标，设定每次进行飞机贴图时，其贴图坐标（nowX，nowY）会以10个单位慢慢向鼠标光标所在的目的点（x，y）接近，直到两个坐标相同为止
	if(nowX < x)
	{
		nowX += 10;
		if(nowX > x)
			nowX = x;
	}
	else
	{
		nowX -=10;
		if(nowX < x)
			nowX = x;
	}

	if(nowY < y)
	{
		nowY += 10;
		if(nowY > y)
			nowY = y;
	}
	else
	{
		nowY -= 10;
		if(nowY < y)
			nowY = y;
	}

	//贴上飞机图
	SelectObject(bufdc,ship);
	BitBlt(mdc,nowX,nowY,100,74,bufdc,0,74,SRCAND);
	BitBlt(mdc,nowX,nowY,100,74,bufdc,0,0,SRCPAINT);

	//子弹的贴图，先判断子弹数目“bcount”的值是否为“0”。若不为0，则对子弹数组中各个还存在的子弹按照其所在的坐标（b[i].x,b[i].y）循环进行贴图操作
	SelectObject(bufdc,bullet);
	if(bcount!=0)
		for(i=0;i<30;i++)
			if(b[i].exist)
			{
				//贴上子弹图
				BitBlt(mdc,b[i].x,b[i].y,10,10,bufdc,0,10,SRCAND);
				BitBlt(mdc,b[i].x,b[i].y,10,10,bufdc,0,0,SRCPAINT);

				//设置下一个子弹的坐标。子弹是又右向左发射的，因此，每次其X轴上的坐标值递减10个单位，这样贴图会产生往左移动的效果。而如果子弹下次的坐标已超出窗口的可见范围（h[i].x<0）,那么子弹设为不存在，并将子弹总数bcount变量值减1.
				b[i].x -= 10;
				if(b[i].x < 0)
				{
					bcount--;
					b[i].exist = false;
				}
			}

	//显示鼠标坐标
	sprintf(str,"鼠标X坐标为%d    ",x);
	TextOut(mdc,0,0,str,strlen(str));
	sprintf(str,"鼠标Y坐标为%d    ",y);
	TextOut(mdc,0,20,str,strlen(str));

	BitBlt(hdc,0,0,640,480,mdc,0,0,SRCCOPY);

	tPre = GetTickCount();

	w += 10;
	if(w==640)
		w = 0;
}

//****消息处理函数***********************************
// 1.处理WM_LBUTTONDOWN消息发射子弹
// 2.处理WM_MOUSEMOVE消息设定飞机贴图坐标
// 3.在窗口结束时恢复鼠标移动区域
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;

	switch (message)
	{
		case WM_KEYDOWN:				//按键按下消息
			if(wParam==VK_ESCAPE)		//按下【Esc】键
				PostQuitMessage(0);
			break;
		case WM_LBUTTONDOWN:			//单击鼠标左键消息
			for(i=0;i<30;i++)
			{
				if(!b[i].exist)
				{
					b[i].x = nowX;		//子弹x坐标
					b[i].y = nowY + 30; //子弹y坐标
					b[i].exist = true;
					bcount++;			//累加子弹数目
					break;
				}
			}
		case WM_MOUSEMOVE:
			x = LOWORD(lParam);			//取得鼠标X坐标
			if(x > 530)					//设置临界坐标
				x = 530;
			else if(x < 0)
				x = 0;

			y = HIWORD(lParam);			//取得鼠标y坐标
			if(y > 380)
				y = 380;
			else if(y < 0)
				y = 0;
				
			break;
		case WM_DESTROY:				//窗口结束消息
			ClipCursor(NULL);			//恢复鼠标移动区域

			DeleteDC(mdc);
			DeleteDC(bufdc);
			DeleteObject(bg);
			DeleteObject(bullet);
			DeleteObject(ship);
			ReleaseDC(hWnd,hdc);

			PostQuitMessage(0);
			break;
		default:						//其他消息
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

