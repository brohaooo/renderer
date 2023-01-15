#include "./pipeline.h"
#include <cstring>



static int is_back_facing(vec3 ndc_pos[3])
{
	vec3 a = ndc_pos[0];
	vec3 b = ndc_pos[1];
	vec3 c = ndc_pos[2];
	float signed_area = a.x() * b.y() - a.y() * b.x() +
		b.x() * c.y() - b.y() * c.x() +
		c.x() * a.y() - c.y() * a.x();   //|AB AC|
	return signed_area <= 0;
}

static vec3 compute_barycentric2D(float x, float y, const vec3* v) 
{
	float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
	float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
	return vec3(c1, c2, 1 - c1 - c2);
}

static void set_color(unsigned char* framebuffer, int x, int y, unsigned char color[])
{
	int i;
	int index = ((WINDOW_HEIGHT - y - 1) * WINDOW_WIDTH + x) * 4; // the origin for pixel is bottom-left, but the framebuffer index counts from top-left

	for (i = 0; i < 3; i++)
		framebuffer[index + i] = color[i];
}

static int is_inside_triangle(float alpha, float beta, float gamma)
{
	int flag = 0;
	// here epsilon is to alleviate precision bug
	if (alpha > -EPSILON && beta > -EPSILON && gamma > -EPSILON)
		flag = 1;

	return flag;
}

static int get_index(int x, int y)
{
	return (WINDOW_HEIGHT - y - 1) * WINDOW_WIDTH + x;
}


typedef enum 
{
	W_PLANE,
	X_RIGHT,
	X_LEFT,
	Y_TOP,
	Y_BOTTOM,
	Z_NEAR,
	Z_FAR
} clip_plane;

// in my implementation, all the w is negative, so here is a little different from openGL
static int is_inside_plane(clip_plane c_plane,vec4 vertex)
{
	switch (c_plane)
	{
		case W_PLANE:
			return vertex.w() <= -EPSILON;
		case X_RIGHT:
			return vertex.x() >= vertex.w();
		case X_LEFT:
			return vertex.x() <= -vertex.w();
		case Y_TOP:
			return vertex.y() >= vertex.w();
		case Y_BOTTOM:
			return vertex.y() <= -vertex.w();
		case Z_NEAR:
			return vertex.z() >= vertex.w();
		case Z_FAR:
			return vertex.z() <= -vertex.w();
		default:
			return 0;
	}
}

// for the deduction of intersection ratio
// refer to: https://fabiensanglard.net/polygon_codec/clippingdocument/Clipping.pdf
static float get_intersect_ratio(vec4 prev, vec4 curv,clip_plane c_plane)
{
	switch (c_plane) 
	{
		case W_PLANE:
			return (prev.w() + EPSILON) / (prev.w() - curv.w());
		case X_RIGHT:
			return (prev.w() - prev.x()) / ((prev.w() - prev.x()) - (curv.w() - curv.x()));
		case X_LEFT:
			return (prev.w() + prev.x()) / ((prev.w() + prev.x()) - (curv.w() + curv.x()));
		case Y_TOP:
			return (prev.w() - prev.y()) / ((prev.w() - prev.y()) - (curv.w() - curv.y()));
		case Y_BOTTOM:
			return (prev.w() + prev.y()) / ((prev.w() + prev.y()) - (curv.w() + curv.y()));
		case Z_NEAR:
			return (prev.w() - prev.z()) / ((prev.w() - prev.z()) - (curv.w() - curv.z()));
		case Z_FAR:
			return (prev.w() + prev.z()) / ((prev.w() + prev.z()) - (curv.w() + curv.z()));
		default:
			return 0;
	}
}

