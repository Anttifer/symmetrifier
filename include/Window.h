#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_map>
#include <vector>


class MainWindow
{
public:
	MainWindow  (int width, int height, const char* title);
	MainWindow  (const MainWindow&) = delete;
	~MainWindow (void);

	MainWindow& operator= (const MainWindow&) = delete;
	operator GLFWwindow*  (void) const {return window_p_;}

	template <typename T>
	using MemberKeyCallback = void (T::*)(int, int, int);
	using KeyCallback       = std::function<void(int, int, int)>;

	template <typename T>
	void add_key_callback(int key, const MemberKeyCallback<T>& callback, T* this_pointer);
	void add_key_callback(int key, const KeyCallback& callback);

	template <typename T>
	using GeneralMemberKeyCallback = void (T::*)(int, int, int, int);
	using GeneralKeyCallback       = std::function<void(int, int, int, int)>;

	template <typename T>
	void add_key_callback(const GeneralMemberKeyCallback<T>& callback, T* this_pointer);
	void add_key_callback(const GeneralKeyCallback& callback);

	template <typename T>
	using MemberCharCallback = void (T::*)(unsigned int);
	using CharCallback       = std::function<void(unsigned int)>;

	template <typename T>
	void add_char_callback(const MemberCharCallback<T>& callback, T* this_pointer);
	void add_char_callback(const CharCallback& callback);

	template <typename T>
	using MemberMouseButtonCallback = void (T::*)(int, int);
	using MouseButtonCallback       = std::function<void(int, int)>;

	template <typename T>
	void add_mouse_button_callback(int button, const MemberMouseButtonCallback<T>& callback, T* this_pointer);
	void add_mouse_button_callback(int button, const MouseButtonCallback& callback);

	template <typename T>
	using GeneralMemberMouseButtonCallback = void (T::*)(int, int, int);
	using GeneralMouseButtonCallback       = std::function<void(int, int, int)>;

	template <typename T>
	void add_mouse_button_callback(const GeneralMemberMouseButtonCallback<T>& callback, T* this_pointer);
	void add_mouse_button_callback(const GeneralMouseButtonCallback& callback);

	template <typename T>
	using MemberMousePosCallback = void (T::*)(double, double);
	using MousePosCallback       = std::function<void(double, double)>;

	template <typename T>
	void add_mouse_pos_callback(const MemberMousePosCallback<T>& callback, T* this_pointer);
	void add_mouse_pos_callback(const MousePosCallback& callback);

	template <typename T>
	using MemberScrollCallback = void (T::*)(double, double);
	using ScrollCallback       = std::function<void(double, double)>;

	template <typename T>
	void add_scroll_callback(const MemberScrollCallback<T>& callback, T* this_pointer);
	void add_scroll_callback(const ScrollCallback& callback);

	template <typename T>
	using MemberPathDropCallback = void (T::*)(int, const char**);
	using PathDropCallback       = std::function<void(int, const char**)>;

	template <typename T>
	void add_path_drop_callback(const MemberPathDropCallback<T>& callback, T* this_pointer);
	void add_path_drop_callback(const PathDropCallback& callback);

private:
	GLFWwindow*                   window_p_;

	using KeyCallbackMap         = std::unordered_map<int, std::vector<KeyCallback>>;
	using MouseButtonCallbackMap = std::unordered_map<int, std::vector<MouseButtonCallback>>;
	using PointerMap             = std::unordered_map<GLFWwindow*, MainWindow*>;

	KeyCallbackMap                key_callback_map_;
	MouseButtonCallbackMap        mouse_button_callback_map_;

	std::vector<GeneralKeyCallback>         general_key_callbacks_;
	std::vector<CharCallback>               char_callbacks_;
	std::vector<GeneralMouseButtonCallback> general_mouse_button_callbacks_;
	std::vector<MousePosCallback>           mouse_pos_callbacks_;
	std::vector<ScrollCallback>             scroll_callbacks_;
	std::vector<PathDropCallback>           path_drop_callbacks_;

	static void master_key_callback          (GLFWwindow*, int, int, int, int);
	static void master_char_callback         (GLFWwindow*, unsigned int);
	static void master_mouse_button_callback (GLFWwindow*, int, int, int);
	static void master_mouse_pos_callback    (GLFWwindow*, double, double);
	static void master_scroll_callback       (GLFWwindow*, double, double);
	static void master_path_drop_callback    (GLFWwindow*, int, const char**);
	static void master_error_callback        (int error, const char* description);

	// This is used by the master callback functions.
	static PointerMap window_by_pointer__;
};

#include "Window.tcc"

#endif // WINDOW_H
