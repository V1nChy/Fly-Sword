#include<Windows.h>
#include<tchar.h>
#include<time.h>
#include"BasicData.h"

#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"Msimg32.lib")


HDC g_hdc = NULL, g_mdc = NULL,g_bufdc=NULL;
HBITMAP g_hSwordMan = NULL, g_hSwordBlade = NULL, g_hsDra = NULL, g_hbDra = NULL, g_hBoom = NULL;
HBITMAP g_hBackground = NULL, g_hUi = NULL, g_hNumber = NULL, g_hEnding = NULL;
HBITMAP g_hExplode = NULL,g_hPao=NULL,g_hStorm=NULL,g_hBossComming=NULL;

DWORD g_tNow = 0, g_tPre = 0;
int  g_ix = 0, g_iy = 0,g_state=0;
int g_iBOffset = 0,g_direct=0,g_nd=0;
RECT r;

List<SmallDragon> sDragon;
List<SwordBullets> Bullet;
List<Explode> explode;
Role role = { 0 };
Boss boss = { 0 };
int Map[80][60];

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool Game_Load(HWND hwnd);
void Game_Begin();
bool Game_CleanUp(HWND hwnd);

void Game_Paint(HWND hwnd);
void Paint_Bullet();
void Paint_SmallDra();
void Paint_Explode();
void Paint_Boss();

void Game_Process();
void Process_Role();
void Process_Bullet();
void Process_BulletSign();
void Process_SmallDra();
void Process_Explode();
void Process_Boss();
void Process_Boss_Role();
void Process_Boss_Bullet();
void Process_Boss_Explode();

