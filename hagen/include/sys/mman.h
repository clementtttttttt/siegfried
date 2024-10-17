#include <sys/types.h>
#include <stddef.h>

#pragma once
enum mmap_flags{

	MAP_SHARED,MAP_PRIVATE,MAP_FIXED
};

enum mmap_prot{
	PROT_READ,PROT_WRITE
};

void *mmap (void *, size_t, int, int, int, off_t);
int munmap (void *, size_t);

