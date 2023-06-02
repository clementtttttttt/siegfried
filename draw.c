#include "font.h"
#include "page.h"
#include "obj_heap.h"
#include "debug.h"
#include "klib.h"

static unsigned char * draw_fb_addr = (unsigned char*)0xfe000000000;
static unsigned long draw_fb_paddr;
static unsigned long w,h,bb, p;

unsigned long tw, th, tcurx, tcury;

//tcurx and y points to a free char that's right after an existing char.'

char *text_buf;
/*
static inline void draw_pixel_at(unsigned long x,unsigned long y, unsigned int color) {
    unsigned long where = (x*bb/8 + y*p);
    draw_fb_addr[where] = color & 255;              // BLUE
    draw_fb_addr[where + 1] = (color >> 8) & 255;   // GREEN
    draw_fb_addr[where + 2] = (color >> 16) & 255;  // RED
    draw_fb_addr[where + 3] = (color >> 24) & 255;  // ALPHA

}*/

void draw_char_at(unsigned char in, unsigned long inx, unsigned long iny){

    if(in == 0 ) return;

    switch(in){
        case 'y':
        case 'g':
        case 'p':
        case 'q':
            iny += 3;
            break;
    }

    unsigned long font_start_x = (in % 8);
    unsigned long font_start_y = (in / 8) *8;

    unsigned long x_inc = bb/8;
    unsigned long y_inc = p - x_inc*8;

    unsigned long where = iny * p + inx * x_inc;

    for(unsigned long y = 0; y < 8; ++y){

        unsigned char comp_byte = font_bits[(font_start_y + y)*font_width/8 + font_start_x];


        for(unsigned long x=0; x<8; ++x){
            where += x_inc;
            if (!(comp_byte & (1<<x)) && in != 0){
                draw_fb_addr[where] = 0xff;              // BLUE
                draw_fb_addr[where + 1] = 0xff;   // GREEN
                draw_fb_addr[where + 2] = 0xff;  // RED
                if(x_inc == 4){
                    draw_fb_addr[where + 3] = 0xff;
                }
            }

        }
        where += y_inc;
    }

}

void draw_swap_textbuf(){

    for(unsigned long y = 0; y < th*tw; y += tw){
        for(unsigned long x=0; x < tw; ++x){
            draw_char_at(text_buf[y+x], x*8, y/tw*10);
        }
    }
}

void draw_setup(unsigned long fb_paddr,unsigned long fbw, unsigned long fbh, unsigned long fbb, unsigned long fbp){
    draw_fb_paddr = fb_paddr;

    w = fbw;
    h = fbh;
    bb = fbb;
    p = fbp;

    for(unsigned long i=0;i<0x10;++i){
        page_alloc_dev((void*) (fb_paddr + 0x200000 * i), (void*) 0xFE000000000 + 0x200000 * i);
    }
    page_flush();

    dbgnumout_hex(w*h/8);
    text_buf = k_obj_alloc(w*h/8);

    tcurx = tcury = 0;

    tw = fbw/8;
    th = fbh /10;

}

void draw_scroll_text_buf(){

    mem_cpy(text_buf, text_buf + tw, tw*(th-1));
    mem_set(text_buf + tw * (th - 1),0, tw);
    mem_set((void*)((unsigned long)draw_fb_addr), 0, p*(h));
    draw_swap_textbuf();


}
void draw_increment_line(){
    tcurx = 0;
    if((tcury+1) >= th){
        draw_scroll_text_buf();
    }
    else{

        ++tcury;

    }

}

void draw_append_text_buf(const char c){

    switch(c){
        case 0:
            return;
            break;

        case 0xa:
            draw_increment_line();
            return;
            break;
    }

    if(tcurx >= tw){
        draw_increment_line();
    }

    unsigned long where = tcury * tw + tcurx;

    text_buf[where] = c;
    ++tcurx;
    draw_swap_textbuf();


}

void draw_string(const char* str){
    --str;

    while(*(++str)){
        draw_append_text_buf(*str);
    }

}

void draw_hex(unsigned long in){
    char hexlookup[] = "0123456789ABCDEF";

    for(int i=0;i<16;++i){
        char c = hexlookup[((in>>((15-i)*4))&0xf)];
        draw_append_text_buf(c);
    }
    draw_append_text_buf('\n');

    draw_swap_textbuf();
}
