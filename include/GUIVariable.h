#ifndef GUIVARIABLE_H
#define GUIVARIABLE_H

template <typename T>
class GUIVariable
{
public:
	template <typename... Args>
	GUIVariable(Args&&... args) :
		default_(std::forward<Args>(args)...),
		internal_(default_),
		tracked_(&internal_)
	{}

	GUIVariable(const GUIVariable&)            = delete;
	GUIVariable& operator=(const GUIVariable&) = delete;

	void track(T& t)
	{
		internal_ = default_;
		default_  = t;
		tracked_  = &t;
	}
	void untrack()
	{
		default_ = internal_;
		tracked_ = &internal_;
	}
	void reset() { *tracked_ = default_; }

	T& operator* () { return *tracked_; }
	T* operator->() { return tracked_; }
	operator T*  () { return tracked_; }

private:
	T  default_;
	T  internal_;
	T* tracked_;
};

#endif // GUIVARIABLE_H
