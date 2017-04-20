#ifndef GLFWIMGUI_H
#define GLFWIMGUI_H

#include "GLObjects.h"

class MainWindow;

class GLFWImGui
{
public:
	GLFWImGui  (MainWindow&);
	~GLFWImGui (void);

	void new_frame            (void);
	void render               (int width = -1, int height = -1, GLuint framebuffer = 0);
	void create_fonts_texture (void);
private:
	void key_callback  (int key, int scancode, int action, int mods);
	void char_callback (unsigned int c);

	MainWindow& window_;

	GL::VAO     vao_;
	GL::Buffer  vbo_;
	GL::Buffer  ebo_;
	GL::Texture fonts_texture_;

	GL::ShaderProgram shader_;
	GLuint            display_size_uniform_;
	GLuint            texture_sampler_uniform_;

	double time_;
	bool   left_clicked_, right_clicked_, middle_clicked_;
	float  mouse_wheel_;

	static const char* get_clipboard_text(void* user_data);
	static void        set_clipboard_text(void* user_data, const char* text);
};
#endif // GLFWIMGUI_H
