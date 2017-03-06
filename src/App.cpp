#include "App.h"

#include "GLFunctions.h"
#include "GLUtils.h"
#include "Examples.h"
#include "imgui.h"
#include <cstdio>

//--------------------

App::App(int argc, char* argv[])
:	window_                (1440, 900, "supersymmetry"),
	time_                  ( (glfwSetTime(0), glfwGetTime()) ),
	gui_                   (window_),
	symmetrifying_         (false),
	show_settings_         (true),
	screen_center_         (0.5, 0.5),
	pixels_per_unit_       (700.0),                           // Initial zoom level.
	zoom_factor_           (1.2)
{
	// Suppress unused parameter warnings.
	(void)argc;
	(void)argv;

	// Mouse callbacks.
	window_.add_mouse_pos_callback(&App::position_callback, this);
	window_.add_mouse_button_callback(GLFW_MOUSE_BUTTON_LEFT, &App::left_click_callback, this);
	window_.add_mouse_button_callback(GLFW_MOUSE_BUTTON_RIGHT, &App::right_click_callback, this);
	window_.add_scroll_callback(&App::scroll_callback, this);

	// Key callbacks.
	window_.add_key_callback(GLFW_KEY_P, &App::print_screen, this);
	window_.add_key_callback(GLFW_KEY_SPACE, [this](int, int action, int){
		if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard)
			this->symmetrifying_ ^= true;
	});
	window_.add_key_callback(GLFW_KEY_ESCAPE, [this](int, int action, int){
		if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard)
			this->show_settings_ ^= true;
	});
	window_.add_key_callback(GLFW_KEY_1, [this](int, int action, int){
		if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard)
			this->tiling_.set_num_domains(1);
	});
	window_.add_key_callback(GLFW_KEY_2, [this](int, int action, int){
		if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard)
			this->tiling_.set_num_domains(4);
	});
	window_.add_key_callback(GLFW_KEY_3, [this](int, int action, int){
		if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard)
			this->tiling_.set_num_domains(9);
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

	// Set default GUI font.
	auto& io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->AddFontFromFileTTF("res/DroidSans.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	gui_.create_fonts_texture();

	load_texture("res/kissa");
	tiling_.set_symmetry_group("333");
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

		render_scene(width, height);

		// Show the result on screen.
		glfwSwapBuffers(window_);

		// Poll events. Minimum FPS = 15.
		glfwWaitEventsTimeout(1 / 15.0);
	}
}

