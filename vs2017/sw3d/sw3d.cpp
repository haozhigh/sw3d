// sw3d.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "sw3d.h"
#include "framebuffer.h"
#include "sw3d_engine.h"

#include <memory>
#include <fstream>
using namespace std;

#include <commdlg.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
DWORD dw_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
RECT wc_offset;									// window and client offset

unique_ptr<FrameBufferWin32> p_fb;		// global framebuffer object
Sw3dEngine g_engine(M_PI / 2.0f, 1.6f);	// engine of soft 3d
unique_ptr<Obj> p_obj;					// smart pointer to Obj
bool need_redraw{ false };      /* redraw obj only after:
                                **  1. window size change
                                **  2. obj object change
                                **  3. view change
                                */

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SW3D, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SW3D));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SW3D));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground	= NULL;
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SW3D);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // calculate global win_client_offset
   RECT rect;
   rect.left = 200;
   rect.top = 200;
   rect.right = 400;
   rect.bottom = 400;
   RECT win_rect = rect;
   if (AdjustWindowRect(&win_rect, dw_style, true) == 0) {
	   MessageBox(NULL, L"Failed to call AdjustWindowRect in InitInstance", L"Error", MB_OK);
   }
   wc_offset.left = rect.left - win_rect.left;
   wc_offset.top = rect.top - win_rect.top;
   wc_offset.right = win_rect.right - rect.right;
   wc_offset.bottom = win_rect.bottom - rect.bottom;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, dw_style,
	   CW_USEDEFAULT, CW_USEDEFAULT, 640 + wc_offset.left + wc_offset.right, 400 + wc_offset.top + wc_offset.bottom,
	   nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void ShowClientRect(HWND hWnd) {
	RECT rect;
	if (GetClientRect(hWnd, &rect) == 0)
		MessageBox(hWnd, L"Failed to call GetClientRect in ShowClientRect", L"Error", MB_OK);
	wstring s = to_wstring(rect.left) + L" " + to_wstring(rect.top) + L" " +
		to_wstring(rect.right) + L" " + to_wstring(rect.bottom);
	MessageBox(hWnd, s.c_str(), L"Report Client Rect", MB_OK);
}

