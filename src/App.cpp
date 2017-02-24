#include "App.h"

#include "GLFunctions.h"
#include "GLUtils.h"
#include <cstdio>

// DEBUG
#include <iostream>

//--------------------

App::App(int argc, char* argv[])
:	window_                (1440, 900, "supersymmetry"),
	input_manager_         (window_),
	cube_                  (Mesh::cube()),
	torus_                 (Mesh::torus(2.0f, 0.7f, 32, 32)),
	symmetrifying_         (false),
	screen_center_         (0, 0),
	pixels_per_unit_       (60.0),                           // Initial zoom level.
	zoom_factor_           (1.2),
	time_                  ( (glfwSetTime(0), glfwGetTime()) )
{
	// Screenshot callback.
	window_.add_key_callback(GLFW_KEY_P, &App::print_screen, this);

	// Input test.
	window_.add_mouse_button_callback(GLFW_MOUSE_BUTTON_LEFT, &App::test_left_click_cb, this);
	window_.add_mouse_pos_callback(&App::test_update_objects_cb, this);
	window_.add_scroll_callback(&App::test_scroll_cb, this);

	window_.add_key_callback(GLFW_KEY_SPACE, [this](int,int action,int){
		if (action == GLFW_PRESS)
			this->symmetrifying_ ^= true;
	});

	glfwSwapInterval(0);

	// Enable depth testing and alpha blending.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Subdivide our plane a couple of times.
	for (int i = 0; i < 2; ++i)
		plane_.subdivide();

	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	debug_tex_   = GL::Texture::empty_2D(width, height);
	auto depth   = GL::Texture::empty_2D_depth(width, height);
	auto fbo     = GL::FBO::simple_C0D(debug_tex_, depth);
	glClearColor(0.1, 0.1, 0.1, 0.0);
	GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, fbo);

	render_pinwheel(width, height, fbo);
	pixels_per_unit_ = 1440.0;

	tiling_.set_symmetry_group("2*22");
	tiling_.symmetrify(debug_tex_);
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

		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		if (symmetrifying_)
		{
			tiling_.symmetrify(debug_tex_);
			render_symmetry_frame(true, width, height);
			// render_image(tiling_.symmetrified_, width, height);
		}
		else
		{
			render_image(debug_tex_, width, height);
			render_symmetry_frame(false, width, height);
		}

		// Show the result on screen.
		glfwSwapBuffers(window_);

		// Poll events.
		glfwWaitEvents();
	}
}

