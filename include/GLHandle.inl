template <typename T>
Handle<T>& Handle<T>::operator=(Handle&& other) noexcept
{
	if (this != std::addressof(other))
	{
		h_ = other.h_;
		other.h_ = 0;
	}

	return *this;
}
