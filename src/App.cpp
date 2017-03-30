#include "App.h"

#include "GLFunctions.h"
#include "GLUtils.h"
#include "Examples.h"
#include "imgui.h"
#include <cstdio>
#include <cstdint>

//--------------------

App::App(int /* argc */, char** /* argv */) :
	window_                (1440, 900, "supersymmetry"),
	time_                  ( (glfwSetTime(0), glfwGetTime()) ),
	gui_                   (window_),
	show_result_           (true),
	show_symmetry_frame_   (true),
	show_export_settings_  (false),
	show_settings_         (true),
	screen_center_         (0.5, 0.5),
	clear_color_           (0.1, 0.1, 0.1),
	pixels_per_unit_       (300.0), // Initial zoom level.
	zoom_factor_           (1.2),
	export_width_          (1600),
	export_height_         (1200),
	export_base_name_      ("image")
{
	// Mouse callbacks.
	window_.add_mouse_pos_callback(&App::position_callback, this);
	window_.add_mouse_button_callback(GLFW_MOUSE_BUTTON_LEFT, &App::left_click_callback, this);
	window_.add_mouse_button_callback(GLFW_MOUSE_BUTTON_RIGHT, &App::right_click_callback, this);
	window_.add_scroll_callback(&App::scroll_callback, this);

	// Key callbacks.
	window_.add_key_callback(GLFW_KEY_P, &App::print_screen, this);
	window_.add_key_callback(GLFW_KEY_SPACE, [this](int, int action, int){
		if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard)
		{
			if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				this->show_symmetry_frame_ ^= true;
			else
				this->show_result_ ^= true;
		}
	});
	window_.add_key_callback(GLFW_KEY_ESCAPE, [this](int, int action, int){
		if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard)
			this->show_settings_ ^= true;
	});
	window_.add_key_callback(GLFW_KEY_1, [this](int, int action, int){
		if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard)
			this->tiling_.set_num_lattice_domains(1);
	});
	window_.add_key_callback(GLFW_KEY_2, [this](int, int action, int){
		if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard)
			this->tiling_.set_num_lattice_domains(4);
	});
	window_.add_key_callback(GLFW_KEY_3, [this](int, int action, int){
		if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard)
			this->tiling_.set_num_lattice_domains(9);
	});

	// Drop callback.
	window_.add_path_drop_callback([this](int count, const char** paths){
		if (count > 0)
			this->load_texture(paths[0]);
	});

	// Disable vsync.
	glfwSwapInterval(0);

	// "Enable" depth testing and alpha blending.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// This probably doesn't work, but worth asking anyway. :)
	glEnable(GL_LINE_SMOOTH);

	// Set default GUI font.
	auto& io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->AddFontFromFileTTF("res/DroidSans.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	gui_.create_fonts_texture();

	load_texture("res/kissa");
	tiling_.set_symmetry_group("333");
	tiling_.set_center({0.5, 0.5});
	tiling_.set_scale(2.0f);

	export_filename_ = export_base_name_ + ".png";

	populate_thumbnail_map();
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

		render_scene(width, height);
		render_gui(width, height);

		// Show the result on screen.
		glfwSwapBuffers(window_);

		// Poll events. Minimum FPS = 15.
		glfwWaitEventsTimeout(1 / 15.0);
	}
}

