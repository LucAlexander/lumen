#ifndef LUMEN_H
#define LUMEN_H

#include <inttypes.h>
#include <stddef.h>
#include <termios.h>

#define READ_BUFFER_SIZE 16
#define KEY_READ_COUNT 256

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
}lumen_texture;

typedef struct lumen_input{
	struct termios save_termios;
	uint8_t term_saved;
	uint8_t key_pressed[KEY_READ_COUNT];
}lumen_input;

typedef struct v2{
	float x;
	float y;
}v2;

typedef struct v4{
	float x1;
	float y1;
	float x2;
	float y2;
}v4;

v4 v4_v2(v2 a, v2 b);

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
void lumen_render_draw_texture(lumen_renderer* renderer, lumen_texture texture, uint32_t x, uint32_t y);

uint8_t check_image_file_header(char* bytes, uint8_t* header, size_t len);
lumen_texture lumen_texture_load(const char* src);
void lumen_texture_free(lumen_texture* texture);

void lumen_render_put(lumen_renderer* renderer);

char* get_ascii_esc_from_color(uint32_t color);
char* lumen_ascii_convert(uint32_t pixel);

void lumen_input_init(lumen_input* input);
void lumen_input_poll(lumen_input* input);
void lumen_input_new_frame(lumen_input* input);

int32_t tty_raw(lumen_input* input, int32_t fd);
int32_t tty_reset(lumen_input* input, int32_t fd);

#endif
