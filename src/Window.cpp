#include "Window.h"

#include <iostream>

MainWindow::PointerMap MainWindow::window_by_pointer__;

//--------------------

MainWindow::MainWindow(int width, int height, const char* title)
:	window_p_(nullptr)
{
	glfwSetErrorCallback(&master_error_callback);
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW.");

	window_p_ = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window_p_)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to open the main window.");
	}

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(window_p_, (mode->width - width) / 2, (mode->height - height) / 2);

	glfwMakeContextCurrent(window_p_);
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cerr << "Error " << glewGetErrorString(err) << std::endl;
		glfwDestroyWindow(window_p_);
		glfwTerminate();
		throw std::runtime_error("Failed to initialize GLEW for the main window.");
	}

	window_by_pointer__[window_p_] = this;
	glfwSetKeyCallback(window_p_, &master_key_callback);
}

MainWindow::~MainWindow(void) {
	glfwDestroyWindow(window_p_);
	glfwTerminate();
}

void MainWindow::add_key_callback(int key, KeyCallback callback)
{
	key_callbacks_[key].push_back(callback);
}

void MainWindow::master_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// This allows us to e.g. have multiple windows work correctly in the future.
	const auto& callbacks = window_by_pointer__.at(window)->key_callbacks_;

	if (callbacks.find(key) != std::end(callbacks))
	{
		for (auto& callback : callbacks.at(key))
			callback(scancode, action, mods);
	}
}

void MainWindow::master_error_callback(int error, const char* description)
{
	fputs(description, stderr);
	getchar();
}
