void kb_setup();


enum kb_ev_types{
	EV_NULL=0, EV_PRESSED, EV_DEPRESSED
	
};

typedef struct kb_ev_s{
	
	int type;
	int code; 
	
} kb_ev_t;
kb_ev_t kb_wait_and_pop_ev();
