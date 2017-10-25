#include "stdafx.h"
#include "todoit.h"
#include "todoitem.h"

#define MAX_LOADSTRING 100
#define IDT_TIMER1 1001
#define IDT_HOTKEY 1002
#define TRANSPARENT_COLOR RGB(255, 0, 255)
#define FONT_SIZE 24
#define TODO_FILENAME "C:\\Users\\Kevin\\Dropbox\\todo.txt"
#define MAX_STRING_SIZE 500

HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
WCHAR* TextArray;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TODOIT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
        return FALSE;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, nullptr, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TODOIT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

// trim from start (in place)
static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
	}));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}

std::string countTodos(size_t& numTodos)
{
	std::ifstream hFile(TODO_FILENAME);
	std::string line;
	std::string firstTodo;
	numTodos = 0;
	while (std::getline(hFile, line)) {
		trim(line);
		if (line.empty())
			continue;
		auto item = TodoItem::init(line);
		if (item.IsDone())
			continue;
		if (firstTodo.empty())
			firstTodo = item.GetTodo();
		++numTodos;
	}
	return firstTodo;
}

void UpdateText(HWND hWnd)
{
	size_t numTodos;
	std::string firstTodo = countTodos(numTodos);
	std::wstring firstTodoW = utf8_to_wstring(firstTodo);
	swprintf_s(TextArray, MAX_STRING_SIZE, L" TODO (%zd) - %wS ", numTodos, firstTodoW.c_str());
	InvalidateRect(hWnd, NULL, TRUE);
}


void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   const auto x = 1150;
   const auto y = 0;
   const auto width = 300;
   const auto height = 200;

   HWND hWnd = CreateWindowExW(
	   WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
	   szWindowClass,
	   szTitle,
	   WS_POPUP,
	   x, y, width, height,
	   /*hWndParent*/ nullptr,
	   /*hMenu*/ nullptr,
	   /*hInstance*/ hInstance,
	   /*lpParam*/ nullptr
   );

   if (!hWnd)
      return FALSE;

   if (!RegisterHotKey(hWnd, IDT_HOTKEY, MOD_ALT | MOD_CONTROL | MOD_SHIFT, /*T*/ 0x54))
	   ErrorExit(L"RegisterHotKey");

	TextArray = new WCHAR[MAX_STRING_SIZE];
	lstrcpyW(TextArray, L"Hello World"); 

	HRGN GGG = CreateRectRgn(0, 0, width, height);
	InvertRgn(GetDC(hWnd), GGG);
	SetWindowRgn(hWnd, GGG, false);

	SetLayeredWindowAttributes(hWnd, TRANSPARENT_COLOR, (BYTE)0, LWA_COLORKEY);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	DeleteObject(GGG);

	SetTimer(hWnd, IDT_TIMER1, 20 * 1000, (TIMERPROC)NULL);
	UpdateText(hWnd);

	return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_TIMER:
	{
		switch (wParam)
		{
		case IDT_TIMER1:
			UpdateText(hWnd);
			return 0;
		}
	}
	case WM_ERASEBKGND:
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		auto hdc = (HDC)(wParam);
		{
			auto brush = CreateSolidBrush(TRANSPARENT_COLOR);
			FillRect(hdc, &rect, brush);
			DeleteObject(brush);
		}
		SelectObject(hdc, GetStockObject(BLACK_BRUSH));
		RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 8, 8);
		break;
	}
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

			HFONT hOldFont;
			auto hFont = CreateFont(FONT_SIZE, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
				CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));

			if (hOldFont = (HFONT)SelectObject(hdc, hFont))
			{
				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, RGB(255, 255, 255));
				int length = lstrlenW(TextArray);
				SIZE size;

				GetTextExtentPoint32(hdc, TextArray, length, &size);
				RECT rect;
				GetWindowRect(hWnd, &rect);
				if (rect.right - rect.left != size.cx || rect.bottom - rect.top != size.cy)
					MoveWindow(hWnd, rect.left, rect.top, size.cx, size.cy, TRUE);
				//TextOut(hdc, 0, 0, TextArray, length);
				GetClientRect(hWnd, &rect);
				DrawText(hdc, TextArray, -1, &rect, DT_LEFT | DT_NOCLIP | DT_SINGLELINE);
				SelectObject(hdc, hOldFont);
			}

            EndPaint(hWnd, &ps);
        }
        break;
	case WM_HOTKEY:
    case WM_DESTROY:
        PostQuitMessage(0);
		delete[] TextArray;
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

