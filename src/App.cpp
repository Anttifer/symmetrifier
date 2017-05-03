#include "App.h"

#include "GLFunctions.h"
#include "GLUtils.h"
#include <cstdio>
#include <cstdint>

//--------------------

App::App(int /* argc */, char** /* argv */) :
	window_                (1440, 900, "supersymmetry"),
	time_                  ( (glfwSetTime(0), glfwGetTime()) ),
	tiling_                (layering_.current_layer().tiling()),
	gui_                   (window_, layering_),

	clear_color_           (0.1, 0.1, 0.1),
	screen_center_         (0.5, 0.5),
	pixels_per_unit_       (300.0),
	show_symmetry_frame_   (true),
	show_result_           (true),
	show_settings_         (true),
	show_export_settings_  (false),
	export_width_          (1600),
	export_height_         (1200),

	zoom_factor_           (1.2)
{
	load_layer_image("res/kissa");

	// Set GUI to track the relevant variables.
	gui_.clear_color_track(clear_color_);
	gui_.screen_center_track(screen_center_);
	gui_.pixels_per_unit_track(pixels_per_unit_);
	gui_.frame_visible_track(show_symmetry_frame_);
	gui_.result_visible_track(show_result_);
	gui_.settings_window_visible_track(show_settings_);
	gui_.export_window_visible_track(show_export_settings_);
	gui_.export_width_track(export_width_);
	gui_.export_height_track(export_height_);

	// Set GUI export button callback.
	gui_.set_export_callback(&App::export_result, this);

	// Mouse callbacks.
	window_.add_mouse_pos_callback    (&App::mouse_position_callback, this);
	window_.add_mouse_button_callback (&App::mouse_button_callback, this);
	window_.add_scroll_callback       (&App::mouse_scroll_callback, this);

	// Keyboard callback.
	window_.add_key_callback(&App::keyboard_callback, this);

	// Path drop callback.
	window_.add_path_drop_callback(&App::path_drop_callback, this);

	// Disable vsync.
	glfwSwapInterval(0);

	// "Enable" depth testing and alpha blending.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// This probably doesn't work, but worth asking anyway. :)
	glEnable(GL_LINE_SMOOTH);

	auto& layer = layering_.current_layer();
	layer.tiling().set_symmetry_group("333");
	layer.tiling().set_center({0.5f, 0.5f});
	layer.tiling().set_scale(2.0f);
}

void App::loop(void)
{
	while (!glfwWindowShouldClose(window_))
	{
		time_ = glfwGetTime();

		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		// Clear the screen. Dark grey is the new black.
		glClearColor(clear_color_.x(), clear_color_.y(), clear_color_.z(), 0);
		GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render_layered_scene(gui_.graphics_area());
		gui_.render(width, height);

		// Show the result on screen.
		glfwSwapBuffers(window_);

		// Poll events. Minimum FPS = 15.
		glfwWaitEventsTimeout(1 / 15.0);
	}
}

void App::render_layered_scene(const Rectangle<int>& viewport, GLuint framebuffer)
{
	const auto& current_layer = layering_.current_layer();

	if (show_result_)
	{
		for (const auto& layer : layering_)
		{
			if (layer.visible())
				render_layer(layer, viewport, framebuffer);
		}

		if (show_symmetry_frame_)
			render_symmetry_frame(current_layer.tiling(), viewport, framebuffer);

		if (show_export_settings_)
			render_export_frame(viewport, framebuffer);
	}
	else
	{
		for (const auto& layer : layering_)
		{
			if (layer.visible() || &layer == &current_layer)
				render_layer_images(layer, viewport, framebuffer);
		}

		// Always render frame when not showing the result.
		render_symmetry_frame(current_layer.tiling(), viewport, framebuffer);
	}
}

