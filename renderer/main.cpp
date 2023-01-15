#include <ctime>
#include <iostream>

#include "./core/macro.h"
#include "./core/tgaimage.h"
#include "./core/model.h"
#include "./core/camera.h"
#include "./core/pipeline.h"
#include "./core/sample.h"
#include "./core/scene.h"
#include "./platform/win32.h"
#include "./shader/shader.h"

using namespace std;

// 定义世界坐标系下相机初始坐标，相机向上坐标（固定值），目标摆放坐标(镜头指向的地点)
const vec3 Eye(0, 10, -20);
const vec3 Up(0, 1, 0);
const vec3 Target(0, 10, 0);

// 导入场景（包含模型和shader）
const scene_t Scenes[]
{
	// {"helmet",build_helmet_scene},
	{"mebius",build_mebius_scene},
	{"triangle",build_triangle_scene},
};

int scenes_number = sizeof(Scenes) / sizeof(scene_t);


void clear_zbuffer(int width, int height, float* zbuffer);
void clear_framebuffer(int width, int height, unsigned char* framebuffer);
void update_matrix(Camera &camera, mat4 view_mat, mat4 projection_mat, IShader *shader_model, IShader *shader_skybox);

int main()
{
	// initialization
	
	// --------------
	// malloc memory for zbuffer and framebuffer
	int width = WINDOW_WIDTH, height = WINDOW_HEIGHT;
	float *zbuffer				= (float *)malloc(sizeof(float) * width * height);
	unsigned char* framebuffer  = (unsigned char *)malloc(sizeof(unsigned char) * width * height * 4);
	memset(framebuffer, 0, sizeof(unsigned char) * width * height * 4);

	// create camera
	Camera camera(Eye, Target, Up, (float)(width) / height);
	// 'p' for prespective, 'o' for orthographic 
	char projection_type = 'p';
	// set mvp matrix
	int fov = 60;
	mat4 model_mat			= mat4::identity();// 模型部分这里不移动，取I
	mat4 view_mat			= mat4_lookat(camera.eye, camera.target, camera.up); //将世界坐标系转化到相机坐标系，相机处于(0,0,0)，看向-z
	mat4 projection_mat	= mat4_perspective(fov, (float)(width)/height, -0.1, -10000);//透视变换
	

	// initialize models and shaders by building a scene
	srand((unsigned int)time(NULL));
	int scene_index = 0 %scenes_number;//rand() % x;
	int prev_scene_index = scene_index;
	int model_num = 0;
	char scene_info[100] = "";

	Model	*model[MAX_MODEL_NUM];
	IShader *shader_model;
	IShader *shader_skybox;
	Scenes[scene_index].build_scene(model, model_num, shader_model, shader_skybox, projection_mat, &camera,scene_info);
	// printf(scene_info);
	// printf("model_num:%i\n",model_num);

	// initialize window
	window_init(width, height, "demo");

	// initialize shader infos
	int ka_ratio = 50;
	int kd_ratio = 50;
	int ks_ratio = 50;

	// render loop
	// -----------
	int num_frames = 0;
	float print_time = platform_get_time();
	while (!window->is_close)
	{
		if(prev_scene_index != scene_index){
			prev_scene_index = scene_index;

			for (int i = 0; i < model_num; i++)
			if (model[i] != NULL)  delete model[i];
			if (shader_model != NULL)  delete shader_model;
			if (shader_skybox != NULL) delete shader_skybox;

			Model	*model[MAX_MODEL_NUM];
			IShader *shader_model;
			IShader *shader_skybox;
			Scenes[scene_index].build_scene(model, model_num, shader_model, shader_skybox, projection_mat, &camera,scene_info);
			// printf("model_num:%i\n",model_num);
			// printf(scene_info);

		}

	
		
		float curr_time = platform_get_time();

		// clear buffer
		clear_framebuffer(width, height, framebuffer);
		clear_zbuffer(width, height, zbuffer);

		// handle events and update view, perspective matrix
		handle_events(camera,fov,projection_type);
		// update fov and projection mode
		if(projection_type=='o'){
	 		projection_mat = mat4_ortho(-10,10,-10,10, -0.1, -100);
			mat4 zero = mat4::zero();
			
			projection_mat = zero - projection_mat; // in homo-clipping, we need projection_mat[3][3] be negative (otherwise I need to write a ortho-clipping case)
			
		}
		else{
			projection_mat	= mat4_perspective(fov, (float)(width)/height, -0.1, -10000);
		}
		
		update_matrix(camera, view_mat, projection_mat, shader_model, shader_skybox);

		// handle events for shader modifying
		if (window->keys['7'])
		{
			ka_ratio+=2;
			ka_ratio = (ka_ratio%200);
			
		}
		if (window->keys['8'])
		{
			kd_ratio+=1;
			kd_ratio = (kd_ratio%100);
			
		}
		if (window->keys['9'])
		{
			ks_ratio+=2;
			ks_ratio = (ks_ratio%200);
			
		}
		shader_model->payload.ka_ratio = ka_ratio;
		shader_model->payload.kd_ratio = kd_ratio;
		shader_model->payload.ks_ratio = ks_ratio;

		// handle events for scene switching
		if (window->keys['1']&&window->keys[VK_CONTROL])
		{
			scene_index = 0;
			scene_index = scene_index%scenes_number;
			
		}
		if (window->keys['2']&&window->keys[VK_CONTROL])
		{
			scene_index = 1;
			scene_index = scene_index%scenes_number;
			
		}
		if (window->keys['3']&&window->keys[VK_CONTROL])
		{
			scene_index = 2;
			scene_index = scene_index%scenes_number;
			
		}



		// draw models
		for (int m = 0; m < model_num; m++)
		{
			// assign model data to shader
			shader_model->payload.model = model[m];
			if(shader_skybox != NULL) shader_skybox->payload.model = model[m];
			
			// select current shader according model type
			IShader *shader;
			if (model[m]->is_skybox)
				shader = shader_skybox;
			else
				shader = shader_model;

			for (int i = 0; i < model[m]->nfaces(); i++)
			{
				draw_triangles(framebuffer, zbuffer, *shader, i);
			}
		}

		// calculate and display FPS
		num_frames += 1;
        if (curr_time - print_time >= 1) {
            int sum_millis = (int)((curr_time - print_time) * 1000);
            int avg_millis = sum_millis / num_frames;
            //printf("fps: %3d, avg: %3d ms\n", num_frames, avg_millis);
			system("cls");
			printf(scene_info);
			printf("camera pos: (%f,%f,%f)\n",camera.eye.x(),camera.eye.y(),camera.eye.z());
			printf("target pos: (%f,%f,%f)\n",camera.target.x(),camera.target.y(),camera.target.z());
			printf("fov: %i\n",fov);
			// printf("x (%f,%f,%f)",camera.x.x(),camera.x.y(),camera.x.z());
			// printf("y (%f,%f,%f)",camera.y.x(),camera.y.y(),camera.y.z());
			// printf("z (%f,%f,%f)\n",camera.z.x(),camera.z.y(),camera.z.z());
			
            num_frames = 0;
            print_time = curr_time;
        }

		// reset mouse information
		window->mouse_info.wheel_delta = 0;
		window->mouse_info.orbit_delta = vec2(0,0);
		window->mouse_info.fv_delta = vec2(0, 0);

		// send framebuffer to window 
		window_draw(framebuffer);
		msg_dispatch();
	}


	// free memory
	for (int i = 0; i < model_num; i++)
		if (model[i] != NULL)  delete model[i];
	if (shader_model != NULL)  delete shader_model;
	if (shader_skybox != NULL) delete shader_skybox;
	
	free(zbuffer);
	free(framebuffer);
	window_destroy();

	system("pause");
	return 0;
}


void clear_zbuffer(int width, int height, float* zbuffer)
{
	for (int i = 0; i < width*height; i++)
		zbuffer[i] = 100000;//infinite far away
}

void clear_framebuffer(int width, int height, unsigned char* framebuffer)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int index = (i * width + j) * 4;

			framebuffer[index + 2] = 122;//B
			framebuffer[index + 1] = 0;//G
			framebuffer[index] = 60;//R
		}
	}
}

void update_matrix(Camera &camera, mat4 view_mat, mat4 projection_mat, IShader *shader_model, IShader *shader_skybox)
{
	view_mat = mat4_lookat(camera.eye, camera.target, camera.up);
	mat4 mvp = projection_mat * view_mat;
	shader_model->payload.camera_view_matrix = view_mat;
	shader_model->payload.mvp_matrix = mvp;

	if (shader_skybox != NULL)
	{
		mat4 view_skybox = view_mat;
		view_skybox[0][3] = 0;
		view_skybox[1][3] = 0;
		view_skybox[2][3] = 0;
		shader_skybox->payload.camera_view_matrix = view_skybox;
		shader_skybox->payload.mvp_matrix = projection_mat * view_skybox;
	}
}