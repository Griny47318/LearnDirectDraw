#pragma comment (lib, "ddraw.lib")

#define WIN32_LEAN_AND_MEAN  // уменьшает размер заголовочных файлов Win32, исключая некоторые из менее часто используемых API



// подключение необходимых библиотек
#include <windows.h>   
#include <windowsx.h> 
#include <initguid.h>

#include <ddraw.h> 

// DEFINES ////////////////////////////////////////////////

// имя класса окна
#define WINDOW_CLASS_NAME L"WINCLASS1"

// пораметры эранна по деффолту
#define SCREEN_WIDTH    640  // размер окна
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      32    // количество битов на пиксель

// TYPES //////////////////////////////////////////////////////

// базовые типы 
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;
typedef unsigned int   UINT;

// MACROS /////////////////////////////////////////////////

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// макрос для определения пикселя в формате 16-bit 5.5.5 
#define _RGB16BIT555(r,g,b) ((b&31) + ((g&31) << 5) + ((r&31) << 10))
// макрос для определения пикселя в формате 16-bit 5.6.5 
#define _RGB16BIT565(r,g,b) ((b&31) + ((g&63) << 5) + ((r&31) << 11))
// макрос для определения пикселя в формате 32 - bit A.8.8.8
#define _RGB32BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

// инициализация структуры DirectDraw
#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// GLOBALS ////////////////////////////////////////////////
HWND      main_window_handle = NULL;    // глобальная переменная дискриптора окна
HINSTANCE hinstance_app      = NULL;    // глобальный дискриптор экземпляра приложения
UINT      window_closed      = 0;

// основные материалы directdraw 

LPDIRECTDRAW7         lpdd         = NULL;   // основной объект DirectDraw
LPDIRECTDRAWSURFACE7  lpddsprimary = NULL;   // структура основной поверхности
LPDIRECTDRAWSURFACE7  lpddsback    = NULL;   // структура вторичной поверхности(задний буфер)
DDSURFACEDESC2        ddsd;                  // структура описания поверхности
DDBLTFX               ddbltfx;               // структура содержащая служебную информацию для функции Blt() 

// Функция установки пикселя 16-bit, в аргумент функции необходимо указать указатель на видео буфер и длинну одной строки экрана в bit.
inline void Plot_Pixel_16(int x, int y,
    int rad, int green, int blue,
    USHORT* video_buffer, int lpitch16)
{
    USHORT pixel = _RGB16BIT555(rad, green, blue);

    video_buffer[x + y * lpitch16] = pixel;
}
// Функция установки пикселя 24-bit, в аргумент функции необходимо указать указатель на видео буфер и длинну одной строки экрана в bit.
inline void Plot_Pixel_24(int x, int y,
    int rad, int green, int blue,
    UCHAR* video_buffer, int lpitch24)
{
    UINT pixel_addr = (3 * x + y * lpitch24);

    video_buffer[pixel_addr]     = blue;
    video_buffer[pixel_addr + 1] = green;
    video_buffer[pixel_addr + 2] = rad;
}
// Функция установки пикселя 32-bit, в аргумент функции необходимо указать указатель на видео буфер и длинну одной строки экрана в bit.
inline void Plot_Pixel_32(int x, int y, int alpha,
    int rad, int green, int blue,
    UINT* video_buffer, int lpitch32)
{
    UINT pixel = _RGB32BIT(alpha, rad, green, blue);

    video_buffer[x + y * lpitch32] = pixel;

    
}

// FUNCTIONS //////////////////////////////////////////////

