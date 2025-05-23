#include "font.h"
#include "page.h"
#include "obj_heap.h"
#include "debug.h"
#include "klib.h"

static unsigned char * draw_fb_addr;
static unsigned long draw_fb_paddr;
static unsigned long w,h,bb, p;

unsigned long tw, th, tcurx, tcury;

//tcurx and y points to a free char that's right after an existing char.'

char *text_buf;
char *text_buf_2;
/*
static inline void draw_pixel_at(unsigned long x,unsigned long y, unsigned int color) {
    unsigned long where = (x*bb/8 + y*p);
    draw_fb_addr[where] = color & 255;              // BLUE
    draw_fb_addr[where + 1] = (color >> 8) & 255;   // GREEN
    draw_fb_addr[where + 2] = (color >> 16) & 255;  // RED
    draw_fb_addr[where + 3] = (color >> 24) & 255;  // ALPHA

}*/

/*
static inline void draw_char_at(unsigned char in, unsigned long inx, unsigned long iny){

    if(in == 0 ) return;

    switch(in){
        case 'y':
        case 'g':
        case 'p':
        case 'q':
            iny += 2;
            break;
    }

    unsigned long font_start_x = (in % 8);
    unsigned long font_start_y = (in / 8) *8;

    unsigned long x_inc = bb/8;
    unsigned long y_inc = p - x_inc*8;

    //unsigned long where = iny * p + inx * x_inc;

    unsigned char *restrict fb_it = draw_fb_addr + iny*p + inx*x_inc;
    
    unsigned char *restrict comp_byte_ptr = font_bits + (font_start_y)*font_width/8 + font_start_x;


    for(unsigned long y = 0; y < 8; ++y){

        

        for(unsigned long x=0; x<8; ++x){
            fb_it += x_inc;
            if (!(*comp_byte_ptr & (1<<x)) && in != 0){
                fb_it[0] = 0xff;              // BLUE
                fb_it[1] = 0xff;   // GREEN
                fb_it[2] = 0xff;  // RED
                if(x_inc == 4){
                    fb_it[3] = 0xff;
                }
            }

	    	    

        }
	            
		    comp_byte_ptr += font_width/8;

        fb_it += y_inc;
    }

}
*/
static inline void draw_char_at_fbptr(unsigned char in, unsigned char *restrict fb_it){

    unsigned long x_inc = bb/8;
    unsigned long y_inc = p - x_inc*8;
    char is_special = 0;
    switch(in){
        case 'y':
        case 'g':
        case 'p':
        case 'q':
        //    fb_it += p*2;
	    is_special = 1;
            break;
    }

    unsigned long font_start_x = (in % 8);
    unsigned long font_start_y = (in / 8) *8;



    //unsigned long where = iny * p + inx * x_inc;

    
    
    unsigned char *restrict comp_byte_ptr = font_bits + (font_start_y)*font_width/8 + font_start_x;

    if(x_inc == 4){
		for(long y = is_special?-2:0; y < 10; ++y){

        

        for(unsigned long x=0; x<8; ++x){
            fb_it += 4;
            if (y < 8 && y >= 0&& !(*comp_byte_ptr & (1<<x)) && in){
                			*(unsigned int*)fb_it = 0xffffffff;

                
            }
	    else{
			*(unsigned int*)fb_it = 0;
	    }

	    	    

      	}
			if(y >= 0)
				comp_byte_ptr += font_width/8;

			fb_it += y_inc;
		}
	}
	else{
				for(long y = is_special?-2:0; y < 10; ++y){

        

        for(unsigned long x=0; x<8; ++x){
            fb_it += x_inc;
            if (y < 8 && y >= 0&& !(*comp_byte_ptr & (1<<x)) && in){
                fb_it[0] = 0xff;              // BLUE
                fb_it[1] = 0xff;   // GREEN
                fb_it[2] = 0xff;  // RED
                
            }
	    else{
		fb_it[0] = 0x0;
		fb_it[1] = 0x0;
		fb_it[2] = 0x0;
	    }

	    	    

      	}
		if(y >= 0)
			comp_byte_ptr += font_width/8;

        fb_it += y_inc;
		}
	}
   

}

void draw_swap_textbuf(){
    char *restrict text_buf_ptr = text_buf;
    char *restrict text_buf_ptr_2 = text_buf_2;

	unsigned char *restrict fb_it =  draw_fb_addr; //+ iny*p + inx*x_inc;
    unsigned long x_inc = bb/8;
    //unsigned long y_inc = p - x_inc*8;
    for(unsigned long ty = 0; ty < th; ++ty){
    		
        for(unsigned long tx=0; tx < tw; ++tx){
            if(*text_buf_ptr != *text_buf_ptr_2){
	    	draw_char_at_fbptr(*text_buf_ptr, fb_it);
		*text_buf_ptr_2 = *text_buf_ptr;
	    }	
	    	
            

	    ++text_buf_ptr;
	    ++text_buf_ptr_2;
            fb_it += x_inc*8;
	}
	fb_it += p*9;
	
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
 
	draw_fb_addr = (unsigned char*)0xfe000000000;

    page_flush();

    text_buf = k_obj_alloc(w*h/10);
    text_buf_2 = k_obj_alloc(w*h/10);

    tcurx = tcury = 0;

    tw = fbw/8;
    th = fbh /10;
    
    mem_set(text_buf,0,w*h/10);
    mem_set(text_buf_2,0,w*h/10);


}

void draw_scroll_text_buf(){

    mem_cpy(text_buf, text_buf + tw, tw*(th-1));
    mem_set(text_buf + tw * (th - 1),0, tw);
    //mem_cpy(text_buf_2, text_buf_2 + tw, tw*(th-1));
    //mem_set(text_buf_2 + tw * (th - 1),0, tw);

    //mem_set((void*)((unsigned long)draw_fb_addr + p*h - p*8), 0, p);


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
		case 8:
			--tcurx;
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


}

void draw_string(const char* str){
	
	
    
    while(*str){
        draw_append_text_buf(*str);
    	++str;
    }
    draw_swap_textbuf();

}

void draw_string_w_sz(const char* str, unsigned int sz){

    while(sz--){
        if(*str == 0){draw_append_text_buf(' ');++str;}
        else
        draw_append_text_buf(*str++);

    }
    draw_swap_textbuf();

}

void hexdump(void *ptr, unsigned long i2){
	    char hexlookup[] = "0123456789ABCDEF";

	char *ptr2 = ptr;
	    draw_append_text_buf('\n');
	
	for(unsigned long i=0;i<i2;++i){        
				if(i % 8 == 0){
				    draw_append_text_buf('\n');

		}
        draw_append_text_buf(hexlookup[*ptr2 >> 4 & 0xf]);
        draw_append_text_buf(hexlookup[*ptr2 & 0xf]);
		draw_append_text_buf(' ');
		++ptr2;
		

	}
	draw_append_text_buf('\n');
    draw_swap_textbuf();

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


void draw_dec(unsigned long in){
	char temp[20]={0};

    for(int i=0;i<20;++i){
       temp[i] = in % 10;
       in /= 10;
    }
    
    	for(int i=19;i>=0;--i){
		draw_append_text_buf(temp[i] + '0');
	}
    draw_append_text_buf('\n');

    draw_swap_textbuf();
}


