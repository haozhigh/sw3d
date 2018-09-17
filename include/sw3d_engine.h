#pragma once


#include <vector>
#include <cmath>
#include <fstream>
using namespace std;

#include "types.h"
#include "obj.h"

#define M_PI 3.14159265358979323846f

class LineRaster {
public:
    LineRaster(float x0, float y0, float x1, float y1, int width, int height) :
        m_steep(false), m_x(0), m_step(0), m_x1(0), m_width(width), m_height(height)
    {
        if (abs(y0 - y1) > abs(x0 - x1)) {
            swap(x0, y0);
            swap(x1, y1);
            m_steep = true;
        }
        if (x0 > x1) {
            swap(x0, x1);
            swap(y0, y1);
        }

        if (x0 == x1)
            return;
        if (x1 < 0 || x0 > m_width)
            return;

        m_step = (y1 - y0) / (x1 - x0);
        if (x0 < 0)
            m_x = 0;
        else
            m_x = floor(x0);
        m_y = y0 + m_step * (m_x - x0);

        m_x1 = min(x1, m_width);
    }

    bool Next(ivec2& coor) {
        while (true) {
            if (m_x + 1 > m_x1)
                return false;

            m_x += 1;
            m_y += m_step;
            coor[0] = static_cast<int>(round(m_x));
            coor[1] = static_cast<int>(round(m_y));

            if (coor[1] >= 0 && coor[1] < m_height)
                break;
        }
        if (m_steep)
            swap(coor[0], coor[1]);
        return true;
    }

private:
    bool m_steep;
    float m_x;
    float m_y;
    float m_step;
    float m_x1;
    int m_width;
    int m_height;
};

class Sw3dEngine {

public:
	Sw3dEngine(float alpha, float r) : m_alpha(alpha), m_r(r),
        m_num_faces(0), m_num_faces_cc(0), m_num_faces_cc_big(0),
        m_log("sw3d_engine.log", ios::out) {
        m_project = mat4{
            1 / (m_r * tan(m_alpha / 2)), 0, 0, 0,
            0,         1 / tan(m_alpha / 2), 0, 0,
            0,                            0, 0, 1,
            0,                            0, 1, 0
        };
    }

	void SetBuffer(uchar* buffer, int width, int height) {
		m_buffer = buffer;
		m_width = width;
		m_height = height;

        /*
        ** set value of m_screen
        */
        m_screen = mat4{
            float(m_width) / 2 / m_r, 0, 0,  float(m_width) / 2,
            0,      float(m_height) / 2, 0, float(m_height) / 2,
            0,                        0, 1,                   0,
            0,                        0, 0,                   1
        };
	}

    wstring GetViewParameters() {
        wstring des;
        des += L"Eye Radius: " + to_wstring(m_radius) + L"\n";
        des += L"Eye Phi: "    + to_wstring(m_phi)    + L"\n";
        des += L"Eye Theta: "  + to_wstring(m_theta)  + L"\n";
        des += L"\n";
        des += L"View Alpha: " + to_wstring(m_alpha)  + L"\n";
        des += L"View Ratio: " + to_wstring(m_r)      + L"\n";
        des += L"\n";
        des += L"Screen Width: " + to_wstring(m_width)+ L"\n";
        des += L"Screen Height: " + to_wstring(m_height)+ L"\n";
        des += L"\n";
        des += L"Total Faces: " + to_wstring(m_num_faces)+ L"\n";
        des += L"Total Faces CC: " + to_wstring(m_num_faces_cc)+ L"\n";
        des += L"Total Faces CC Big: " + to_wstring(m_num_faces_cc_big)+ L"\n";
        return des;
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
		Line(x1, y1, x4, y4);
		Line(x4, y4, x3, y3);
		Line(x3, y3, x2, y2);
		Line(x2, y2, x5, y5);
		Line(x5, y5, x1, y1);
	}

	void ResetView(Obj& obj) {
		m_theta = M_PI / 2;
		m_phi = 0.0f;
		m_radius = sqrt(pow(obj.xmax - obj.xmin, 2.0f) +
			pow(obj.ymax - obj.ymin, 2.0f) + 
			pow(obj.zmax - obj.zmin, 2.0f));
		m_radius_min = m_radius / 2.0f * 1.01f;

		UpdateView();
	}

