#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

#include "types.h"

struct Obj {
    vector<vec4> v;     // geometric vertices
    vector<vec3> vt;    // txture vertices
    vector<vec3> vn;    // vertex normals

    vector<vector<uivec3>> f;    // faces

    vec3 origin;        // origin of this object in world coordinate
    vec3 axis_u;        // axis u of this object described in world axis x,y,z
    vec3 axis_v;        // axis v of this object described in world axis x,y,z
    vec3 axis_w;        // axis w of this object described in world axis x,y,z
    

    Obj(string obj_filepath) {
        ifstream ifile(obj_filepath);
        if (!ifile.is_open()) {
            cerr << "Failed to open obj file: " << obj_filepath << endl;
            exit(-1);
        }

        string line;
        while (ifile.good()) {
            getline(ifile, line);
            if (line.empty())
                continue;

            if (ioutil::Str::StartsWith(line, "v ")) {
                auto spans = ioutil::Str::Split(line, ' ');
                float x = stof(spans[1], nullptr);
                float y = stof(spans[2], nullptr);
                float z = stof(spans[3], nullptr);
                float w = spans.size() > 4 ? stof(spans[4], nullptr) : 1.0f;
                v.emplace_back(x, y, z, w);
            }
            else if (ioutil::Str::StartsWith(line, "vn ")) {
                auto spans = ioutil::Str::Split(line, ' ');
                float i = stof(spans[1], nullptr);
                float j = stof(spans[2], nullptr);
                float k = stof(spans[3], nullptr);
                vn.emplace_back(i, j, k);
            }
            else if (ioutil::Str::StartsWith(line, "vt ")) {
                auto spans = ioutil::Str::Split(line, ' ');
                float u = stof(spans[1], nullptr);
                float v = stof(spans[2], nullptr);
                float w = spans.size() > 3 ? stof(spans[3], nullptr) : 0.0f;
            }
            else if (ioutil::Str::StartsWith(line, "f ")) {
                auto spans = ioutil::Str::Split(line, ' ');
                vector<uivec3> face;
                for (unsigned i = 1; i < spans.size(); i++) {
                    auto spans_inner = ioutil::Str::Split(spans[i], '/');
                    /*
                    ** vt_id and vn_id is optional argument
                    ** so value of 0 stands for empty vt_id or vn_id
                    ** since in obj files ids starts from 1
                    */
                    unsigned v_id = stoi(spans_inner[0], nullptr, 10);
                    unsigned vt_id = spans_inner[1].size() > 0 ? stoi(spans_inner[1], nullptr, 10) : 0;
                    unsigned vn_id = spans_inner[2].size() > 0 ? stoi(spans_inner[2], nullptr, 10) : 0;
                    face.emplace_back(v_id, vt_id, vn_id);
                }
                f.emplace_back(move(face));
            }
        }

        ifile.close();
    }

    /*
    ** copy constructor
    */
    Obj(const Obj& rhs): v(rhs.v), vt(rhs.vt), vn(rhs.vn), f(rhs.f) {
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

    void Transform(const mat4& view, const mat4& project, const mat4 screen) {
        mat4 model{ axis_u[0], axis_v[0], axis_w[0], origin[0],
                    axis_u[1], axis_v[1], axis_w[1], origin[1],
                    axis_u[2], axis_v[2], axis_w[2], origin[2],
                    0,         0,         0,          1 };
        model = view * model;
        model = project * model;
        model = screen * model;

        for (auto& vertex : v)
            vertex = model * vertex;
    }
};