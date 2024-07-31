#include <sys/types.h>

#pragma once
enum mmap_flags{

	MAP_SHARED,MAP_PRIVATE
};

enum mmap_prot{
	PROT_READ,PROT_WRITE
};

void *mmap (void *, size_t, int, int, int, off_t);
int munmap (void *, size_t);

