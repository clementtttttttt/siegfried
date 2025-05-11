/*

   Copyright 2009 Pierre KRIEGER

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
   
*/

#ifndef _STDC_STDIO_H_
#define _STDC_STDIO_H_

#include <stddef.h>
#include <stdarg.h>


#define BUFSIZ 4096 

#ifdef __cplusplus
extern "C" {
#endif


typedef void FILE;
int printf(const char *restrict fmt, ...);
int		snprintf		(char* s, size_t n, const char* format, ...);
int		sprintf		(char* s, const char* format, ...);
int		sscanf		(const char* s, const char* format, ...);
int		vsnprintf		(char* s, size_t n, const char* format, va_list arg);
int		vsprintf		(char* s, const char* format, va_list arg);
int		vsscanf		(const char* s, const char* format, va_list arg);
int puts(const char *s);
FILE * fopen(const char *path, const char *modes);
int 		fflush		(void * stream);
int fprintf ( void * stream, const char * format, ... );
int fclose(FILE* stream);
#define EOF (-1)
#warning STUB STDERR
#define stderr (void*)42

#ifdef __cplusplus
}
#endif




#endif
