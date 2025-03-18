//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include "math.h"

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false);

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;
// секция данных игры  
struct sprite {
    float x, y, width, height, rad, dx, dy, speed;
    bool status;
    HBITMAP hBitmap;//хэндл к спрайту шарика 

    void show()
    {
        ShowBitmap(window.context, x, y, width, height, hBitmap);
    }

    bool checkCollision(float x_, float y_)
    {
        return (x < x_ and y < y_ and (x + width) > x_ and (y + height) > y_);
    }

};

sprite racket;//ракетка игрока
sprite enemy;//ракетка противника
sprite ball;//шарик
const int bricks_x = 20;
const int bricks_y = 7;
sprite wall[bricks_x][bricks_y];

struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;



HBITMAP hBack;// хэндл для фонового изображения

//cекция кода

void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
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
    racket.speed = 30;//скорость перемещения ракетки
    racket.x = window.width / 2.;//ракетка посередине окна
    racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракетки

    enemy.x = racket.x;//х координату оппонета ставим в ту же точку что и игрока

    ball.dy = -(rand() % 65 + 35) / 100.;//формируем вектор полета шарика
    ball.dx = -(1 - ball.dy);//формируем вектор полета шарика
    ball.speed = 16;
    ball.rad = 20;
    ball.x = racket.x;//x координата шарика - на середие ракетки
    ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки

    game.score = 0;
    game.balls = 9;

   
}

void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
{
   // PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содежрит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
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
    //поиграем шрифтами и цветами
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//буфер для текста
    _itoa_s(game.score, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt
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

    hMemDC = CreateCompatibleDC(hDC); // Создаем контекст памяти, совместимый с контекстом отображения
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// Выбираем изображение bitmap в контекст памяти

    if (hOldbm) // Если не было ошибок, продолжаем работу
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // Определяем размеры изображения

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
        }

        SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
    }

    DeleteDC(hMemDC); // Удаляем контекст памяти
}

HPEN pen;
void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//задний фон
    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ракетка игрока
    
    for (int i = 0; i < bricks_y; i++) {
        for (int j = 0; j < bricks_x; j++) {
            if (wall[j][i].status == true) wall[j][i].show();
        }
    };

    if (ball.dy < 0 && (enemy.x - racket.width / 4 > ball.x || ball.x > enemy.x + racket.width / 4))
    {
        //имитируем разумность оппонента. на самом деле, компьютер никогда не проигрывает, и мы не считаем попадает ли его ракетка по шарику
        //вместо этого, мы всегда делаем отскок от потолка, а раектку противника двигаем - подставляем под шарик
        //движение будет только если шарик летит вверх, и только если шарик по оси X выходит за пределы половины длины ракетки
        //в этом случае, мы смешиваем координаты ракетки и шарика в пропорции 9 к 1
        enemy.x = ball.x * .1 + enemy.x * .9;
    }

    //ShowBitmap(window.context, enemy.x - racket.width / 2, 0, racket.width, racket.height, enemy.hBitmap);//ракетка оппонента
    ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// шарик

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
    racket.x = max(racket.x, racket.width / 2.);//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
    racket.x = min(racket.x, window.width - racket.width / 2.);//аналогично для правого угла
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
    if (ball.y > window.height - ball.rad - racket.height)//шарик пересек линию отскока - горизонталь ракетки
    {
        if (!tail && ball.x >= racket.x - racket.width / 2. - ball.rad && ball.x <= racket.x + racket.width / 2. + ball.rad)//шарик отбит, и мы не в режиме обработки хвоста
        {
            game.score++;//за каждое отбитие даем одно очко
           // ball.speed += 5. / game.score;//но увеличиваем сложность - прибавляем скорости шарику
            ball.dy *= -1;//отскок
            racket.width -= 10. / game.score;//дополнительно уменьшаем ширину ракетки - для сложности
            ProcessSound("bounce.wav");//играем звук отскока
        }
        else
        {//шарик не отбит

            tail = true;//дадим шарику упасть ниже ракетки

            if (ball.y - ball.rad > window.height)//если шарик ушел за пределы окна
            {
                game.balls--;//уменьшаем количество "жизней"

                ProcessSound("fail.wav");//играем звук

                if (game.balls < 0) { //проверка условия окончания "жизней"

                    MessageBoxA(window.hWnd, "game over", "", MB_OK);//выводим сообщение о проигрыше
                    InitGame();//переинициализируем игру
                }

                ball.dy = (rand() % 65 + 35) / 100.;//задаем новый случайный вектор для шарика
                ball.dx = -(1 - ball.dy);
                ball.x = racket.x;//инициализируем координаты шарика - ставим его на ракетку
                ball.y = racket.y - ball.rad;
                game.action = false;//приостанавливаем игру, пока игрок не нажмет пробел
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
    //обрабатываем стены, потолок и пол. принцип - угол падения равен углу отражения, а значит, для отскока мы можем просто инвертировать часть вектора движения шарика
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
        //если игра в активном режиме - перемещаем шарик
        ball.x += ball.dx * ball.speed;
        ball.y += ball.dy * ball.speed;
    }
    else
    {
        //иначе - шарик "приклеен" к ракетке
        ball.x = racket.x;
    }
}

void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.hWnd, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

   // mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    ShowCursor(NULL);
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        ShowScore();//рисуем очик и жизни
        ProcessInput();//опрос клавиатуры
        LimitRacket();//проверяем, чтобы ракетка не убежала за экран

        ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в картетку
        ProcessBall();//перемещаем шарик
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)
    }

}
