#include "./camera.h"

#include "../platform/win32.h"

Camera::Camera(vec3 e, vec3 t, vec3 up, float aspect):
	eye(e),target(t),up(up),aspect(aspect),eye_reset(e),target_reset(t)
{}

Camera::~Camera()
{}



void handle_mouse_events(Camera& camera)
{
	// 左键是button0，右键是button1
	if (window->buttons[0])
	{
		vec2 cur_pos = get_mouse_pos();
		window->mouse_info.orbit_delta = window->mouse_info.orbit_pos - cur_pos;
		window->mouse_info.orbit_pos = cur_pos;
	}

	if (window->buttons[1])
	{
		vec2 cur_pos = get_mouse_pos();
		window->mouse_info.fv_delta = window->mouse_info.fv_pos - cur_pos;
		window->mouse_info.fv_pos = cur_pos;
	}

	// 根据存储的鼠标信息挪动镜头
	vec3 from_target = camera.eye - camera.target;			// vector point from target to camera's position
	float radius = from_target.norm();

	float phi     = (float)atan2(from_target[0], from_target[2]); // azimuth angle, angle between from_target and z-axis, [-pi, pi]
	float theta   = (float)acos(from_target[1] / radius);		  // zenith angle, angle between from_target and y-axis, [0, pi]
	float x_delta = window->mouse_info.orbit_delta[0] / window->width;
	float y_delta = window->mouse_info.orbit_delta[1] / window->height;

	// for mouse wheel
	radius *= (float)pow(0.95, window->mouse_info.wheel_delta);

	float factor = 1.5 * PI;
	// for mouse left button
	phi	  += x_delta * factor;
	theta += y_delta * factor;
	if (theta > PI) theta = PI - EPSILON * 100;
	if (theta < 0)  theta = EPSILON * 100;

	// 普通视角模式
	if(camera.mode == 'a'){
		camera.eye[0] = camera.target[0] + radius * sin(phi) * sin(theta);
		camera.eye[1] = camera.target[1] + radius * cos(theta);
		camera.eye[2] = camera.target[2] + radius * sin(theta) * cos(phi);
	}
	// 第一人称相机模式
	else{
		camera.target[0] = camera.eye[0] - radius * sin(phi) * sin(theta);
		camera.target[1] = camera.eye[1] - radius * cos(theta);
		camera.target[2] = camera.eye[2] - radius * sin(theta) * cos(phi);
	}
	

	// for mouse right button
	factor  = radius * (float)tan(60.0 / 360 * PI) * 2.2;
	x_delta = window->mouse_info.fv_delta[0] / window->width;
	y_delta = window->mouse_info.fv_delta[1] / window->height;
	vec3 left = x_delta * factor * camera.x;
	vec3 up   = y_delta * factor * camera.y;

	camera.eye += (left - up);
	camera.target += (left - up);
}

void handle_key_events(Camera& camera,int & fov,char & projection_type)
{
	float distance = (camera.target - camera.eye).norm();

	if (window->keys['W'])
	{
		// camera.eye += -10.0 / window->width * camera.z*distance;
		camera.eye += -0.05f*camera.z;
		camera.target += -0.05f*camera.z;
	}
	if (window->keys['S'])
	{
		// camera.eye += 0.05f*camera.z;
		camera.eye += 0.05f*camera.z;
		camera.target += 0.05f*camera.z;
	}
	if (window->keys['Q'])
	{
		camera.eye += 0.05f*camera.y;
		camera.target += 0.05f*camera.y;
	}
	if (window->keys['E'])
	{
		camera.eye += -0.05f*camera.y;
		camera.target += -0.05f*camera.y;
	}
	if (window->keys['A'])
	{
		camera.eye += -0.05f*camera.x;
		camera.target += -0.05f*camera.x;
	}
	if (window->keys['D'])
	{
		camera.eye += 0.05f*camera.x;
		camera.target += 0.05f*camera.x;
	}
	if (window->keys[VK_ESCAPE])
	{
		window->is_close = 1;
	}
	// reset
	if (window->keys['R'])
	{
		camera.eye = camera.eye_reset;
		camera.target = camera.target_reset;
	}
	// 切换视角模式
	if (window->keys['1'])
	{	
		camera.mode = 'a';
	}
	if (window->keys['2'])
	{	
		camera.mode = 'b';
	}

	if (window->keys['4'])
	{	
		if(fov <= 120)fov += 1;
	}
	if (window->keys['3'])
	{	
		if(fov >= 30)fov -= 1;
	}
	if (window->keys['5'])
	{	
		projection_type = 'p';
	}
	if (window->keys['6'])
	{	
		projection_type = 'o';
	}
}
 
void handle_events(Camera& camera,int & fov,char & projection_type)
{
	//calculate camera axis
	// 相机坐标系下的z指向看向方向（相机前方），x指向相机右方（由z与世界坐标的up方向叉乘得出），y指向相机上方（根据右手坐标系zx轴叉乘得出）
	camera.z = unit_vector(camera.eye - camera.target); 
	camera.x = unit_vector(cross(camera.up, camera.z));
	camera.y = unit_vector(cross(camera.z, camera.x));

	//mouse and keyboard events
	handle_mouse_events(camera);
	handle_key_events(camera,fov,projection_type);
}