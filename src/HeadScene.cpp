#include "HeadScene.h"

#include "GLUtils.h"
#include "GLFunctions.h"

// TEST
#include <iostream>

//--------------------

HeadScene::HeadScene(HeadScene&& other)
:	head_					(std::move(other.head_)),
	head_diffuse_			(std::move(other.head_diffuse_)),
	head_normal_			(std::move(other.head_normal_)),
	head_env_map_			(std::move(other.head_env_map_)),
	head_shader_			(std::move(other.head_shader_)),

	head_uniforms_			(std::move(other.head_uniforms_)),

	dir_light_buffer_		(std::move(other.dir_light_buffer_)),
	dir_light_texture_		(std::move(other.dir_light_texture_)),

	point_light_buffer_		(std::move(other.point_light_buffer_)),
	point_light_texture_	(std::move(other.point_light_texture_))
{}

HeadScene& HeadScene::operator=(HeadScene&& other) {
	if (this != &other) {
		head_					= std::move(other.head_);
		head_diffuse_			= std::move(other.head_diffuse_);
		head_normal_			= std::move(other.head_normal_);
		head_env_map_			= std::move(other.head_env_map_);
		head_shader_			= std::move(other.head_shader_);

		head_uniforms_			= std::move(other.head_uniforms_);

		dir_light_buffer_		= std::move(other.dir_light_buffer_);
		dir_light_texture_		= std::move(other.dir_light_texture_);

		point_light_buffer_		= std::move(other.point_light_buffer_);
		point_light_texture_	= std::move(other.point_light_texture_);
	}

	return *this;
}

void HeadScene::render(double time, bool normals_from_texture,
                       int width, int height, GLuint framebuffer)
{
	// Perspective.
	Eigen::Matrix4f P = GLUtils::perspective(width, height, PI / 2, 0.01f);

	// View.
#define ENV
#if defined(RING)
	Eigen::Vector3f target = Eigen::Vector3f(0.0, 0.0, 0.0);
	Eigen::Vector3f eye = target + Eigen::Vector3f(0.2 * std::sin(0.8 * time), 0.0, 0.2 * std::cos(0.8*time));
	Eigen::Vector3f up = Eigen::Vector3f(0, 1, 0);
	Eigen::Matrix4f V = GLUtils::look_at(eye, target, up);

	set_light_config(V);
	render_head_ring(16, V, P, normals_from_texture, false, width, height, framebuffer);
#elif defined(WALL)
	Eigen::Vector3f target = Eigen::Vector3f(0.3 * std::sin(0.8 * time), 0.3 * std::cos(0.8 * time), -1);
	Eigen::Vector3f eye = Eigen::Vector3f(0.0, 0.0, 0.0);
	Eigen::Vector3f up = Eigen::Vector3f(0, 1, 0);
	Eigen::Matrix4f V = GLUtils::look_at(eye, target, up);

	set_light_config(V);
	render_head_wall(1.5f, 1.0f, 5, 3, V, P, normals_from_texture, false, width, height, framebuffer);
#elif defined(ENV)
	Eigen::Vector3f target = Eigen::Vector3f(0.0, 0.0, -0.075);
	Eigen::Vector3f eye = target + Eigen::Vector3f(0.25 * std::sin(0.8 * time), 0.0, 0.25 * std::cos(0.8 * time));
	Eigen::Vector3f up = Eigen::Vector3f(0, 1, 0);
	Eigen::Matrix4f V = GLUtils::look_at(eye, target, up);

	for (static char c = 0; !c++;) // Do only once.
		create_env_map_ring(16, target, 0.02, 1024);

	set_light_config(V);
	render_head_ring(16, V, P, normals_from_texture, false, width, height, framebuffer);
	render_head(Eigen::Matrix4f::Identity(), V, P, normals_from_texture, true, width, height, framebuffer);
#else
	Eigen::Vector3f target = Eigen::Vector3f(0.0, 0.0, -0.075);
	Eigen::Vector3f eye = target + Eigen::Vector3f(0.3 * std::sin(0.8 * time), 0.0, 0.3 * std::cos(0.8 * time));
	Eigen::Vector3f up = Eigen::Vector3f(0, 1, 0);
	Eigen::Matrix4f V = GLUtils::look_at(eye, target, up);

	set_light_config(V);
	render_head(Eigen::Matrix4f::Identity(), V, P, normals_from_texture, false, width, height, framebuffer);
#endif
}

