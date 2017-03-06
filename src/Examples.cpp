#include "Examples.h"

#include <GLFW/glfw3.h>
#include "GLObjects.h"
#include "GLUtils.h"
#include "ShaderCanvas.h"
#include "PinwheelPlane.h"

//--------------------

namespace Examples
{

void render_wave(int width, int height, GLuint framebuffer)
{
	double time = glfwGetTime();
	static ShaderCanvas canvas;
	static auto shader = GL::ShaderProgram(
		GL::ShaderObject::vertex_passthrough(),
		GL::ShaderObject::from_file(GL_FRAGMENT_SHADER, "shaders/wave_frag.glsl"));

	// Find uniform locations once.
	static GLuint screen_size_uniform;
	static GLuint time_uniform;
	static bool init = [&](){
		screen_size_uniform     = glGetUniformLocation(shader, "uScreenSize");
		time_uniform            = glGetUniformLocation(shader, "uTime");
		return true;
	}();
	(void)init; // Suppress unused variable warning.

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform2i(screen_size_uniform, width, height);
	glUniform1f(time_uniform, time);

	glBindVertexArray(canvas.vao_);
	glDrawArrays(canvas.primitive_type_, 0, canvas.num_vertices_);

	// Clean up.
	glBindVertexArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void render_pinwheel(const Eigen::Vector2f& screen_center, double pixels_per_unit,
                     int width, int height, GLuint framebuffer)
{
	double time = glfwGetTime();
	static PinwheelPlane plane;

	static auto shader = GL::ShaderProgram::from_files(
		"shaders/pinwheel_vert.glsl",
		"shaders/pinwheel_frag.glsl");

	// Find uniform locations once.
	static GLuint screen_size_uniform;
	static GLuint screen_center_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint time_uniform;
	static bool init = [&](){
		screen_size_uniform     = glGetUniformLocation(shader, "uScreenSize");
		screen_center_uniform   = glGetUniformLocation(shader, "uScreenCenter");
		pixels_per_unit_uniform = glGetUniformLocation(shader, "uPixelsPerUnit");
		time_uniform            = glGetUniformLocation(shader, "uTime");

		// Subdivide our plane a couple of times.
		for (int i = 0; i < 2; ++i)
			plane.subdivide();

		return true;
	}();
	(void)init; // Suppress unused variable warning.

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform2i  (screen_size_uniform, width, height);
	glUniform2fv (screen_center_uniform, 1, screen_center.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit);
	glUniform1f  (time_uniform, time);

	const auto& mesh = plane.mesh();
	glBindVertexArray(mesh.vao_);
	glDrawArrays(mesh.primitive_type_, 0, mesh.num_vertices_);

	// Clean up.
	glBindVertexArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void render_texture(const GL::Texture& texture, int width, int height, GLuint framebuffer)
{
	static ShaderCanvas canvas;
	static auto shader = GL::ShaderProgram::simple();

	// Find uniform locations once.
	static GLuint texture_flag_uniform;
	static GLuint texture_sampler_uniform;
	static bool init = [&](){
		texture_flag_uniform    = glGetUniformLocation(shader, "uTextureFlag");
		texture_sampler_uniform = glGetUniformLocation(shader, "uTextureSampler");
		return true;
	}();
	(void)init; // Suppress unused variable warning.
	
	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, texture);

	glViewport(0, 0, width, height);

	// Set the shader program, uniforms and textures, and draw.
	glUseProgram(shader);

	glUniform1i(texture_sampler_uniform, 1);
	glUniform1i(texture_flag_uniform, GL_TRUE);

	glBindVertexArray(canvas.vao_);
	glDrawArrays(canvas.primitive_type_, 0, canvas.num_vertices_);

	// Clean up.
	glBindVertexArray(0);

	glUniform1i(texture_flag_uniform, GL_FALSE);

	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void render_mesh(const Mesh& mesh, int width, int height, GLuint framebuffer)
{
	double time = glfwGetTime();
	static auto shader = GL::ShaderProgram::simple();

	// Find uniform locations once.
	static GLuint model_to_clip_uniform;
	static GLuint normal_to_world_uniform;
	static GLuint time_uniform;
	static bool init = [&](){
		model_to_clip_uniform   = glGetUniformLocation(shader, "uModelToClip");
		normal_to_world_uniform = glGetUniformLocation(shader, "uNormalToWorld");
		time_uniform            = glGetUniformLocation(shader, "uTime");
		return true;
	}();
	(void)init; // Suppress unused variable warning.

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Camera.
	Eigen::Vector3f eye        = Eigen::Vector3f(4 * std::sin(time), 2, 4*std::cos(time));
	Eigen::Matrix4f view       = GLUtils::look_at(eye);
	Eigen::Matrix4f projection = GLUtils::perspective(width, height, PI / 2);

	// We'll assume that the mesh is already in world space.
	Eigen::Matrix4f model_to_clip   = projection * view;
	Eigen::Matrix3f normal_to_world = Eigen::Matrix3f::Identity();

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniformMatrix4fv (model_to_clip_uniform, 1, GL_FALSE, model_to_clip.data());
	glUniformMatrix3fv (normal_to_world_uniform, 1, GL_FALSE, normal_to_world.data());
	glUniform1f        (time_uniform, time);

	glBindVertexArray(mesh.vao_);
	glDrawArrays(mesh.primitive_type_, 0, mesh.num_vertices_);
	glBindVertexArray(0);

	// Clean up.
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void render_on_mesh(const GL::Texture& texture, const Mesh& mesh, int width, int height, GLuint framebuffer)
{
	double time = glfwGetTime();
	static auto shader = GL::ShaderProgram::simple();

	// Find uniform locations once.
	static GLuint model_to_clip_uniform;
	static GLuint normal_to_world_uniform;
	static GLuint texture_flag_uniform;
	static GLuint texture_sampler_uniform;
	static bool init = [&](){
		model_to_clip_uniform	= glGetUniformLocation(shader, "uModelToClip");
		normal_to_world_uniform	= glGetUniformLocation(shader, "uNormalToWorld");
		texture_flag_uniform	= glGetUniformLocation(shader, "uTextureFlag");
		texture_sampler_uniform	= glGetUniformLocation(shader, "uTextureSampler");
		return true;
	}();
	(void)init; // Suppress unused variable warning.

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, texture);


	glViewport(0, 0, width, height);

	// Camera.
	Eigen::Vector3f eye        = Eigen::Vector3f(4 * std::sin(time), 2, 4 * std::cos(time));
	Eigen::Matrix4f view       = GLUtils::look_at(eye);
	Eigen::Matrix4f projection = GLUtils::perspective(width, height, PI / 2);

	// We'll assume that the mesh is already in world space.
	Eigen::Matrix4f model_to_clip   = projection * view;
	Eigen::Matrix3f normal_to_world = Eigen::Matrix3f::Identity();

	// Set the shader program, uniforms and textures, and draw.
	glUseProgram(shader);

	glUniformMatrix4fv (model_to_clip_uniform, 1, GL_FALSE, model_to_clip.data());
	glUniformMatrix3fv (normal_to_world_uniform, 1, GL_FALSE, normal_to_world.data());
	glUniform1i        (texture_sampler_uniform, 1);
	glUniform1i        (texture_flag_uniform, GL_TRUE);

	glBindVertexArray(mesh.vao_);
	glDrawArrays(mesh.primitive_type_, 0, mesh.num_vertices_);

	// Clean up.
	glBindVertexArray(0);

	glUniform1i(texture_flag_uniform, GL_FALSE);

	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

} // namespace Examples