	void ZoomIn() {
		m_radius /= 1.2f;
		if (m_radius >= m_radius_min)
			m_radius = m_radius_min;
		UpdateView();
	}

	void ZoomOut() {
		m_radius *= 1.2f;
		UpdateView();
	}

	void RotateLeft() {
		m_phi -= M_PI / 8.0f;
		if (m_phi < 0.0f)
			m_phi += M_PI * 2.0f;
		UpdateView();
	}

	void RotateRight() {
		m_phi += M_PI / 8.0f;
		if (m_phi > M_PI * 2.0f)
			m_phi -= M_PI * 2.0f;
		UpdateView();
	}

	void RotateUp() {
        if (m_theta - M_PI / 8.0f > 0) {
		    m_theta -= M_PI / 8.0f;
            UpdateView();
        }
	}

	void RotateDown() {
        if (m_theta + M_PI / 8.0f < M_PI) {
		    m_theta += M_PI / 8.0f;
            UpdateView();
        }
	}

    void DrawObj(Obj& obj) {
        for (int y = 0; y < m_height; y++)
            for (int x = 0; x < m_width; x++) {
                m_buffer[Offset(x, y, 0)] = 255;
                m_buffer[Offset(x, y, 1)] = 255;
                m_buffer[Offset(x, y, 2)] = 255;
                m_buffer[Offset(x, y, 3)] = 255;
            }

        mat4 model2world { obj.axis_u[0], obj.axis_v[0], obj.axis_w[0], obj.origin[0],
                           obj.axis_u[1], obj.axis_v[1], obj.axis_w[1], obj.origin[1],
                           obj.axis_u[2], obj.axis_v[2], obj.axis_w[2], obj.origin[2],
                           0,             0,             0,             1 };
        mat4 model2view  { m_view * model2world };
        mat4 model2ndc   { m_project * model2view };

        // write matrix to log
        m_log << "Model to World: " << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m_log << model2world[i][j] << " ";
            }
            m_log << endl;
        }
        m_log << "World to View: " << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m_log << m_view[i][j] << " ";
            }
            m_log << endl;
        }
        m_log << "View to ndc: " << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m_log << m_project[i][j] << " ";
            }
            m_log << endl;
        }

        obj.vv.clear();
        obj.vv.reserve(obj.v.size());
        for (const auto& vertex : obj.v) {
            obj.vv.push_back(m_screen * homo(model2ndc * vertex));
        }

        m_num_faces = obj.vv.size();
        m_num_faces_cc = 0;
        m_num_faces_cc_big = 0;
        for (const auto& face : obj.f) {
            const auto& v0{ obj.vv[face[0][0] - 1] };
            const auto& v1{ obj.vv[face[1][0] - 1] };
            const auto& v2{ obj.vv[face[2][0] - 1] };
            vec2 edge0{ v1[0] - v0[0], v1[1] - v0[1] };
            vec2 edge1{ v2[0] - v1[0], v2[1] - v1[1] };
            vec2 edge0_cc90{ -edge0[1], edge0[0] };     // counter clockwise 90 degrees of edge0

            if (edge0_cc90.Dot(edge1) <= 0)
                continue;
            m_num_faces_cc += 1;

            if (edge0.Dot(edge1) < 0.1f)
                continue;
            m_num_faces_cc_big += 1;

            for (unsigned i = 0; i < face.size(); i++) {
                const auto& v1{ obj.vv[face[i][0] - 1] };
                const auto& v2{ obj.vv[face[(i + 1) % face.size()][0] - 1] };
                //Line(v1[0], v1[1], v2[0], v2[1]);
                Line(v1[0], v1[1], v2[0], v2[1]);
            }
        }

        vec4 v0{ 0, 0, 0, 1 };
        vec4 vx{ m_radius * 2, 0, 0, 1 };
        vec4 vy{ 0, m_radius * 2, 0, 1 };
        vec4 vz0{ 0, 0, m_radius * 2, 1 };
        vec4 vz1{ 0, 0, -m_radius * 2, 1 };
        mat4 world2ndc{ m_project * m_view };
        v0 = m_screen * homo(world2ndc * v0);
        vx = m_screen * homo(world2ndc * vx);
        vy = m_screen * homo(world2ndc * vy);
        vz0 = m_screen * homo(world2ndc * vz0);
        vz1 = m_screen * homo(world2ndc * vz1);
        //Line(v0[0], v0[1], vx[0], vx[1]);
        //Line(v0[0], v0[1], vy[0], vy[1]);
        //Line(vz0[0], vz0[1], vz1[0], vz1[1]);
    }

