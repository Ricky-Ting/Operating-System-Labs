#include <game.h>
#include <klib.h>
void init_screen();
void splash();
int my_read_key();
void move(int key_dire);
int in_bound(int bx, int by);
enum {	_UP=0, _DOWN=1, _LEFT=2, _RIGHT=3 };
int dire[4][2]= { {0,-SIDE}, {0,SIDE}, {-SIDE,0}, {SIDE,0} };
int w,h;
int px,py;
int main() {
  // Operating system is a C program
  _ioe_init();
  init_screen();
	px = w/2; py = h/2;
	//printf("px=%d, py=%d\n",px,py);
  splash();
	unsigned long last = 0;
	unsigned long current;
  while (1) {
    int key = my_read_key();
		if(key != _KEY_NONE) {
				switch(key) {
					case _KEY_UP:   	move(_UP);	break;	
					case _KEY_DOWN:	move(_DOWN);	break;
					case _KEY_LEFT:	move(_LEFT);	break;
					case _KEY_RIGHT:	move(_RIGHT);	break;
					default: break;
				}
		}
		current = uptime();
		if(current - last > 500) {
			splash();
			last = current;
		}
  }
  return 0;
}

void move(int key_dire) {
	if( in_bound(px+dire[key_dire][0],py+dire[key_dire][1]) ) {
		px+=dire[key_dire][0];
		py+=dire[key_dire][1];
	}	
}

int in_bound(int bx, int by) {
	if( bx>=0 && bx+SIDE<=w  && by>=0 && by+SIDE<=h )
		return 1;
	return 0;
}

int my_read_key() {
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  #define KEYNAME(key) \
    [_KEY_##key] = #key,
  static const char *key_names[] = {
    _KEYS(KEYNAME)
  };
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode != _KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
		return event.keycode;
  }
	return _KEY_NONE;
}


void init_screen() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
}

void mydraw_rect(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: allocated on stack
  _DEV_VIDEO_FBCTL_t event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTL, &event, sizeof(event));
}


void splash() {
/*
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1)) {
        draw_rect(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
    }
  }
*/
	mydraw_rect(px, py, SIDE, SIDE, 0xffffff); 
}


