#include "App.h"

#include "GLFunctions.h"
#include "GLUtils.h"
#include "Examples.h"
#include "imgui.h"
#include <cstdio>
#include <cstdint>

//--------------------

App::App(int /* argc */, char** /* argv */)
:	window_                (1440, 900, "supersymmetry"),
	time_                  ( (glfwSetTime(0), glfwGetTime()) ),
	gui_                   (window_),
	show_result_           (false),
	show_settings_         (true),
	screen_center_         (0.5, 0.5),
	pixels_per_unit_       (500.0),                           // Initial zoom level.
	zoom_factor_           (1.2)
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
			this->show_result_ ^= true;
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
	tiling_.set_center({0.5, 0.5});
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
	if (show_result_)
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
	glUniform1i  (texture_flag_uniform, show_result_);

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
	static bool show_usage = false;

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
		ImGui::SetNextWindowSize({335, 0}, ImGuiSetCond_Once);
		ImGui::SetNextWindowPos({0, main_menu_height}, ImGuiSetCond_Once);
		if (ImGui::Begin("Settings", &show_settings_, flags))
		{
			// Symmetry groups.
			ImGui::Text("Symmetry groups");
			ImGui::Separator();

			ImGui::Text("");                        ImGui::SameLine(95);
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
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();


			// View settings.
			ImGui::Text("View settings");
			ImGui::Separator();

			ImGui::Text("Show result:"); ImGui::SameLine(130);
			ImGui::Checkbox("##Show result", &show_result_);

			ImGui::Text("Screen center:"); ImGui::SameLine(130);
			ImGui::PushItemWidth(-65.0f);
			ImGui::DragFloat2("##Screen center", screen_center_.data(), 0.01f);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if(ImGui::Button("Reset##Reset screen center"))
				screen_center_ = {0.5, 0.5};

			// We need a float, not a double.
			float pixels_per_unit = pixels_per_unit_;
			ImGui::Text("Zoom level:"); ImGui::SameLine(130);
			ImGui::PushItemWidth(-65.0f);
			if (ImGui::DragFloat("##Zoom level", &pixels_per_unit))
				pixels_per_unit_ = pixels_per_unit;
			ImGui::PopItemWidth();
			ImGui::SameLine(0, 12);
			if(ImGui::Button("Reset##Reset zoom level"))
				pixels_per_unit_ = 700.0;
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();


			// Frame settings.
			ImGui::Text("Frame settings");
			ImGui::Separator();

			// TODO: Really show frame.
			bool show_frame = !show_result_;
			ImGui::Text("Show frame:"); ImGui::SameLine(140);
			if (ImGui::Checkbox("##Show frame", &show_frame))
				show_result_ ^= true;

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
				tiling_.set_scale(1.0);

			int num_domains = tiling_.num_domains();
			bool domains_changed = false;
			ImGui::Text("Domains:"); ImGui::SameLine(140);
			domains_changed |= ImGui::RadioButton("1##Domains 1", &num_domains, 1); ImGui::SameLine();
			domains_changed |= ImGui::RadioButton("4##Domains 2", &num_domains, 4); ImGui::SameLine();
			domains_changed |= ImGui::RadioButton("9##Domains 3", &num_domains, 9); ImGui::SameLine();
			if (domains_changed)
				tiling_.set_num_domains(num_domains);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
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
			if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
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
		if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			if (y_offset < 0)
				tiling_.multiply_scale(zoom_factor_);
			else if (y_offset > 0 && tiling_.scale() > 0.001)
				tiling_.multiply_scale(1 / zoom_factor_);
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
