#include "resource.h"
#include <winuser.h>
#define TRAY_NOTIFY     WM_APP + 1
#define CLASS_NAME      L"DigitClock"
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define DEFAULT_HEIGHT 16
#define BORDER 12
#define EDGE 6
#define GET_X_LPARAM(lParam) ((int)(short)LOWORD(lParam))
#define GET_Y_LPARAM(lParam) ((int)(short)HIWORD(lParam))

BOOL BkIsDark;
COLORREF MaskColor;
COLORREF TextColor;

void DrawClock(HWND hWnd, HDC hdc, int FontHeight, HBRUSH hMask);
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBitmap);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nCmdShow) {
    WNDCLASSEX wcex = {
        sizeof(wcex),
        CS_HREDRAW | CS_VREDRAW,
        WndProc,
        0,0,
        hInst,
        NULL, LoadCursor(NULL, IDC_ARROW),
        NULL,
        NULL,
        CLASS_NAME,
        NULL
    };

    RegisterClassEx(&wcex);

    HWND hWnd = CreateWindowEx(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED,
            CLASS_NAME,
            CLASS_NAME,
            WS_POPUP,
            10, 10, 300, 150,
            NULL,
            (HMENU)NULL,
            hInst,
            NULL
            );

    ShowWindow(hWnd, nCmdShow);

    BkIsDark = TRUE;
    MaskColor = RGB(0, 0, 0);
    TextColor = RGB(255, 255, 255);
	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);

    MSG msg;
    while(GetMessage(&msg, NULL, 0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
    static int FontHeight;
    static HBITMAP hBitmap, hMaskBitmap;
    static BOOL bShow;
    static RECT WindowSize;
    static RECT crt, wrt;
    static HFONT hFont;
    HDC hdc, hMemDC, hMaskDC;
    HBITMAP hOldBitmap, hOldMaskBitmap;
    HBRUSH hBrush;
    HFONT hOldFont;
    PAINTSTRUCT ps;
    SIZE TimeMaxSize, DateMaxSize;

    NOTIFYICONDATA nid;
    HMENU hMenu, hPopupMenu;
    POINT Mouse;

    WCHAR temp[0x20];

    switch(iMessage){
        case WM_CREATE:
            FontHeight = DEFAULT_HEIGHT * 3;
            hFont = CreateFont(
                    FontHeight,
                    0,
                    0,
                    0,
                    FW_NORMAL,
                    FALSE,
                    FALSE,
                    FALSE,
                    ANSI_CHARSET,
                    OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY,
                    FF_MODERN,
                    L"Consolas"
                    );

            ZeroMemory(&nid, sizeof(nid));
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hWnd;
            nid.uID = 0;
            nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
            nid.uCallbackMessage = TRAY_NOTIFY;
            nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
            wcscpy(nid.szTip, L"DigitClock"); 
            Shell_NotifyIcon(NIM_ADD, &nid);

            hdc = GetDC(hWnd);
            hOldFont = (HFONT)SelectObject(hdc, hFont);
            wsprintf(temp, L"88:88:88 %lc", 0x25A0);
            GetTextExtentPoint32(hdc, temp, 8, &TimeMaxSize);
            wcscpy(temp, L"금요일, 88월 88일");
            GetTextExtentPoint32(hdc, temp, 12, &DateMaxSize);
            SelectObject(hdc, hOldFont);
            ReleaseDC(hWnd, hdc);

            SetRect(&WindowSize, 0, 0, max(DateMaxSize.cx, TimeMaxSize.cx) + 1, DateMaxSize.cy + TimeMaxSize.cy + 1);
            SetWindowPos(hWnd, NULL, WindowSize.left, WindowSize.top, WindowSize.right - WindowSize.left, WindowSize.bottom - WindowSize.top, SWP_NOZORDER);

            bShow = TRUE;
            SetTimer(hWnd, 1, 100, NULL);
            return 0;

        case TRAY_NOTIFY:
            // wParam == uID, lParam == Action
            switch(lParam){
                case WM_RBUTTONDOWN:
                    hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU1));
                    hPopupMenu = GetSubMenu(hMenu, 0);
                    GetCursorPos(&Mouse);
                    if(bShow){
                        CheckMenuItem(hMenu, IDM_SHOW, MF_BYCOMMAND | MF_CHECKED);
                        CheckMenuItem(hMenu, IDM_HIDE, MF_BYCOMMAND | MF_UNCHECKED);
                    }else{
                        CheckMenuItem(hMenu, IDM_SHOW, MF_BYCOMMAND | MF_UNCHECKED);
                        CheckMenuItem(hMenu, IDM_HIDE, MF_BYCOMMAND | MF_CHECKED);
                    }

                    CheckMenuItem(hMenu, IDM_BKDARK, MF_BYCOMMAND | (BkIsDark ? MF_CHECKED : MF_UNCHECKED));
                    SetForegroundWindow(hWnd);
                    TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Mouse.x, Mouse.y, 0, hWnd, NULL);
                    SetForegroundWindow(hWnd);
                    DestroyMenu(hPopupMenu);
                    DestroyMenu(hMenu);
                    break;
            }
            return 0;

        case WM_COMMAND:
            switch(wParam){
                case IDM_SHOW:
                    if(bShow){ break; }
                    bShow = TRUE;
                    SetTimer(hWnd, 1, 100, NULL);
                    SetWindowPos(hWnd, NULL, WindowSize.left, WindowSize.top, WindowSize.right - WindowSize.left, WindowSize.bottom - WindowSize.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                    break;

                case IDM_HIDE:
                    if(!bShow){ break; }
                    bShow = FALSE;
                    KillTimer(hWnd, 1);
                    SetWindowPos(hWnd, NULL, 0,0,0,0, SWP_NOZORDER | SWP_HIDEWINDOW);
                    break;

                case IDM_BKDARK:
                    BkIsDark = !BkIsDark;
                    MaskColor = BkIsDark ? RGB(0, 0, 0) : RGB(255, 255, 255);
                    TextColor = BkIsDark ? RGB(255, 255, 255) : RGB(0, 0, 0);
                    SetLayeredWindowAttributes(hWnd, MaskColor, 255, LWA_ALPHA | LWA_COLORKEY);
                    SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                    break;

                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
            }
            return 0;

        case WM_TIMER:
            switch(wParam){
                case 1:
                    hdc = GetDC(hWnd);
                    hMemDC = CreateCompatibleDC(hdc);
                    hMaskDC = CreateCompatibleDC(hdc);

                    if(hBitmap == NULL){
                        GetClientRect(hWnd, &crt);
                        hBitmap = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
                        hMaskBitmap = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
                    }

                    hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
                    hOldMaskBitmap = (HBITMAP)SelectObject(hMaskDC, hMaskBitmap);

                    hBrush = CreateSolidBrush(MaskColor);
                    FillRect(hMemDC, &crt, hBrush);
                    hOldFont = (HFONT)SelectObject(hMaskDC, hFont);
                    DrawClock(hWnd, hMaskDC, FontHeight, hBrush);
                    SelectObject(hMaskDC, hOldFont);
                    DeleteObject(hBrush);

                    TransparentBlt(hMemDC, 0,0, crt.right, crt.bottom, hMaskDC, 0,0, crt.right, crt.bottom, MaskColor);

                    SelectObject(hMaskDC, hOldMaskBitmap);
                    SelectObject(hMemDC, hOldBitmap);
                    DeleteDC(hMaskDC);
                    DeleteDC(hMemDC);
                    ReleaseDC(hWnd, hdc);
                    break;
            }
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;

        case WM_SIZE:
            if(SIZE_MINIMIZED != wParam){
                if(hBitmap){
                    DeleteObject(hBitmap);
                    hBitmap = NULL;
                }

                if(hMaskBitmap){
                    DeleteObject(hMaskBitmap);
                    hMaskBitmap = NULL;
                }

            }
            return 0;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            DrawBitmap(hdc, 0, 0, hBitmap);
            EndPaint(hWnd, &ps);
            return 0;

        case WM_WINDOWPOSCHANGING:
            {
                MONITORINFOEX miex;
                memset(&miex, 0, sizeof(miex));
                miex.cbSize = sizeof(miex);

                HMONITOR hCurrentMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                GetMonitorInfo(hCurrentMonitor, &miex);

                LPWINDOWPOS lpwp = (LPWINDOWPOS)lParam;
                int SideSnap = 10;

                if (abs(lpwp->x - miex.rcMonitor.left) < SideSnap) {
                    lpwp->x = miex.rcMonitor.left;
                } else if (abs(lpwp->x + lpwp->cx - miex.rcMonitor.right) < SideSnap) {
                    lpwp->x = miex.rcMonitor.right - lpwp->cx;
                } 
                if (abs(lpwp->y - miex.rcMonitor.top) < SideSnap) {
                    lpwp->y = miex.rcMonitor.top;
                } else if (abs(lpwp->y + lpwp->cy - miex.rcMonitor.bottom) < SideSnap) {
                    lpwp->y = miex.rcMonitor.bottom - lpwp->cy;
                }
            }
            return 0;

        case WM_NCHITTEST:
            Mouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            GetWindowRect(hWnd, &wrt);

            if(Mouse.x >= wrt.left + BORDER && Mouse.x <= wrt.right - BORDER && Mouse.y >= wrt.top + BORDER && Mouse.y <= wrt.bottom - BORDER){
                return HTCAPTION;
            }
            break;

        case WM_DESTROY:
            KillTimer(hWnd, 1);
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hWnd;
            nid.uID = 0;
            Shell_NotifyIcon(NIM_DELETE, &nid);
            DeleteObject(hFont);
            DeleteObject(hMaskBitmap);
            DeleteObject(hBitmap);
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

void DrawClock(HWND hWnd, HDC hdc, int FontHeight, HBRUSH hMask) {
    static WCHAR* yoil[] = {
        L"일요일",
        L"월요일",
        L"화요일",
        L"수요일",
        L"목요일",
        L"금요일",
        L"토요일",
    };

    SYSTEMTIME st;
    GetLocalTime(&st);
    WCHAR Time[0x40];
    WCHAR *ptr;

    int hour, min, sec, x, y;

    hour = ((st.wHour > 12) ? (st.wHour - 12) : st.wHour);
    if(hour == 12) { hour = 0; }
    min = st.wMinute;
    sec = st.wSecond;

    wsprintf(
            Time,
            L"%02d:%02d:%02d %s %lc\r\n%s, %02d월 %02d일",
            hour, min, sec,
            ((st.wHour > 12) ? L"PM" : L"AM"),
            0x25A0,
            yoil[st.wDayOfWeek],
            st.wMonth,
            st.wDay
            );

    RECT crt;
    GetClientRect(hWnd, &crt);
    FillRect(hdc, &crt, hMask);
    int BkMode = SetBkMode(hdc, TRANSPARENT);
    COLORREF prev = SetTextColor(hdc, TextColor);

    x = 0, y = 0;
    ptr = wcstok(Time, L"\r\n");
    while(ptr){
        TextOut(hdc, x, y, ptr, wcslen(ptr));
        y += FontHeight;
        ptr = wcstok(NULL, L"\r\n");
    }

    SetTextColor(hdc, prev);
    SetBkMode(hdc, BkMode);
}

void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBitmap){
	HDC hMemDC = CreateCompatibleDC(hdc);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	BitBlt(hdc, x,y, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
}
