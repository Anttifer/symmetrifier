#ifndef GUICREATEMEMBER_H
#define GUICREATEMEMBER_H

#define CREATE_MEMBER(type, name)\
public:\
	/* TODO: Maybe allow setting only the non-tracked, internal variables in the future? */\
	const type& name           (void)          const { return *name##_; }\
	void        set_##name     (const type& i)       { *name##_ = i; }\
	\
	void        name##_track   (type& t)\
	{\
		name##_internal_ = name##_default_;\
		name##_default_  = t;\
		name##_          = &t;\
	}\
	void        name##_untrack (void)\
	{\
		name##_default_ = name##_internal_;\
		name##_         = &name##_internal_;\
	}\
private:\
	type  name##_default_;\
	type  name##_internal_ = name##_default_;\
	type* name##_          = &name##_internal_;

#endif // GUICREATEMEMBER_H
