void draw_setup(unsigned long fb_paddr,unsigned long fbw, unsigned long fbh, unsigned long fbb, unsigned long fbp);
void draw_string(const char* str);
void draw_hex(unsigned long in);
void draw_dec(unsigned long in);
void draw_scroll_text_buf();
void draw_string_w_sz(const char* str, unsigned int sz);
void draw_swap_textbuf();
void hexdump(void *ptr, int i);
