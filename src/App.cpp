#include "App.h"

#include "GLFunctions.h"
#include "GLUtils.h"
#include <cstdio>

//--------------------

App::App(int argc, char* argv[])
:	window_                (1440, 900, "supersymmetry"),
	input_manager_         (window_),
	cube_                  (Mesh::cube()),
	torus_                 (Mesh::torus(2.0f, 0.7f, 32, 32)),
	pixels_per_unit_       (8192.0),                           // Initial zoom level.
	time_                  ( (glfwSetTime(0), glfwGetTime()) )
{
	// Screenshot callback.
	window_.add_key_callback(GLFW_KEY_P, &App::print_screen, this);

	// Enable depth testing.
	glEnable(GL_DEPTH_TEST);

	// Subdivide our plane a couple of times.
	for (int i = 0; i < 8; ++i)
		plane_.subdivide();
}

void App::loop(void)
{
	while (!glfwWindowShouldClose(window_))
	{
		// Clear the screen. Dark grey is the new black.
		glClearColor(0.1, 0.1, 0.1, 1);
		GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get current time for use in the shaders.
		time_ = glfwGetTime();

		// Poll events and deal with user input.
		glfwWaitEvents();
		update_objects();

		// Render the pinwheel.
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);
		render_pinwheel(width, height);

		// Show the result on screen.
		glfwSwapBuffers(window_);
	}
}

// TODO: Reimplement using callbacks and std::bind.
void App::update_objects(void)
{
	static struct {
		Eigen::Vector2f press_position = {0.0, 0.0};
		Eigen::Vector2f plane_position = {0.0, 0.0};
		bool left_depressed  = false;
		bool right_depressed = false;
	} state;

	if (input_manager_.left_button_pressed())
	{
		if (!state.left_depressed)
		{
			state.left_depressed = true;
			state.press_position = input_manager_.mouse_position();
			state.plane_position = plane_.position();
		}

		const auto& drag_position = input_manager_.mouse_position() - state.press_position;;
		plane_.set_position(state.plane_position + scale_to_world(drag_position));
	}
	else
		state.left_depressed = false;
}

Eigen::Vector2f App::scale_to_world(const Eigen::Vector2f& v)
{
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	return { v.x() * width / pixels_per_unit_,
	         v.y() * height / pixels_per_unit_ };
}

void App::render_extruded_mesh(const Mesh& mesh, int width, int height, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/simple_vert.glsl",
		"shaders/explode_geom.glsl",
		"shaders/simple_frag.glsl");

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

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniformMatrix4fv (model_to_clip_uniform, 1, GL_FALSE, model_to_clip.data());
	glUniformMatrix3fv (normal_to_world_uniform, 1, GL_FALSE, normal_to_world.data());
	glUniform1f        (time_uniform, time_);

	glBindVertexArray(mesh.vao_);
	glDrawArrays(mesh.primitive_type_, 0, mesh.num_vertices_);
	glBindVertexArray(0);

	// Clean up.
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_pinwheel(int width, int height, GLuint framebuffer)
{
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
		return true;
	}();

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Get the data required by the shader.
	const Eigen::Vector2f& screen_center = -plane_.position();

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform2i  (screen_size_uniform, width, height);
	glUniform2fv (screen_center_uniform, 1, screen_center.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform1f  (time_uniform, time_);

	const auto& mesh = plane_.mesh();
	glBindVertexArray(mesh.vao_);
	glDrawArrays(mesh.primitive_type_, 0, mesh.num_vertices_);

	// Clean up.
	glBindVertexArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_wave(int width, int height, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram(
		GL::ShaderObject::vertex_passthrough(),
		GL::ShaderObject::from_file(GL_FRAGMENT_SHADER, "shaders/wave_frag.glsl"));

	// Find uniform locations once.
	static GLuint screen_size_uniform;
	static GLuint time_uniform;
	static GLuint texture_flag_uniform;
	static GLuint texture_sampler_uniform;
	static bool init = [&](){
		screen_size_uniform     = glGetUniformLocation(shader, "uScreenSize");
		time_uniform            = glGetUniformLocation(shader, "uTime");
		texture_flag_uniform    = glGetUniformLocation(shader, "uTextureFlag");
		texture_sampler_uniform = glGetUniformLocation(shader, "uTextureSampler");
		return true;
	}();

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

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
	static auto shader = GL::ShaderProgram::simple();

	// Find uniform locations once.
	static GLuint texture_flag_uniform;
	static GLuint texture_sampler_uniform;
	static bool init = [&](){
		texture_flag_uniform    = glGetUniformLocation(shader, "uTextureFlag");
		texture_sampler_uniform = glGetUniformLocation(shader, "uTextureSampler");
		return true;
	}();
	
	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Set the shader program, uniforms and textures, and draw.
	glUseProgram(shader);

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

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Camera.
	Eigen::Vector3f eye        = Eigen::Vector3f(4 * std::sin(time_), 2, 4*std::cos(time_));
	Eigen::Matrix4f view       = GLUtils::look_at(eye);
	Eigen::Matrix4f projection = GLUtils::perspective(width, height, PI / 2);

	// We'll assume that the mesh is already in world space.
	Eigen::Matrix4f model_to_clip   = projection * view;
	Eigen::Matrix3f normal_to_world = Eigen::Matrix3f::Identity();

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniformMatrix4fv (model_to_clip_uniform, 1, GL_FALSE, model_to_clip.data());
	glUniformMatrix3fv (normal_to_world_uniform, 1, GL_FALSE, normal_to_world.data());
	glUniform1f        (time_uniform, time_);

	glBindVertexArray(mesh.vao_);
	glDrawArrays(mesh.primitive_type_, 0, mesh.num_vertices_);
	glBindVertexArray(0);

	// Clean up.
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_on_mesh(const GL::Texture& texture, const Mesh& mesh, int width, int height, GLuint framebuffer)
{
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

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Camera.
	Eigen::Vector3f eye        = Eigen::Vector3f(4 * std::sin(time_), 2, 4 * std::cos(time_));
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

void App::print_screen(int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		// TODO: Threading.
		auto texture = GL::Texture::empty_2D(width, height);
		auto depth   = GL::Texture::empty_2D_depth(width, height);
		auto fbo     = GL::FBO::simple_C0D(texture, depth);

		render_pinwheel(width, height, fbo);

		// TODO: Query screenshot name from user.
		GL::tex_to_png(texture, "screenshot.png");
		printf("Screenshot saved. (screenshot.png)\n");
	}
}

//--------------------
