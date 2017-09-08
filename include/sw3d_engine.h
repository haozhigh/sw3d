#pragma once


#include <vector>
using namespace std;

#include "types.h"
#include "obj.h"

class Sw3dEngine {
private:
    mat4 m_view;
    mat4 m_project;
    mat4 m_screen;



public:
    Sw3dEngine() {

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

};
