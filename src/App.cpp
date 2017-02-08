#include "App.h"

#include "GLFunctions.h"
#include "GLUtils.h"

//--------------------

App::App(int argc, char* argv[])
:	window_                (1440, 900, "supersymmetry"),
	cube_                  (Mesh::cube()),
	torus_                 (Mesh::torus(2.0f, 0.7f, 12, 12)),
	mesh_shader_           (GL::ShaderProgram::simple()),               // For rasterized rendering.
	wave_shader_           (GL::ShaderProgram(                          // For entirely fragment-shader-based rendering.
		                        GL::ShaderObject::vertex_passthrough(), // This doesn't transform the vertices at all.
		                        GL::ShaderObject::from_file(GL_FRAGMENT_SHADER, "shaders/wave_frag.glsl"))),
	time_                  ( (glfwSetTime(0), glfwGetTime()) )
{
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	// Not used at the moment.
	// This allows rendering into a texture instead of on screen.
	image_ = GL::Texture::empty_2D(width, height);
	framebuffer_ = GL::FBO::simple_C0(image_);

	// This is not strictly necessary if only rendering the wave.
	// With the meshes, try to comment it out and see what happens.
	glEnable(GL_DEPTH_TEST);
}

void App::loop(void)
{
	while (!glfwWindowShouldClose(window_))
	{
		// Get current time for use in the shaders.
		time_ = glfwGetTime();

		// Get screen size in pixels.
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		// Clear the screen. Dark red is the new black.
		glClearColor(0.15, 0.1, 0.1, 1);
		GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render a wave?
		// render_wave(width, height);

		// Render a torus?
		render_mesh(torus_, width, height);

		// Render a cube?
		// render_mesh(cube_, width, height);

		// Show the result on screen.
		glfwSwapBuffers(window_);

		// Check if it's time to exit.
		glfwPollEvents();
	}
}

void App::render_wave(int width, int height, GLuint framebuffer)
{
	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Get uniform locations.
	GLuint screen_size_uniform;
	GLuint time_uniform;

	screen_size_uniform = glGetUniformLocation(wave_shader_, "uScreenSize");
	time_uniform        = glGetUniformLocation(wave_shader_, "uTime");

	// Set the shader program and uniforms, and draw.
	glUseProgram(wave_shader_);

	glUniform2i(screen_size_uniform, width, height);
	glUniform1f(time_uniform, time_);

	glBindVertexArray(canvas_.vao_);
	glDrawArrays(canvas_.primitive_type_, 0, canvas_.num_vertices_);

	// Clean up.
	glBindVertexArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_texture(const GL::Texture& texture, int width, int height, GLuint framebuffer)
{
	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Get uniform locations.
	GLuint texture_flag_uniform, texture_sampler_uniform;
	
	texture_flag_uniform = glGetUniformLocation(mesh_shader_, "uTextureFlag");
	texture_sampler_uniform = glGetUniformLocation(mesh_shader_, "uTextureSampler");

	// Set the shader program, uniforms and textures, and draw.
	glUseProgram(mesh_shader_);

	glUniform1i(texture_sampler_uniform, 1);

	glUniform1i(texture_flag_uniform, GL_TRUE);
	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(canvas_.vao_);
	glDrawArrays(canvas_.primitive_type_, 0, canvas_.num_vertices_);

	// Clean up.
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glUniform1i(texture_flag_uniform, GL_FALSE);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_mesh(const Mesh& mesh, int width, int height, GLuint framebuffer)
{
	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Camera.
	Eigen::Vector3f eye = Eigen::Vector3f(4 * std::sin(time_), 2, 4*std::cos(time_));
	Eigen::Matrix4f view = GLUtils::look_at(eye);
	Eigen::Matrix4f projection = GLUtils::perspective(width, height, PI / 2);

	// We'll assume that the mesh is already in world space.
	Eigen::Matrix4f model_to_clip = projection * view;
	Eigen::Matrix3f normal_to_world = Eigen::Matrix3f::Identity();

	// Get uniform locations from OpenGL.
	GLuint model_to_clip_uniform, normal_to_world_uniform;
	
	model_to_clip_uniform = glGetUniformLocation(mesh_shader_, "uModelToClip");
	normal_to_world_uniform = glGetUniformLocation(mesh_shader_, "uNormalToWorld");

	// Set the shader program and uniforms, and draw.
	glUseProgram(mesh_shader_);

	glUniformMatrix4fv(model_to_clip_uniform, 1, GL_FALSE, model_to_clip.data());
	glUniformMatrix3fv(normal_to_world_uniform, 1, GL_FALSE, normal_to_world.data());

	glBindVertexArray(mesh.vao_);
	glDrawArrays(mesh.primitive_type_, 0, mesh.num_vertices_);
	glBindVertexArray(0);

	// Clean up.
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_on_mesh(const GL::Texture& texture, const Mesh& mesh, int width, int height, GLuint framebuffer)
{
	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Camera.
	Eigen::Vector3f eye					= Eigen::Vector3f(4 * std::sin(time_), 2, 4 * std::cos(time_));
	Eigen::Matrix4f view				= GLUtils::look_at(eye);
	Eigen::Matrix4f projection			= GLUtils::perspective(width, height, PI / 2);

	// We'll assume that the mesh is already in world space.
	Eigen::Matrix4f model_to_clip		= projection * view;
	Eigen::Matrix3f normal_to_world		= Eigen::Matrix3f::Identity();

	// Get the uniform locations from OpenGL.
	GLuint model_to_clip_uniform, normal_to_world_uniform;
	GLuint texture_flag_uniform, texture_sampler_uniform;

	model_to_clip_uniform				= glGetUniformLocation(mesh_shader_, "uModelToClip");
	normal_to_world_uniform				= glGetUniformLocation(mesh_shader_, "uNormalToWorld");
	texture_flag_uniform				= glGetUniformLocation(mesh_shader_, "uTextureFlag");
	texture_sampler_uniform				= glGetUniformLocation(mesh_shader_, "uTextureSampler");

	// Set the shader program, uniforms and textures, and draw.
	glUseProgram(mesh_shader_);

	glUniformMatrix4fv(model_to_clip_uniform, 1, GL_FALSE, model_to_clip.data());
	glUniformMatrix3fv(normal_to_world_uniform, 1, GL_FALSE, normal_to_world.data());
	glUniform1i(texture_sampler_uniform, 1);

	glUniform1i(texture_flag_uniform, GL_TRUE);
	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(mesh.vao_);
	glDrawArrays(mesh.primitive_type_, 0, mesh.num_vertices_);

	// Clean up.
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glUniform1i(texture_flag_uniform, GL_FALSE);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

//--------------------
