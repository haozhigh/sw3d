

#include "stdafx.h"

class FrameBufferWin32 {

public:

	FrameBufferWin32(HWND hWnd) {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		m_hdc = CreateCompatibleDC(hdc);
		RECT client_rect;
		GetClientRect(hWnd, &client_rect);
		m_width = client_rect.right - client_rect.left;
		m_height = client_rect.bottom - client_rect.top;
		m_hbitmap = CreateCompatibleBitmap(hdc, m_width, m_height);
		m_origin = SelectObject(m_hdc, m_hbitmap);
		EndPaint(hWnd, &ps);
	}

	HDC GetDC() { return m_hdc; }
	LONG GetWidth() { return m_width; }
	LONG GetHeight() { return m_height; }

	~FrameBufferWin32() {
		SelectObject(m_hdc, m_origin);
		DeleteObject(m_hbitmap);
		DeleteDC(m_hdc);
	}

private:
	HDC m_hdc;
	HBITMAP m_hbitmap;
	HGDIOBJ m_origin;
	LONG m_width;
	LONG m_height;
};