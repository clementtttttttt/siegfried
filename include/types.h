#ifndef _STAT_H
#define _STAT_H

#include <stdint.h>

typedef long ino_t;

typedef long off_t;

typedef __uint128_t sigset_t;
typedef long pid_t;
typedef long		ssize_t;
typedef unsigned long size_t;
typedef __uint128_t time_t;
typedef unsigned long uid_t;
typedef unsigned long gid_t;
typedef unsigned short mode_t;
typedef long clock_t;
typedef unsigned long wctype_t;


struct stat {
	uint16_t	st_dev;
	uint16_t	__pad1;
	unsigned long	st_ino;
	uint16_t	st_mode;
	uint16_t	st_nlink;
	uint16_t	st_uid;
	uint16_t	st_gid;
	uint16_t	st_rdev;
	uint16_t	__pad2;
	unsigned long	st_size;
	unsigned long	st_blksize;
	unsigned long	st_blocks;
	time_t		st_atime;
	unsigned long	__unused1;
	time_t		st_mtime;
	unsigned long	__unused2;
	time_t		st_ctime;
	unsigned long	__unused3;
	unsigned long	__unused4;
	unsigned long	__unused5;
};

#endif