void Map_Sign(int begin_x,int begin_y,int end_x,int end_y,int number);
bool Map_Judge(int begin_x, int begin_y, int end_x, int end_y,int number);
void Explode_start(int x, int y,int kind);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	//注册窗口信息
	WNDCLASSEX W = { 0 };
	W.cbSize = sizeof(WNDCLASSEX);
	W.style = CS_VREDRAW | CS_HREDRAW;
	W.lpfnWndProc = (WNDPROC)WndProc;
	W.hInstance = hInstance;
	W.hIcon = (HICON)LoadImage(NULL,L"icon.ico",IMAGE_ICON,0,0,LR_LOADFROMFILE|LR_DEFAULTSIZE);
	W.hCursor = LoadCursor(NULL, IDC_ARROW);
	W.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	W.lpszClassName = ClassName;
	RegisterClassEx(&W);

	//创建窗口
	HWND hwnd = CreateWindow(ClassName, WindowTitle, WS_OVERLAPPEDWINDOW,
		(GetSystemMetrics(SM_CXSCREEN) / 2) - 400,
		(GetSystemMetrics(SM_CYSCREEN) / 2) - 315,
		WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
	if (!hwnd)
		return FALSE;

	//窗口显示
	//MoveWindow(hwnd,400,100,800,600,true);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	//初始化程序
	if (!Game_Load(hwnd))
	{
		MessageBox(hwnd, L"资源初始化失败", L"消息窗口", 0);
		return FALSE;
	}

	//消息循环
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			g_tNow = GetTickCount();
			if (g_tNow - g_tPre >= 15 && g_state)
			{
				Game_Process();
				Game_Paint(hwnd);
			}
		}
	}

	UnregisterClass(ClassName, hInstance);
	return msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
	{
		 switch (wParam)
		 {
			 case VK_ESCAPE:
				DestroyWindow(hwnd);
				PostQuitMessage(0);
				break;
			 case VK_F5:
				 Game_Begin();
		 }
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		SwordBullets add = { role.x-20, role.y+50, true };
		Bullet.Insert(add);
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		g_ix = LOWORD(lParam);
		if (g_ix > WINDOW_WIDTH - 150)
			g_ix = WINDOW_WIDTH - 150;
		else if (g_ix <0)
			g_ix = 0;
			g_iy = HIWORD(lParam);
		if (g_iy>WINDOW_HEIGHT - 140)
			g_iy = WINDOW_HEIGHT - 140;
		else if (g_iy < 0)
			g_iy = 0;
		return 0;
	}
	case WM_DESTROY:
		Game_CleanUp(hwnd);
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);

	}
}
bool Game_Load(HWND hwnd)
{
	srand((unsigned)time(NULL));

	HBITMAP bmp;
	g_hdc = GetDC(hwnd);
	g_mdc = CreateCompatibleDC(g_hdc);
	g_bufdc = CreateCompatibleDC(g_hdc);
	bmp = CreateCompatibleBitmap(g_hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
	SelectObject(g_mdc,bmp);
	DeleteObject(bmp);

	//载入位图
	g_hSwordMan = (HBITMAP)LoadImage(NULL,L"swordman.bmp",IMAGE_BITMAP,1260,200,LR_LOADFROMFILE);
	g_hSwordBlade = (HBITMAP)LoadImage(NULL, L"swordblade.bmp", IMAGE_BITMAP, 50, 15, LR_LOADFROMFILE);
	g_hBackground = (HBITMAP)LoadImage(NULL, L"bg.bmp", IMAGE_BITMAP, WINDOW_WIDTH, WINDOW_HEIGHT, LR_LOADFROMFILE);
	g_hUi = (HBITMAP)LoadImage(NULL, L"UI.bmp", IMAGE_BITMAP, WINDOW_WIDTH, WINDOW_HEIGHT, LR_LOADFROMFILE);
	g_hsDra = (HBITMAP)LoadImage(NULL,L"SmallDragon.bmp",IMAGE_BITMAP,400,150,LR_LOADFROMFILE);
	g_hBoom = (HBITMAP)LoadImage(NULL,L"Boom.bmp",IMAGE_BITMAP,800,174,LR_LOADFROMFILE);
	g_hNumber = (HBITMAP)LoadImage(NULL, L"Score.bmp", IMAGE_BITMAP, 200, 25, LR_LOADFROMFILE);
	g_hEnding = (HBITMAP)LoadImage(NULL, L"Ending.bmp", IMAGE_BITMAP, 800, 400, LR_LOADFROMFILE);
	g_hbDra = (HBITMAP)LoadImage(NULL,L"Boss.bmp",IMAGE_BITMAP,800,800,LR_LOADFROMFILE);
	g_hExplode = (HBITMAP)LoadImage(NULL, L"Explode.bmp", IMAGE_BITMAP, 800, 574, LR_LOADFROMFILE);
	g_hPao = (HBITMAP)LoadImage(NULL, L"Pao.bmp", IMAGE_BITMAP, 1200, 1800, LR_LOADFROMFILE);
	g_hStorm = (HBITMAP)LoadImage(NULL, L"Storm.bmp", IMAGE_BITMAP, 1260, 300, LR_LOADFROMFILE);
	g_hBossComming = (HBITMAP)LoadImage(NULL, L"BossComming.bmp", IMAGE_BITMAP, 800, 600, LR_LOADFROMFILE);

	memset(Map,0,sizeof(Map));

	//初始人物坐标
	g_ix = 500;
	g_iy = 100;
	role.x = 500;
	role.y = 100;
	role.life = 286;
	role.exist = true;
	role.score = 0;
	role.tNow = GetTickCount();

	//限制鼠标范围，初始位置
	POINT pt, lt, rb;
	pt.x = 500;
	pt.y = 100;
	ClientToScreen(hwnd,&pt);
	SetCursorPos(pt.x,pt.y);
	ShowCursor(false);

	GetClientRect(hwnd,&r);
	lt.x = r.left;
	lt.y = r.top;
	rb.x = r.right;
	rb.y = r.bottom;
	ClientToScreen(hwnd,&lt);
	ClientToScreen(hwnd,&rb);

	r.left = lt.x+5;
	r.top = lt.y+5;
	r.right = rb.x;
	r.bottom = rb.y;
	ClipCursor(&r);

	for (int i = 0; i < 8; i++)
	{
		SmallDragon add = { -100 * i, rand() % 600, 0, 0, true, GetTickCount ()};
		sDragon.Insert(add);
	}
	g_state = 1;
	Game_Paint(hwnd);
	return TRUE;

}
void Game_Process()
{
	memset(Map, 0, sizeof(Map));
	switch (g_state)
	{
	case 1:
	{
			  Process_Role();
			  Process_Explode();
			  Process_BulletSign();
			  Process_SmallDra();
			  Process_Bullet();
	}
		break;
	case 2:
	{
			  Process_Boss();
			  Process_Boss_Role();
			  Process_Boss_Explode();
			  Process_Boss_Bullet();
	}
		break;
	}

	//地图滚动速度
	g_iBOffset += 5;
	if (g_iBOffset == WINDOW_WIDTH)
		g_iBOffset = 0;

	//进入打BOSS
	if (role.score == SCORE_BOSS && g_state==1)
	{
		sDragon.DeleteAll();
		Bullet.DeleteAll();
		explode.DeleteAll();
		g_state = 2;

		boss = { 10, 250, 0, 0,0, 360, 0, true, GetTickCount(), GetTickCount() };

		for (int i = 0; i < 1000; i++)
		{

			SelectObject(g_bufdc, g_hBackground);
			BitBlt(g_mdc, 0, 0, g_iBOffset, WINDOW_HEIGHT, g_bufdc, WINDOW_WIDTH - g_iBOffset, 0, SRCCOPY);
			BitBlt(g_mdc, g_iBOffset, 0, WINDOW_WIDTH - g_iBOffset, WINDOW_HEIGHT, g_bufdc, 0, 0, SRCCOPY);
			g_iBOffset += 10;
			if (g_iBOffset == WINDOW_WIDTH)
				g_iBOffset = 0;

			SelectObject(g_bufdc, g_hNumber);
			TransparentBlt(g_mdc, 440, 30, 20, 25, g_bufdc, role.score / 100 * 20, 0, 20, 25, RGB(0, 0, 0));
			TransparentBlt(g_mdc, 460, 30, 20, 25, g_bufdc, role.score / 10 % 10 * 20, 0, 20, 25, RGB(0, 0, 0));
			TransparentBlt(g_mdc, 480, 30, 20, 25, g_bufdc, role.score % 10 * 20, 0, 20, 25, RGB(0, 0, 0));

			SelectObject(g_bufdc, g_hSwordMan);
			BitBlt(g_mdc, role.x, role.y, 140, 100, g_bufdc, 0, 100, SRCAND);
			BitBlt(g_mdc, role.x, role.y, 140, 100, g_bufdc, 0, 0, SRCPAINT);

			BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);

			role.x -= 10;
			if (role.x < -140)
			{
				role.x = 500;
				break;
			}

			Sleep(35);
		}
	}

}
void Game_Paint(HWND hwnd)
{
	//载入地图
	SelectObject(g_bufdc,g_hBackground);
	BitBlt(g_mdc,0,0,g_iBOffset,WINDOW_HEIGHT,g_bufdc,WINDOW_WIDTH-g_iBOffset,0,SRCCOPY);
	BitBlt(g_mdc,g_iBOffset,0,WINDOW_WIDTH-g_iBOffset,WINDOW_HEIGHT,g_bufdc,0,0,SRCCOPY);
	
	SelectObject(g_bufdc, g_hUi);
	BitBlt(g_mdc, 0, 0, 400, 90, g_bufdc, 0, 90, SRCAND);
	BitBlt(g_mdc, 0, 0, 400, 90, g_bufdc, 0, 0, SRCPAINT);

	if (role.life>0)
	{
		BitBlt(g_mdc, 100, 0, role.life, 90, g_bufdc, 400, 90, SRCAND);
		BitBlt(g_mdc, 100, 0, role.life, 90, g_bufdc, 400, 0, SRCPAINT);
	}

	if (g_state == 1)
	{
		SelectObject(g_bufdc, g_hNumber);
		TransparentBlt(g_mdc, 440, 30, 20, 25, g_bufdc, role.score / 100 * 20, 0, 20, 25, RGB(0, 0, 0));
		TransparentBlt(g_mdc, 460, 30, 20, 25, g_bufdc, role.score / 10 % 10 * 20, 0, 20, 25, RGB(0, 0, 0));
		TransparentBlt(g_mdc, 480, 30, 20, 25, g_bufdc, role.score % 10 * 20, 0, 20, 25, RGB(0, 0, 0));
	}
	//主角
	SelectObject(g_bufdc,g_hSwordMan);
	BitBlt(g_mdc, role.x, role.y, 140, 100, g_bufdc, role.state*140, 100, SRCAND);
	BitBlt(g_mdc, role.x, role.y, 140, 100, g_bufdc, role.state*140, 0, SRCPAINT);
	

	switch (g_state)
	{
	case 1:
	{
			  Paint_Explode();
			  Paint_Bullet();
			  Paint_SmallDra();
	}
		break;
	case 2:
	{
			  Paint_Bullet();
			  Paint_Explode();
			  Paint_Boss();
	}
		break;
	}

	//游戏结束
	SelectObject(g_bufdc, g_hEnding);
	if (role.life <= 0)
	{
		BitBlt(g_mdc, 200, 150, 400, 200, g_bufdc, 400, 200, SRCAND);
		BitBlt(g_mdc, 200, 150, 400, 200, g_bufdc, 400, 0, SRCPAINT);
		g_state = 0;
	}
	else if (g_state == 2 && boss.life <= 0)
	{
		BitBlt(g_mdc, 200, 150, 400, 200, g_bufdc, 0, 200, SRCAND);
		BitBlt(g_mdc, 200, 150, 400, 200, g_bufdc, 0, 0, SRCPAINT);
		g_state = 0;
	}

	BitBlt(g_hdc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,g_mdc,0,0,SRCCOPY);
	
	g_tPre = GetTickCount();
}

