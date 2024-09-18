

typedef struct {
	unsigned long inode;
	char *name;
	
} DIR;


struct dirent{
	char *d_name;
	unsigned char d_type;
};

enum{
	DT_UNKNOWN=1,DT_REG,DT_DIR,DT_FIFO,DT_SOCK,DT_CHR,DT_BLK,DT_LNK

};

DIR *opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
