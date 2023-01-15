#pragma once
#include "./maths.h"

class Camera
{
public:
	Camera(vec3 e, vec3 t, vec3 up, float aspect);
	~Camera();

	vec3 eye;//相机的世界坐标
	vec3 target;//相机看向地点的世界坐标（不代表被观测物体在这里）
	vec3 up;//固定不变的，（0，1，0），是对于相机参考的 ‘上’ 方向
	// 相机坐标系下的z指向看向方向（相机前方），x指向相机右方（由z与世界坐标的up方向叉乘得出），y指向相机上方（根据右手坐标系zx轴叉乘得出）
	vec3 x;
	vec3 y;
	vec3 z;
	float aspect;//宽高比
	// 用来重置镜头
	const vec3 eye_reset;//相机初始世界坐标
	const vec3 target_reset;//相机初始看向地点的世界坐标
	// 相机模式：“a”默认渲染展示模式，“b”第一人称
	char mode = 'a';
};
 
//处理相机移动、fov调整、透视模式切换
void handle_events(Camera& camera,int & fov,char & projection_type);