bool Game_CleanUp(HWND hwnd)
{
	DeleteObject(g_hBackground);
	DeleteDC(g_bufdc);
	DeleteDC(g_mdc);
	ReleaseDC(hwnd, g_hdc);
	return TRUE;
}
void Game_Begin()
{
	role.x = 500;
	role.y = 100;
	role.life = 286;
	role.exist = true;
	role.score = 0;
	Map_Sign(50, 11, 62, 21, ROLE);

	sDragon.DeleteAll();
	Bullet.DeleteAll();
	explode.DeleteAll();
	g_state = 1;
}


void Map_Sign(int begin_x, int begin_y, int end_x, int end_y,int number)
{
	if (begin_x < 0)
		begin_x = 0;
	if (end_x>80)
		end_x = 80;
	if (begin_y < 0)
		begin_y = 0;
	if (end_y>60)
		end_y=60;

	for (int i = begin_x; i < end_x; i++)
	{
		for (int j = begin_y; j < end_y; j++)
		{
			Map[i][j] = number;
		}

	}
}
bool Map_Judge(int begin_x, int begin_y, int end_x, int end_y, int number)
{
	if (begin_x < 0)
		begin_x = 0;
	if (end_x>80)
		end_x = 80;
	if (begin_y < 0)
		begin_y = 0;
	if (end_y>60)
		end_y = 60;

	for (int i = begin_x; i < end_x; i++)
	{
		for (int j = begin_y; j < end_y; j++)
		{
			if (Map[i][j] == number)
				return true;
		}

	}
	return false;
}
void Explode_start(int x, int y,int kind)
{
	Explode add;
	for (int i = 0; i < 20; i++)
	{
		switch (rand() % 4)
		{
		case 0:
			add = { x, y, rand() % 5 + 1, -rand() % 5 - 1, true, kind,0 };
			break;
		case 1:
			add = { x, y, -rand() % 5 - 1, -rand() % 5 - 1, true, kind, 0 };
			break;
		case 2:
			add = { x, y, -rand() % 5 - 1, rand() % 5 + 1, true, kind, 0 };
			break;
		case 3:
			add = { x, y, rand() % 5 + 1, rand() % 5 + 1, true, kind, 0 };
			break;
		}
		explode.Insert(add);
	}
	
}


