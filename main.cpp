#include "resource.h"
#include <winuser.h>
#define TRAY_NOTIFY     WM_APP + 1
#define CLASS_NAME      L"DigitClock"

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nCmdShow) {
    WNDCLASSEX wcex = {
        sizeof(wcex),
        CS_HREDRAW | CS_VREDRAW,
        WndProc,
        0,0,
        hInst,
        NULL, LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW+1),
        NULL,
        CLASS_NAME,
        NULL
    };

    RegisterClassEx(&wcex);

    HWND hWnd = CreateWindowEx(
                WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
                CLASS_NAME,
                CLASS_NAME,
                WS_POPUP | WS_BORDER | WS_VISIBLE,
                10, 10, 300, 150,
                NULL,
                (HMENU)NULL,
                hInst,
                NULL
            );

    ShowWindow(hWnd, nCmdShow);

    MSG msg;
    while(GetMessage(&msg, NULL, 0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
    NOTIFYICONDATA nid;
    HMENU hMenu, hPopupMenu;
    POINT Mouse;

    SYSTEMTIME st;
    WCHAR temp[0x20];

    HDC hdc;
    PAINTSTRUCT ps;

    static BOOL bShow;
    static SIZE MaxSize;
    static RECT WindowSize;
    static WCHAR Time[0x20];

    switch(iMessage){
        case WM_CREATE:
            ZeroMemory(&nid, sizeof(nid));
            ZeroMemory(Time, sizeof(Time));
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hWnd;
            nid.uID = 0;
            nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
            nid.uCallbackMessage = TRAY_NOTIFY;
            nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
            wcscpy(temp, L"88:88:88");
            hdc = GetDC(hWnd);
            GetTextExtentPoint32(hdc, temp, 8, &MaxSize);
            ReleaseDC(hWnd, hdc);
            wcscpy(nid.szTip, L"DigitClock"); 
            Shell_NotifyIcon(NIM_ADD, &nid);
            SetRect(&WindowSize, 0, 0, MaxSize.cx + 1, MaxSize.cy + 1);
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

                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
            }
            return 0;

        case WM_TIMER:
            switch(wParam){
                case 1:
                    // time
                    GetLocalTime(&st);
                    wsprintf(Time, L"%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
                    break;
            }
            InvalidateRect(hWnd, NULL, TRUE);
            return 0;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            TextOut(hdc, 0,0, Time, wcslen(Time));
            EndPaint(hWnd, &ps);
            return 0;

        case WM_DESTROY:
            KillTimer(hWnd, 1);
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hWnd;
            nid.uID = 0;
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, iMessage, wParam, lParam);
}
