#ifndef __WCTYPE_H
#define __WCTYPE_H
#include <sys/types.h>
#warning iswspace is a stub
#include <wchar.h>
#include <wctype.h>
int iswspace(wint_t ch){
	return 0;
}

int iswprint( wint_t ch );
int       iswctype(wint_t, wctype_t);
wctype_t  wctype(const char *);
int iswblank( wint_t ch );
#endif
