#pragma once
#include "./macro.h"
#include "./maths.h"
#include "../shader/shader.h"
#include "../platform/win32.h"

const int WINDOW_HEIGHT = 800;
const int WINDOW_WIDTH = 800;

//rasterize triangle
void rasterize(vec4 *clipcoord_attri, unsigned char* framebuffer, float *zbuffer, IShader& shader);
void draw_triangles(unsigned char* framebuffer, float *zbuffer,IShader& shader,int nface);