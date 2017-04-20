template <typename... Args>
void Layer::add_image(Args&&... args)
{
	consistent_ = false;
	images_.emplace_back(std::forward<Args>(args)...);
}
