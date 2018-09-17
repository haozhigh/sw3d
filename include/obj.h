#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <limits>
using namespace std;

#include "common.h"
#include "types.h"

struct Obj {
    vector<vec4> v;     // geometric vertices
    vector<vec4> vv;    // geometric vertices after transform to screen space
    vector<vec3> vt;    // txture vertices
    vector<vec3> vn;    // vertex normals

    vector<vector<uivec3>> f;    // faces

    vec3 origin;        // origin of this object in world coordinate
    vec3 axis_u;        // axis u of this object described in world axis x,y,z
    vec3 axis_v;        // axis v of this object described in world axis x,y,z
    vec3 axis_w;        // axis w of this object described in world axis x,y,z

	// bounding box of all vertices
	float xmin;
	float xmax;
	float ymin;
	float ymax;
	float zmin;
	float zmax;
    

	Obj(wstring obj_filepath):
		xmin(FLT_MAX),
		xmax(-FLT_MAX),
		ymin(FLT_MAX),
		ymax(-FLT_MAX),
		zmin(FLT_MAX),
		zmax(-FLT_MAX)
	{
        ifstream ifile(obj_filepath.c_str());
        ExitAsssert(ifile.is_open(), wstring(L"Failed to open obj file: ") + obj_filepath);

        string line;
        while (ifile.good()) {
            getline(ifile, line);
            if (line.empty())
                continue;

            auto spans { Split(line, ' ') };
            if (StartsWith(line, "v ")) {
                float x{ stof(spans[1], nullptr) };
                float y{ stof(spans[2], nullptr) };
                float z{ stof(spans[3], nullptr) };
                float w{ spans.size() > 4 ? stof(spans[4], nullptr) : 1.0f };
				v.emplace_back(vec4{ x, y, z, w });

				// update bounding box
				xmin = min(xmin, x);
				xmax = max(xmax, x);
				ymin = min(ymin, y);
				ymax = max(ymax, y);
				zmin = min(zmin, z);
				zmax = max(zmax, z);
            }
            else if (StartsWith(line, "vn ")) {
                float i{ stof(spans[1], nullptr) };
                float j{ stof(spans[2], nullptr) };
                float k{ stof(spans[3], nullptr) };
				vn.emplace_back(vec3{ i, j, k });
            }
            else if (StartsWith(line, "vt ")) {
                float u{ stof(spans[1], nullptr) };
                float v{ stof(spans[2], nullptr) };
                float w{ spans.size() > 3 ? stof(spans[3], nullptr) : 0.0f };
				vt.emplace_back(vec3{ u, v, w });
            }
            else if (StartsWith(line, "f ")) {
                vector<uivec3> face;
                for (unsigned i = 1; i < spans.size(); i++) {
                    auto spans_inner{ Split(spans[i], '/') };
                    /*
                    ** vt_id and vn_id is optional argument
                    ** so value of 0 stands for empty vt_id or vn_id
                    ** since in obj files ids starts from 1
                    */
                    unsigned v_id = stoi(spans_inner[0], nullptr, 10);
                    unsigned vt_id = spans_inner[1].size() > 0 ? stoi(spans_inner[1], nullptr, 10) : 0;
                    unsigned vn_id = spans_inner[2].size() > 0 ? stoi(spans_inner[2], nullptr, 10) : 0;
					face.emplace_back(uivec3{ v_id, vt_id, vn_id });
                }

                if (face.size() > 3) {
                    for (unsigned i = 1; i < face.size() - 1; i++) {
                        vector<uivec3> sub_face;
                        sub_face.push_back(face[0]);
                        sub_face.push_back(face[i]);
                        sub_face.push_back(face[i + 1]);
                        f.push_back(move(sub_face));
                    }
                }
                else {
                    f.push_back(move(face));
                }
            }
        }
        ifile.close();

		// move object's center of bounding box to (0, 0, 0)
		// if v has w value in .obj file, below codes are not right
		float xmid = (xmin + xmax) / 2;
		float ymid = (ymin + ymax) / 2;
		float zmid = (zmin + zmax) / 2;
		for (auto& p : v) {
			p[0] -= xmid;
			p[1] -= ymid;
			p[2] -= zmid;
		}

        /*
        ** set default values for origin, axis_u, axis_v, axis_w
        */
        origin = vec3{ 0, 0, 0 };
        axis_u = vec3{ 1, 0, 0 };
        axis_v = vec3{ 0, 1, 0 };
        axis_w = vec3{ 0, 0, 1 };

        /*
        ** clockwise should be checked screen space not here
        */
        /*
        MakeFaceCounterClockWise();
        */
    }

    /*
    ** copy constructor
    */
    Obj(const Obj& rhs):
		v(rhs.v),
		vv(rhs.vv),
		vt(rhs.vt),
		vn(rhs.vn),
		f(rhs.f),
		origin(rhs.origin),
		axis_u(rhs.axis_u),
		axis_v(rhs.axis_v),
		axis_w(rhs.axis_w),
		xmin(rhs.xmin),
		xmax(rhs.xmax),
		ymin(rhs.ymin),
		ymax(rhs.ymax),
		zmin(rhs.zmin),
		zmax(rhs.zmax)
	{
    }

    /*
    ** set origin and direction of this object
    */
    void SetLocation(vec3 o, vec3 u, vec3 v, vec3 w) {
        origin = o;
        axis_u = u;
        axis_v = v;
        axis_w = w;
    }

    private:
    void MakeFaceCounterClockWise() {
        for (auto& face : f) {
            for (const auto& it : face) {
                ExitAsssert(it[2] == face[0][2], "Normal vectors in a face does not match");
                vec4 v0_1{ v[face[1][0] - 1] - v[face[0][0] - 1] };
                vec4 v1_2{ v[face[2][0] - 1] - v[face[1][0] - 1] };
                vec3 edge0{ v0_1[0], v0_1[1], v0_1[2] };
                vec3 edge1{ v1_2[0], v1_2[1], v1_2[2] };
                vec3 n{ edge0.Cross(edge1) };
                if (n.Dot(vn[face[0][2] - 1]) >= 0) {
                }
                else {
                    vector<uivec3> face_cpy{ face };
                    for (unsigned i = 0; i < face.size(); i++)
                        face[i] = face_cpy[face.size() - 1 - i];
                }
            }
        }
    }

};
