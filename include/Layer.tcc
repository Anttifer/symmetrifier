template <typename... Args>
void Layer::add_image(Args&&... args)
{
	consistent_ = false;
	images_.emplace_back(std::forward<Args>(args)...);
	current_index_ = size() - 1;
}

template <typename... Args>
void Layer::insert_image(size_t index, Args&&... args)
{
	consistent_ = false;

	index   = std::min(index, size());
	auto it = std::begin(images_) + index;

	images_.emplace(it, std::forward<Args>(args)...);
	current_index_ = index;
}
