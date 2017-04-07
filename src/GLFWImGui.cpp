#include "GLFWImGui.h"

#include "Window.h"
#include "imgui.h"

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))


GLFWImGui::GLFWImGui(MainWindow& window) :
	window_                  (window),
	shader_                  (GL::ShaderProgram::from_files(
		                          "shaders/gui_vert.glsl",
		                          "shaders/gui_frag.glsl")),
	display_size_uniform_    (glGetUniformLocation(shader_, "uDisplaySize")),
	texture_sampler_uniform_ (glGetUniformLocation(shader_, "uTextureSampler")),
	time_                    (glfwGetTime()),
	left_clicked_            (false),
	right_clicked_           (false),
	middle_clicked_          (false),
	mouse_wheel_             (0.0)
{
	auto& io = ImGui::GetIO();

	io.KeyMap[ImGuiKey_Tab]        = GLFW_KEY_TAB;
	io.KeyMap[ImGuiKey_LeftArrow]  = GLFW_KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow]    = GLFW_KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow]  = GLFW_KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp]     = GLFW_KEY_PAGE_UP;
	io.KeyMap[ImGuiKey_PageDown]   = GLFW_KEY_PAGE_DOWN;
	io.KeyMap[ImGuiKey_Home]       = GLFW_KEY_HOME;
	io.KeyMap[ImGuiKey_End]        = GLFW_KEY_END;
	io.KeyMap[ImGuiKey_Delete]     = GLFW_KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace]  = GLFW_KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Enter]      = GLFW_KEY_ENTER;
	io.KeyMap[ImGuiKey_Escape]     = GLFW_KEY_ESCAPE;
	io.KeyMap[ImGuiKey_A]          = GLFW_KEY_A;
	io.KeyMap[ImGuiKey_C]          = GLFW_KEY_C;
	io.KeyMap[ImGuiKey_V]          = GLFW_KEY_V;
	io.KeyMap[ImGuiKey_X]          = GLFW_KEY_X;
	io.KeyMap[ImGuiKey_Y]          = GLFW_KEY_Y;
	io.KeyMap[ImGuiKey_Z]          = GLFW_KEY_Z;

	window_.add_mouse_button_callback(GLFW_MOUSE_BUTTON_LEFT, [this](int action, int){
		if (action == GLFW_PRESS)
			this->left_clicked_ = true;
	});
	window_.add_mouse_button_callback(GLFW_MOUSE_BUTTON_RIGHT, [this](int action, int){
		if (action == GLFW_PRESS)
			this->right_clicked_ = true;
	});
	window_.add_mouse_button_callback(GLFW_MOUSE_BUTTON_MIDDLE, [this](int action, int){
		if (action == GLFW_PRESS)
			this->middle_clicked_ = true;
	});

	window_.add_scroll_callback([this](double /* x_offset */, double y_offset){
		this->mouse_wheel_ += (float)y_offset;
	});

	window_.add_key_callback(&GLFWImGui::key_callback, this);
	window_.add_char_callback(&GLFWImGui::char_callback, this);

	create_fonts_texture();

	io.RenderDrawListsFn = NULL;
	io.SetClipboardTextFn = &GLFWImGui::set_clipboard_text;
	io.GetClipboardTextFn = &GLFWImGui::get_clipboard_text;
	io.ClipboardUserData = (GLFWwindow*)window_;

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

GLFWImGui::~GLFWImGui(void)
{
	ImGui::GetIO().Fonts->TexID = 0;
	ImGui::Shutdown();
}

void GLFWImGui::new_frame(void)
{
	auto& io = ImGui::GetIO();

	int win_width, win_height;
	int fb_width, fb_height;
	glfwGetWindowSize(window_, &win_width, &win_height);
	glfwGetFramebufferSize(window_, &fb_width, &fb_height);

	float scale_width  = win_width  > 0 ? fb_width  / (float)win_width  : 0;
	float scale_height = win_height > 0 ? fb_height / (float)win_height : 0;

	io.DisplaySize             = {(float)win_width, (float)win_height};
	io.DisplayFramebufferScale = {scale_width, scale_height};

	auto current_time = glfwGetTime();
	io.DeltaTime      = (float)(current_time - time_);
	time_             = current_time;

	if (glfwGetWindowAttrib(window_, GLFW_FOCUSED))
	{
		double mouse_x, mouse_y;
		glfwGetCursorPos(window_, &mouse_x, &mouse_y);
		io.MousePos = {(float)mouse_x, (float)mouse_y};
	}
	else
		io.MousePos = {-1, -1};

	io.MouseDown[GLFW_MOUSE_BUTTON_LEFT]   = left_clicked_   || glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT);
	io.MouseDown[GLFW_MOUSE_BUTTON_RIGHT]  = right_clicked_  || glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT);
	io.MouseDown[GLFW_MOUSE_BUTTON_MIDDLE] = middle_clicked_ || glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_MIDDLE);
	left_clicked_ = right_clicked_ = middle_clicked_ = false;

	io.MouseWheel = mouse_wheel_;
	mouse_wheel_ = 0.0;

	glfwSetInputMode(window_, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

	ImGui::NewFrame();
}

