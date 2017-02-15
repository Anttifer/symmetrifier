template <typename T>
void MainWindow::add_key_callback(int key, MemberKeyCallback<T> callback, T* this_pointer)
{
	using namespace std::placeholders;
	add_key_callback(key, std::bind(callback, this_pointer, _1, _2, _3));
}
