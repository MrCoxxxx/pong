//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include "math.h"

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false);

struct {
    HWND hWnd;//����� ����
    HDC device_context, context;// ��� ��������� ���������� (��� �����������)
    int width, height;//���� �������� ������� ���� ������� ������� ���������
} window;
// ������ ������ ����  
struct sprite {
    float x, y, width, height, rad, dx, dy, speed;
    bool status;
    HBITMAP hBitmap;//����� � ������� ������ 

    void show()
    {
        ShowBitmap(window.context, x, y, width, height, hBitmap);
    }

    bool checkCollision(float x_, float y_)
    {
        return (x < x_ and y < y_ and (x + width) > x_ and (y + height) > y_);
    }

};

sprite racket;//������� ������
sprite enemy;//������� ����������
sprite ball;//�����
const int bricks_x = 20;
const int bricks_y = 7;
sprite wall[bricks_x][bricks_y];

struct {
    int score, balls;//���������� ��������� ����� � ���������� "������"
    bool action = false;//��������� - �������� (����� ������ ������ ������) ��� ����
} game;



HBITMAP hBack;// ����� ��� �������� �����������

//c����� ����

void InitGame()
{
    //� ���� ������ ��������� ������� � ������� ������� gdi
    //���� ������������� - ����� ������ ������ ����� � .exe 
    //��������� ������ LoadImageA ��������� � ������� ��������, ��������� �������� ����� ������������� � ������� ���� �������
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    enemy.hBitmap = (HBITMAP)LoadImageA(NULL, "racket_enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    for (int i = 0; i < bricks_y; i++) {
        for (int j = 0; j < bricks_x; j++) {
            wall[j][i].hBitmap = enemy.hBitmap;
            wall[j][i].width = window.width / bricks_x;
            wall[j][i].height = (window.height / 3) / bricks_y;
            wall[j][i].x = wall[j][i].width * j;
            wall[j][i].y = (wall[j][i].height * i) + (window.height / 3);
            wall[j][i].status = true;
            
        }
    };
    //------------------------------------------------------

    racket.width = 300;
    racket.height = 50;
    racket.speed = 30;//�������� ����������� �������
    racket.x = window.width / 2.;//������� ���������� ����
    racket.y = window.height - racket.height;//���� ���� ���� ������ - �� ������ �������

    enemy.x = racket.x;//� ���������� �������� ������ � �� �� ����� ��� � ������

    ball.dy = -(rand() % 65 + 35) / 100.;//��������� ������ ������ ������
    ball.dx = -(1 - ball.dy);//��������� ������ ������ ������
    ball.speed = 16;
    ball.rad = 20;
    ball.x = racket.x;//x ���������� ������ - �� ������� �������
    ball.y = racket.y - ball.rad;//����� ����� ������ �������

    game.score = 0;
    game.balls = 9;

   
}

void ProcessSound(const char* name)//������������ ���������� � ������� .wav, ���� ������ ������ � ��� �� ����� ��� � ���������
{
   // PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//���������� name �������� ��� �����. ���� ASYNC ��������� ����������� ���� ���������� � ����������� ���������
}

bool checkIntersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float& outX, float& outY) {
    // y = kx+b
    float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (denom == 0) return false;
    float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    float u = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / denom;
    if (t < 0 || t > 1 || u < 0 || u > 1) return false;
    outX = x1 + t * (x2 - x1);
    outY = y1 + t * (y2 - y1);

    return true;
}

void checkNerest(bool& inter, float& iX, float& iY, float& l, float& interX, float& interY) {
    inter = true;
    float dx = iX - ball.x;
    float dy = iY - ball.y;
    float cl = sqrt(dx * dx + dy * dy);
    if (cl < l)
    {
        interX = iX;
        interY = iY;
        l = cl;
    }
}

void ShowScore()
{
    //�������� �������� � �������
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//����� ��� ������
    _itoa_s(game.score, txt, 10);//�������������� �������� ���������� � �����. ����� �������� � ���������� txt
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    _itoa_s(game.balls, txt, 10);
    TextOutA(window.context, 10, 100, "Balls", 5);
    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
}

void ProcessInput()
{
    //if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
    //if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;

    POINT cp;
    GetCursorPos(&cp);
    ScreenToClient(window.hWnd, &cp);
    racket.x = cp.x;

    if (!game.action && GetAsyncKeyState(VK_SPACE))
    {
        game.action = true;
        ProcessSound("bounce.wav");
    }
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // ������� �������� ������, ����������� � ���������� �����������
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// �������� ����������� bitmap � �������� ������

    if (hOldbm) // ���� �� ���� ������, ���������� ������
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // ���������� ������� �����������

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//��� ������� ������� ����� ����� ��������������� ��� ����������
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // ������ ����������� bitmap
        }

        SelectObject(hMemDC, hOldbm);// ��������������� �������� ������
    }

    DeleteDC(hMemDC); // ������� �������� ������
}

