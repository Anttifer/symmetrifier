template <typename... Args>
void Layering::add_layer(Args&&... args)
{
	layers_.emplace_back(std::forward<Args>(args)...);
	current_index_ = size() - 1;
}

template <typename... Args>
void Layering::insert_layer(size_t index, Args&&... args)
{
	index   = std::min(index, size());
	auto it = std::begin(layers_) + index;

	layers_.emplace(it, std::forward<Args>(args)...);
	current_index_ = index;
}
