#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_map>
#include <vector>


template <typename T>
using MemberKeyCallback = void (T::*)(int, int, int);
using KeyCallback       = std::function<void(int, int, int)>;

template <typename T>
using MemberMouseButtonCallback = void (T::*)(int, int);
using MouseButtonCallback       = std::function<void(int, int)>;

template <typename T>
using MemberMousePosCallback = void (T::*)(double, double);
using MousePosCallback       = std::function<void(double, double)>;

template <typename T>
using MemberScrollCallback = void (T::*)(double, double);
using ScrollCallback       = std::function<void(double, double)>;


class MainWindow {
public:
	MainWindow  (int width, int height, const char* title);
	MainWindow  (const MainWindow&) = delete;
	~MainWindow (void);

	MainWindow& operator= (const MainWindow&) = delete;
	operator GLFWwindow*  (void) const {return window_p_;}

	// TODO: Add general callback without the key parameter.
	template <typename T>
	void add_key_callback(int key, const MemberKeyCallback<T>& callback, T* this_pointer);
	void add_key_callback(int key, const KeyCallback& callback);

	// TODO: Add general callback without the button parameter.
	template <typename T>
	void add_mouse_button_callback(int button, const MemberMouseButtonCallback<T>& callback, T* this_pointer);
	void add_mouse_button_callback(int button, const MouseButtonCallback& callback);

	template <typename T>
	void add_mouse_pos_callback(const MemberMousePosCallback<T>& callback, T* this_pointer);
	void add_mouse_pos_callback(const MousePosCallback& callback);

	template <typename T>
	void add_scroll_callback(const MemberScrollCallback<T>& callback, T* this_pointer);
	void add_scroll_callback(const ScrollCallback& callback);

private:
	GLFWwindow*                   window_p_;

	using KeyCallbackMap         = std::unordered_map<int, std::vector<KeyCallback>>;
	using MouseButtonCallbackMap = std::unordered_map<int, std::vector<MouseButtonCallback>>;
	using PointerMap             = std::unordered_map<GLFWwindow*, MainWindow*>;

	KeyCallbackMap                key_callback_map_;
	MouseButtonCallbackMap        mouse_button_callback_map_;
	std::vector<MousePosCallback> mouse_pos_callbacks_;
	std::vector<ScrollCallback>   scroll_callbacks_;

	static void master_key_callback          (GLFWwindow*, int, int, int, int);
	static void master_mouse_button_callback (GLFWwindow*, int, int, int);
	static void master_mouse_pos_callback    (GLFWwindow*, double, double);
	static void master_scroll_callback       (GLFWwindow*, double, double);
	static void master_error_callback        (int error, const char* description);

	// This is used by the master callback functions.
	static PointerMap window_by_pointer__;
};

#include "Window.tcc"

#endif // WINDOW_H
