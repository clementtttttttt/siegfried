#define O_RDONLY  00
#define O_WRONLY  01
#define O_RDWR    02
#define O_NONBLOCK 04
#define O_CREAT 8
#define O_EXCL 16
#define O_TRUNC 32
#define O_APPEND 64
#define O_NDELAY 128
int fcntl (int __fd, int __cmd, ...);
int open(const char *path, int oflag, ... );

#define FD_CLOEXEC	1	/* actually anything with low bit set goes */

#define F_DUPFD		0	/* dup */
#define F_GETFD		1	/* get f_flags */
#define F_SETFD		2	/* set f_flags */
#define F_GETFL		3	/* more flags (cloexec) */
#define F_SETFL		4
#define F_GETLK		5	/* not implemented */
#define F_SETLK		6
#define F_SETLKW	7
