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

class MainWindow {
public:
	MainWindow  (int width, int height, const char* title);
	MainWindow  (const MainWindow&) = delete;
	~MainWindow (void);

	MainWindow& operator= (const MainWindow&) = delete;
	operator GLFWwindow*  (void) const {return window_p_;}

	template <typename T>
	void add_key_callback(int key, MemberKeyCallback<T> callback, T* this_pointer);
	void add_key_callback(int key, KeyCallback callback);

private:
	using KeyCallbackMap = std::unordered_map<int, std::vector<KeyCallback>>;
	using PointerMap     = std::unordered_map<GLFWwindow*, MainWindow*>;

	GLFWwindow* window_p_;
	KeyCallbackMap key_callbacks_;

	static void master_key_callback   (GLFWwindow*, int, int, int, int);
	static void master_error_callback (int error, const char* description);

	// This is used by the master callback functions.
	static PointerMap window_by_pointer__;
};

#include "Window.tcc"

#endif // WINDOW_H
