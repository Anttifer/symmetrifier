#ifndef HEADSCENE_H
#define HEADSCENE_H

#include "GLObjects.h"
#include "Mesh.h"

//--------------------

class HeadScene {
public:
	HeadScene(const HeadScene&) = delete;
	HeadScene(HeadScene&&);

	HeadScene& operator=(const HeadScene&) = delete;
	HeadScene& operator=(HeadScene&&);

	void render(double time, bool normals_from_texture,
	            int width, int height, GLuint framebuffer = 0);

	static HeadScene simple(void);
private:
	// Head.
	Mesh              head_;
	GL::Texture       head_diffuse_;
	GL::Texture       head_normal_;
	GL::Texture       head_env_map_;
	GL::ShaderProgram head_shader_;

	struct HeadUniforms {
		GLuint model_to_clip;
		GLuint model_to_view;
		GLuint normal_model_to_view;
		GLuint view_to_world;

		GLuint diffuse_sampler;
		GLuint normal_sampler;
		GLuint env_sampler;

		GLuint specular_color;
		GLuint specular_exponent;
		GLuint normals_from_texture;
		GLuint use_env;

		GLuint dir_light_sampler;
		GLuint point_light_sampler;
	} head_uniforms_;

	// Lights.
	GL::Buffer dir_light_buffer_;
	GL::Texture dir_light_texture_;

	GL::Buffer point_light_buffer_;
	GL::Texture point_light_texture_;
private:
	HeadScene(void){}
	void set_light_config(const Eigen::Matrix4f& V);
	void render_head(const Eigen::Matrix4f& model,
	                 const Eigen::Matrix4f& view,
	                 const Eigen::Matrix4f& projection,
	                 bool normals_from_texture, bool env_mapped,
	                 int width, int height, GLuint framebuffer = 0);

	void render_head_ring(int n,
	                      const Eigen::Matrix4f& view,
	                      const Eigen::Matrix4f& projection,
	                      bool normals_from_texture, bool env_mapped,
	                      int width, int height, GLuint framebuffer = 0);
	void render_head_wall(float x_len, float y_len, int x_num, int y_num,
	                      const Eigen::Matrix4f& V,
	                      const Eigen::Matrix4f& P,
	                      bool normals_from_texture, bool env_mapped,
	                      int width, int height, GLuint framebuffer = 0);
	
	void create_env_map_ring(int n,
	                         const Eigen::Vector3f& position,
	                         float cube_side, int resolution);
};
#endif // HEADSCENE_H