void App::render_layer(const Layer& layer, const Rectangle<int>& viewport, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/tiling_vert.glsl",
		"shaders/tiling_frag.glsl");

	// Find uniform locations once.
	static GLuint instance_num_uniform;
	static GLuint position_uniform;
	static GLuint t1_uniform;
	static GLuint t2_uniform;
	static GLuint viewport_size_uniform;
	static GLuint view_center_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint texture_coordinate_uniform;
	static GLuint texture_sampler_uniform;
	static bool init = [&](){
		instance_num_uniform       = glGetUniformLocation(shader, "uNumInstances");
		position_uniform           = glGetUniformLocation(shader, "uPos");
		t1_uniform                 = glGetUniformLocation(shader, "uT1");
		t2_uniform                 = glGetUniformLocation(shader, "uT2");
		viewport_size_uniform      = glGetUniformLocation(shader, "uScreenSize");
		view_center_uniform        = glGetUniformLocation(shader, "uScreenCenter");
		pixels_per_unit_uniform    = glGetUniformLocation(shader, "uPixelsPerUnit");
		texture_coordinate_uniform = glGetUniformLocation(shader, "uTexCoords");
		texture_sampler_uniform    = glGetUniformLocation(shader, "uTextureSampler");
		return true;
	}();
	(void)init; // Suppress unused variable warning.

	const auto& domain_texture = layer.domain_texture();

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, domain_texture);

	glViewport(viewport.x, viewport.y, viewport.width, viewport.height);

	const auto plane_side_length = 10;
	const auto num_instances = plane_side_length * plane_side_length;

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	const auto& tiling = layer.tiling();

	const auto& tiling_position = layer.to_world(tiling.position());
	const auto& tiling_t1       = layer.to_world_direction(tiling.t1());
	const auto& tiling_t2       = layer.to_world_direction(tiling.t2());

	glUniform1i  (instance_num_uniform, num_instances);
	glUniform2fv (position_uniform, 1, tiling_position.data());
	glUniform2fv (t1_uniform, 1, tiling_t1.data());
	glUniform2fv (t2_uniform, 1, tiling_t2.data());
	glUniform2i  (viewport_size_uniform, viewport.width, viewport.height);
	glUniform2fv (view_center_uniform, 1, screen_center_.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform2fv (texture_coordinate_uniform, 6, layer.domain_coordinates()[0].data());
	glUniform1i  (texture_sampler_uniform, 1);

	const auto& mesh = tiling.mesh();

	glBindVertexArray(mesh.vao_);
	glDrawArraysInstanced(mesh.primitive_type_, 0, mesh.num_vertices_, num_instances);

	// Clean up.
	glBindVertexArray(0);

	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_layer_images(const Layer& layer, const Rectangle<int>& viewport, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/image_vert.glsl",
		"shaders/image_frag.glsl");

	// Find uniform locations once.
	static GLuint viewport_size_uniform;
	static GLuint view_center_uniform;
	static GLuint image_position_uniform;
	static GLuint image_t1_uniform;
	static GLuint image_t2_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint texture_sampler_uniform;
	static bool init = [&](){
		viewport_size_uniform   = glGetUniformLocation(shader, "uScreenSize");
		view_center_uniform     = glGetUniformLocation(shader, "uScreenCenter");
		image_position_uniform  = glGetUniformLocation(shader, "uImagePos");
		image_t1_uniform        = glGetUniformLocation(shader, "uImageT1");
		image_t2_uniform        = glGetUniformLocation(shader, "uImageT2");
		pixels_per_unit_uniform = glGetUniformLocation(shader, "uPixelsPerUnit");
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

	glViewport(viewport.x, viewport.y, viewport.width, viewport.height);

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);
	glBindVertexArray(canvas_.vao_);

	glUniform2i  (viewport_size_uniform, viewport.width, viewport.height);
	glUniform2fv (view_center_uniform, 1, screen_center_.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform1i  (texture_sampler_uniform, 1);

	for (const auto& image : layer)
	{
		const auto& image_position = layer.to_world(image.position());
		const auto& image_t1       = layer.to_world_direction(image.t1());
		const auto& image_t2       = layer.to_world_direction(image.t2());

		glBindTexture(GL_TEXTURE_2D, image.texture());

		glUniform2fv (image_position_uniform, 1, image_position.data());
		glUniform2fv (image_t1_uniform, 1, image_t1.data());
		glUniform2fv (image_t2_uniform, 1, image_t2.data());

		glDrawArrays(canvas_.primitive_type_, 0, canvas_.num_vertices_);
	}

	// Clean up.
	glBindVertexArray(0);
	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

[[deprecated]]
void App::render_tiling_hq(const Tiling& tiling, int width, int height, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/tiling_hq_vert.glsl",
		"shaders/tiling_hq_frag.glsl");

	// Find uniform locations once.
	static GLuint num_instances_uniform;
	static GLuint frame_position_uniform;
	static GLuint t1_uniform;
	static GLuint t2_uniform;
	static GLuint screen_size_uniform;
	static GLuint screen_center_uniform;
	static GLuint image_position_uniform;
	static GLuint image_t1_uniform;
	static GLuint image_t2_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint num_domains_uniform;
	static GLuint mesh_sampler_uniform;
	static GLuint texture_sampler_uniform;
	static bool init = [&](){
		num_instances_uniform      = glGetUniformLocation(shader, "uNumInstances");
		frame_position_uniform     = glGetUniformLocation(shader, "uFramePos");
		t1_uniform                 = glGetUniformLocation(shader, "uT1");
		t2_uniform                 = glGetUniformLocation(shader, "uT2");
		screen_size_uniform        = glGetUniformLocation(shader, "uScreenSize");
		screen_center_uniform      = glGetUniformLocation(shader, "uScreenCenter");
		image_position_uniform     = glGetUniformLocation(shader, "uImagePos");
		image_t1_uniform           = glGetUniformLocation(shader, "uImageT1");
		image_t2_uniform           = glGetUniformLocation(shader, "uImageT2");
		pixels_per_unit_uniform    = glGetUniformLocation(shader, "uPixelsPerUnit");
		num_domains_uniform        = glGetUniformLocation(shader, "uNumSymmetryDomains");
		mesh_sampler_uniform       = glGetUniformLocation(shader, "uMeshSampler");
		texture_sampler_uniform    = glGetUniformLocation(shader, "uTextureSampler");
		return true;
	}();
	(void)init; // Suppress unused variable warning.

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, tiling.base_image());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, tiling.mesh_texture());
	glActiveTexture(GL_TEXTURE1);

	glViewport(0, 0, width, height);

	const auto plane_side_length = 10;
	const auto num_instances = plane_side_length * plane_side_length;

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	const auto& mesh = tiling.mesh();

	glUniform1i  (num_instances_uniform, num_instances);
	glUniform2fv (frame_position_uniform, 1, tiling.position().data());
	glUniform2fv (t1_uniform, 1, tiling.t1().data());
	glUniform2fv (t2_uniform, 1, tiling.t2().data());
	glUniform2i  (screen_size_uniform, width, height);
	glUniform2fv (screen_center_uniform, 1, screen_center_.data());
	glUniform2fv (image_position_uniform, 1, tiling.image_position().data());
	glUniform2fv (image_t1_uniform, 1, tiling.image_t1().data());
	glUniform2fv (image_t2_uniform, 1, tiling.image_t2().data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform1i  (num_domains_uniform, tiling.num_symmetry_domains());
	glUniform1i  (texture_sampler_uniform, 1);
	glUniform1i  (mesh_sampler_uniform, 2);

	glBindVertexArray(mesh.vao_);
	glDrawArraysInstanced(mesh.primitive_type_, 0, mesh.num_vertices_, num_instances);

	// Clean up.
	glBindVertexArray(0);

	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_symmetry_frame(const Tiling& tiling, const Rectangle<int>& viewport, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/frame_vert.glsl",
		"shaders/frame_frag.glsl");

	// Find uniform locations once.
	static GLuint instance_num_uniform;
	static GLuint position_uniform;
	static GLuint t1_uniform;
	static GLuint t2_uniform;
	static GLuint viewport_size_uniform;
	static GLuint view_center_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint render_overlay_uniform;
	static bool init = [&](){
		instance_num_uniform    = glGetUniformLocation(shader, "uNumInstances");
		position_uniform        = glGetUniformLocation(shader, "uPos");
		t1_uniform              = glGetUniformLocation(shader, "uT1");
		t2_uniform              = glGetUniformLocation(shader, "uT2");
		viewport_size_uniform   = glGetUniformLocation(shader, "uScreenSize");
		view_center_uniform     = glGetUniformLocation(shader, "uScreenCenter");
		pixels_per_unit_uniform = glGetUniformLocation(shader, "uPixelsPerUnit");
		render_overlay_uniform  = glGetUniformLocation(shader, "uRenderOverlay");
		return true;
	}();
	(void)init; // Suppress unused variable warning.

	// Create overlay mesh once.
	static Mesh overlay;
	static bool init_overlay = [&](){
		overlay.positions_ = {
			{0, 0, 0}, {1, 0, 0}, {1, 1, 0},
			{1, 1, 0}, {0, 1, 0}, {0, 0, 0}
		};
		overlay.update_buffers();
		return true;
	}();
	(void)init_overlay;

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(viewport.x, viewport.y, viewport.width, viewport.height);

	const auto plane_side_length = 10;
	const auto num_instances = show_result_ ? plane_side_length * plane_side_length : tiling.num_lattice_domains();

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform1i  (instance_num_uniform, num_instances);
	glUniform2fv (position_uniform, 1, tiling.position().data());
	glUniform2fv (t1_uniform, 1, tiling.t1().data());
	glUniform2fv (t2_uniform, 1, tiling.t2().data());
	glUniform2i  (viewport_size_uniform, viewport.width, viewport.height);
	glUniform2fv (view_center_uniform, 1, screen_center_.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform1i  (render_overlay_uniform, GL_FALSE);

	const auto& frame = tiling.frame();

	glBindVertexArray(frame.vao_);
	glDrawArraysInstanced(GL_LINES, 0, frame.num_vertices_, num_instances);

	glUniform1i  (instance_num_uniform, tiling.num_lattice_domains());
	glUniform1i  (render_overlay_uniform, GL_TRUE);

	glBindVertexArray(overlay.vao_);
	glDrawArraysInstanced(overlay.primitive_type_, 0, overlay.num_vertices_, tiling.num_lattice_domains());

	// Clean up.
	glBindVertexArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_export_frame(const Rectangle<int>& viewport, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram(
		GL::ShaderObject::vertex_passthrough(),
		GL::ShaderObject::from_file(GL_FRAGMENT_SHADER, "shaders/crop_frag.glsl")
	);

	// Find uniform locations once.
	static GLuint viewport_size_uniform;
	static GLuint stripe_size_uniform;
	static GLuint vertical_crop_uniform;
	static bool init = [&](){
		viewport_size_uniform   = glGetUniformLocation(shader, "uScreenSize");
		stripe_size_uniform     = glGetUniformLocation(shader, "uStripeSize");
		vertical_crop_uniform   = glGetUniformLocation(shader, "uVerticalCrop");
		return true;
	}();
	(void)init; // Suppress unused variable warning.

	float AR = viewport.width / (float)viewport.height;
	float crop_AR = export_width_ / (float)export_height_;

	// stripe_size is the width or height of the visible region.
	// In the other direction the region extends through the entire window.
	float stripe_size;
	if (crop_AR > AR)
		stripe_size = viewport.width / crop_AR;
	else
		stripe_size = viewport.height * crop_AR;

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(viewport.x, viewport.y, viewport.width, viewport.height);

	glUseProgram(shader);

	glUniform2i (viewport_size_uniform, viewport.width, viewport.height);
	glUniform1f (stripe_size_uniform, stripe_size);
	glUniform1i (vertical_crop_uniform, crop_AR > AR);

	glBindVertexArray(canvas_.vao_);
	glDrawArrays(canvas_.primitive_type_, 0, canvas_.num_vertices_);

	// Clean up.
	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::mouse_position_callback(double x, double y)
{
	if (gui_.capturing_mouse())
		return;

	auto& layer = layering_.current_layer();

	if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		Eigen::Vector2f position  = screen_to_view(x, y);
		const auto& drag_position = position - press_position_;
		const auto& layer_drag    = layer.from_world_direction(drag_position);

		if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			if (layer.has_current_image())
				layer.current_image().set_position(object_static_position_ + layer_drag);
			else
				layer.tiling().set_position(object_static_position_ + layer_drag);
		}
		else if (glfwGetKey(window_, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
		{
			// TODO: Image deformations.
			if (!layer.has_current_image())
				layer.tiling().deform(layer_drag);
		}
		else
			screen_center_ = screen_center_static_position_ - drag_position;
	}
	else if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		// TODO: Global rotation.
		if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			Eigen::Vector2f world_press_position = view_to_world(press_position_);
			Eigen::Vector2f world_position       = screen_to_world(x, y);

			const auto& layer_press_position = layer.from_world(world_press_position);
			const auto& layer_position       = layer.from_world(world_position);

			if (layer.has_current_image())
			{
				auto& image = layer.current_image();

				Eigen::Vector2f press_wrt_center    = layer_press_position - image.center();
				Eigen::Vector2f position_wrt_center = layer_position       - image.center();

				double det = (Eigen::Matrix2f() << press_wrt_center, position_wrt_center).finished().determinant();
				double dot = press_wrt_center.dot(position_wrt_center);
				double drag_rotation = std::atan2(det, dot);

				image.set_rotation(object_static_rotation_ + drag_rotation);
			}
			else
			{
				auto& tiling = layer.tiling();

				Eigen::Vector2f press_wrt_center    = layer_press_position - tiling.center();
				Eigen::Vector2f position_wrt_center = layer_position       - tiling.center();

				double det = (Eigen::Matrix2f() << press_wrt_center, position_wrt_center).finished().determinant();
				double dot = press_wrt_center.dot(position_wrt_center);
				double drag_rotation = std::atan2(det, dot);

				tiling.set_rotation(object_static_rotation_ + drag_rotation);
			}
		}
	}
}

void App::mouse_button_callback(int button, int action, int /* mods */)
{
	if (action != GLFW_PRESS)
		return;

	auto& layer = layering_.current_layer();

	double x, y;
	glfwGetCursorPos(window_, &x, &y);
	press_position_ = screen_to_view(x, y);

	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		screen_center_static_position_ = screen_center_;

		if (layer.has_current_image())
			object_static_position_ = layer.as_const().current_image().position();
		else
		{
			object_static_position_ = layer.as_const().tiling().position();
			layer.tiling().set_deform_origin(layer.from_world(view_to_world(press_position_)));
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (layer.has_current_image())
			object_static_rotation_ = layer.as_const().current_image().rotation();
		else
			object_static_rotation_ = layer.as_const().tiling().rotation();
	}
}

void App::mouse_scroll_callback(double /* x_offset */, double y_offset)
{
	if (gui_.capturing_mouse())
		return;

	auto& layer         = layering_.current_layer();
	const auto& ctiling = layer.as_const().tiling();

	if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		if (layer.has_current_image())
		{
			const auto& cimage = layer.as_const().current_image();

			if (y_offset < 0)
				layer.current_image().multiply_scale(zoom_factor_);
			else if (y_offset > 0 && cimage.scale() > 0.001)
				layer.current_image().multiply_scale(1 / zoom_factor_);
		}
		else
		{
			if (y_offset < 0)
				layer.tiling().multiply_scale(zoom_factor_);
			else if (y_offset > 0 && ctiling.scale() > 0.001)
				layer.tiling().multiply_scale(1 / zoom_factor_);
		}
	}
	else
	{
		if (y_offset > 0)
			pixels_per_unit_ *=  zoom_factor_;
		else if (y_offset < 0)
			pixels_per_unit_ /=  zoom_factor_;
	}
}

