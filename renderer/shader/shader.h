#pragma once
#include "../core/macro.h"
#include "../core/maths.h"
#include "../core/model.h"
#include "../core/camera.h"

struct light
{
	vec3 pos;
	vec3 intensity;
};

typedef struct cubemap 
{
	TGAImage *faces[6];
}cubemap_t;

typedef struct iblmap 
{
	int mip_levels;
	cubemap_t *irradiance_map;
	cubemap_t *prefilter_maps[15];
	TGAImage *brdf_lut;
} iblmap_t;

typedef struct
{
	//light_matrix for shadow mapping, (to do)
	mat4 model_matrix;
	mat4 camera_view_matrix;
	mat4 light_view_matrix;
	mat4 camera_perp_matrix;
	mat4 light_perp_matrix;
	mat4 mvp_matrix;

	Camera *camera;
	Model *model;

	//vertex attribute
	vec3 normal_attri[3];
	vec2 uv_attri[3];
	vec3 worldcoord_attri[3];
	vec4 clipcoord_attri[3];

	//for homogeneous clipping
	vec3 homo_clipping_normal[MAX_VERTEX];
	vec2 homo_clipping_uv[MAX_VERTEX];
	vec3 homo_clipping_worldcoord[MAX_VERTEX];
	vec4 homo_clipping_clipcoord[MAX_VERTEX];
	vec3 tmp_normal[MAX_VERTEX];
	vec2 tmp_uv[MAX_VERTEX];
	vec3 tmp_worldcoord[MAX_VERTEX];
	vec4 tmp_clipcoord[MAX_VERTEX];

	//for image-based lighting
	iblmap_t *iblmap;

	// for fragment shader control:
	int ka_ratio;
	int kd_ratio;
	int ks_ratio;

}payload_t;

class IShader
{
public:
	payload_t payload;
	virtual void vertex_shader(int nfaces, int nvertex) {}
	virtual vec3 fragment_shader(float alpha, float beta, float gamma) { return vec3(0, 0, 0); }
};

class PhongShader:public IShader
{
public:
	void vertex_shader(int nfaces, int nvertex);
	vec3 fragment_shader(float alpha, float beta, float gamma);

};


class SkyboxShader :public IShader
{
public:
	void vertex_shader(int nfaces, int nvertex);
	vec3 fragment_shader(float alpha, float beta, float gamma);
};