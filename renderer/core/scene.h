#pragma once
#include "./tgaimage.h"
#include "../shader/shader.h"

typedef struct 
{
	const char *scene_name;
	void (*build_scene)(Model *model[], int &m, IShader *&shader_use, IShader *&shader_skybox, mat4 perspective, Camera *camera,char (&scene_info)[100]);
} scene_t;

// TGAImage *texture_from_file(const char *file_name);
// void load_ibl_map(payload_t &p, const char* env_path);


// void build_helmet_scene(Model *model[], int &m, IShader *&shader_use, IShader *&shader_skybox, mat4 perspective, Camera *camera);

void build_mebius_scene(Model *model[], int &m, IShader *&shader_use, IShader *&shader_skybox, mat4 perspective, Camera *camera,char (&scene_info)[100]);
void build_triangle_scene(Model *model[], int &m, IShader *&shader_use, IShader *&shader_skybox, mat4 perspective, Camera *camera,char (&scene_info)[100]);