static int clip_with_plane(clip_plane c_plane, int num_vert, payload_t &payload)
{
	int out_vert_num = 0;
	int previous_index, current_index;
	// int is_odd = (c_plane + 1) % 2;
	
	// set the right in and out datas
	vec4 (&homo_clipping_clipcoord)[MAX_VERTEX]	 = payload.homo_clipping_clipcoord;
	vec3 (&homo_clipping_worldcoord)[MAX_VERTEX]  = payload.homo_clipping_worldcoord;
	vec3 (&homo_clipping_normal)[MAX_VERTEX]		 = payload.homo_clipping_normal;
	vec2 (&homo_clipping_uv)[MAX_VERTEX]			 = payload.homo_clipping_uv;
	vec4 (&tmp_clipcoord)[MAX_VERTEX]  = payload.tmp_clipcoord;
	vec3 (&tmp_worldcoord)[MAX_VERTEX] = payload.tmp_worldcoord;
	vec3 (&tmp_normal)[MAX_VERTEX]	 = payload.tmp_normal;
	vec2 (&tmp_uv)[MAX_VERTEX]		 = payload.tmp_uv;

	// tranverse all the edges from first vertex
	for (int i = 0; i < num_vert; i++)
	{
		current_index   = i;
		previous_index  = (i - 1 + num_vert) % num_vert;
		vec4 cur_vertex = homo_clipping_clipcoord[current_index];
		vec4 pre_vertex = homo_clipping_clipcoord[previous_index];

		int is_cur_inside = is_inside_plane(c_plane, cur_vertex);
		// printf("!!!!! %f %f %f %f\n",cur_vertex[0],cur_vertex[1],cur_vertex[2],cur_vertex[3]);
		// printf("z:%f",(cur_vertex[2]/cur_vertex[3]));
		// if(is_cur_inside){
		// 	printf("inside");
		// }
		int is_pre_inside = is_inside_plane(c_plane, pre_vertex);
		if (is_cur_inside != is_pre_inside)
		{
			float ratio = get_intersect_ratio(pre_vertex,cur_vertex,c_plane);

			tmp_clipcoord[out_vert_num]  = vec4_lerp(pre_vertex,cur_vertex,ratio);
			tmp_worldcoord[out_vert_num] = vec3_lerp(homo_clipping_worldcoord[previous_index],homo_clipping_worldcoord[current_index],ratio);
			tmp_normal[out_vert_num]     = vec3_lerp(homo_clipping_normal[previous_index],homo_clipping_normal[current_index],ratio);
			tmp_uv[out_vert_num]         = vec2_lerp(homo_clipping_uv[previous_index],homo_clipping_uv[current_index],ratio);

			out_vert_num++;
		}

		if (is_cur_inside)
		{
			tmp_clipcoord[out_vert_num]  = cur_vertex;
			tmp_worldcoord[out_vert_num] = homo_clipping_worldcoord[current_index];
			tmp_normal[out_vert_num]     = homo_clipping_normal[current_index];
			tmp_uv[out_vert_num]         = homo_clipping_uv[current_index];

			out_vert_num++;
		}
	}

	// printf("asdfsdaf:%i",sizeof(tmp_clipcoord));
	memcpy(homo_clipping_clipcoord,tmp_clipcoord,sizeof(tmp_clipcoord));
	memcpy(homo_clipping_worldcoord,tmp_worldcoord,sizeof(tmp_worldcoord));
	memcpy(homo_clipping_normal,tmp_normal,sizeof(tmp_normal));
	memcpy(homo_clipping_uv,tmp_uv,sizeof(tmp_normal));


	return out_vert_num;
}

static int homo_clipping(payload_t &payload)
{
	int num_vertex = 3;
	num_vertex = clip_with_plane(W_PLANE, num_vertex, payload);
	num_vertex = clip_with_plane(X_RIGHT, num_vertex, payload);
	num_vertex = clip_with_plane(X_LEFT, num_vertex, payload);
	num_vertex = clip_with_plane(Y_TOP, num_vertex, payload);
	num_vertex = clip_with_plane(Y_BOTTOM, num_vertex, payload);
	num_vertex = clip_with_plane(Z_NEAR, num_vertex, payload);
	num_vertex = clip_with_plane(Z_FAR, num_vertex, payload);
	return num_vertex;
}

static void transform_attri(payload_t &payload,int index0,int index1,int index2)
{
	payload.clipcoord_attri[0]	= payload.homo_clipping_clipcoord[index0];
	payload.clipcoord_attri[1]	= payload.homo_clipping_clipcoord[index1];
	payload.clipcoord_attri[2]	= payload.homo_clipping_clipcoord[index2];
	payload.worldcoord_attri[0] = payload.homo_clipping_worldcoord[index0];
	payload.worldcoord_attri[1] = payload.homo_clipping_worldcoord[index1];
	payload.worldcoord_attri[2] = payload.homo_clipping_worldcoord[index2];
	payload.normal_attri[0]		= payload.homo_clipping_normal[index0];
	payload.normal_attri[1]		= payload.homo_clipping_normal[index1];
	payload.normal_attri[2]		= payload.homo_clipping_normal[index2];
	payload.uv_attri[0]			= payload.homo_clipping_uv[index0];
	payload.uv_attri[1]			= payload.homo_clipping_uv[index1];
	payload.uv_attri[2]			= payload.homo_clipping_uv[index2];

	// payload.clipcoord_attri[0][2] = -0;
	// payload.clipcoord_attri[1][2] = -0;
	// payload.clipcoord_attri[2][2] = -0;


}
//drawline
//void drawline(int x0, int y0, int x1, int y1, unsigned char* framebuffer)
//{
//	int dx = x1 - x0;
//	int dy = y1 - y0;
//
//	int dx1 = fabs(dx);
//	int dy1 = fabs(dy);
//	int px = 2 * dy1;
//	int py = 2 * dx1;
//
//	//����
//	if (dx1 > dy1)
//	{
//		if (x0 > x1)
//		{
//			int tempX = x0, tempY = y0;
//			x0 = x1; y0 = y1;
//			x1 = tempX, y1 = tempY;
//		}
//
//		int y = y0;
//		for (int x = x0; x <= x1; x++)
//		{
//			//image.set(x, y, white);
//			set_color(framebuffer, x, y, white1);
//
//			//��������˰����ǰ��һ��������ȷ�ģ��ú�˼��
//			if (px > dx1)
//			{
//				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
//				{
//					y += 1;
//				}
//				else
//				{
//					y -= 1;
//				}
//				px += dy1 * 2; // +k
//				px -= 2 * dx1; // +1
//			}
//			else
//			{
//				px += dy1 * 2;// +k
//			}
//		}
//
//	}
//	//��
//	else
//	{
//		if (y0 > y1)
//		{
//			int tempX = x0, tempY = y0;
//			x0 = x1; y0 = y1;
//			x1 = tempX, y1 = tempY;
//		}
//
//		int x = x0;
//		for (int y = y0; y <= y1; y++)
//		{
//			//image.set(x, y, white);
//
//			set_color(framebuffer, x, y, white1);
//			//��������˰����ǰ��һ��������ȷ�ģ��ú�˼��
//			if (py > dy1)
//			{
//				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
//				{
//					x += 1;
//				}
//				else
//				{
//					x -= 1;
//				}
//				py += dx1 * 2;
//				py -= 2 * dy1;
//			}
//			else
//			{
//				py += dx1 * 2;
//			}
//		}
//	}
//}