void HeadScene::render_head(const Eigen::Matrix4f& M,
                            const Eigen::Matrix4f& V,
                            const Eigen::Matrix4f& P,
                            bool normals_from_texture, bool env_mapped,
                            int width, int height, GLuint framebuffer)
{
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Model to view for vertices & normals.
	Eigen::Matrix4f model_to_view = V * M;
	Eigen::Matrix3f normal_model_to_view = model_to_view.topLeftCorner<3, 3>().inverse().transpose();

	// Model to clip.
	Eigen::Matrix4f model_to_clip = P * model_to_view;

	// View to world.
	Eigen::Matrix4f view_to_world = V.inverse();

	// Uniforms.
	glUseProgram(head_shader_);
	glUniformMatrix4fv(head_uniforms_.model_to_clip, 1, GL_FALSE, model_to_clip.data());
	glUniformMatrix4fv(head_uniforms_.model_to_view, 1, GL_FALSE, model_to_view.data());
	glUniform1i(head_uniforms_.normals_from_texture, normals_from_texture);
	glUniform1i(head_uniforms_.use_env, env_mapped);
	glUniformMatrix3fv(head_uniforms_.normal_model_to_view, 1, GL_FALSE, normal_model_to_view.data());
	glUniformMatrix4fv(head_uniforms_.view_to_world, 1, GL_FALSE, view_to_world.data());

	// Draw!
	glBindVertexArray(head_.vao_);
	glDrawArrays(head_.primitive_type_, 0, head_.num_vertices_);

	// Clean up.
	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void HeadScene::set_light_config(const Eigen::Matrix4f& V) {
	// Directional lights.
	const Eigen::Vector3f dir_world[] = {
		Eigen::Vector3f(0.0, 1.0, -0.2 ),	// Direction.
		Eigen::Vector3f(0.2, 0.2,  0.15),	// Color.

		Eigen::Vector3f(0.0, 1.0, 0.2),
		Eigen::Vector3f(0.2, 0.2, 0.15)
	};
	const size_t dir_len = sizeof(dir_world) / sizeof(dir_world[0]);

	// Point lights.
	const Eigen::Vector3f point_world[] = {
		Eigen::Vector3f(0.2, 0.1,  0.1),	// Position.
		Eigen::Vector3f(1.0, 1.0,  0.8),	// Color.
		Eigen::Vector3f(0.0, 2.0, 20.0),	// Attenuation.

		Eigen::Vector3f(-0.2, 0.1, -0.1),
		Eigen::Vector3f( 0.5, 0.5,  0.4),
		Eigen::Vector3f( 0.0, 2.0,  5.0),

		Eigen::Vector3f(0.3, 0.3, -0.7),
		Eigen::Vector3f(1.0, 1.0,  0.8),
		Eigen::Vector3f(0.0, 2.0, 15.0),

		Eigen::Vector3f(-0.3, -0.3, -0.7),
		Eigen::Vector3f( 0.5,  0.5,  0.4),
		Eigen::Vector3f( 0.0,  2.0,  5.0)
	};

	const size_t point_len = sizeof(point_world) / sizeof(point_world[0]);

	static Eigen::Vector3f dir_view[dir_len];
	static Eigen::Vector3f point_view[point_len];
	static bool init = [&]() { // TODO: Profile this.
		return
			memcpy(dir_view, dir_world, sizeof(dir_world)),
			memcpy(point_view, point_world, sizeof(point_world)),
			true;
	}();
	(void)init;

	Eigen::Matrix3f dir_world_to_view = V.topLeftCorner<3, 3>();

	for (size_t i = 0; i < dir_len; i += 2)
		dir_view[i] = dir_world_to_view * dir_world[i];

	for (size_t i = 0; i < point_len; i += 3) {
		auto v4 = (Eigen::Vector4f() << point_world[i], 1).finished();
		point_view[i] = (V * v4).head<3>();
	}

	glBindBuffer(GL_ARRAY_BUFFER, dir_light_buffer_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(dir_view), dir_view, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, point_light_buffer_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point_view), point_view, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	dir_light_texture_ = GL::Texture::buffer_texture(dir_light_buffer_, GL_RGB32F);
	point_light_texture_ = GL::Texture::buffer_texture(point_light_buffer_, GL_RGB32F);

	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_BUFFER, dir_light_texture_);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_BUFFER, point_light_texture_);
	glActiveTexture(old_active);
}

