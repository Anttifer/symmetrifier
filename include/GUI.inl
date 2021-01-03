template <typename T>
void GUI::set_export_callback(const MemberExportCallback<T>& callback, T* this_pointer)
{
	using namespace std::placeholders;
	set_export_callback(std::bind(callback, this_pointer, _1, _2, _3));
}
