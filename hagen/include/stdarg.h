#ifndef _STDC_STDARG_H_
#define _STDC_STDARG_H_

#include <stddef.h>

/*
typedef size_t*			va_list;
#define va_start(l,p) 		((l) = ((size_t*)(&(p))) + 1)
#define va_arg(l,t)		(*((t*)(l++)))
#define va_end(l)		((void)0)
#define va_copy(d,s)		((d) = (s))
*/

typedef __builtin_va_list va_list;
#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#define va_copy(d,s)    __builtin_va_copy(d,s)

#endif
