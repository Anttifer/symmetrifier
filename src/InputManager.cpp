#include "InputManager.h"
#include "Window.h"
#include <Eigen/Geometry>


InputManager::InputManager(MainWindow& window)
:	window_(window)
{}

Eigen::Vector2f InputManager::mouse_position(void)
{
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	double xpos, ypos;
	glfwGetCursorPos(window_, &xpos, &ypos);

	// The origin is in the top left corner.
	xpos = xpos / width * 2 - 1;
	ypos = 1 - ypos / height * 2;

	return {xpos, ypos};
}

bool InputManager::left_button_pressed(void)
{
	return glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT);
}