LRESULT CALLBACK WindowProc(HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam)
{
    // основной обработчик сообщений системы
    PAINTSTRUCT		ps;		// структура с информацией для рисования клиенсткой обласни окна
    HDC				hdc;	// дискриптор устройства
    char buffer[80];        // буфер строки

    // проверка сообщения 
    switch (msg)
    {
    case WM_CREATE:
    {
        // данное сообщение обрабатывается при создании окна
        return(0);
    } break;

    case WM_PAINT:
    {
        // Сообщение WM_PAINT отправляется, когда система или другое приложение делает запрос на рисование части окна приложени 
        // Функция BeginPaint подготавливает указанное окно для рисования и заполняет структуру PAINTSTRUCT информацией о картине
        hdc = BeginPaint(hwnd, &ps);

        // завершение процесса рисования
        EndPaint(hwnd, &ps);

        // успешное завершение
        return(0);
    } break;

    case WM_DESTROY:
    {

        // завершение приложения и отправление сообщения WM_QUIT
        PostQuitMessage(0);

        // успешное завершение
        return(0);
    } break;

    default:break;

    } // конец оператора switch

    // данная функция обробатывает любое сообщение которое не попало в оператор switch
    return (DefWindowProc(hwnd, msg, wparam, lparam));

} // заершение CALLBACK функции WindowProc

///////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////

int Game_Main(void* parms = NULL, int num_parms = 0)
{
    

    
   
    RECT source_rect, // Исходный прямоугольник
         dest_rect; // Прямоугольник назначения

    // Подстраховка против повторного запуска
    if (window_closed)
        return(0);

    // проверка на нажатие клавиши ESC и отправления сообщения WM_CLOSE
    if (KEYDOWN(VK_ESCAPE))
    {
        PostMessage(main_window_handle, WM_CLOSE, 0, 0);
        window_closed = 1;
    } 

    // Выбор случайного исходного прямоугольника
    int x1 = rand() % SCREEN_WIDTH;
    int y1 = rand() % SCREEN_HEIGHT;
    int x2 = rand() % SCREEN_WIDTH;
    int y2 = rand() % SCREEN_HEIGHT;

    // Выбор случайного прямоугольника назначения
    int x3 = rand() % SCREEN_WIDTH;
    int y3 = rand() % SCREEN_HEIGHT;
    int x4 = rand() % SCREEN_WIDTH;
    int y4 = rand() % SCREEN_HEIGHT;

    // Задание структуры RECT для заполнения области между угловыми точками (x1,y1) и (x2,y2) на исходной поверхности
    source_rect.left = x1;
    source_rect.top = y1;
    source_rect.right = x2;
    source_rect.bottom = y2;

    // Задание структуры RECT для заполнения области между угловыми точками (x3,y3) и (x4,y4) на поверхности назначения
    dest_rect.left = x3;
    dest_rect.top = y3;
    dest_rect.right = x4;
    dest_rect.bottom = y4;

    // Вызов блиттера
    if (FAILED(lpddsprimary->Blt(
        &dest_rect,   // Область назначения
        lpddsback,    // Исходная поверхность
        &source_rect, // Исходная область
        DDBLT_WAIT,   // Управляющие флаги
        NULL)))       // Указатель на структуру DDBLTFX, в которой хранится необходимая информация
        return(0);


    // усмешное завершение работы функции
    return(1);
} // завершение функции Game_Main

////////////////////////////////////////////////////////////

int Game_Init(void* parms = NULL, int num_parms = 0)
{
   

    // создание объекта IDirectDraw interface 7 
    if (FAILED(DirectDrawCreateEx(NULL, (void**)&lpdd, IID_IDirectDraw7, NULL)))
        return(0);


    // установка связи между глобальным дискриптором окна и объектом DirectDraw в полно экранный режим
    if (FAILED(lpdd->SetCooperativeLevel(main_window_handle,
        DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX |
        DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT)))
        return(0);

    // установка режима отоброжения 640x480x16
    if (FAILED(lpdd->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, 0, 0)))
        return(0);

    // заполнение структуры поверхности
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.dwBackBufferCount = 1;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;

    // создание первичной поверхности
    if (FAILED(lpdd->CreateSurface(&ddsd, &lpddsprimary, NULL)))
        return(0);

    ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
    // присвоение первичной поверхности задний буфер
    if (FAILED(lpddsprimary->GetAttachedSurface(&ddsd.ddsCaps, &lpddsback)))
        return(0);

    DDRAW_INIT_STRUCT(ddsd);

    // блокирование заднего буффера
    if (FAILED(lpddsback->Lock(NULL, &ddsd,
        DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,
        NULL)))
        return(0);

    // получения указателя к поферхности заднего буфера и длинны одной строки в 32-bit режиме
    UINT* back_buffer = (UINT*)ddsd.lpSurface;
    UINT lpitch32 = (UINT)(ddsd.lPitch >> 2);

    // заполнения поферхности по строчно случайным цветом
    for (int i = 0; i < 480; i++) {
        UINT pixel = _RGB32BIT(0, rand() % 256, rand() % 256, rand() % 256);
        for (int j = 0; j < 640; j++) {
            back_buffer[j + i * 640] = pixel;
        }
    }

    // разблокирование заднего буфера
    if (FAILED(lpddsback->Unlock(NULL)))
        return(0);
    
    // усмешное завершение работы функции
    return(1);

} // завершение функции Game_Init