void App::render_image(const GL::Texture& image, int width, int height, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/image_vert.glsl",
		"shaders/image_frag.glsl");

	// Find uniform locations once.
	static GLuint aspect_ratio_uniform;
	static GLuint screen_size_uniform;
	static GLuint screen_center_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint texture_sampler_uniform;
	static bool init = [&](){
		aspect_ratio_uniform    = glGetUniformLocation(shader, "uAR");
		screen_size_uniform     = glGetUniformLocation(shader, "uScreenSize");
		screen_center_uniform   = glGetUniformLocation(shader, "uScreenCenter");
		pixels_per_unit_uniform = glGetUniformLocation(shader, "uPixelsPerUnit");
		texture_sampler_uniform = glGetUniformLocation(shader, "uTextureSampler");
		return true;
	}();

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	auto AR = image.width_ / (float)image.height_;

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform1f  (aspect_ratio_uniform, AR);
	glUniform2i  (screen_size_uniform, width, height);
	glUniform2fv (screen_center_uniform, 1, screen_center_.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform1i  (texture_sampler_uniform, 1);

	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, image);

	glBindVertexArray(canvas_.vao_);
	glDrawArrays(canvas_.primitive_type_, 0, canvas_.num_vertices_);

	// Clean up.
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_symmetry_frame(bool symmetrifying, int width, int height, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/tiling_vert.glsl",
		"shaders/tiling_geom.glsl",
		"shaders/tiling_frag.glsl");

	// Find uniform locations once.
	static GLuint position_uniform;
	static GLuint t1_uniform;
	static GLuint t2_uniform;
	static GLuint screen_size_uniform;
	static GLuint screen_center_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint texture_sampler_uniform;
	static GLuint texture_flag_uniform;
	static bool init = [&](){
		position_uniform        = glGetUniformLocation(shader, "uPos");
		t1_uniform              = glGetUniformLocation(shader, "uT1");
		t2_uniform              = glGetUniformLocation(shader, "uT2");
		screen_size_uniform     = glGetUniformLocation(shader, "uScreenSize");
		screen_center_uniform   = glGetUniformLocation(shader, "uScreenCenter");
		pixels_per_unit_uniform = glGetUniformLocation(shader, "uPixelsPerUnit");
		texture_sampler_uniform = glGetUniformLocation(shader, "uTextureSampler");
		texture_flag_uniform    = glGetUniformLocation(shader, "uTextureFlag");
		return true;
	}();

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform2fv (position_uniform, 1, tiling_.position_.data());
	glUniform2fv (t1_uniform, 1, tiling_.t1_.data());
	glUniform2fv (t2_uniform, 1, tiling_.t2_.data());
	glUniform2i  (screen_size_uniform, width, height);
	glUniform2fv (screen_center_uniform, 1, screen_center_.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform1i  (texture_sampler_uniform, 1);
	glUniform1i  (texture_flag_uniform, symmetrifying_);

	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, tiling_.domain_texture_);

	glBindVertexArray(tiling_.mesh_.vao_);
	glDrawArraysInstanced(tiling_.mesh_.primitive_type_, 0, tiling_.mesh_.num_vertices_, 400);

	// Clean up.
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

Eigen::Vector2f App::scale_to_world(const Eigen::Vector2f& v)
{
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	return { v.x() * (0.5 * width) / pixels_per_unit_,
	         v.y() * (0.5 * height) / pixels_per_unit_ };
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
		printf("Taking screenshot...\n");

		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		// TODO: Threading.
		auto texture = GL::Texture::empty_2D(width, height);
		auto depth   = GL::Texture::empty_2D_depth(width, height);
		auto fbo     = GL::FBO::simple_C0D(texture, depth);
		GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, fbo);

		tiling_.symmetrify(debug_tex_);
		render_symmetry_frame(true, width, height, fbo);

		// TODO: Query screenshot name from user.
		GL::tex_to_png(texture, "screenshot.png");

		printf("Screenshot saved. (screenshot.png)\n");
	}
}

void App::test_update_objects_cb(double x, double y)
{
	if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);
		Eigen::Vector2f position = {x / width * 2 - 1, 1 - y / height * 2};
		const auto& drag_position = position - press_position_;


		if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			tiling_.position_ = tiling_static_position_ + scale_to_world(drag_position);
		else
		{
			plane_.set_position(plane_static_position_ + scale_to_world(drag_position));
			screen_center_ = screen_center_static_position_ - scale_to_world(drag_position);
		}

	}
}

// TODO: Rewrite input manager.
void App::test_left_click_cb(int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		press_position_ = input_manager_.mouse_position();

		plane_static_position_ = plane_.position();
		screen_center_static_position_ = screen_center_;
		tiling_static_position_ = tiling_.position_;
	}
}

void App::test_scroll_cb(double xoffset, double yoffset)
{
	if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		if (yoffset < 0)
		{
			tiling_.t1_ *= zoom_factor_;
			tiling_.t2_ *= zoom_factor_;
		}
		else if (yoffset > 0)
		{
			tiling_.t1_ /= zoom_factor_;
			tiling_.t2_ /= zoom_factor_;
		}
	}
	else
	{
		if (yoffset > 0)
			pixels_per_unit_ *=  zoom_factor_;
		else if (yoffset < 0)
			pixels_per_unit_ /=  zoom_factor_;
	}
}

//--------------------
