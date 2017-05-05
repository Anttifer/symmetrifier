template <typename T>
void MainWindow::add_key_callback(int key, const MemberKeyCallback<T>& callback, T* this_pointer)
{
	using namespace std::placeholders;
	add_key_callback(key, std::bind(callback, this_pointer, _1, _2, _3));
}

template <typename T>
void MainWindow::add_key_callback(const GeneralMemberKeyCallback<T>& callback, T* this_pointer)
{
	using namespace std::placeholders;
	add_key_callback(std::bind(callback, this_pointer, _1, _2, _3, _4));
}

template <typename T>
void MainWindow::add_char_callback(const MemberCharCallback<T>& callback, T* this_pointer)
{
	using namespace std::placeholders;
	add_char_callback(std::bind(callback, this_pointer, _1));
}

template <typename T>
void MainWindow::add_mouse_button_callback(int button, const MemberMouseButtonCallback<T>& callback, T* this_pointer)
{
	using namespace std::placeholders;
	add_mouse_button_callback(button, std::bind(callback, this_pointer, _1, _2));
}

template <typename T>
void MainWindow::add_mouse_button_callback(const GeneralMemberMouseButtonCallback<T>& callback, T* this_pointer)
{
	using namespace std::placeholders;
	add_mouse_button_callback(std::bind(callback, this_pointer, _1, _2, _3));
}

template <typename T>
void MainWindow::add_mouse_pos_callback(const MemberMousePosCallback<T>& callback, T* this_pointer)
{
	using namespace std::placeholders;
	add_mouse_pos_callback(std::bind(callback, this_pointer, _1, _2));
}

template <typename T>
void MainWindow::add_scroll_callback(const MemberScrollCallback<T>& callback, T* this_pointer)
{
	using namespace std::placeholders;
	add_scroll_callback(std::bind(callback, this_pointer, _1, _2));
}

template <typename T>
void MainWindow::add_path_drop_callback(const MemberPathDropCallback<T>& callback, T* this_pointer)
{
	using namespace std::placeholders;
	add_path_drop_callback(std::bind(callback, this_pointer, _1, _2));
}