void Process_Role()
{
	if (role.x < g_ix)
	{
		role.x += 10;
		if (role.x>g_ix)
			role.x = g_ix;
	}
	else
	{
		role.x -= 10;
		if (role.x < g_ix)
			role.x = g_ix;
	}
	if (role.y < g_iy)
	{
		role.y += 10;
		if (role.y>g_iy)
			role.y = g_iy;
	}
	else
	{
		role.y -= 10;
		if (role.y < g_iy)
			role.y = g_iy;
	}
	if (g_tNow - role.tNow >= 100)
	{
		role.state++;
		if (role.state == 9)
			role.state = 0;
		role.tNow = GetTickCount();
	}
	Map_Sign(role.x / 10+1, role.y / 10 + 2, (role.x / 10) + 12, (role.y / 10) + 11, ROLE);
}
void Process_Bullet()
{
	Node<SwordBullets> *pBullet = Bullet.Return_head();
	pBullet = pBullet->next;
	while (pBullet != Bullet.Return_head())
	{
		if (Map_Judge(pBullet->Data.x / 10, pBullet->Data.y / 10, pBullet->Data.x / 10 + 5, pBullet->Data.y / 10 + 1, SMALLDRAGON))
		{
			pBullet->Data.exist = false;
			pBullet = pBullet->next;
		}
		else
		{
			pBullet->Data.x -= 10;
			if (pBullet->Data.x < 0)
				pBullet = Bullet.Delete(pBullet);
			else
				pBullet = pBullet->next;
		}
	}
}
void Process_BulletSign()
{
	Node<SwordBullets> *pBullet = Bullet.Return_head();
	pBullet = pBullet->next;
	while (pBullet != Bullet.Return_head())
	{
		if (!pBullet->Data.exist)
		{
			Map_Sign(pBullet->Data.x / 10, pBullet->Data.y / 10, pBullet->Data.x / 10 + 5, pBullet->Data.y / 10 + 1, SWORDBLADE);
			pBullet = Bullet.Delete(pBullet);
		}
		else
		{
			pBullet = pBullet->next;
		}
	}
}
void Process_SmallDra()
{
	if (sDragon.Return_size() < SMALLDRAGON_NUM)
	{
		SmallDragon add = { -100, rand() % 600, 0, 0, true, GetTickCount ()};
		sDragon.Insert(add);
	}
	Node<SmallDragon> *psDragon = sDragon.Return_head();
	psDragon = psDragon->next;
	while (psDragon != sDragon.Return_head())
	{
		if (Map_Judge(psDragon->Data.x / 10, psDragon->Data.y / 10, psDragon->Data.x / 10 + 10, psDragon->Data.y / 10 + 6, SWORDBLADE))
		{
			Explode_start(psDragon->Data.x, psDragon->Data.y,EXPLODE1);
			role.score++;
			psDragon = sDragon.Delete(psDragon);
			continue;
		}
			if (g_tNow - psDragon->Data.tNow >= 300)
			{
				psDragon->Data.state++;
				if (psDragon->Data.state == 4)
					psDragon->Data.state = 0;
				psDragon->Data.direct = rand() % 4;

				psDragon->Data.tNow = GetTickCount();
			}
			psDragon->Data.x += 2;

			switch (psDragon->Data.direct)
			{
			case 0:
				psDragon->Data.x -= rand() % 4;
				psDragon->Data.y -= rand() % 4;
				break;
			case 1:
				psDragon->Data.x -= rand() % 4;
				psDragon->Data.y += rand() % 4;
				break;
			case 2:
				psDragon->Data.y -= rand() % 4;
				psDragon->Data.x += rand() % 4;
				break;
			case 3:
				psDragon->Data.y += rand() % 4;
				psDragon->Data.x += rand() % 4;
				break;
			}
			if (psDragon->Data.y > 530)
				psDragon->Data.y = 530;
			if (psDragon->Data.y < -100)
				psDragon->Data.y = -100;
			if (psDragon->Data.x < -700)
				psDragon->Data.x = -700;

			if (psDragon->Data.x <800 && Map_Judge(psDragon->Data.x / 10, psDragon->Data.y / 10, psDragon->Data.x / 10 + 10, psDragon->Data.y / 10 + 6, ROLE))
			{
				explode.Insert({ psDragon->Data.x, psDragon->Data.y, 0, 0, true, EXPLODE2, 0 });
				psDragon = sDragon.Delete(psDragon);
				role.life-=10;
			}
			else if (psDragon->Data.x > 800)
			{
				psDragon = sDragon.Delete(psDragon);
			}
			else
			{
				if (psDragon->Data.x + 100 > 10 && psDragon->Data.y + 60 > 10 && psDragon->Data.x < 800 && psDragon->Data.y < 600)
					Map_Sign(psDragon->Data.x / 10, psDragon->Data.y / 10, psDragon->Data.x / 10 + 10, psDragon->Data.y / 10 + 6, SMALLDRAGON);
				psDragon = psDragon->next;
			}
		
	}
}
void Process_Explode()
{
	Node<Explode> *pBoom = explode.Return_head();
	pBoom = pBoom->next;
	while (pBoom != explode.Return_head())
	{

		pBoom->Data.x += pBoom->Data.direct_x;
		pBoom->Data.y += pBoom->Data.direct_y;

		if (pBoom->Data.kind == EXPLODE2)
		{
			pBoom->Data.state += 2;
			if (pBoom->Data.state == 80)
				pBoom->Data.exist = false;
		}

		if (pBoom->Data.x <800 && pBoom->Data.x>0 && pBoom->Data.y<600 && pBoom->Data.y>0
			&& Map_Judge(pBoom->Data.x / 10, pBoom->Data.y / 10, pBoom->Data.x / 10 + 1, pBoom->Data.y / 10 + 1, ROLE))
		{
			pBoom = explode.Delete(pBoom);
			role.life-=5;
		}
		else if (pBoom->Data.x >800 || pBoom->Data.x<0 || pBoom->Data.y>600 || pBoom->Data.y < 0 || !pBoom->Data.exist)
		{
			pBoom = explode.Delete(pBoom);
		}
		else
			pBoom = pBoom->next;

		if (pBoom->Data.kind != 2 && pBoom->Data.x <800 && pBoom->Data.x>0 && pBoom->Data.y<600 && pBoom->Data.y>0)
			Map_Sign(pBoom->Data.x / 10, pBoom->Data.y / 10, pBoom->Data.x / 10 + 1, pBoom->Data.y / 10 + 1, EXPLODE1);
	}
}
void Process_Boss()
{
	if (boss.skill == 1 && boss.state == 3)
	{
		boss.skill_state++;
		if (boss.skill_state == 96)
		{
			boss.skill = 0;
			boss.state = 0;
			boss.skill_state = 0;
		}
	}
	else if (g_tNow - boss.tNow >= 300)
	{
		boss.direct = rand() % 4;
		boss.state++;
		if (boss.state == 4)
			boss.state = 0;

		boss.tNow = GetTickCount();
	}
	if (g_tNow - boss.tKill >= 5000)
	{
		boss.skill = 1;
		boss.tKill = GetTickCount();
	}
	if (boss.skill != 1)
	{

		switch (boss.direct)
		{
		case 0:
			boss.x -= rand() % 2;
			boss.y -= rand() % 2;
			break;
		case 1:
			boss.x -= rand() % 2;
			boss.y += rand() % 2;
			break;
		case 2:
			boss.y -= rand() % 2;
			boss.x += rand() % 2;
			break;
		case 3:
			boss.y += rand() % 2;
			boss.x += rand() % 2;
			break;
		}
		if (boss.y < 150)
			boss.y = 150;
		if (boss.y>350)
			boss.y = 350;
		if (boss.x<-50)
			boss.x = -50;
		if (boss.x>300)
			boss.x = 300;
	}
}
void Process_Boss_Role()
{
	if (role.x < g_ix)
	{
		role.x += 10;
		if (role.x>g_ix)
			role.x = g_ix;
	}
	else
	{
		role.x -= 10;
		if (role.x < g_ix)
			role.x = g_ix;
	}
	if (role.y < g_iy)
	{
		role.y += 10;
		if (role.y>g_iy)
			role.y = g_iy;
	}
	else
	{
		role.y -= 10;
		if (role.y < g_iy)
			role.y = g_iy;
	}

	if (role.x >= boss.x && role.x < boss.x + 200 && role.y>boss.y && role.y+100<boss.y+200)
	{
		role.x = boss.x + 200;

	}
	else if ( role.x < boss.x + 200 && role.y + 100 >= boss.y && role.y<boss.y)
	{
		role.y = boss.y-100;
		//role.life--;
	}
	else if ( role.x < boss.x + 200 && role.y<boss.y+200 && role.y+100 > boss.y+200)
	{
		role.y = boss.y + 200;
		//role.life--;
	}

	if (boss.skill == 1 && boss.skill_state>=48)
	{
		if (role.x>boss.x + 200 && role.y > boss.y-25 && role.y < boss.y + 125)
			role.life-=5;
	}


	if (g_tNow - role.tNow >= 100)
	{
		role.state++;
		if (role.state == 9)
			role.state = 0;
		role.tNow = GetTickCount();
	}
	Map_Sign(role.x / 10 + 1, role.y / 10 + 2, (role.x / 10) + 12, (role.y / 10) + 11, ROLE);
}
void Process_Boss_Bullet()
{
	Node<SwordBullets> *pBullet = Bullet.Return_head();
	pBullet = pBullet->next;
	while (pBullet != Bullet.Return_head())
	{
		if (pBullet->Data.x < boss.x + 200 && pBullet->Data.y>boss.y && pBullet->Data.y + 10 < boss.y + 200)
		{
			boss.life-=1;
			pBullet = Bullet.Delete(pBullet);
			continue;
		}
		pBullet->Data.x -= 10;
		if (pBullet->Data.x < 0)
			pBullet = Bullet.Delete(pBullet);
		else
			pBullet = pBullet->next;
		
	}
}
void Process_Boss_Explode()
{
	if (explode.Return_size() < STORM_NUM)
	{
		explode.Insert({ rand() % 600, rand() % 400, 0, 0, TRUE, STORM, 0, GetTickCount() });
	}
	Node<Explode> *pBoom = explode.Return_head();
	pBoom = pBoom->next;
	while (pBoom != explode.Return_head())
	{
		if (g_tNow - pBoom->Data.tNow >= 200)
		{
			pBoom->Data.state++;
			if (pBoom->Data.state == 7)
			{
					pBoom->Data.exist = false;
			}
			pBoom->Data.tNow = GetTickCount();
		}

		if (pBoom->Data.state>3 && pBoom->Data.exist && pBoom->Data.x <800 && pBoom->Data.x>0 && pBoom->Data.y<600 && pBoom->Data.y>0
			&& Map_Judge(pBoom->Data.x / 10+3, pBoom->Data.y / 10, pBoom->Data.x / 10 + 15, pBoom->Data.y / 10 + 12, ROLE))
		{
			pBoom->Data.exist = false;
			role.life-=5;
		}
		else if (pBoom->Data.x >800 || pBoom->Data.x<0 || pBoom->Data.y>600 || pBoom->Data.y < 0 || pBoom->Data.exist == false)
		{
			pBoom = explode.Delete(pBoom);
		}
		else
			pBoom = pBoom->next;

	}
}