void App::keyboard_callback(int key, int /* scancode */, int action, int /* mods */)
{
	if (action != GLFW_PRESS || gui_.capturing_keyboard())
		return;

	if (key == GLFW_KEY_SPACE)
	{
		if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			show_symmetry_frame_ ^= true;
		else
			show_result_ ^= true;
	}
	else if (key == GLFW_KEY_1)
		layering_.current_layer().tiling().set_num_lattice_domains(1);
	else if (key == GLFW_KEY_2)
		layering_.current_layer().tiling().set_num_lattice_domains(4);
	else if (key == GLFW_KEY_3)
		layering_.current_layer().tiling().set_num_lattice_domains(9);
}

void App::path_drop_callback(int count, const char** paths)
{
	for (int i = 0; i < count; ++i)
		load_layer_image(paths[i]);
}

void App::load_layer_image(const char* filename)
{
	std::string path = filename;
	std::string basename;

	auto dir_end = path.find_last_of("\\/");
	if (dir_end >= path.size() - 1)
		basename = path;// Basename is entire path if no directory delimiters present.
	else
		basename = path.substr(dir_end + 1);

	bool successful = false;
	auto texture = GL::Texture::from_png(path.c_str(), successful);

	if (!successful)
		basename = "ERROR";

	// Even in case of error - just to draw attention.
	layering_.current_layer().add_image(basename, std::move(texture));
}