void rasterize(vec4 *clipcoord_attri, unsigned char *framebuffer, float *zbuffer, IShader &shader)
{
	vec3 ndc_pos[3];
	vec3 screen_pos[3];
	unsigned char c[3];
	int width  = window->width;
	int height = window->height;
	int is_skybox = shader.payload.model->is_skybox;

	// homogeneous division
	for (int i = 0; i < 3; i++)
	{
		ndc_pos[i][0] = clipcoord_attri[i][0] / clipcoord_attri[i].w();
		ndc_pos[i][1] = clipcoord_attri[i][1] / clipcoord_attri[i].w();
		ndc_pos[i][2] = clipcoord_attri[i][2] / clipcoord_attri[i].w();
	}

	// viewport transformation
	for (int i = 0; i < 3; i++)
	{
		screen_pos[i][0] = 0.5*(width-1)*(ndc_pos[i][0] + 1.0);
		screen_pos[i][1] = 0.5*(height-1)*(ndc_pos[i][1] + 1.0);
		screen_pos[i][2] = is_skybox ? 1000:-ndc_pos[i][2];	//view space z-value
		// screen_pos[i][2] = is_skybox ? 1000:-clipcoord_attri[i].w(); //view space z-value in SRender, it doesn't work when using ortho-clipping
	}

	// backface clip (skybox didnit need it)
	if (!is_skybox)
	{
		if (is_back_facing(ndc_pos))
			return;
	}

	// get bounding box
	float xmin = 10000, xmax = -10000, ymin = 10000, ymax = -10000;
	for (int i = 0; i < 3; i++) 
	{
		xmin = float_min(xmin, screen_pos[i][0]);
		xmax = float_max(xmax, screen_pos[i][0]);
		ymin = float_min(ymin, screen_pos[i][1]);
		ymax = float_max(ymax, screen_pos[i][1]);
	}

	// rasterization
	for (int x = (int)xmin; x <= (int)xmax; x++)
	{
		for (int y = (int)ymin; y <= (int)ymax; y++)
		{
			vec3 barycentric = compute_barycentric2D((float)(x + 0.5), (float)(y + 0.5), screen_pos);
			float alpha = barycentric.x(); float beta = barycentric.y(); float gamma = barycentric.z();

			if (is_inside_triangle(alpha, beta, gamma))
			{
				int index = get_index(x, y);
				//interpolation correct term
				float normalizer = 1.0 / (alpha / clipcoord_attri[0].w() + beta / clipcoord_attri[1].w() + gamma / clipcoord_attri[2].w());
				//for larger z means away from camera, needs to interpolate z-value as a property			
				float z = (alpha * screen_pos[0].z() / clipcoord_attri[0].w() + beta * screen_pos[1].z() / clipcoord_attri[1].w() +
					gamma * screen_pos[2].z() / clipcoord_attri[2].w()) * normalizer;


				if (zbuffer[index] > z)
				{
					zbuffer[index] = z;
					vec3 color = shader.fragment_shader(alpha, beta, gamma);

					//clamp color value
					for (int i = 0; i < 3; i++)
					{
						c[i] = (int)float_clamp(color[i], 0, 255);
					}
					set_color(framebuffer, x, y, c);
				}
			}
		}
	}
}


void draw_triangles(unsigned char *framebuffer, float *zbuffer, IShader &shader, int nface)
{
	// vertex shader
	for (int i = 0; i < 3; i++)
	{
		shader.vertex_shader(nface, i);
	}

	// homogeneous clipping
	// 边缘情况下会把一个三角形切割成五边形（三个三角形），四边形（两个三角形）或者一个小三角形
	int num_vertex = homo_clipping(shader.payload);
	// int num_vertex = 3;

	// if(num_vertex>3){
	// 	printf("!!! %i",num_vertex);
	// }
	// printf("!!! %i",num_vertex);

	// triangle assembly and reaterize
	for (int i = 0; i < num_vertex - 2; i++) {
		int index0 = 0;
		int index1 = i + 1;
		int index2 = i + 2;
		// transform data to real vertex attri
		transform_attri(shader.payload, index0, index1, index2);

		rasterize(shader.payload.clipcoord_attri, framebuffer,zbuffer,shader);
	}
}