void HeadScene::create_env_map_ring(int n,
                                    const Eigen::Vector3f& position,
                                    float cube_side, int resolution)
{
	head_env_map_ = GL::Texture::empty_cube(resolution);
	GL::Texture cube_depth = GL::Texture::empty_2D_depth(resolution, resolution);
	GL::FBO framebuffer;

	// Perspective.
	Eigen::Matrix4f P = GLUtils::perspective(resolution, resolution, PI / 2, cube_side / 2);

	const Eigen::Vector3f targets[] = {
		Eigen::Vector3f(1, 0, 0), Eigen::Vector3f(-1, 0, 0),
		Eigen::Vector3f(0, 1, 0), Eigen::Vector3f(0, -1, 0),
		Eigen::Vector3f(0, 0, 1), Eigen::Vector3f(0, 0, -1)
	};

	// This convention makes no sense whatsoever.
	const Eigen::Vector3f ups[] = {
		Eigen::Vector3f(0, -1, 0), Eigen::Vector3f(0, -1, 0),
		Eigen::Vector3f(0, 0, 1), Eigen::Vector3f(0, 0, -1),
		Eigen::Vector3f(0, -1, 0), Eigen::Vector3f(0, -1, 0)
	};

	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cube_depth, 0);

	for (int i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, head_env_map_, 0);

		glViewport(0, 0, resolution, resolution);
		GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, framebuffer);

		Eigen::Vector3f target = position + targets[i];
		Eigen::Vector3f up = ups[i];
		Eigen::Matrix4f V = GLUtils::look_at(position, target, up);

		set_light_config(V);
		render_head_ring(n, V, P, true, false, resolution, resolution, framebuffer);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, head_env_map_);
	glActiveTexture(old_active);
}

void HeadScene::render_head_ring(int n,
                                 const Eigen::Matrix4f& V,
                                 const Eigen::Matrix4f& P,
                                 bool normals_from_texture, bool env_mapped,
                                 int width, int height, GLuint framebuffer)
{
	for (int i = 0; i < n; ++i) {
		Eigen::Matrix4f M = Eigen::Matrix4f::Identity();
		float rot = i * 2 * PI / n;
		M.topLeftCorner<3, 3>() = Eigen::AngleAxisf(rot, Eigen::Vector3f(0, 1, 0)).toRotationMatrix();
		M.topRightCorner<3, 1>() = M.topLeftCorner<3, 3>() * Eigen::Vector3f(0, 0, -0.4);

		render_head(M, V, P, normals_from_texture, env_mapped, width, height, framebuffer);
	}
}

void HeadScene::render_head_wall(float x_len, float y_len, int x_num, int y_num,
                                 const Eigen::Matrix4f& V,
                                 const Eigen::Matrix4f& P,
                                 bool normals_from_texture, bool env_mapped,
                                 int width, int height, GLuint framebuffer)
{
	Eigen::Matrix3f rots[2];
	rots[0] = Eigen::AngleAxisf(PI / 4, Eigen::Vector3f(0, 1, 0)).toRotationMatrix();
	rots[1] = Eigen::AngleAxisf(-PI / 4, Eigen::Vector3f(0, 1, 0)).toRotationMatrix();

	int index = 0;
	for (int i = 0; i < 5; ++i) {
		index = (index + 1) % 2;
		for (int j = 0; j < 5; ++j) {
			float x_pos = -x_len / 2 + j * x_len / x_num;
			float y_pos = -y_len / 2 + i * y_len / y_num;

			auto pos = Eigen::Vector3f(x_pos, y_pos, -1.2);
			Eigen::Matrix4f M = (Eigen::Matrix4f() << rots[index], pos, 0, 0, 0, 1).finished();

			render_head(M, V, P, normals_from_texture, env_mapped, width, height, framebuffer);
		}
	}
}