// TODO: Add support to larger exports rendered in smaller tiles.
void App::export_result(int export_width, int export_height, const char* export_filename)
{
	printf("Exporting...\n");

	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	auto texture = GL::Texture::empty_2D(export_width, export_height);
	auto depth   = GL::Texture::empty_2D_depth(export_width, export_height);
	auto fbo     = GL::FBO::simple_C0D(texture, depth);

	// We don't want transparency in the resulting PNG.
	// Thus we set the clear color alpha to 1 and change our blending function
	// to prefer destination alpha (this is the clear color alpha, i.e. 1).
	glClearColor(clear_color_.x(), clear_color_.y(), clear_color_.z(), 1);
	GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, fbo);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

	// We want to keep the zoom level irrespective of resolution chosen.
	double ppu_old = pixels_per_unit_;
	pixels_per_unit_ = std::max( export_width / (float)width, export_height / (float)height) * ppu_old;

	render_tiling_hq(tiling_, export_width, export_height, fbo);

	pixels_per_unit_ = ppu_old;

	// Reset the blending function.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GL::tex_to_png(texture, export_filename);
	printf("Export finished (%s)\n", export_filename);
}

Eigen::Vector2f App::screen_to_view(double x, double y)
{
	// Invert y - OpenGL and hence graphics_area work from bottom to top.
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);
	y = height - y;

	const auto& view = gui_.graphics_area();
	Eigen::Vector2f view_center = {view.x + view.width / 2.0f,
	                               view.y + view.height / 2.0f};

	return { (x - view_center.x()) / pixels_per_unit_,
	         (y - view_center.y()) / pixels_per_unit_ };
}

Eigen::Vector2f App::view_to_world(const Eigen::Vector2f& v)
{
	return screen_center_ + v;
}

Eigen::Vector2f App::screen_to_world(double x, double y)
{
	return view_to_world(screen_to_view(x, y));
}

//--------------------
