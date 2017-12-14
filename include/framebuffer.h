
#include <new>
#include <iostream>
using namespace std;

#include "stdafx.h"

class FrameBufferWin32 {

public:

	FrameBufferWin32(HWND hWnd) : m_buffer(nullptr) {
		HDC hdc = GetDC(hWnd);
		m_hdc = CreateCompatibleDC(hdc);
		RECT client_rect;
		GetClientRect(hWnd, &client_rect);
		m_width = client_rect.right - client_rect.left;
		m_height = client_rect.bottom - client_rect.top;
		m_hbitmap = CreateCompatibleBitmap(hdc, m_width, m_height);
		m_origin = SelectObject(m_hdc, m_hbitmap);
		ReleaseDC(hWnd, hdc);

		m_buffer = new(nothrow) unsigned char[m_width * m_height * 4 * sizeof(unsigned char)];
		if (m_buffer == nullptr) {
			cerr << "failed to allocate buffer space" << endl;
		}
	}

	void UpdateBitmap() {
		for (int y = 0; y < m_height; y++)
			for (int x = 0; x < m_width; x++) {
				SetPixel(m_hdc, x, y, RGB(m_buffer[Offset(x, y, 0)],
					m_buffer[Offset(x, y, 1)], m_buffer[Offset(x, y, 2)]));
			}
	}

	HDC GetMemoryDC() { return m_hdc; }
	LONG GetWidth() { return m_width; }
	LONG GetHeight() { return m_height; }
	unsigned char* GetBuffer() { return m_buffer; }

	~FrameBufferWin32() {
		if (m_buffer != nullptr)
			delete[] m_buffer;

		SelectObject(m_hdc, m_origin);
		DeleteObject(m_hbitmap);
		DeleteDC(m_hdc);
	}
private:
	int Offset(int x, int y, int c) const {
		return y * m_width * 4 + x * 4 + c;
	}

private:
	HDC m_hdc;
	HBITMAP m_hbitmap;
	HGDIOBJ m_origin;
	LONG m_width;
	LONG m_height;
	unsigned char* m_buffer;
};