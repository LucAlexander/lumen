#ifndef LUMEN_H
#define LUMEN_H

#include <inttypes.h>
#include <stddef.h>
#include <linux/input.h>
#include <termios.h>

#define READ_BUFFER_SIZE 16
#define KEY_READ_COUNT 128
#define INPUT_EVENT_FILE "/dev/input/by-path/platform-i8042-serio-0-event-kbd"

typedef enum LUMEN_INPUT_EVENT_VALUE{
	LUMEN_INPUT_RELEASED = 0,
	LUMEN_INPUT_PRESSED = 1,
	LUMEN_INPUT_HELD = 2
}LUMEN_INPUT_EVENT_VALUE;

typedef enum LUMEN_BLENDMODE{
	LUMEN_BLENDMODE_NONE,
	LUMEN_BLENDMODE_ALPHA
}LUMEN_BLENDMODE;

typedef enum LUMEN_FILE_TYPE{
	LUMEN_READ_NONE,
	LUMEN_READ_PNG,
	LUMEN_READ_BMP,
	LUMEN_READ_PPM
}LUMEN_FILE_TYPE;

typedef struct lumen_renderer{
	// rgba
	uint32_t* pixels;
	uint32_t w;
	uint32_t h;
	uint32_t render_color;
	LUMEN_BLENDMODE blendmode;
}lumen_renderer;

typedef struct lumen_texture{
	uint32_t* pixels;
	uint32_t w;
	uint32_t h;
	int32_t origin_x;
	int32_t origin_y;
	float angle;
	float scale_x;
	float scale_y;
}lumen_texture;

typedef struct lumen_input{
	uint8_t term_saved;
	uint8_t key_pressed[KEY_READ_COUNT];
	uint8_t key_released[KEY_READ_COUNT];
	uint8_t key_held[KEY_READ_COUNT];
	struct input_event event;
	struct termios save_termios;
	int32_t file_d;
}lumen_input;

typedef struct v2{
	float x;
	float y;
}v2;

typedef struct v3{
	float x;
	float y;
	float z;
}v3;

typedef struct v4{
	float x1;
	float y1;
	float x2;
	float y2;
}v4;

v4 v4_v2(v2 a, v2 b);
void rotate_v2(v2 origin, v2* point, float angle);

void lumen_renderer_init(lumen_renderer* renderer, uint32_t w, uint32_t h);
void lumen_renderer_free(lumen_renderer* renderer);

void lumen_render_set_color_rgb(lumen_renderer* renderer, uint8_t r, uint8_t g, uint8_t b);
void lumen_render_set_color_hex(lumen_renderer* renderer, uint32_t color);
void lumen_render_set_alpha(lumen_renderer* renderer, uint8_t alpha);

void lumen_render_reset(lumen_renderer* renderer);
void lumen_render_set_pixel(lumen_renderer* renderer, uint32_t x, uint32_t y, uint32_t pixel);
void lumen_render_set_pixel_v2(lumen_renderer* renderer, v2 point, uint32_t pixel);
void lumen_render_draw_line(lumen_renderer* renderer, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
void lumen_render_draw_line_v2(lumen_renderer* renderer, v2 a, v2 b);
void lumen_render_draw_circle(lumen_renderer* renderer, int32_t x, int32_t y, int32_t r);
void lumen_render_draw_ellipse(lumen_renderer* renderer, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void lumen_render_draw_rect(lumen_renderer* renderer, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
void lumen_render_draw_texture(lumen_renderer* renderer, lumen_texture texture, int32_t x, int32_t y);
void lumen_render_draw_triangle_wireframe(lumen_renderer* renderer, v2 a, v2 b, v2 c);
void lumen_render_draw_triangle(lumen_renderer* renderer, v2 a, v2 b, v2 c);

uint8_t check_image_file_header(char* bytes, uint8_t* header, size_t len);
lumen_texture lumen_texture_load(const char* src);
void lumen_texture_free(lumen_texture* texture);

void lumen_render_put(lumen_renderer* renderer);

char* get_ascii_esc_from_color(uint32_t color);
char* lumen_ascii_convert(uint32_t pixel);

void lumen_input_init(lumen_input* input);
void lumen_input_close(lumen_input* input);
void lumen_input_poll(lumen_input* input);
void lumen_input_event_parse(lumen_input* input);
void lumen_input_new_frame(lumen_input* input);

int32_t tty_raw(lumen_input* input, int32_t fd);
int32_t tty_reset(lumen_input* input, int32_t fd);

float* mat_mult(float* a, uint32_t w, uint32_t h, float* b, uint32_t ww, uint32_t hh);

typedef enum LUMEN_SCANCODE{
	LINP_ESCAPE=1,
	LINP_1=2,
	LINP_2=3,
	LINP_3=4,
	LINP_4=5,
	LINP_5=6,
	LINP_6=7,
	LINP_7=8,
	LINP_8=9,
	LINP_9=10,
	LINP_0=11,
	LINP_MINUS=12,
	LINP_EQUAL=13,
	LINP_BACKSPACE=14,
	LINP_TAB=15,
	LINP_Q=16,
	LINP_W=17,
	LINP_E=18,
	LINP_R=19,
	LINP_T=20,
	LINP_Y=21,
	LINP_U=22,
	LINP_I=23,
	LINP_O=24,
	LINP_P=25,
	LINP_LBRACKET=26,
	LINP_RBRACKET=27,
	LINP_ENTER=28,
	LINP_LCONTROL=29,
	LINP_A=30,
	LINP_S=31,
	LINP_D=32,
	LINP_F=33,
	LINP_G=34,
	LINP_H=35,
	LINP_J=36,
	LINP_K=37,
	LINP_L=38,
	LINP_SEMICOLON=39,
	LINP_APOSTROPHE=40,
	LINP_BACKTICK=41,
	LINP_LSHIFT=42,
	LINP_BACKSLASH=43,
	LINP_Z=4,
	LINP_X=45,
	LINP_C=46,
	LINP_V=47,
	LINP_B=48,
	LINP_N=49,
	LINP_M=50,
	LINP_COMMA=51,
	LINP_PERIOD=52,
	LINP_SLASH=53,
	LINP_RSHIFT=54,
	LINP_LALT=56,
	LINP_CAPSLOCK=58,
	LINP_F1=59,
	LINP_F2=60,
	LINP_F3=61,
	LINP_F4=62,
	LINP_F5=63,
	LINP_F6=64,
	LINP_F7=65,
	LINP_F8=66,
	LINP_F9=67,
	LINP_F10=68,
	LINP_F11=87,
	LINP_F12=88,
	LINP_RCONTROL=97,
	LINP_PRINTSCREEN=99,
	LINP_RALT=100,
	LINP_UP=103,
	LINP_LEFT=105,
	LINP_RIGHT=106,
	LINP_DOWN=108,
	LINP_INSERT=110,
	LINP_DELETE=111,
	LINP_SUPER=125,
	LINP_COUNT
}LUMEN_SCANCODE;

#endif