void App::render_scene(int width, int height, GLuint framebuffer)
{
	if (symmetrifying_)
	{
		// Don't symmetrify if already consistent.
		if (!tiling_.consistent())
			tiling_.symmetrify(base_image_);
		render_symmetry_frame(true, width, height, framebuffer);
	}
	else
	{
		render_image(base_image_, width, height, framebuffer);
		render_symmetry_frame(false, width, height, framebuffer);
	}
	render_gui(width, height, framebuffer);
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
	const auto num_instances = symmetrifying ? plane_side_length * plane_side_length : tiling_.num_domains();

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

void App::render_gui(int width, int height, GLuint framebuffer)
{
	static bool show_usage       = false;

	// We need these for positioning the windows.
	float main_menu_height;
	auto& io = ImGui::GetIO();

	gui_.new_frame();

	// Main menu.
	if (ImGui::BeginMainMenuBar())
	{
		main_menu_height = ImGui::GetWindowSize().y;
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Quit", "Alt+F4"))
				glfwSetWindowShouldClose(window_, GLFW_TRUE);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Show usage", NULL, show_usage))
				show_usage ^= true;

			if (ImGui::MenuItem("Show settings", "Esc", show_settings_))
				show_settings_ ^= true;

			ImGui::EndMenu();
		}
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
			ImGui::TextWrapped("Click and drag to move around.");
			ImGui::Bullet();
			ImGui::TextWrapped("Control + drag to move the symmetrification frame.");
			ImGui::Bullet();
			ImGui::TextWrapped("Control + right drag to rotate the symmetrification frame.");
			ImGui::Bullet();
			ImGui::TextWrapped("Scroll to zoom.");
			ImGui::Bullet();
			ImGui::TextWrapped("Control + scroll to resize the symmetrification frame.");
			ImGui::Bullet();
			ImGui::TextWrapped("Spacebar to toggle the symmetrified view.");
		}
		ImGui::End();
	}

	// Settings window.
	if (show_settings_)
	{
		auto flags = 0;
		ImGui::SetNextWindowSize({370, 420}, ImGuiSetCond_Once);
		ImGui::SetNextWindowPos({0, main_menu_height}, ImGuiSetCond_Once);
		if (ImGui::Begin("Settings", &show_settings_, flags))
		{
			ImGui::TextWrapped("Symmetry groups:");
			ImGui::Spacing();

			ImGui::Columns(3, "groups");
			ImGui::Separator();

			ImGui::NextColumn();
			ImGui::TextWrapped("No reflections"); ImGui::NextColumn();
			ImGui::TextWrapped("Reflections"); ImGui::NextColumn();
			ImGui::Separator();

			ImGui::TextWrapped("No rotations"); ImGui::NextColumn();

			if (ImGui::Selectable("o (p1)", !strncmp(tiling_.symmetry_group(), "o", 8)))
				tiling_.set_symmetry_group("o");
			if (ImGui::Selectable("xx (pg)", !strncmp(tiling_.symmetry_group(), "xx", 8)))
				tiling_.set_symmetry_group("xx");
			ImGui::NextColumn();

			if (ImGui::Selectable("** (pm)", !strncmp(tiling_.symmetry_group(), "**", 8)))
				tiling_.set_symmetry_group("**");
			if (ImGui::Selectable("*x (cm)", !strncmp(tiling_.symmetry_group(), "*x", 8)))
				tiling_.set_symmetry_group("*x");
			ImGui::NextColumn();
			ImGui::Separator();

			ImGui::TextWrapped("2-fold rotations"); ImGui::NextColumn();

			if (ImGui::Selectable("2222 (p2)", !strncmp(tiling_.symmetry_group(), "2222", 8)))
				tiling_.set_symmetry_group("2222");
			if (ImGui::Selectable("22x (pgg)", !strncmp(tiling_.symmetry_group(), "22x", 8)))
				tiling_.set_symmetry_group("22x");
			ImGui::NextColumn();

			if (ImGui::Selectable("*2222 (pmm)", !strncmp(tiling_.symmetry_group(), "*2222", 8)))
				tiling_.set_symmetry_group("*2222");
			if (ImGui::Selectable("2*22 (cmm)", !strncmp(tiling_.symmetry_group(), "2*22", 8)))
				tiling_.set_symmetry_group("2*22");
			if (ImGui::Selectable("22* (pmg)", !strncmp(tiling_.symmetry_group(), "22*", 8)))
				tiling_.set_symmetry_group("22*");
			ImGui::NextColumn();
			ImGui::Separator();

			ImGui::TextWrapped("3-fold rotations"); ImGui::NextColumn();

			if (ImGui::Selectable("333 (p3)", !strncmp(tiling_.symmetry_group(), "333", 8)))
				tiling_.set_symmetry_group("333");
			ImGui::NextColumn();

			if (ImGui::Selectable("*333 (p3m1)", !strncmp(tiling_.symmetry_group(), "*333", 8)))
				tiling_.set_symmetry_group("*333");
			if (ImGui::Selectable("3*3 (p31m)", !strncmp(tiling_.symmetry_group(), "3*3", 8)))
				tiling_.set_symmetry_group("3*3");
			ImGui::NextColumn();
			ImGui::Separator();

			ImGui::TextWrapped("4-fold rotations"); ImGui::NextColumn();

			if (ImGui::Selectable("442 (p4)", !strncmp(tiling_.symmetry_group(), "442", 8)))
				tiling_.set_symmetry_group("442");
			ImGui::NextColumn();

			if (ImGui::Selectable("*442 (p4m)", !strncmp(tiling_.symmetry_group(), "*442", 8)))
				tiling_.set_symmetry_group("*442");
			if (ImGui::Selectable("4*2 (p4g)", !strncmp(tiling_.symmetry_group(), "4*2", 8)))
				tiling_.set_symmetry_group("4*2");
			ImGui::NextColumn();
			ImGui::Separator();

			ImGui::TextWrapped("6-fold rotations"); ImGui::NextColumn();

			if (ImGui::Selectable("632 (p6)", !strncmp(tiling_.symmetry_group(), "632", 8)))
				tiling_.set_symmetry_group("632");
			ImGui::NextColumn();

			if (ImGui::Selectable("*632 (p6m)", !strncmp(tiling_.symmetry_group(), "*632", 8)))
				tiling_.set_symmetry_group("*632");
			ImGui::NextColumn();
			ImGui::Separator();
			ImGui::Columns(1);
		}
		ImGui::End();
	}

	gui_.render(width, height, framebuffer);
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

			if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				tiling_.set_position(tiling_static_position_ + screen_to_world(drag_position));
			else
				screen_center_ = screen_center_static_position_ - screen_to_world(drag_position);
		}
		else if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			// TODO: Implement this in a more elegant way.
			if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			{
				int width, height;
				glfwGetFramebufferSize(window_, &width, &height);
				Eigen::Vector2f position = {x / width * 2 - 1, 1 - y / height * 2};

				Eigen::Vector2f world_press = screen_center_ + screen_to_world(press_position_);
				Eigen::Vector2f world_current = screen_center_ + screen_to_world(position);

				Eigen::Vector2f press_vec = world_press - tiling_.position();
				Eigen::Vector2f current_vec = world_current - tiling_.position();
				if (tiling_.num_domains() % 2)
				{
					press_vec   -= (tiling_.t1() + tiling_.t2()) / 2;
					current_vec -= (tiling_.t1() + tiling_.t2()) / 2;
				}

				double det = (Eigen::Matrix2f() << press_vec, current_vec).finished().determinant();
				double dot = press_vec.dot(current_vec);
				double drag_rotation = std::atan2(det, dot);

				tiling_.set_rotation(tiling_static_rotation_ + drag_rotation);
			}
		}
	}
}

