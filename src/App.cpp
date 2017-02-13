#include "App.h"

#include "GLFunctions.h"
#include "GLUtils.h"

// a bit hackish, these are defined at the bottom
static bool _print_key_pressed(bool = false);
static void _key_callback(GLFWwindow*, int, int, int, int);

//--------------------

App::App(int argc, char* argv[])
:	window_                (1440, 900, "supersymmetry"),
	cube_                  (Mesh::cube()),
	torus_                 (Mesh::torus(2.0f, 0.7f, 32, 32)),
	time_                  ( (glfwSetTime(0), glfwGetTime()) )
{
	// Set the key callback function for this window.
	glfwSetKeyCallback(window_, _key_callback);

	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	// A framebuffer allows rendering into a texture instead of on screen.
	image_ = GL::Texture::empty_2D(width, height);
	depth_ = GL::Texture::empty_2D_depth(width, height);
	framebuffer_ = GL::FBO::simple_C0D(image_, depth_);

	// Enable depth testing.
	glEnable(GL_DEPTH_TEST);

	// Subdivide our plane a couple of times.
	for (int i = 0; i < 5; ++i)
		plane_.subdivide();
}

void App::loop(void)
{
	while (!glfwWindowShouldClose(window_))
	{
		// Get current time for use in the shaders.
		time_ = glfwGetTime();

		// Get screen size in pixels, adjust image_ as necessary.
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		if (width != image_.width_ || height != image_.height_)
		{
			image_       = GL::Texture::empty_2D(width, height);
			depth_       = GL::Texture::empty_2D_depth(width, height);
			framebuffer_ = GL::FBO::simple_C0D(image_, depth_);
		}

		// Clear the screen and the framebuffer. Dark red is the new black.
		glClearColor(0.15, 0.1, 0.1, 1);
		GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0, 0.0, 0.0, 1);
		GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, framebuffer_);

		// Render the pinwheel into a texture.
		render_extruded_mesh(cube_, width, height, framebuffer_);

		// Then render the texture on a cube.
		render_on_mesh(image_, cube_, width, height);

		// Show the result on screen.
		glfwSwapBuffers(window_);

		// Poll events.
		glfwPollEvents();

		// Screenshot?
		if (_print_key_pressed())
			screenshot(width, height);
	}
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

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform2i  (screen_size_uniform, width, height);
	glUniform2fv (screen_center_uniform, 1, plane_.screen_center().data());
	glUniform1f  (pixels_per_unit_uniform, plane_.pixels_per_unit());
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
	glUniform1i(texture_sampler_uniform, 1);
	glUniform1i(texture_flag_uniform, GL_FALSE);

	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, image_);

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

void App::screenshot(int width, int height)
{
	auto texture = GL::Texture::empty_2D(width, height);
	auto depth   = GL::Texture::empty_2D_depth(width, height);
	auto fbo     = GL::FBO::simple_C0D(texture, depth);

	// Change me too!
	render_pinwheel(width, height, fbo);

	GL::tex_to_png(texture, "screenshot.png");
}

bool _print_key_pressed(bool set_pressed)
{
	static bool was_pressed = false;

	bool return_value = was_pressed;
	was_pressed = set_pressed;

	return return_value;
}

void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		_print_key_pressed(true);
}

//--------------------