void Paint_Bullet()
{
	SelectObject(g_bufdc, g_hSwordBlade);
	if (Bullet.Return_size() != 0)
	{
		Node<SwordBullets> *pBullet = Bullet.Return_head();
		pBullet = pBullet->next;
		while (pBullet != Bullet.Return_head())
		{
			TransparentBlt(g_mdc, pBullet->Data.x, pBullet->Data.y, 50, 15, g_bufdc, 0, 0, 50, 15, RGB(0, 0, 0));
			pBullet = pBullet->next;
		}
	}
}
void Paint_SmallDra()
{
	SelectObject(g_bufdc, g_hsDra);
	if (sDragon.Return_size() != 0)
	{
		Node<SmallDragon> *psDragon = sDragon.Return_head();
		psDragon = psDragon->next;
		while (psDragon != sDragon.Return_head())
		{
			BitBlt(g_mdc, psDragon->Data.x, psDragon->Data.y, 100, 75, g_bufdc, psDragon->Data.state * 100, 75, SRCAND);
			BitBlt(g_mdc, psDragon->Data.x, psDragon->Data.y, 100, 75, g_bufdc, psDragon->Data.state * 100, 0, SRCPAINT);
			psDragon = psDragon->next;
		}
	}
}
void Paint_Explode()
{
	Node<Explode> *pBoom = explode.Return_head();
	pBoom = pBoom->next;
	while (pBoom != explode.Return_head())
	{
		switch (pBoom->Data.kind)
		{
		case EXPLODE1:
			SelectObject(g_bufdc, g_hExplode);
			BitBlt(g_mdc, pBoom->Data.x, pBoom->Data.y, 15, 15, g_bufdc, 0, 559, SRCAND);
			BitBlt(g_mdc, pBoom->Data.x, pBoom->Data.y, 15, 15, g_bufdc, 0, 544, SRCPAINT);
			break;
		case EXPLODE2:
			SelectObject(g_bufdc, g_hExplode);
			BitBlt(g_mdc, pBoom->Data.x, pBoom->Data.y, 100, 72, g_bufdc, pBoom->Data.state / 10 * 100, 472, SRCAND);
			BitBlt(g_mdc, pBoom->Data.x, pBoom->Data.y, 100, 72, g_bufdc, pBoom->Data.state / 10 * 100, 400, SRCPAINT); 
			break;
		case STORM:
			SelectObject(g_bufdc, g_hStorm);
			BitBlt(g_mdc, pBoom->Data.x, pBoom->Data.y, 180, 149, g_bufdc, pBoom->Data.state * 180, 150, SRCAND);
			BitBlt(g_mdc, pBoom->Data.x, pBoom->Data.y, 180, 149, g_bufdc, pBoom->Data.state * 180, 0, SRCPAINT);
			break;
		}
		pBoom = pBoom->next;
	}
}
void Paint_Boss()
{
	SelectObject(g_bufdc, g_hUi);
	if (boss.life>0)
	{
		BitBlt(g_mdc, 410, 20, boss.life, 35, g_bufdc, 0, 215, SRCAND);
		BitBlt(g_mdc, 410, 20, boss.life, 35, g_bufdc, 0, 180, SRCPAINT);
	}

	SelectObject(g_bufdc, g_hbDra);
	BitBlt(g_mdc, boss.x, boss.y, 200, 200, g_bufdc, boss.state * 200, boss.skill * 400 + 200, SRCAND);
	BitBlt(g_mdc, boss.x, boss.y, 200, 200, g_bufdc, boss.state * 200, boss.skill * 400, SRCPAINT);

	if (boss.skill == 1 && boss.state==3)
	{
		SelectObject(g_bufdc, g_hPao);
		if (boss.skill_state < 48)
		{
			BitBlt(g_mdc, boss.x + 200, boss.y + 25, 1200, 150, g_bufdc, 0, boss.skill_state / 8 * 150 + 900, SRCAND);
			BitBlt(g_mdc, boss.x + 200, boss.y + 25, 1200, 150, g_bufdc, 0, boss.skill_state / 8 * 150, SRCPAINT);
		}
		else
		{
			BitBlt(g_mdc, boss.x + 200, boss.y + 25, 1200, 150, g_bufdc, 0, 1650, SRCAND);
			BitBlt(g_mdc, boss.x + 200, boss.y + 25, 1200, 150, g_bufdc, 0, 750, SRCPAINT);
		}
	}
}