HPEN pen;
void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//������ ���
    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ������� ������
    
    for (int i = 0; i < bricks_y; i++) {
        for (int j = 0; j < bricks_x; j++) {
            if (wall[j][i].status == true) wall[j][i].show();
        }
    };

    if (ball.dy < 0 && (enemy.x - racket.width / 4 > ball.x || ball.x > enemy.x + racket.width / 4))
    {
        //��������� ���������� ���������. �� ����� ����, ��������� ������� �� �����������, � �� �� ������� �������� �� ��� ������� �� ������
        //������ �����, �� ������ ������ ������ �� �������, � ������� ���������� ������� - ����������� ��� �����
        //�������� ����� ������ ���� ����� ����� �����, � ������ ���� ����� �� ��� X ������� �� ������� �������� ����� �������
        //� ���� ������, �� ��������� ���������� ������� � ������ � ��������� 9 � 1
        enemy.x = ball.x * .1 + enemy.x * .9;
    }

    //ShowBitmap(window.context, enemy.x - racket.width / 2, 0, racket.width, racket.height, enemy.hBitmap);//������� ���������
    ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// �����

    /*if (!pen) pen = CreatePen(PS_SOLID, 0, RGB(255, 255, 255));
    SelectObject(window.context, pen);

    float x1 = 0; float y1 = 0;
    float x2 = 1500; float y2 = 500;
    float x3 = 500; float y3 = -220;
    float x4 = 0; float y4 = 500;
    
    float iX = 0; float iY = 0;

    MoveToEx(window.context, x1, y1, NULL);
    LineTo(window.context, x2, y2);
    MoveToEx(window.context, x3, y3, NULL);
    LineTo(window.context, x4, y4);
    checkIntersect(x1, y1, x2, y2, x3, y3, x4, y4, iX, iY);
    //SetPixel(window.context, iX, iY, RGB(255, 0, 0));
    float r = 10;
    Ellipse(window.context, iX - r, iY - r, iX + r, iY + r);
    */


    


}

void LimitRacket()
{
    racket.x = max(racket.x, racket.width / 2.);//���� ��������� ������ ���� ������� ������ ����, �������� �� ����
    racket.x = min(racket.x, window.width - racket.width / 2.);//���������� ��� ������� ����
}

void CheckWalls()
{
    if (ball.x < ball.rad || ball.x > window.width - ball.rad)
    {
        ball.dx *= -1;
        ProcessSound("bounce.wav");
    }
}

void CheckRoof()
{
    if (ball.y < ball.rad)
    {
        ball.dy *= -1;
        ProcessSound("bounce.wav");
    }
}

bool tail = false;

void CheckFloor()
{
    if (ball.y > window.height - ball.rad - racket.height)//����� ������� ����� ������� - ����������� �������
    {
        if (!tail && ball.x >= racket.x - racket.width / 2. - ball.rad && ball.x <= racket.x + racket.width / 2. + ball.rad)//����� �����, � �� �� � ������ ��������� ������
        {
            game.score++;//�� ������ ������� ���� ���� ����
           // ball.speed += 5. / game.score;//�� ����������� ��������� - ���������� �������� ������
            ball.dy *= -1;//������
            racket.width -= 10. / game.score;//������������� ��������� ������ ������� - ��� ���������
            ProcessSound("bounce.wav");//������ ���� �������
        }
        else
        {//����� �� �����

            tail = true;//����� ������ ������ ���� �������

            if (ball.y - ball.rad > window.height)//���� ����� ���� �� ������� ����
            {
                game.balls--;//��������� ���������� "������"

                ProcessSound("fail.wav");//������ ����

                if (game.balls < 0) { //�������� ������� ��������� "������"

                    MessageBoxA(window.hWnd, "game over", "", MB_OK);//������� ��������� � ���������
                    InitGame();//������������������ ����
                }

                ball.dy = (rand() % 65 + 35) / 100.;//������ ����� ��������� ������ ��� ������
                ball.dx = -(1 - ball.dy);
                ball.x = racket.x;//�������������� ���������� ������ - ������ ��� �� �������
                ball.y = racket.y - ball.rad;
                game.action = false;//���������������� ����, ���� ����� �� ������ ������
                tail = false;
            }
        }
    }
}

enum class side_ {
    left, right, top, bottom
};