void ResizeWindow(HWND hWnd, LONG left, LONG top, LONG c_width, LONG c_height) {
	if (SetWindowPos(hWnd, NULL, left, top,
		c_width + wc_offset.left + wc_offset.right,
		c_height + wc_offset.top + wc_offset.bottom,
		SWP_NOZORDER) == 0)
		MessageBox(hWnd, L"Failed to call SetWindowPos in ResizeWindow", L"Error", MB_OK);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//static ofstream ofile("a.txt", std::ios::out);
	static bool allow_resize = false;
	static RECT last_rect;
	RECT rect;
    switch (message)
    {
	case WM_KEYDOWN:
		switch (wParam) {
		case 0x31:		// 1 key
			PostMessage(hWnd, WM_COMMAND, IDM_RESIZE_640_400, 0);
			break;
		case 0x32:		// 2 key
			PostMessage(hWnd, WM_COMMAND, IDM_RESIZE_960_600, 0);
			break;
		case 0x33:		// 3 key
			PostMessage(hWnd, WM_COMMAND, IDM_RESIZE_1280_800, 0);
			break;
		case 0x34:		// 4 key
			PostMessage(hWnd, WM_COMMAND, IDM_RESIZE_1600_1000, 0);
			break;
		case 0x35:		// 5 key
			PostMessage(hWnd, WM_COMMAND, IDM_RESIZE_1920_1200, 0);
			break;
		case 0x36:		// 6 key
			PostMessage(hWnd, WM_COMMAND, IDM_RESIZE_2240_1400, 0);
			break;
		case 0x4F:		// O key
			if ((GetKeyState(VK_LCONTROL) & 0x8000) || (GetKeyState(VK_RCONTROL) & 0x8000))
				PostMessage(hWnd, WM_COMMAND, IDM_OPEN, 0);
            else {
                g_engine.ZoomOut();
                need_redraw = true;
                InvalidateRect(hWnd, NULL, false);
            }
			break;
        case VK_LEFT:
            g_engine.RotateLeft();
            need_redraw = true;
            InvalidateRect(hWnd, NULL, false);
            break;
        case VK_RIGHT:
            g_engine.RotateRight();
            need_redraw = true;
            InvalidateRect(hWnd, NULL, false);
            break;
        case VK_UP:
            g_engine.RotateUp();
            need_redraw = true;
            InvalidateRect(hWnd, NULL, false);
            break;
        case VK_DOWN:
            g_engine.RotateDown();
            need_redraw = true;
            InvalidateRect(hWnd, NULL, false);
            break;
        case 0x49:  // I key
            g_engine.ZoomIn();
            need_redraw = true;
            InvalidateRect(hWnd, NULL, false);
            break;
		default:
			break;
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case IDM_OPEN:
				WCHAR fn[128];
				fn[0] = '\0';
				OPENFILENAME ofn;
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hWnd;
				ofn.hInstance = NULL;
				ofn.lpstrFilter = L"Obj Model Files\0*.obj\0\0";
				ofn.lpstrCustomFilter = NULL;
				ofn.lpstrFile = fn;
				ofn.nMaxFile = 128;
				ofn.lpstrFileTitle = NULL;
				ofn.lpstrInitialDir = NULL;
				ofn.lpstrTitle = L"Open a Obj File";
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
				ofn.lpstrDefExt = NULL;
				if (GetOpenFileName(&ofn) != 0) {
					//MessageBox(hWnd, ofn.lpstrFile, L"IDM_OPEN", MB_OKCANCEL);
					p_obj = make_unique<Obj>(wstring(ofn.lpstrFile));
                    g_engine.ResetView(*p_obj);
					wstring ws_tmp = to_wstring(p_obj->xmin) + L" " + to_wstring(p_obj->xmax) +
						to_wstring(p_obj->ymin) + L" " + to_wstring(p_obj->ymax) +
						to_wstring(p_obj->zmin) + L" " + to_wstring(p_obj->zmax);
					//MessageBox(hWnd, ws_tmp.data(), L"Obj Bounding Box", MB_OK);
                    need_redraw = true;
                    InvalidateRect(hWnd, NULL, false);
				}
				else {
					MessageBox(hWnd, L"Failed to open a file", L"IDM_OPEN", MB_OK);
				}
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case IDM_RESIZE_640_400:
				ResizeWindow(hWnd, last_rect.left, last_rect.top, 640, 400);
				allow_resize = true;
				break;
			case IDM_RESIZE_960_600:
				ResizeWindow(hWnd, last_rect.left, last_rect.top, 960, 600);
				allow_resize = true;
				break;
			case IDM_RESIZE_1280_800:
				ResizeWindow(hWnd, last_rect.left, last_rect.top, 1280, 800);
				allow_resize = true;
				break;
			case IDM_RESIZE_1600_1000:
				ResizeWindow(hWnd, last_rect.left, last_rect.top, 1600, 1000);
				allow_resize = true;
				break;
			case IDM_RESIZE_1920_1200:
				ResizeWindow(hWnd, last_rect.left, last_rect.top, 1920, 1200);
				allow_resize = true;
				break;
			case IDM_RESIZE_2240_1400:
				ResizeWindow(hWnd, last_rect.left, last_rect.top, 2240, 1400);
				allow_resize = true;
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            //PAINTSTRUCT ps;
            //HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
			//HDC m_hdc = CreateCompatibleDC(hdc);
			//RECT client_rect;
			//GetClientRect(hWnd, &client_rect);
			//auto width = client_rect.right - client_rect.left;
			//auto height = client_rect.bottom - client_rect.top;
			//HBITMAP m_bitmap = CreateCompatibleBitmap(hdc, width, height);
			//SelectObject(m_hdc, m_bitmap);
			//for (int x = 0; x < width; x++)
			//	for (int y = 0; y < height; y++)
			//		if (x >= 100 && x <= 150)
			//			SetPixel(m_hdc, x, y, RGB(0, 0, 0));
			//		else
			//			SetPixel(m_hdc, x, y, RGB(255, 255, 255));
			//BitBlt(hdc, 0, 0, width, height, m_hdc, 0, 0, SRCCOPY);

		//g_engine.DrawCircle();
        //g_engine.DrawStar();
        if (p_obj != nullptr) {
            if (need_redraw) {
                need_redraw = false;
                g_engine.DrawObj(*p_obj);
                p_fb->UpdateBitmap();
            }
        }


		if (GetUpdateRect(hWnd, NULL, false) != 0) {
			HDC hdc = GetDC(hWnd);
			BitBlt(hdc, 0, 0, p_fb->GetWidth(), p_fb->GetHeight(),
				p_fb->GetMemoryDC(), 0, 0, SRCCOPY);
			ReleaseDC(hWnd, hdc);
		}

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

        /*
        ** draw text of sw3d engine paremeters
        */
        wstring des{ g_engine.GetViewParameters() };
        RECT text_rect{ 10, 10, 10, 10 };
        DrawText(hdc, des.c_str(), -1, &text_rect, DT_BOTTOM | DT_TOP | DT_CALCRECT);
        DrawText(hdc, des.c_str(), -1, &text_rect, DT_BOTTOM | DT_TOP);

		EndPaint(hWnd, &ps);
        }
        break;
	case WM_SIZE:
		// get rect of window
		if (GetWindowRect(hWnd, &rect) != 0)
			last_rect = rect;
		else
			MessageBox(hWnd, L"Failed to get window rect in WM_SIZE", L"Error", MB_OK);

		p_fb = make_unique<FrameBufferWin32>(hWnd);
		g_engine.SetBuffer(p_fb->GetBuffer(), p_fb->GetWidth(), p_fb->GetHeight());

        need_redraw = true;
        InvalidateRect(hWnd, NULL, false);

		break;
	case WM_SIZING:
		if (allow_resize)
			allow_resize = false;
		else
			*((RECT*)lParam) = last_rect;

		//const RECT& rect = *((RECT*)lParam);
		//wstring s = to_wstring(rect.left) + L" " + to_wstring(rect.right) + L" " +
		//	to_wstring(rect.top) + L" " + to_wstring(rect.bottom);
		//MessageBox(hWnd, s.c_str(), L"WM_SIZING", MB_OK);
		break;
	case WM_MOVE:
		if (GetWindowRect(hWnd, &rect) != 0)
			last_rect = rect;
		else
			MessageBox(hWnd, L"Failed to get window rect in WM_MOVE", L"Error", MB_OK);
		break;
	case WM_ERASEBKGND:
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
