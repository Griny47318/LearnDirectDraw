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


// инициализация структуры DirectDraw
#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// GLOBALS ////////////////////////////////////////////////
HWND      main_window_handle = NULL; // глобальная переменная дискриптора окна
HINSTANCE hinstance_app = NULL;      // глобальный дискриптор экземпляра приложения

// основные материалы directdraw 

LPDIRECTDRAW7         lpdd = NULL;           // основной объект DirectDraw
LPDIRECTDRAWSURFACE7  lpddsprimary = NULL;   // структура основной поверхности
DDSURFACEDESC2        ddsd;                  // структура описания поверхности




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
    

    // проверка на нажатие клавиши ESC и отправления сообщения WM_CLOSE
    if (KEYDOWN(VK_ESCAPE))
        SendMessage(main_window_handle, WM_CLOSE, 0, 0);


    
    // очистка структуры описания поверхности
    DDRAW_INIT_STRUCT(ddsd);

    // блокировка основной поверхности
    if (FAILED(lpddsprimary->Lock(NULL, &ddsd,
        DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,
        NULL)))
        return(0);

 

    // разблокирование основной поверхности
    if (FAILED(lpddsprimary->Unlock(NULL)))
        return(0);

    // успешное завершение функции
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
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    // создание первичной поверхности
    if (FAILED(lpdd->CreateSurface(&ddsd, &lpddsprimary, NULL)))
        return(0);

    // усмешное завершение работы функции
    return(1);

} // завершение функции Game_Init

/////////////////////////////////////////////////////////////

int Game_Shutdown(void* parms = NULL, int num_parms = 0)
{
   
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