void ProcessRoom()
{
    //������������ �����, ������� � ���. ������� - ���� ������� ����� ���� ���������, � ������, ��� ������� �� ����� ������ ������������� ����� ������� �������� ������
    CheckWalls();
    CheckRoof();
    CheckFloor();

    if (!pen) pen = CreatePen(PS_SOLID, 0, RGB(255, 255, 255));
    SelectObject(window.context, pen);

    POINT p;
    GetCursorPos(&p);
    ScreenToClient(window.hWnd, &p);
   // ball.x = p.x;
   // ball.y = p.y;

    float interX = 0;
    float interY = 0;
    bool inter = false;
    float l = 10000;
    float iX = 0; float iY = 0;
    int lasti;
    int lastj;

    float minvec = 0; float fullvec = 0;

    side_ side;

    for (int i = 0; i < bricks_y; i++) 
    {
        for (int j = 0; j < bricks_x; j++) 
        {
            if (wall[j][i].status == true) 
            {
                

                if (checkIntersect(ball.x, ball.y, ball.x + ball.dx * ball.speed, ball.y + ball.y * ball.speed,
                    wall[j][i].x, wall[j][i].y, wall[j][i].x + wall[j][i].width, wall[j][i].y, iX, iY))
                {
                    side = side_::top;
                    lasti = i;
                    lastj = j;
                    checkNerest(inter, iX, iY, l, interX, interY);
                }
                
                if (checkIntersect(ball.x, ball.y, ball.x + ball.dx * ball.speed, ball.y + ball.dy * ball.speed,
                    wall[j][i].x, wall[j][i].y + wall[j][i].height, wall[j][i].x + wall[j][i].width, wall[j][i].y + wall[j][i].height, iX, iY))
                {
                    side = side_::bottom;
                    lasti = i;
                    lastj = j;
                    checkNerest(inter, iX, iY, l, interX, interY);
                }
                
                if (checkIntersect(ball.x, ball.y, ball.x + ball.dx * ball.speed, ball.y + ball.dy * ball.speed,
                    wall[j][i].x, wall[j][i].y, wall[j][i].x, wall[j][i].y + wall[j][i].height, iX, iY))
                {
                    side = side_::left;
                    lasti = i;
                    lastj = j;
                    checkNerest(inter, iX, iY, l, interX, interY);
                }
                
                if (checkIntersect(ball.x, ball.y, ball.x + ball.dx * ball.speed, ball.y + ball.dy * ball.speed,
                    wall[j][i].x + wall[j][i].width, wall[j][i].y, wall[j][i].x + wall[j][i].width, wall[j][i].y + wall[j][i].height, iX, iY))
                {
                    side = side_::right;
                    lasti = i;
                    lastj = j;
                    checkNerest(inter, iX, iY, l, interX, interY);
                }

                /*if (wall[j][i].checkCollision(ball.x, ball.y) == true)
                {
                    int left = ball.x - wall[j][i].x;
                    int right = (wall[j][i].x + wall[j][i].width) - ball.x;
                    int up = ball.y - wall[j][i].y;
                    int down = (wall[j][i].y + wall[j][i].height) - ball.y;

                    int minHorizontal = min(left, right);
                    int minVertical = min(up, down);

                    if (minVertical < minHorizontal)
                    {
                        ball.dy *= -1;
                    }
                    else
                    {
                        ball.dx *= -1;

                    }

                    
                    wall[j][i].status = false;
                    return;
                }*/
                    
            }
        }
    };

    //----------
    if (inter)
    {
        if (side == side_::left || side == side_::right) 
        {
            ball.dx *= -1;
            minvec = sqrt((iX - ball.x) * (iX - ball.x) + (iY - ball.y) * (iY - ball.y));
            fullvec = sqrt((ball.dx - ball.x) * (ball.dx - ball.x) + (ball.dy - ball.y) * (ball.dy - ball.y));

        }

        if (side == side_::top || side == side_::bottom)
        {
            ball.dy *= -1;
            minvec = sqrt((iX - ball.x) * (iX - ball.x) + (iY - ball.y) * (iY - ball.y));
            fullvec = sqrt((ball.dx - ball.x) * (ball.dx - ball.x) + (ball.dy - ball.y) * (ball.dy - ball.y));
        }
        wall[lastj][lasti].status = false;
        float r = 10;
        Ellipse(window.context, interX - r, interY - r, interX + r, interY + r);
        MoveToEx(window.context, ball.x, ball.y, NULL);
        LineTo(window.context, ball.x+ball.dx*ball.speed, ball.y+ball.dy * ball.speed);
    }

}

void ProcessBall()
{
    if (game.action)
    {
        //���� ���� � �������� ������ - ���������� �����
        ball.x += ball.dx * ball.speed;
        ball.y += ball.dy * ball.speed;
    }
    else
    {
        //����� - ����� "��������" � �������
        ball.x = racket.x;
    }
}

void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//�� ������ ���� ������� ����� ��������� ���������� ��� ���������
    window.width = r.right - r.left;//���������� ������� � ���������
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//������ �����
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//����������� ���� � ���������
    GetClientRect(window.hWnd, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    
    InitWindow();//����� �������������� ��� ��� ����� ��� ��������� � ����
    InitGame();//����� �������������� ���������� ����

   // mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    ShowCursor(NULL);
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//������ ���, ������� � �����
        ShowScore();//������ ���� � �����
        ProcessInput();//����� ����������
        LimitRacket();//���������, ����� ������� �� ������� �� �����

        ProcessRoom();//������������ ������� �� ���� � �������, ��������� ������ � ��������
        ProcessBall();//���������� �����
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//�������� ����� � ����
        Sleep(16);//���� 16 ���������� (1/���������� ������ � �������)
    }

}
