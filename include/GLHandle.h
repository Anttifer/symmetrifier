#ifndef GLHANDLE_H
#define GLHANDLE_H

#include <type_traits>
#include <memory>

namespace GL
{
// This is a wrapper around OpenGL object handles.
// Handles are created using the GenFuncP pointed to by the first template
// parameter, and destroyed using the DelFuncP pointed to by the second.

// If these pointers are not given, the object is a noncopyable
// but movable GLuint which defaults to zero and is zeroed when moved from.

using GenFuncP = void(*)(GLsizei, GLuint*);
using DelFuncP = void(*)(GLsizei, const GLuint*);

template <const GenFuncP* gen = nullptr, const DelFuncP* del = nullptr>
class Handle
{
public:
	Handle  (void)           noexcept;
	Handle  (GLuint h)       noexcept : h_(h)        {}
	Handle  (Handle&& other) noexcept : h_(other.h_) { other.h_ = 0; }
	~Handle (void);

	Handle& operator= (Handle&& other) noexcept;

	// This class is standard-layout; these exist just as a convenience
	// for getting the right pointer type without casting.
	const GLuint* operator& () const { return &h_; }
	GLuint*       operator& ()       { return &h_; }

	operator      GLuint    () const { return h_; }
private:
	GLuint h_;
};

//--------------------

template <const GenFuncP* gen, const DelFuncP* del>
Handle<gen, del>::Handle(void) noexcept : h_(0)
{
	if constexpr (gen != nullptr)
		(*gen)(1, &h_);
}

template <const GenFuncP* gen, const DelFuncP* del>
Handle<gen, del>::~Handle(void)
{
	if constexpr (del != nullptr)
		(*del)(1, &h_);
}

template <const GenFuncP* gen, const DelFuncP* del>
auto Handle<gen, del>::operator=(Handle&& other) noexcept -> Handle&
{
	if (this != std::addressof(other))
	{
		if constexpr (del != nullptr)
			(*del)(1, &h_);

		h_ = other.h_;
		other.h_ = 0;
	}

	return *this;
}
}
#endif // GLHANDLE_H
