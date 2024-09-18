#warning iswspace is a stub
typedef unsigned long wctype_t; 

int iswspace(wint_t ch){
	return 0;
}

int       iswctype(wint_t, wctype_t);
wctype_t  wctype(const char *);