void App::left_click_callback(int action, int mods)
{
	(void)mods; // Suppress unused parameter warning.

	if (action == GLFW_PRESS)
	{
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		double x, y;
		glfwGetCursorPos(window_, &x, &y);

		press_position_ = {x / width * 2 - 1, 1 - y / height * 2};

		screen_center_static_position_ = screen_center_;
		tiling_static_position_        = tiling_.position();
	}
}

void App::right_click_callback(int action, int mods)
{
	(void)mods; // Suppress unused parameter warning.

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

void App::scroll_callback(double x_offset, double y_offset)
{
	// Don't do anything if ImGui is grabbing input.
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		(void)x_offset; // Suppress unused parameter warning.

		if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			if (y_offset < 0)
				tiling_.set_scale(zoom_factor_);
			else if (y_offset > 0)
				tiling_.set_scale(1 / zoom_factor_);
		}
		else
		{
			if (y_offset > 0)
				pixels_per_unit_ *=  zoom_factor_;
			else if (y_offset < 0)
				pixels_per_unit_ /=  zoom_factor_;
		}
	}
}

void App::print_screen(int scancode, int action, int mods)
{
	// Suppress unused parameter warnings.
	(void)scancode;
	(void)mods;

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
		glClearColor(0.1, 0.1, 0.1, 1);
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
	base_image_ = GL::Texture::from_png(filename);
	tiling_.set_inconsistent();
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
