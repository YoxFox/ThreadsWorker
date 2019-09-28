#ifndef __CONSTRUCTORDEFINES_H__
#define __CONSTRUCTORDEFINES_H__

#define COPY_FORBIDDEN(class_name) \
    class_name(const class_name&) = delete; \
    class_name& operator=(const class_name&) = delete; \

#define MOVE_FORBIDDEN(class_name) \
    class_name& operator=(class_name&&) = delete; \
    class_name(class_name&&) = delete; \

#define COPY_MOVE_FORBIDDEN(class_name) \
    class_name(const class_name&) = delete; \
    class_name& operator=(const class_name&) = delete; \
    class_name& operator=(class_name&&) = delete; \
    class_name(class_name&&) = delete; \
	
#endif // __CONSTRUCTORDEFINES_H__