void GLFWImGui::render(int width, int height, GLuint framebuffer)
{
	auto& io = ImGui::GetIO();

	int fb_width  = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if (fb_width == 0 || fb_height == 0)
		return;

	ImGui::Render();
	auto draw_data = ImGui::GetDrawData();

	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// Save previous state.
	GLint     old_fbo;                   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	GLint     old_active;                glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	GLint     old_tex;
	GLboolean old_blend_enabled        = glIsEnabled(GL_BLEND);
	GLint     old_blend_eq_rgb;          glGetIntegerv(GL_BLEND_EQUATION_RGB, &old_blend_eq_rgb);
	GLint     old_blend_eq_alpha;        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &old_blend_eq_alpha);
	GLint     old_blend_func_src;        glGetIntegerv(GL_BLEND_SRC, &old_blend_func_src);
	GLint     old_blend_func_dst;        glGetIntegerv(GL_BLEND_DST, &old_blend_func_dst);
	GLboolean old_cull_face_enabled    = glIsEnabled(GL_CULL_FACE);
	GLboolean old_depth_test_enabled   = glIsEnabled(GL_DEPTH_TEST);
	GLboolean old_scissor_test_enabled = glIsEnabled(GL_SCISSOR_TEST);
	GLint     old_scissor_box[4];        glGetIntegerv(GL_SCISSOR_BOX, old_scissor_box);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glActiveTexture(GL_TEXTURE0);

	glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);

	if (framebuffer != 0)
		glViewport(0, 0, width, height);
	else
		glViewport(0, 0, fb_width, fb_height);

	glUseProgram(shader_);

	glUniform2f(display_size_uniform_, io.DisplaySize.x, io.DisplaySize.y);
	glUniform1i(texture_sampler_uniform_, 0);

	glBindVertexArray(vao_);

	for (int n = 0; n < draw_data->CmdListsCount; ++n)
	{
		const auto       cmd_list          = draw_data->CmdLists[n];
		const ImDrawIdx* idx_buffer_offset = 0;

		glBindBuffer(GL_ARRAY_BUFFER, vbo_);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ImDrawVert) * cmd_list->VtxBuffer.Size, cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ImDrawIdx) * cmd_list->IdxBuffer.Size, cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
		{
			const auto pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
				pcmd->UserCallback(cmd_list, pcmd);
			else
			{
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
			}
			idx_buffer_offset += pcmd->ElemCount;
		}
	}

	// Clean up.
	glBindVertexArray(0);
	glUseProgram(0);

	glScissor(old_scissor_box[0], old_scissor_box[1], old_scissor_box[2], old_scissor_box[3]);
    if (old_scissor_test_enabled) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    if (old_depth_test_enabled) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (old_cull_face_enabled) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	glBlendFunc(old_blend_func_src, old_blend_func_dst);
	glBlendEquationSeparate(old_blend_eq_rgb, old_blend_eq_alpha);
	if (old_blend_enabled) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void GLFWImGui::create_fonts_texture(void)
{
	auto& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;

	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, fonts_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, old_tex);

	fonts_texture_.width_  = width;
	fonts_texture_.height_ = height;

	io.Fonts->TexID = (void*)(intptr_t)(GLuint)fonts_texture_;
}

void GLFWImGui::key_callback(int key, int /* scancode */, int action, int mods)
{
	auto& io = ImGui::GetIO();
	if (action == GLFW_PRESS)
		io.KeysDown[key] = true;
	if (action == GLFW_RELEASE)
		io.KeysDown[key] = false;

	(void)mods; // I heard mods aren't reliable across systems.
	io.KeyCtrl  = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT]   || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt   = io.KeysDown[GLFW_KEY_LEFT_ALT]     || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER]   || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void GLFWImGui::char_callback(unsigned int c)
{
	auto& io = ImGui::GetIO();
	if (c > 0 && c < 0x10000)
		io.AddInputCharacter((unsigned short)c);
}

const char* GLFWImGui::get_clipboard_text(void* user_data)
{
	return glfwGetClipboardString((GLFWwindow*)user_data);
}

void GLFWImGui::set_clipboard_text(void* user_data, const char* text)
{
	glfwSetClipboardString((GLFWwindow*)user_data, text);
}
