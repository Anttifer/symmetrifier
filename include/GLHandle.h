#ifndef GLHANDLE_H
#define GLHANDLE_H

#include <type_traits>
#include <memory>

namespace GL
{
// This is a thin wrapper around (OpenGL) integer handles, which makes
// them noncopyable but movable, zeroing the moved-from handle. This makes
// it harder to accidentally end up with duplicate handles and simplifies
// the move operations of OpenGL objects.
template <typename T>
class Handle
{
	static_assert(std::is_integral_v<T>);

public:
	Handle (void)                    : h_(0)        {}
	Handle (T h)                     : h_(h)        {}
	Handle (Handle&& other) noexcept : h_(other.h_) { other.h_ = 0; }

	Handle&  operator= (Handle&& other) noexcept;

	operator T         () const { return h_; }

	// This class is standard-layout; these exist just as a convenience
	// for getting the right pointer type without casting.
	const T* operator& () const { return &h_; }
	T*       operator& ()       { return &h_; }
private:
	T h_;
};

#include "GLHandle.inl"
}
#endif // GLHANDLE_H
