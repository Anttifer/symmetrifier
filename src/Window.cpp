#include "Window.h"

#include <iostream>

//--------------------

static void error_callback(int error, const char* description) {
	fputs(description, stderr);
	getchar();
}

//--------------------

MainWindow::MainWindow(int width, int height, const char* title)
:	window_p_(nullptr)
{
	glfwSetErrorCallback(&error_callback);
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW.");

	window_p_ = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window_p_) {
		glfwTerminate();
		throw std::runtime_error("Failed to open the main window.");
	}

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(window_p_, (mode->width - width) / 2, (mode->height - height) / 2);

	glfwMakeContextCurrent(window_p_);
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "Error " << glewGetErrorString(err) << std::endl;
		glfwDestroyWindow(window_p_);
		glfwTerminate();
		throw std::runtime_error("Failed to initialize GLEW for the main window.");
	}
}

MainWindow::~MainWindow(void) {
	glfwDestroyWindow(window_p_);
	glfwTerminate();
}
