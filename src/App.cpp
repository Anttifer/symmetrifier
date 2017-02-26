#include "App.h"

#include "GLFunctions.h"
#include "GLUtils.h"
#include "Examples.h"
#include <cstdio>

// DEBUG
#include <iostream>

//--------------------

App::App(int argc, char* argv[])
:	window_                (1440, 900, "supersymmetry"),
	symmetrifying_         (false),
	screen_center_         (0.0, 0.0),
	pixels_per_unit_       (720.0),                           // Initial zoom level.
	zoom_factor_           (1.2),
	time_                  ( (glfwSetTime(0), glfwGetTime()) )
{
	// Key callbacks.
	window_.add_key_callback(GLFW_KEY_P, &App::print_screen, this);
	window_.add_key_callback(GLFW_KEY_SPACE, [this](int,int action,int){
		if (action == GLFW_PRESS)
			this->symmetrifying_ ^= true;
	});

	// Mouse callbacks.
	window_.add_mouse_button_callback(GLFW_MOUSE_BUTTON_LEFT, &App::test_left_click_cb, this);
	window_.add_mouse_pos_callback(&App::test_update_objects_cb, this);
	window_.add_scroll_callback(&App::test_scroll_cb, this);

	glfwSwapInterval(0);

	// Enable depth testing and alpha blending.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	debug_tex_   = GL::Texture::empty_2D(width, height);
	auto depth   = GL::Texture::empty_2D_depth(width, height);
	auto fbo     = GL::FBO::simple_C0D(debug_tex_, depth);
	glClearColor(0, 0, 0, 0);
	GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, fbo);

	Examples::render_pinwheel(screen_center_, pixels_per_unit_, width, height, fbo);

	pixels_per_unit_ = 900.0;
	screen_center_   = {0.5, 0.5};
	tiling_.set_symmetry_group("2*22");
}

void App::loop(void)
{
	while (!glfwWindowShouldClose(window_))
	{
		time_ = glfwGetTime();

		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		// Clear the screen. Dark grey is the new black.
		glClearColor(0.1, 0.1, 0.1, 0);
		GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (symmetrifying_)
		{
			// Prevent needless symmetrifying, which might be expensive.
			if (!tiling_.consistent())
				tiling_.symmetrify(debug_tex_);
			render_symmetry_frame(true, width, height);
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
	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, image);

	glViewport(0, 0, width, height);

	auto AR = image.width_ / (float)image.height_;

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform1f  (aspect_ratio_uniform, AR);
	glUniform2i  (screen_size_uniform, width, height);
	glUniform2fv (screen_center_uniform, 1, screen_center_.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform1i  (texture_sampler_uniform, 1);

	glBindVertexArray(canvas_.vao_);
	glDrawArrays(canvas_.primitive_type_, 0, canvas_.num_vertices_);

	// Clean up.
	glBindVertexArray(0);

	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_symmetry_frame(bool symmetrifying, int width, int height, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/tiling_vert.glsl",
		"shaders/tiling_geom.glsl",
		"shaders/tiling_frag.glsl");

	// Find uniform locations once.
	static GLuint instance_num_uniform;
	static GLuint position_uniform;
	static GLuint t1_uniform;
	static GLuint t2_uniform;
	static GLuint screen_size_uniform;
	static GLuint screen_center_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint texture_sampler_uniform;
	static GLuint texture_flag_uniform;
	static bool init = [&](){
		instance_num_uniform    = glGetUniformLocation(shader, "uNumInstances");
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
	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, tiling_.domain_texture());

	glViewport(0, 0, width, height);

	const auto plane_side_length = 10;
	const auto num_instances = symmetrifying ? plane_side_length * plane_side_length : 1;

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform1i  (instance_num_uniform, num_instances);
	glUniform2fv (position_uniform, 1, tiling_.position().data());
	glUniform2fv (t1_uniform, 1, tiling_.t1().data());
	glUniform2fv (t2_uniform, 1, tiling_.t2().data());
	glUniform2i  (screen_size_uniform, width, height);
	glUniform2fv (screen_center_uniform, 1, screen_center_.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform1i  (texture_sampler_uniform, 1);
	glUniform1i  (texture_flag_uniform, symmetrifying_);

	const auto& mesh = tiling_.mesh();

	glBindVertexArray(mesh.vao_);
	glDrawArraysInstanced(mesh.primitive_type_, 0, mesh.num_vertices_, num_instances);

	// Clean up.
	glBindVertexArray(0);

	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

// TODO: Figure out where this should go.
Eigen::Vector2f App::screen_to_world(const Eigen::Vector2f& v)
{
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	return { v.x() * (0.5 * width) / pixels_per_unit_,
	         v.y() * (0.5 * height) / pixels_per_unit_ };
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
			tiling_.set_position(tiling_static_position_ + screen_to_world(drag_position));
		else
			screen_center_ = screen_center_static_position_ - screen_to_world(drag_position);
	}
}

void App::test_left_click_cb(int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		double xpos, ypos;
		glfwGetCursorPos(window_, &xpos, &ypos);

		press_position_ = {xpos / width * 2 - 1, 1 - ypos / height * 2};

		screen_center_static_position_ = screen_center_;
		tiling_static_position_        = tiling_.position();
	}
}

void App::test_scroll_cb(double xoffset, double yoffset)
{
	if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		if (yoffset < 0)
			tiling_.set_scale(zoom_factor_);
		else if (yoffset > 0)
			tiling_.set_scale(1 / zoom_factor_);
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