/////////////////////////////////////////////////////////////

int Game_Shutdown(void* parms = NULL, int num_parms = 0)
{
    // освобождение данных из заднего буффера
    if (lpddsback) {
        lpddsback->Release();
        lpddsback = NULL;
    }
    // освобождение данных из основной поверхности
    if (lpddsprimary)
    {
        lpddsprimary->Release();
        lpddsprimary = NULL;
    } 
    
    // освобождение структуры DirectDraw7
    if (lpdd)
    {
        lpdd->Release();
        lpdd = NULL;
    } 

    // усмешное завершение работы функции
    return(1);

} // конец функции Game_Shutdown()

// WINMAIN ////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hinstance,
    HINSTANCE hprevinstance,
    LPSTR lpcmdline,
    int ncmdshow)
{

    WNDCLASSEX winclass; // объект класса окна которое мы создаем
    HWND	   hwnd;	 // дискриптор окна
    MSG		   msg;		 // сообщениен
    HDC        hdc;      // контекст графического устройства

    // заполнение структуры окна
    winclass.cbSize = sizeof(WNDCLASSEX);
    winclass.style = CS_DBLCLKS | CS_OWNDC |
        CS_HREDRAW | CS_VREDRAW;
    winclass.lpfnWndProc = WindowProc;
    winclass.cbClsExtra = 0;
    winclass.cbWndExtra = 0;
    winclass.hInstance = hinstance;
    winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    winclass.lpszMenuName = NULL;
    winclass.lpszClassName = WINDOW_CLASS_NAME;
    winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // присвоение дискриптора экземпляра приложения к глобольной переменно
    hinstance_app = hinstance;

    // регистрация класса окна
    if (!RegisterClassEx(&winclass))
        return(0);

    // функция создания окна
    if (!(hwnd = CreateWindowEx(NULL,                  
        WINDOW_CLASS_NAME,     
        L"DirectDraw 16-Bit Full-Screen Demo", 
        WS_POPUP | WS_VISIBLE,
        0, 0,	  
        SCREEN_WIDTH, SCREEN_HEIGHT,  
        NULL,	  
        NULL,	  
        hinstance,
        NULL)))	
        return(0);

    // сохранения дискриптора окна в глобальной переменной
    main_window_handle = hwnd;

    // инициализация основных функций игры
    if (!(Game_Init()))
        return -1;

    // начало основного цикла
    while (TRUE)
    {
        // проверка на наличие сообщения в очереди, если оно есть, взять его
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // проверка на сообщения выхода
            if (msg.message == WM_QUIT)
                break;

            // перевод виртуальных клавиш в символьное обозначение
            TranslateMessage(&msg);

            // отправление сообщения в CALLBACK функцию
            DispatchMessage(&msg);
        } 

        // основная функция игры
        Game_Main();

    } // конец основнорго цикла

    // закрытие основных функций и освобождение выделенной памяти
    Game_Shutdown();

    
    return(msg.wParam);

} // завершение приложения

///////////////////////////////////////////////////////////

