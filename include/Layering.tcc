template <typename... Args>
void Layering::add_layer(Args&&... args)
{
	layers_.emplace_back(std::forward<Args>(args)...);
	current_index_ = size() - 1;
}
