#pragma once


#include <vector>
using namespace std;

#include "types.h"
#include "obj.h"

class Sw3dEngine {

public:
    Sw3dEngine() {

    }

	void SetBuffer(uchar* buffer, int width, int height) {
		m_buffer = buffer;
		m_width = width;
		m_height = height;
	}

	void DrawCircle() {
		int half_width{ m_width / 2 };
		int half_height{ m_height / 2 };
		int r_max{ (m_height / 4 * 3) * (m_height / 4 * 3) };
		int r_min{ (m_height / 2) * (m_height / 2) };

		for (int y = 0; y < m_height; y++)
			for (int x = 0; x < m_width; x++) {
				int rr{ (y - half_height) * (y - half_height) +
				(x - half_width) * (x - half_width) };
				if (rr >= r_min && rr <= r_max) {
					m_buffer[Offset(x, y, 0)] = 0;
					m_buffer[Offset(x, y, 1)] = 0;
					m_buffer[Offset(x, y, 2)] = 0;
					m_buffer[Offset(x, y, 3)] = 255;
				}
				else {
					m_buffer[Offset(x, y, 0)] = 128;
					m_buffer[Offset(x, y, 1)] = 128;
					m_buffer[Offset(x, y, 2)] = 128;
					m_buffer[Offset(x, y, 3)] = 255;
				}
			}
	}

	void DrawStar() {
		for (int y = 0; y < m_height; y++)
			for (int x = 0; x < m_width; x++) {
				m_buffer[Offset(x, y, 0)] = 255;
				m_buffer[Offset(x, y, 1)] = 255;
				m_buffer[Offset(x, y, 2)] = 255;
				m_buffer[Offset(x, y, 3)] = 255;
			}

		float width{ static_cast<float>(m_width) };
		float height{ static_cast<float>(m_height) };
		float x1{ width / 2 };
		float y1{ 0 };
		float x2{ 0 };
		float y2{ height / 3 };
		float x3{ width - 1 };
		float y3{ height / 3 };
		float x4{ 0 };
		float y4{ height - 1 };
		float x5{ width - 1 };
		float y5{ height - 1 };
		LineAA(x1, y1, x4, y4);
		LineAA(x4, y4, x3, y3);
		LineAA(x3, y3, x2, y2);
		LineAA(x2, y2, x5, y5);
		LineAA(x5, y5, x1, y1);
		//LineAA(0.0f, 0.0f, 6.0f, 2.0f);
	}

    void LookAt(vec3 eye, vec3 center, vec3 up) {
        vec3 z{ eye - center };
        vec3 x{ up.Cross(z) };
        vec3 y{ z.Cross(x) };
        x.Normalize();
        y.Normalize();
        z.Normalize();
        mat3 P{ x, y, z };
        mat3 P_inv{ P.Inv() };
        vec3 eye_transformed{ P_inv * eye };
        m_view = mat4{ P_inv.data[0][0], P_inv.data[0][1], P_inv.data[0][2], -eye_transformed[0],
                       P_inv.data[1][0], P_inv.data[1][1], P_inv.data[1][2], -eye_transformed[1],
                       P_inv.data[2][0], P_inv.data[2][1], P_inv.data[2][2], -eye_transformed[2],
                       0,                0,                0,                1 };
    }

    void DrawObj(Obj& obj) {
        mat4 model2world { obj.axis_u[0], obj.axis_v[0], obj.axis_w[0], obj.origin[0],
                           obj.axis_u[1], obj.axis_v[1], obj.axis_w[1], obj.origin[1],
                           obj.axis_u[2], obj.axis_v[2], obj.axis_w[2], obj.origin[2],
                           0,             0,             0,             1 };
        mat4 model2view  { m_view * model2world };
        mat4 model2ndc   { m_project * model2view };
        mat4 model2screen{ m_screen * model2ndc };

        obj.vv.clear();
        obj.vv.reserve(obj.v.size());
        for (const auto& vertex : obj.v) {
            obj.vv.push_back(model2screen * vertex);
        }
    }

private:
	int Offset(int x, int y, int c) const {
		return y * m_width * 4 + x * 4 + c;
	}

	/*
	** Xiaolin Wu's line algorithm
	*/
	void LineAA(float x0, float y0, float x1, float y1) {
		auto plot = [&](float x, float y, float c) {
			int ix{ static_cast<int>(x) };
			int iy{ static_cast<int>(y) };
			uchar ic{ static_cast<uchar>(round((1 - c) * 255)) };
			if (ix >= m_width || iy >= m_height)
				return;
			m_buffer[Offset(ix, iy, 0)] = ic;
			m_buffer[Offset(ix, iy, 1)] = ic;
			m_buffer[Offset(ix, iy, 2)] = ic;
			m_buffer[Offset(ix, iy, 3)] = 255;
		};
		auto ipart = [&](float x) {return floor(x); };
		auto round = [&](float x) {return ipart(x + 0.5f); };
		auto fpart = [&](float x) {return x - floor(x); };
		auto rfpart = [&](float x) {return 1.0f - fpart(x); };
		auto swap = [&](float& x, float& y) {
			float t = x;
			x = y;
			y = t;
		};

		bool steep{ abs(y1 - y0) > abs(x1 - x0) };
		if (steep) {
			swap(x0, y0);
			swap(x1, y1);
		}
		if (x0 > x1) {
			swap(x0, x1);
			swap(y0, y1);
		}

		float dx{ x1 - x0 };
		float dy{ y1 - y0 };
		float gradient;
		if (dx == 0.0)
			gradient = 1.0;
		else
			gradient = dy / dx;

		float xend{ round(x0) };
		float yend{ y0 + gradient * (xend - x0) };
		float xgap{ rfpart(x0 + 0.5f) };
		float xpxl1{ xend };
		float ypxl1{ ipart(yend) };
		if (steep) {
			plot(ypxl1, xpxl1, rfpart(yend) * xgap);
			plot(ypxl1 + 1, xpxl1, fpart(yend) * xgap);
		}
		else {
			plot(xpxl1, ypxl1, rfpart(yend) * xgap);
			plot(xpxl1, ypxl1 + 1, fpart(yend) * xgap);
		}
		float intery{ yend + gradient };

		xend = round(x1);
		yend = y1 + gradient * (xend - x1);
		xgap = fpart(x1 + 0.5f);
		float xpxl2{ xend };
		float ypxl2{ ipart(yend) };
		if (steep) {
			plot(ypxl2, xpxl2, rfpart(yend) * xgap);
			plot(ypxl2 + 1, xpxl2, fpart(yend) * xgap);
		}
		else {
			plot(xpxl2, ypxl2, rfpart(yend) * xgap);
			plot(xpxl2, ypxl2 + 1, fpart(yend) * xgap);
		}

		if (steep) {
			for (float x{ xpxl1 + 1 }; x < xpxl2; x++) {
				plot(ipart(intery), x, rfpart(intery));
				plot(ipart(intery) + 1, x, fpart(intery));
				intery += gradient;
			}
		}
		else {
			for (float x{ xpxl1 + 1 }; x < xpxl2; x++) {
				plot(x, ipart(intery), rfpart(intery));
				plot(x, ipart(intery) + 1, fpart(intery));
				intery += gradient;
			}
		}
	}

private:
    mat4 m_view;
    mat4 m_project;
    mat4 m_screen;


	int m_width;
	int m_height;
	uchar* m_buffer;

};
