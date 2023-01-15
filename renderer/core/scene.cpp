#include "./scene.h"



void build_mebius_scene(Model *model[], int &m, IShader *&shader_use, IShader *&shader_skybox, mat4 perspective, Camera *camera, char (&scene_info)[100])
{
	m = 1;
	const char* modelname[] = {
		"../obj/mebius/mebius.obj"
	};

	int vertex = 0, face = 0;
	const char* scene_name = "mebius";
	PhongShader *shader_phong = new PhongShader();

	for (int i = 0; i < m; i++)
	{
		model[i] = new Model(modelname[i], 0, 1);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	shader_phong->payload.camera_perp_matrix = perspective;
	shader_phong->payload.camera = camera;

	shader_use = shader_phong;
	shader_skybox = NULL;

	// printf("scene name:%s\n", scene_name);
	// printf("model number:%d\n", m);
	// printf("vertex:%d faces:%d\n", vertex, face);
	sprintf(scene_info,"scene name:%s\nmodel number:%d\nvertex:%d faces:%d\n", scene_name,m,vertex,face);
}

void build_triangle_scene(Model *model[], int &m, IShader *&shader_use, IShader *&shader_skybox, mat4 perspective, Camera *camera, char (&scene_info)[100])
{
	m = 1;
	const char* modelname[] = {
		"../obj/triangle/triangle.obj"
	};

	int vertex = 0, face = 0;
	const char* scene_name = "triangle";
	PhongShader *shader_phong = new PhongShader();

	for (int i = 0; i < m; i++)
	{
		model[i] = new Model(modelname[i], 0, 1);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	shader_phong->payload.camera_perp_matrix = perspective;
	shader_phong->payload.camera = camera;

	shader_use = shader_phong;
	shader_skybox = NULL;

	// printf("scene name:%s\n", scene_name);
	// printf("model number:%d\n", m);
	// printf("vertex:%d faces:%d\n", vertex, face);
	sprintf(scene_info,"scene name:%s\nmodel number:%d\nvertex:%d faces:%d\n", scene_name,m,vertex,face);

}



