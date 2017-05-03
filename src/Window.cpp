#include "Window.h"

#include <iostream>

MainWindow::PointerMap MainWindow::window_by_pointer__;

//--------------------

MainWindow::MainWindow(int width, int height, const char* title) :
	window_p_(nullptr)
{
	glfwSetErrorCallback(&master_error_callback);
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW.");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	window_p_ = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window_p_)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to open the main window.");
	}

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(window_p_, (mode->width - width) / 2, (mode->height - height) / 2);

	glfwMakeContextCurrent(window_p_);
	glewExperimental = GL_TRUE;
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
	glfwSetCharCallback(window_p_, &master_char_callback);
	glfwSetMouseButtonCallback(window_p_, &master_mouse_button_callback);
	glfwSetCursorPosCallback(window_p_, &master_mouse_pos_callback);
	glfwSetScrollCallback(window_p_, &master_scroll_callback);
	glfwSetDropCallback(window_p_, &master_path_drop_callback);
}

MainWindow::~MainWindow(void) {
	glfwDestroyWindow(window_p_);
	glfwTerminate();
}

void MainWindow::add_key_callback(int key, const KeyCallback& callback)
{
	key_callback_map_[key].push_back(callback);
}

void MainWindow::add_key_callback(const GeneralKeyCallback& callback)
{
	general_key_callbacks_.push_back(callback);
}

void MainWindow::add_char_callback(const CharCallback& callback)
{
	char_callbacks_.push_back(callback);
}

void MainWindow::add_mouse_button_callback(int button, const MouseButtonCallback& callback)
{
	mouse_button_callback_map_[button].push_back(callback);
}

void MainWindow::add_mouse_button_callback(const GeneralMouseButtonCallback& callback)
{
	general_mouse_button_callbacks_.push_back(callback);
}

void MainWindow::add_mouse_pos_callback(const MousePosCallback& callback)
{
	mouse_pos_callbacks_.push_back(callback);
}

void MainWindow::add_scroll_callback(const ScrollCallback& callback)
{
	scroll_callbacks_.push_back(callback);
}

void MainWindow::add_path_drop_callback(const PathDropCallback& callback)
{
	path_drop_callbacks_.push_back(callback);
}

void MainWindow::master_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	const auto& callback_map = window_by_pointer__.at(window)->key_callback_map_;
	const auto& callbacks    = window_by_pointer__.at(window)->general_key_callbacks_;

	if (callback_map.find(key) != std::end(callback_map))
	{
		for (auto& callback : callback_map.at(key))
			callback(scancode, action, mods);
	}

	for (auto& callback : callbacks)
		callback(key, scancode, action, mods);
}

void MainWindow::master_char_callback(GLFWwindow* window, unsigned int c)
{
	const auto& callbacks = window_by_pointer__.at(window)->char_callbacks_;

	for (auto& callback : callbacks)
		callback(c);
}

void MainWindow::master_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	const auto& callback_map = window_by_pointer__.at(window)->mouse_button_callback_map_;
	const auto& callbacks    = window_by_pointer__.at(window)->general_mouse_button_callbacks_;

	if (callback_map.find(button) != std::end(callback_map))
	{
		for (auto& callback : callback_map.at(button))
			callback(action, mods);
	}

	for (auto& callback : callbacks)
		callback(button, action, mods);
}

void MainWindow::master_mouse_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	const auto& callbacks = window_by_pointer__.at(window)->mouse_pos_callbacks_;

	for (auto& callback : callbacks)
		callback(xpos, ypos);
}

void MainWindow::master_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	const auto& callbacks = window_by_pointer__.at(window)->scroll_callbacks_;

	for (auto& callback : callbacks)
		callback(xoffset, yoffset);
}

void MainWindow::master_path_drop_callback(GLFWwindow* window, int count, const char** paths)
{
	const auto& callbacks = window_by_pointer__.at(window)->path_drop_callbacks_;

	for (auto& callback : callbacks)
		callback(count, paths);
}

void MainWindow::master_error_callback(int error, const char* description)
{
	(void)error;
	fputs(description, stderr);
	getchar();
}