private:
	int Offset(int x, int y, int c) const {
		return y * m_width * 4 + x * 4 + c;
	}

	void UpdateView() {
		vec3 eye;
		vec3 up;

		eye[0] = m_radius * sin(m_theta) * cos(m_phi);
		eye[1] = m_radius * sin(m_theta) * sin(m_phi);
		eye[2] = m_radius * cos(m_theta);
		up = eye.Cross(vec3({ 0.0f, 0.0f, 1.0f })).Cross(eye);

		LookAt(eye, vec3({ 0.0f, 0.0f, 0.0f }), up);
	}

	// set world space to view space transform matrix
    void LookAt(vec3 eye, vec3 center, vec3 up) {
        vec3 z{ eye - center };
        vec3 x{ up.Cross(z) };
        vec3 y{ z.Cross(x) };
        x.Normalize();
        y.Normalize();
        z.Normalize();

        m_log << "Eye: " << endl;
        for (int i = 0; i < 3; i++)
            m_log << eye[i] << " ";
        m_log << endl;
        m_log << "Up: " << endl;
        for (int i = 0; i < 3; i++)
            m_log << up[i] << " ";
        m_log << endl;
        m_log << "X: " << endl;
        for (int i = 0; i < 3; i++)
            m_log << x[i] << " ";
        m_log << endl;
        m_log << "Y: " << endl;
        for (int i = 0; i < 3; i++)
            m_log << y[i] << " ";
        m_log << endl;
        m_log << "Z: " << endl;
        for (int i = 0; i < 3; i++)
            m_log << z[i] << " ";
        m_log << endl;


        mat3 P{ x, y, z };
        mat3 P_inv{ P.Inv() };

        m_log << "P: " << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m_log << P[i][j] << " ";
            }
            m_log << endl;
        }
        m_log << "P_inv: " << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m_log << P_inv[i][j] << " ";
            }
            m_log << endl;
        }

        vec3 eye_transformed{ P_inv * eye };
        m_view = mat4{ P_inv.data[0][0], P_inv.data[0][1], P_inv.data[0][2], -eye_transformed[0],
                       P_inv.data[1][0], P_inv.data[1][1], P_inv.data[1][2], -eye_transformed[1],
                       P_inv.data[2][0], P_inv.data[2][1], P_inv.data[2][2], -eye_transformed[2],
                       0,                0,                0,                1 };
    }

    void Line(float x0, float y0, float x1, float y1) {
        LineRaster lr{ x0, y0, x1, y1, m_width, m_height };
        ivec2 p;

        while (lr.Next(p)) {
			m_buffer[Offset(p[0], p[1], 0)] = 0;
			m_buffer[Offset(p[0], p[1], 1)] = 0;
			m_buffer[Offset(p[0], p[1], 2)] = 0;
			m_buffer[Offset(p[0], p[1], 3)] = 255;
        }
    }


	/*
	** Xiaolin Wu's line algorithm
	*/
    void LineAA(float x0, float y0, float x1, float y1, ivec3 channel_flag = ivec3{ 1, 1, 1 }) {
		auto plot = [&](float x, float y, float c) {
			int ix{ static_cast<int>(x) };
			int iy{ static_cast<int>(y) };
			uchar ic{ static_cast<uchar>(round((1 - c) * 255)) };
			if (ix >= m_width || iy >= m_height)
				return;
			if (ix < 0 || iy < 0)
				return;
			m_buffer[Offset(ix, iy, 0)] = ic * channel_flag[0];
			m_buffer[Offset(ix, iy, 1)] = ic * channel_flag[1];
			m_buffer[Offset(ix, iy, 2)] = ic * channel_flag[2];
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

	// dimension of output framebuffer
	int m_width;
	int m_height;
	uchar* m_buffer;

	// eye locaton in spherical coordinate system
	float m_radius;
	float m_theta;
	float m_phi;
	float m_radius_min;

	// frustum charactistics
	const float m_r;		// ratio of frustum width to height
	const float m_alpha;	// vertical view angle of frustum

    // counters
    int m_num_faces;
    int m_num_faces_cc;     // counter clockwise
    int m_num_faces_cc_big;
    ofstream m_log;
};
