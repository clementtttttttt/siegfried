typedef unsigned short tcflag_t;
typedef unsigned short speed_t;
typedef unsigned short cc_t;

#define NCCS 128

struct termios{
	tcflag_t c_lflag;
  cc_t     c_cc[NCCS];
};


#define TCSANOW 1
#define TCSADRAIN 2
#define TCSAFLUSH 3

#warning stub VSUSP and TOSTOP
#define TOSTOP 1
#define VSUSP 2
enum{
	ICANON=1


};
int tcsetattr(int fildes, int optional_actions,
       const struct termios *termios_p);
int tcgetattr(int fildes, struct termios *termios_p);
