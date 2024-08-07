

#ifndef _STDC_SETJMP_H
#define _STDC_SETJMP_H

typedef unsigned long _jmp_buf[8];

typedef struct jmp_buf_struct{
	_jmp_buf _jb;
	

} jmp_buf;
void longjmp(jmp_buf in, int value);
int setjmp(jmp_buf in);

#endif
