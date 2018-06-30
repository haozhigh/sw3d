
### 1 Fixed Function in Graphic Pipeline
#### 1.1 Rasterization

Addition formulas:
<div align="center"><img src="./latex/addition_formulas.png"></div>

2D rotate matrix:
<div align="center"><img src="./latex/2d_rotate.png"></div>

2D matrix to rotate a vector 90 degress counter-clockwise:
<div align="center"><img src="./latex/2d_rotate_90.png"></div>

To judge vector v1 is pointing to the right side of vector v0:
<div align="center"><img src="./latex/judge_vector_right_side.png"></div>

In OpenGL, default visiable triangles are counter-clockwise, thus left side of three edges form the triangle aera.

#### 1.2 Coordinate System Transform

Describe (u,v,w) space axis in (x,y,z) space:
<div align="center"><img src="./latex/coordinate_system_transform_1.png"></div>

**P** converts a vector from (u,v,w) space to (x,y,z) space; Inv(**P**) converts a vector from (x,y,z) space to (u,v,w) space:
<div align="center"><img src="./latex/coordinate_system_transform_2.png"></div>

For points transformation:
<div align="center"><img src="./latex/coordinate_system_transform_3.png"></div>

#### 1.3 Tangent Space

Triangle ABC in (x,y,z) space and (t,b,n) space:
<div align="center"><img src="./latex/tangent_space_1.png"></div>

For 2 edges of the triangle:
<div align="center"><img src="./latex/tangent_space_2.png"></div>

Then:
<div align="center"><img src="./latex/tangent_space_3.png"></div>

#### 1.4 Perspective Projection

In perspective projection, r,n,f[α, β] defines a frustum:

+ f: z of far plane
+ n: z of near plane
+ r: ratio of width to height of view wnidow
+ d: z of view window
+ α: view angle in the y axis direction
+ β: view angle in the x axis direction

In the y axis direction:
<div align="center"><img src="./geogebra/project_y.png"></div>
<div align="center"><img src="./latex/project_y.png"></div>

And in the x axis direction:
<div align="center"><img src="./geogebra/project_x.png"></div>
<div align="center"><img src="./latex/project_x.png"></div>

Put together:
<div align="center"><img src="./latex/project.png"></div>

To make A+B/z 0 at near plane and -1 at far plane:
<div align="center"><img src="./latex/project_coe.png"></div>

### 2 Linear Algebra
#### 2.1 Cross Product

Cross product definition differs in right-hand coordinate system and left-hand coordinate system ensuring that:
<div align="center"><img src="./latex/cross_product_1.png"></div>

From this, it can be inferred that:
<div align="center"><img src="./latex/cross_product_2.png"></div>