void App::render_scene(int width, int height, GLuint framebuffer)
{
	if (show_result_)
	{
		// Don't symmetrify if already consistent.
		if (!tiling_.consistent())
			tiling_.symmetrify(base_image_);
		render_tiling(width, height, framebuffer);

		if (show_symmetry_frame_)
			render_symmetry_frame(width, height, framebuffer);

		if (show_export_settings_)
			render_export_frame(width, height, framebuffer);
	}
	else
	{
		render_image(base_image_, width, height, framebuffer);

		// Always render frame when not showing the result.
		render_symmetry_frame(width, height, framebuffer);
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
	(void)init; // Suppress unused variable warning.

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

void App::render_tiling(int width, int height, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/tiling_vert.glsl",
		"shaders/tiling_frag.glsl");

	// Find uniform locations once.
	static GLuint instance_num_uniform;
	static GLuint position_uniform;
	static GLuint t1_uniform;
	static GLuint t2_uniform;
	static GLuint screen_size_uniform;
	static GLuint screen_center_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint texture_coordinate_uniform;
	static GLuint texture_sampler_uniform;
	static bool init = [&](){
		instance_num_uniform       = glGetUniformLocation(shader, "uNumInstances");
		position_uniform           = glGetUniformLocation(shader, "uPos");
		t1_uniform                 = glGetUniformLocation(shader, "uT1");
		t2_uniform                 = glGetUniformLocation(shader, "uT2");
		screen_size_uniform        = glGetUniformLocation(shader, "uScreenSize");
		screen_center_uniform      = glGetUniformLocation(shader, "uScreenCenter");
		pixels_per_unit_uniform    = glGetUniformLocation(shader, "uPixelsPerUnit");
		texture_coordinate_uniform = glGetUniformLocation(shader, "uTexCoords");
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
	glBindTexture(GL_TEXTURE_2D, tiling_.domain_texture());

	glViewport(0, 0, width, height);

	const auto plane_side_length = 10;
	const auto num_instances = plane_side_length * plane_side_length;

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform1i  (instance_num_uniform, num_instances);
	glUniform2fv (position_uniform, 1, tiling_.position().data());
	glUniform2fv (t1_uniform, 1, tiling_.t1().data());
	glUniform2fv (t2_uniform, 1, tiling_.t2().data());
	glUniform2i  (screen_size_uniform, width, height);
	glUniform2fv (screen_center_uniform, 1, screen_center_.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform2fv (texture_coordinate_uniform, 6, tiling_.domain_coordinates()[0].data());
	glUniform1i  (texture_sampler_uniform, 1);

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

void App::render_tiling_hq(int width, int height, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/tiling_hq_vert.glsl",
		"shaders/tiling_hq_frag.glsl");

	// Find uniform locations once.
	static GLuint num_instances_uniform;
	static GLuint aspect_ratio_uniform;
	static GLuint frame_position_uniform;
	static GLuint t1_uniform;
	static GLuint t2_uniform;
	static GLuint screen_size_uniform;
	static GLuint screen_center_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint num_domains_uniform;
	static GLuint mesh_sampler_uniform;
	static GLuint texture_sampler_uniform;
	static bool init = [&](){
		num_instances_uniform      = glGetUniformLocation(shader, "uNumInstances");
		aspect_ratio_uniform       = glGetUniformLocation(shader, "uAR");
		frame_position_uniform     = glGetUniformLocation(shader, "uFramePos");
		t1_uniform                 = glGetUniformLocation(shader, "uT1");
		t2_uniform                 = glGetUniformLocation(shader, "uT2");
		screen_size_uniform        = glGetUniformLocation(shader, "uScreenSize");
		screen_center_uniform      = glGetUniformLocation(shader, "uScreenCenter");
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
	glBindTexture(GL_TEXTURE_2D, base_image_);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, tiling_.mesh_texture());
	glActiveTexture(GL_TEXTURE1);

	glViewport(0, 0, width, height);

	const auto plane_side_length = 10;
	const auto num_instances = plane_side_length * plane_side_length;

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	auto AR = base_image_.width_ / (float)base_image_.height_;
	const auto& mesh = tiling_.mesh();

	glUniform1i  (num_instances_uniform, num_instances);
	glUniform1f  (aspect_ratio_uniform, AR);
	glUniform2fv (frame_position_uniform, 1, tiling_.position().data());
	glUniform2fv (t1_uniform, 1, tiling_.t1().data());
	glUniform2fv (t2_uniform, 1, tiling_.t2().data());
	glUniform2i  (screen_size_uniform, width, height);
	glUniform2fv (screen_center_uniform, 1, screen_center_.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform1i  (num_domains_uniform, tiling_.num_symmetry_domains());
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

void App::render_symmetry_frame(int width, int height, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/frame_vert.glsl",
		"shaders/frame_frag.glsl");

	// Find uniform locations once.
	static GLuint instance_num_uniform;
	static GLuint position_uniform;
	static GLuint t1_uniform;
	static GLuint t2_uniform;
	static GLuint screen_size_uniform;
	static GLuint screen_center_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint render_overlay_uniform;
	static bool init = [&](){
		instance_num_uniform    = glGetUniformLocation(shader, "uNumInstances");
		position_uniform        = glGetUniformLocation(shader, "uPos");
		t1_uniform              = glGetUniformLocation(shader, "uT1");
		t2_uniform              = glGetUniformLocation(shader, "uT2");
		screen_size_uniform     = glGetUniformLocation(shader, "uScreenSize");
		screen_center_uniform   = glGetUniformLocation(shader, "uScreenCenter");
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

	glViewport(0, 0, width, height);

	const auto plane_side_length = 10;
	const auto num_instances = show_result_ ? plane_side_length * plane_side_length : tiling_.num_lattice_domains();

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform1i  (instance_num_uniform, num_instances);
	glUniform2fv (position_uniform, 1, tiling_.position().data());
	glUniform2fv (t1_uniform, 1, tiling_.t1().data());
	glUniform2fv (t2_uniform, 1, tiling_.t2().data());
	glUniform2i  (screen_size_uniform, width, height);
	glUniform2fv (screen_center_uniform, 1, screen_center_.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform1i  (render_overlay_uniform, GL_FALSE);

	const auto& frame = tiling_.frame();

	glBindVertexArray(frame.vao_);
	glDrawArraysInstanced(GL_LINES, 0, frame.num_vertices_, num_instances);

	glUniform1i  (instance_num_uniform, tiling_.num_lattice_domains());
	glUniform1i  (render_overlay_uniform, GL_TRUE);

	glBindVertexArray(overlay.vao_);
	glDrawArraysInstanced(overlay.primitive_type_, 0, overlay.num_vertices_, tiling_.num_lattice_domains());

	// Clean up.
	glBindVertexArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_export_frame(int width, int height, GLuint framebuffer)
{
	static auto shader = GL::ShaderProgram(
		GL::ShaderObject::vertex_passthrough(),
		GL::ShaderObject::from_file(GL_FRAGMENT_SHADER, "shaders/crop_frag.glsl")
	);

	// Find uniform locations once.
	static GLuint screen_size_uniform;
	static GLuint stripe_size_uniform;
	static GLuint vertical_crop_uniform;
	static bool init = [&](){
		screen_size_uniform   = glGetUniformLocation(shader, "uScreenSize");
		stripe_size_uniform   = glGetUniformLocation(shader, "uStripeSize");
		vertical_crop_uniform = glGetUniformLocation(shader, "uVerticalCrop");
		return true;
	}();
	(void)init; // Suppress unused variable warning.

	float AR = width / (float)height;
	float crop_AR = export_width_ / (float)export_height_;

	// stripe_size is the width or height of the visible region.
	// In the other direction the region extends through the entire window.
	float stripe_size;
	if (crop_AR > AR)
		stripe_size = width / crop_AR;
	else
		stripe_size = height * crop_AR;

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, width, height);

	glUseProgram(shader);

	glUniform2i (screen_size_uniform, width, height);
	glUniform1f (stripe_size_uniform, stripe_size);
	glUniform1i (vertical_crop_uniform, crop_AR > AR);

	glBindVertexArray(canvas_.vao_);
	glDrawArrays(canvas_.primitive_type_, 0, canvas_.num_vertices_);

	// Clean up.
	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void App::render_gui(int width, int height, GLuint framebuffer)
{
	static bool show_usage = true;

	// We need these for positioning the windows.
	static float main_menu_height;
	static float usage_window_height;
	auto& io = ImGui::GetIO();

	gui_.new_frame();

	// Main menu.
	if (ImGui::BeginMainMenuBar())
	{
		main_menu_height = ImGui::GetWindowSize().y;
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Show settings", "Esc", show_settings_))
				show_settings_ ^= true;

			if (ImGui::MenuItem("Show usage", NULL, show_usage))
				show_usage ^= true;

			if (ImGui::MenuItem("Show export settings", NULL, show_export_settings_))
				show_export_settings_ ^= true;

			if (ImGui::MenuItem("Quit", "Alt+F4"))
				glfwSetWindowShouldClose(window_, GLFW_TRUE);

			ImGui::EndMenu();
		}

		// if (ImGui::BeginMenu("View"))
		// {
		// 	if (ImGui::MenuItem("Show usage", NULL, show_usage))
		// 		show_usage ^= true;

		// 	if (ImGui::MenuItem("Show settings", "Esc", show_settings_))
		// 		show_settings_ ^= true;

		// 	ImGui::EndMenu();
		// }
		ImGui::EndMainMenuBar();
	}

	// Usage window.
	if (show_usage)
	{
		auto flags = ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
		ImGui::SetNextWindowSize({350, 0}, ImGuiSetCond_Appearing);
		ImGui::SetNextWindowPos({io.DisplaySize.x - 350, main_menu_height}, ImGuiSetCond_Appearing);
		if (ImGui::Begin("Usage", &show_usage, flags))
		{
			ImGui::Bullet();
			ImGui::TextWrapped("Drag and drop the PNG image to symmetrify in this window.");
			ImGui::Bullet();
			ImGui::TextWrapped("Drag to move the frame.");
			ImGui::Bullet();
			ImGui::TextWrapped("Shift + drag to move the view.");
			ImGui::Bullet();
			ImGui::TextWrapped("Right drag to rotate the frame.");
			ImGui::Bullet();
			ImGui::TextWrapped("Scroll to scale the frame.");
			ImGui::Bullet();
			ImGui::TextWrapped("Shift + scroll to zoom.");
			usage_window_height = ImGui::GetWindowSize().y;
		}
		ImGui::End();
	}

	// Settings window.
	if (show_settings_)
	{
		auto flags = 0;
		ImGui::SetNextWindowSize({335, 0}, ImGuiSetCond_Once);
		ImGui::SetNextWindowPos({0, main_menu_height}, ImGuiSetCond_Once);
		if (ImGui::Begin("Settings", &show_settings_, flags))
		{
			// show_symmetry_groups();
			show_symmetry_buttons();

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			show_frame_settings();

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			show_view_settings();

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			// show_export_settings();

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
		}
		ImGui::End();
	}

	if (show_export_settings_)
	{
		auto flags = ImGuiWindowFlags_ShowBorders;
		ImGui::SetNextWindowSize({350, 0}, ImGuiSetCond_Once);
		ImGui::SetNextWindowPos({io.DisplaySize.x - 350, usage_window_height + main_menu_height}, ImGuiSetCond_Once);
		if (ImGui::Begin("Export settings", &show_export_settings_, flags))
		{
			show_export_settings();
		}
		ImGui::End();
	}

	gui_.render(width, height, framebuffer);
}

void App::show_symmetry_buttons(void)
{
	ImGui::Text("Choose the symmetry group");
	ImGui::Separator();

	auto current_group = tiling_.symmetry_group();

	ImGui::Text("Current:"); ImGui::SameLine(148); ImGui::Text(current_group);
	ImGui::Dummy({0, 0}); ImGui::SameLine(100);
	ImGui::PushID("Group choice");
	if (ImGui::ImageButton((ImTextureID)(GLuint)thumbnail_map_[current_group], {120, 120}, {0, 1}, {1, 0}, 5))
		ImGui::OpenPopup("Choose a symmetry group");

	auto modal_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
	if (ImGui::BeginPopupModal("Choose a symmetry group", NULL, modal_flags))
	{
		modal_symmetry_buttons_1();
		ImGui::EndPopup();
	}
	ImGui::PopID();
}

void App::show_symmetry_groups(void)
{
	ImGui::Text("Symmetry groups");
	ImGui::Separator();

	ImGui::Dummy({0, 0});                   ImGui::SameLine(95);
	ImGui::Text("No reflections");          ImGui::SameLine(215);
	ImGui::Text("Reflections");
	ImGui::Spacing();

	ImGui::PushTextWrapPos(80.0f);
	ImGui::TextWrapped("No rotations");     ImGui::SameLine(95);
	ImGui::PopTextWrapPos();

	ImGui::BeginGroup();
	if (ImGui::Selectable("o", !strncmp(tiling_.symmetry_group(), "o", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("o");
	ImGui::SameLine(25); ImGui::Text("(p1)");
	if (ImGui::Selectable("xx", !strncmp(tiling_.symmetry_group(), "xx", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("xx");
	ImGui::SameLine(25); ImGui::Text("(pg)");
	ImGui::EndGroup();                      ImGui::SameLine(215);

	ImGui::BeginGroup();
	if (ImGui::Selectable("**", !strncmp(tiling_.symmetry_group(), "**", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("**");
	ImGui::SameLine(25); ImGui::Text("(pm)");
	if (ImGui::Selectable("*x", !strncmp(tiling_.symmetry_group(), "*x", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("*x");
	ImGui::SameLine(25); ImGui::Text("(cm)");
	ImGui::EndGroup();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::PushTextWrapPos(80.0f);
	ImGui::TextWrapped("2-fold rotations"); ImGui::SameLine(95);
	ImGui::PopTextWrapPos();

	ImGui::BeginGroup();
	if (ImGui::Selectable("2222", !strncmp(tiling_.symmetry_group(), "2222", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("2222");
	ImGui::SameLine(45); ImGui::Text("(p2)");
	if (ImGui::Selectable("22x", !strncmp(tiling_.symmetry_group(), "22x", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("22x");
	ImGui::SameLine(45); ImGui::Text("(pgg)");
	ImGui::EndGroup();                      ImGui::SameLine(215);

	ImGui::BeginGroup();
	if (ImGui::Selectable("*2222", !strncmp(tiling_.symmetry_group(), "*2222", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("*2222");
	ImGui::SameLine(55); ImGui::Text("(pmm)");
	if (ImGui::Selectable("2*22", !strncmp(tiling_.symmetry_group(), "2*22", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("2*22");
	ImGui::SameLine(55); ImGui::Text("(cmm)");
	if (ImGui::Selectable("22*", !strncmp(tiling_.symmetry_group(), "22*", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("22*");
	ImGui::SameLine(55); ImGui::Text("(pmg)");
	ImGui::EndGroup();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::PushTextWrapPos(80.0f);
	ImGui::TextWrapped("3-fold rotations"); ImGui::SameLine(95);
	ImGui::PopTextWrapPos();

	ImGui::BeginGroup();
	if (ImGui::Selectable("333", !strncmp(tiling_.symmetry_group(), "333", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("333");
	ImGui::SameLine(35); ImGui::Text("(p3)");
	ImGui::EndGroup();                      ImGui::SameLine(215);

	ImGui::BeginGroup();
	if (ImGui::Selectable("*333", !strncmp(tiling_.symmetry_group(), "*333", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("*333");
	ImGui::SameLine(45); ImGui::Text("(p3m1)");
	if (ImGui::Selectable("3*3", !strncmp(tiling_.symmetry_group(), "3*3", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("3*3");
	ImGui::SameLine(45); ImGui::Text("(p31m)");
	ImGui::EndGroup();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::PushTextWrapPos(80.0f);
	ImGui::TextWrapped("4-fold rotations"); ImGui::SameLine(95);
	ImGui::PopTextWrapPos();

	ImGui::BeginGroup();
	if (ImGui::Selectable("442", !strncmp(tiling_.symmetry_group(), "442", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("442");
	ImGui::SameLine(35); ImGui::Text("(p4)");
	ImGui::EndGroup();                      ImGui::SameLine(215);

	ImGui::BeginGroup();
	if (ImGui::Selectable("*442", !strncmp(tiling_.symmetry_group(), "*442", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("*442");
	ImGui::SameLine(45); ImGui::Text("(p4m)");
	if (ImGui::Selectable("4*2", !strncmp(tiling_.symmetry_group(), "4*2", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("4*2");
	ImGui::SameLine(45); ImGui::Text("(p4g)");
	ImGui::EndGroup();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::PushTextWrapPos(80.0f);
	ImGui::TextWrapped("6-fold rotations"); ImGui::SameLine(95);
	ImGui::PopTextWrapPos();

	ImGui::BeginGroup();
	if (ImGui::Selectable("632", !strncmp(tiling_.symmetry_group(), "632", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("632");
	ImGui::SameLine(35); ImGui::Text("(p6)");
	ImGui::EndGroup();                      ImGui::SameLine(215);

	ImGui::BeginGroup();
	if (ImGui::Selectable("*632", !strncmp(tiling_.symmetry_group(), "*632", 8), 0, {110, 0}))
		tiling_.set_symmetry_group("*632");
	ImGui::SameLine(45); ImGui::Text("(p6m)");
	ImGui::EndGroup();
}

void App::modal_symmetry_buttons_1(void)
{
	bool modal_should_close = false;

	auto create_button_group = [&](const char* symmetry_group, int label_offset = 60)
	{
		ImGui::BeginGroup();
		ImGui::Dummy({0, 0}); ImGui::SameLine(label_offset);
		ImGui::Text(symmetry_group);
		ImGui::PushID(symmetry_group);
		if (ImGui::ImageButton((ImTextureID)(GLuint)thumbnail_map_[symmetry_group], {120, 120}, {0, 1}, {1, 0}, 10))
		{
			tiling_.set_symmetry_group(symmetry_group);
			modal_should_close = true;
		}
		ImGui::PopID();
		ImGui::EndGroup();
	};

	ImGui::PushID("modal buttons");

	// No rotations.
	create_button_group("o");
	ImGui::SameLine();
	create_button_group("xx");
	ImGui::SameLine();
	create_button_group("*x");
	ImGui::SameLine();
	create_button_group("**");

	// 2-fold rotations.
	create_button_group("2222", 45);
	ImGui::SameLine();
	create_button_group("22x", 45);
	ImGui::SameLine();
	create_button_group("22*", 45);
	ImGui::SameLine();
	create_button_group("2*22", 45);
	ImGui::SameLine();
	create_button_group("*2222", 45);

	// 3-fold rotations.
	create_button_group("333", 50);
	ImGui::SameLine();
	create_button_group("3*3", 50);
	ImGui::SameLine();
	create_button_group("*333", 50);

	// 4-fold rotations.
	create_button_group("442", 50);
	ImGui::SameLine();
	create_button_group("4*2", 50);
	ImGui::SameLine();
	create_button_group("*442", 50);

	// 6-fold rotations.
	create_button_group("632", 50);
	ImGui::SameLine();
	create_button_group("*632", 50);
	ImGui::SameLine();

	ImGui::PopID();

	if (modal_should_close)
		ImGui::CloseCurrentPopup();
}

void App::show_view_settings(void)
{
	ImGui::Text("View settings");
	ImGui::Separator();

	ImGui::Text("Show result:"); ImGui::SameLine(140);
	ImGui::Checkbox("##Show result", &show_result_);
	ImGui::SameLine(0, 12);
	ImGui::TextDisabled("(Spacebar)");

	ImGui::Text("Screen center:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-65.0f);
	ImGui::DragFloat2("##Screen center", screen_center_.data(), 0.01f);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if(ImGui::Button("Reset##Reset screen center"))
		screen_center_ = {0.5, 0.5};

	// We need a float, not a double.
	float pixels_per_unit = pixels_per_unit_;
	ImGui::Text("Zoom level:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragFloat("##Zoom level", &pixels_per_unit))
		pixels_per_unit_ = pixels_per_unit;
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 12);
	if (ImGui::Button("Reset##Reset zoom level"))
		pixels_per_unit_ = 300.0;

	ImGui::Text("Background:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-1.0f);
	ImGui::ColorEdit3("##Background color", clear_color_.data());
	ImGui::PopItemWidth();
	ImGui::Dummy({0, 0}); ImGui::SameLine(140);
	if (ImGui::Button("Reset color##Reset background color"))
		clear_color_ = {0.1, 0.1, 0.1};
	ImGui::SameLine();
	// ImGui::Button("Pick color...");
	// if (ImGui::IsItemHovered())
	// {
	// 	ImGui::BeginTooltip();
	// 	ImGui::Text("Not implemented yet :)");
	// 	ImGui::EndTooltip();
	// }
}

void App::show_frame_settings(void)
{
	ImGui::Text("Frame settings");
	ImGui::Separator();

	ImGui::Text("Show frame:"); ImGui::SameLine(140);
	ImGui::Checkbox("##Show frame", &show_symmetry_frame_);
	ImGui::SameLine(0, 12); ImGui::TextDisabled("(Ctrl + Spacebar)");

	auto frame_position = tiling_.center();
	ImGui::Text("Frame position:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragFloat2("##Frame position", frame_position.data(), 0.01f))
		tiling_.set_center(frame_position);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Button("Reset##Reset frame position"))
		tiling_.set_center({0.5, 0.5});

	float frame_rotation = tiling_.rotation() / M_PI * 180.0f;
	ImGui::Text("Frame rotation:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragFloat("##Frame rotation", &frame_rotation, 0.5f))
		tiling_.set_rotation(frame_rotation / 180.0 * M_PI);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 12);
	if (ImGui::Button("Reset##Reset frame rotation"))
		tiling_.set_rotation(0.0);

	float frame_scale = tiling_.scale();
	ImGui::Text("Frame scale:"); ImGui::SameLine(140);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragFloat("##Frame scale", &frame_scale, 0.01f, 0.001f, FLT_MAX))
		tiling_.set_scale(frame_scale);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 12);
	if (ImGui::Button("Reset##Reset frame scale"))
		tiling_.set_scale(2.0);

	// int num_domains = tiling_.num_lattice_domains();
	// bool domains_changed = false;
	// ImGui::Text("Domains:"); ImGui::SameLine(140);
	// domains_changed |= ImGui::RadioButton("1##Domains 1", &num_domains, 1); ImGui::SameLine();
	// domains_changed |= ImGui::RadioButton("4##Domains 2", &num_domains, 4); ImGui::SameLine();
	// domains_changed |= ImGui::RadioButton("9##Domains 3", &num_domains, 9); ImGui::SameLine();
	// if (domains_changed)
	// 	tiling_.set_num_lattice_domains(num_domains);
}

void App::show_export_settings(void)
{
	ImGui::Text("Export settings");
	ImGui::Separator();

	int resolution[] = {export_width_, export_height_};
	ImGui::Text("Resolution:"); ImGui::SameLine(120);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::DragInt2("##Resolution", resolution, 1.0f, 512, 4096))
	{
		export_width_  = std::max(512, std::min(resolution[0], 4096));
		export_height_ = std::max(512, std::min(resolution[1], 4096));
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Button("Reset##Reset resolution"))
	{
		export_width_  = 1600;
		export_height_ = 1200;
	}
	ImGui::Dummy({0, 0}); ImGui::SameLine(120);
	if (ImGui::Button("Fit to window"))
	{
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);
		export_width_  = width;
		export_height_ = height;
	}

	char buffer[256] = {'\0'};
	std::strncpy(buffer, export_filename_.c_str(), 255);
	ImGui::Text("Export as:"); ImGui::SameLine(120);
	ImGui::PushItemWidth(-65.0f);
	if (ImGui::InputText("##Filename", buffer, 256, ImGuiInputTextFlags_CharsNoBlank))
		export_filename_ = buffer;
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 12);
	if (ImGui::Button("Reset##Reset filename"))
		export_filename_ = export_base_name_ + '_' + tiling_.symmetry_group() + ".png";

	bool ready_to_export = false;
	ImGui::Dummy({0, 0}); ImGui::SameLine(120);
	if (ImGui::Button("Export"))
	{
		if (export_filename_.empty())
			ImGui::OpenPopup("No filename");
		else
			ready_to_export = true;
	}
	auto modal_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
	if (ImGui::BeginPopupModal("No filename", NULL, modal_flags))
	{
		ImVec2 button_size = {140, 0.0f};
		auto default_name = export_base_name_ + '_' + tiling_.symmetry_group() + ".png";

		ImGui::Text("No filename set.");
		ImGui::Text("Use the default name \"%s\"?", default_name.c_str());
		if (ImGui::Button("OK##Export OK", button_size))
		{
			export_filename_ = default_name;
			ready_to_export = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel##Export cancel", button_size))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
	if (ready_to_export)
		export_result();
}

void App::position_callback(double x, double y)
{
	// Don't do anything if ImGui is grabbing input.
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			int width, height;
			glfwGetFramebufferSize(window_, &width, &height);
			Eigen::Vector2f position = {x / width * 2 - 1, 1 - y / height * 2};
			const auto& drag_position = position - press_position_;

			if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				screen_center_ = screen_center_static_position_ - screen_to_world(drag_position);
			else if (glfwGetKey(window_, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
				tiling_.deform(screen_to_world(drag_position));
			else
				tiling_.set_position(tiling_static_position_ + screen_to_world(drag_position));
		}
		else if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			// if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			if (true)
			{
				int width, height;
				glfwGetFramebufferSize(window_, &width, &height);
				Eigen::Vector2f position = {x / width * 2 - 1, 1 - y / height * 2};

				Eigen::Vector2f world_press_position = screen_center_ + screen_to_world(press_position_);
				Eigen::Vector2f world_position       = screen_center_ + screen_to_world(position);

				// This doesn't change during rotation - could be cached if deemed necessary.
				Eigen::Vector2f press_wrt_center    = world_press_position - tiling_.center();
				Eigen::Vector2f position_wrt_center = world_position       - tiling_.center();

				double det = (Eigen::Matrix2f() << press_wrt_center, position_wrt_center).finished().determinant();
				double dot = press_wrt_center.dot(position_wrt_center);
				double drag_rotation = std::atan2(det, dot);

				tiling_.set_rotation(tiling_static_rotation_ + drag_rotation);
			}
		}
	}
}

void App::left_click_callback(int action, int /* mods */)
{
	if (action == GLFW_PRESS)
	{
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		double x, y;
		glfwGetCursorPos(window_, &x, &y);

		press_position_ = {x / width * 2 - 1, 1 - y / height * 2};

		screen_center_static_position_ = screen_center_;
		tiling_static_position_        = tiling_.position();

		tiling_.set_deform_origin(screen_center_ + screen_to_world(press_position_));
	}
}

void App::right_click_callback(int action, int /* mods */)
{
	if (action == GLFW_PRESS)
	{
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		double x, y;
		glfwGetCursorPos(window_, &x, &y);

		press_position_ = {x / width * 2 - 1, 1 - y / height * 2};
		tiling_static_rotation_ = tiling_.rotation();
	}
}

void App::scroll_callback(double /* x_offset */, double y_offset)
{
	// Don't do anything if ImGui is grabbing input.
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			if (y_offset > 0)
				pixels_per_unit_ *=  zoom_factor_;
			else if (y_offset < 0)
				pixels_per_unit_ /=  zoom_factor_;
		}
		else
		{
			if (y_offset < 0)
				tiling_.multiply_scale(zoom_factor_);
			else if (y_offset > 0 && tiling_.scale() > 0.001)
				tiling_.multiply_scale(1 / zoom_factor_);
		}
	}
}

// TODO: Add support to larger exports rendered in smaller tiles.
void App::export_result(void)
{
	printf("Exporting...\n");

	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	auto texture = GL::Texture::empty_2D(export_width_, export_height_);
	auto depth   = GL::Texture::empty_2D_depth(export_width_, export_height_);
	auto fbo     = GL::FBO::simple_C0D(texture, depth);

	// We don't want transparency in the resulting PNG.
	// Thus we set the clear color alpha to 1 and change our blending function
	// to prefer destination alpha (this is the clear color alpha, i.e. 1).
	glClearColor(clear_color_.x(), clear_color_.y(), clear_color_.z(), 1);
	GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, fbo);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

	// We want to keep the zoom level irrespective of resolution chosen.
	double ppu_old = pixels_per_unit_;
	pixels_per_unit_ = std::max( export_width_ / (float)width, export_height_ / (float)height) * ppu_old;

	render_tiling_hq(export_width_, export_height_, fbo);

	pixels_per_unit_ = ppu_old;

	// Reset the blending function.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GL::tex_to_png(texture, export_filename_.c_str());
	printf("Export finished (%s)\n", export_filename_.c_str());
}

void App::print_screen(int /* scancode */, int action, int /* mods */)
{
	if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard)
	{
		printf("Taking screenshot...\n");

		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		// TODO: Threading.
		auto texture = GL::Texture::empty_2D(width, height);
		auto depth   = GL::Texture::empty_2D_depth(width, height);
		auto fbo     = GL::FBO::simple_C0D(texture, depth);

		// We don't want transparency in the resulting PNG.
		// Thus we set the clear color alpha to 1 and change our blending function
		// to prefer destination alpha (this is the clear color alpha, i.e. 1).
		glClearColor(clear_color_.x(), clear_color_.y(), clear_color_.z(), 1);
		GL::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, fbo);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

		render_scene(width, height, fbo);

		// Reset the blending function.
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// TODO: Query screenshot name from user.
		GL::tex_to_png(texture, "screenshot.png");

		printf("Screenshot saved. (screenshot.png)\n");
	}
}

void App::load_texture(const char* filename)
{
	tiling_.set_inconsistent();
	base_image_ = GL::Texture::from_png(filename);

	// We'll use nearest neighbor filtering.
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, base_image_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, old_tex);
}

void App::populate_thumbnail_map(void)
{
	thumbnail_map_["o"]     = GL::Texture::from_png("res/thumbnails/o");
	thumbnail_map_["xx"]    = GL::Texture::from_png("res/thumbnails/xx");
	thumbnail_map_["*x"]    = GL::Texture::from_png("res/thumbnails/_x");
	thumbnail_map_["**"]    = GL::Texture::from_png("res/thumbnails/__");

	thumbnail_map_["2222"]  = GL::Texture::from_png("res/thumbnails/2222");
	thumbnail_map_["22x"]   = GL::Texture::from_png("res/thumbnails/22x");
	thumbnail_map_["22*"]   = GL::Texture::from_png("res/thumbnails/22_");
	thumbnail_map_["2*22"]  = GL::Texture::from_png("res/thumbnails/2_22");
	thumbnail_map_["*2222"] = GL::Texture::from_png("res/thumbnails/_2222");

	thumbnail_map_["333"]   = GL::Texture::from_png("res/thumbnails/333");
	thumbnail_map_["3*3"]   = GL::Texture::from_png("res/thumbnails/3_3");
	thumbnail_map_["*333"]  = GL::Texture::from_png("res/thumbnails/_333");

	thumbnail_map_["442"]   = GL::Texture::from_png("res/thumbnails/442");
	thumbnail_map_["4*2"]   = GL::Texture::from_png("res/thumbnails/4_2");
	thumbnail_map_["*442"]  = GL::Texture::from_png("res/thumbnails/_442");

	thumbnail_map_["632"]   = GL::Texture::from_png("res/thumbnails/632");
	thumbnail_map_["*632"]  = GL::Texture::from_png("res/thumbnails/_632");
}

// TODO: Figure out where this should go.
Eigen::Vector2f App::screen_to_world(const Eigen::Vector2f& v)
{
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	return { v.x() * (0.5 * width) / pixels_per_unit_,
	         v.y() * (0.5 * height) / pixels_per_unit_ };
}

//--------------------
