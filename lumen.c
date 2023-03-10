#include "lumen.h"
#include "asciiEsc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

void lumen_renderer_init(lumen_renderer* renderer, uint32_t w, uint32_t h){
	renderer->pixels = calloc(w*h,sizeof(uint32_t));
	renderer->w = w;
	renderer->h = h;
	renderer->render_color = 0xffffffff;
	renderer->blendmode = LUMEN_BLENDMODE_ALPHA;
}

void lumen_renderer_free(lumen_renderer* renderer){
	free(renderer->pixels);
}

void lumen_render_reset(lumen_renderer* renderer){
	memset(renderer->pixels, 0, renderer->w*renderer->h*sizeof(uint32_t));
}

void lumen_render_put(lumen_renderer* renderer){
	char* reset_prompt = A_CURSOR_HOME;
	printf(reset_prompt);
	uint32_t x, y;
	char* endline = "\n\r";
	char* clear = "\033[0m";
	size_t cap = renderer->w*renderer->h;
	size_t len = 0;
	char* line = malloc(cap);
	strcpy(line, "");
	for (y=0;y<renderer->h;++y){
		uint32_t row = y*renderer->w;
		for (x=0;x<renderer->w;++x){
			char* ascii_pixel = lumen_ascii_convert(renderer->pixels[row+x]);
			len += strlen(ascii_pixel);
			if (len >= cap){
				line = realloc(line, len+1);
				cap = len+1;
			}
			strcat(line, ascii_pixel);
			free(ascii_pixel);
		}
		len += strlen(endline);
		if (len >= cap){
			line = realloc(line, len+1);
			cap = len+1;
		}
		strcat(line, endline);
	}
	printf(line);
	free(line);
	printf(clear);
}

char* get_ascii_esc_from_color(uint32_t color){
	uint8_t red = (color >> 24) & 0xff;
	uint8_t green = (color >> 16) & 0xff;
	uint8_t blue = (color >> 8) & 0xff;
	if (red+green+blue == 0){
		return "30";
	}
	if (red > green && red > blue){
		return "31";
	}
	if (green > red && green > blue){
		return "32";
	}
	if (blue > red && blue > green){
		return "34";
	}
	return "39";
}

char* lumen_ascii_convert(uint32_t pixel){
	uint8_t alpha = pixel & 0xff;
	char* gradient = " .:-=+*#%@@";
	uint32_t intensity = alpha/(0xff/(strlen(gradient)-1));
	char* output = malloc(8);
	strcpy(output, "\033[1;");
	char* col = get_ascii_esc_from_color(pixel);
	strcat(output, col);
	strcat(output, "m  ");
	output[strlen(output)-1] = gradient[intensity];
	output[strlen(output)-2] = gradient[intensity];
	return output;
}

void lumen_render_set_pixel(lumen_renderer* renderer, uint32_t x, uint32_t y, uint32_t pixel){
	if (x > renderer->w || y > renderer->h) return;
	uint32_t background = renderer->pixels[(y*renderer->w)+x];
	uint8_t bgr = ((background >> 24) & 0xff);
	uint8_t bgg = (background >> 16) & 0xff;
	uint8_t bgb = (background >> 8) & 0xff;
	uint8_t fgr = (pixel >> 24) & 0xff;
	uint8_t fgg = (pixel >> 16) & 0xff;
	uint8_t fgb = (pixel >> 8) & 0xff;
	float fga = (pixel & 0xff)/255.f;
	switch (renderer->blendmode){
		case LUMEN_BLENDMODE_ALPHA:{
			uint8_t r = (fgr*fga) + (bgr*(1.0-fga));
			uint8_t g = (fgg*fga) + (bgg*(1.0-fga));
			uint8_t b = (fgb*fga) + (bgb*(1.0-fga));
			uint32_t output = (r << 24) + (g << 16) + (b << 8) + 0xff;
			renderer->pixels[(y*renderer->w)+x]  = output;
		}
		default:{
			renderer->pixels[(y*renderer->w)+x] = pixel;
		}
	}
}

void lumen_render_set_pixel_v2(lumen_renderer* renderer, v2 point, uint32_t pixel){
	lumen_render_set_pixel(renderer, point.x, point.y, pixel);
}

void lumen_render_draw_line(lumen_renderer* renderer, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2){
	int dx, dy, err, sx, sy, e2;
	dx = abs(x2-x1);
	dy = -abs(y2-y1);
	sx = x1<x2 ? 1 : -1;
	sy = y1<y2 ? 1 : -1;
	err = dx + dy;
	for (;;){
		lumen_render_set_pixel(renderer, x1, y1, renderer->render_color);
		if (x1 == x2 && y1 == y2) break;
		e2 = 2*err;
		if (e2 >= dy){
			err += dy;
			x1 += sx;
		}
		if (e2 <= dx){
			err += dx;
			y1 += sy;
		}
	}
}

void lumen_render_draw_circle(lumen_renderer* renderer, int32_t x, int32_t y, int32_t r){
	int32_t xx = -r, yy = 0, err = 2-2*r;
	do{
		lumen_render_set_pixel(renderer, x-xx, y+yy, renderer->render_color);
		lumen_render_set_pixel(renderer, x-yy, y-xx, renderer->render_color);
		lumen_render_set_pixel(renderer, x+xx, y-yy, renderer->render_color);
		lumen_render_set_pixel(renderer, x+yy, y+xx, renderer->render_color);
		r = err;
		if (r <= yy) err += ++yy*2+1;
		if (r > xx || err > yy) err += ++xx*2+1;
	}while(xx<0);
}

void lumen_render_draw_ellipse(lumen_renderer* renderer, int32_t x1, int32_t y1, int32_t x2, int32_t y2){
	int32_t a = abs(x2-x1);
	int32_t b = abs(y2-y1);
	int32_t b1 = b&1;
	int64_t dx = 4*(1-a)*b*b;
	int64_t dy = 4*(b1+1)*a*a;
	int64_t err = dx+dy+b1*a*a;
	int64_t e2;
	if (x1 > x2) {
		x1 = x2;
		x2 += a;
	}
	if (y1 > y2){
		y1 = y2;
	}
	y1 += (b+1)/2;
	y2 = y1-b1;
	a *= 8*a;
	b1 = 8*b*b;
	do{
		lumen_render_set_pixel(renderer, x2, y1, renderer->render_color);
		lumen_render_set_pixel(renderer, x1, y1, renderer->render_color);
		lumen_render_set_pixel(renderer, x1, y2, renderer->render_color);
		lumen_render_set_pixel(renderer, x2, y2, renderer->render_color);
		e2 = 2*err;
		if (e2 <= dy){
			y1++;
			y2--;
			err += dy += a;
		}
		if (e2 >= dx || 2*err > dy){
			x1++;
			x2--;
			err += dx += b1;
		}
	}while(x1 <= x2);
	while(y1-y2 < b){
		lumen_render_set_pixel(renderer, x1-1, y1, renderer->render_color);
		lumen_render_set_pixel(renderer, x2+1, y1++, renderer->render_color);
		lumen_render_set_pixel(renderer, x1-1, y2, renderer->render_color);
		lumen_render_set_pixel(renderer, x2+1, y2--, renderer->render_color);
	}
}

void lumen_render_draw_rect(lumen_renderer* renderer, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2){
	lumen_render_draw_line(renderer, x1, y1, x2, y1);
	lumen_render_draw_line(renderer, x1, y1, x1, y2);
	lumen_render_draw_line(renderer, x2, y2, x1, y2);
	lumen_render_draw_line(renderer, x2, y2, x2, y1);
}

void lumen_render_draw_line_v2(lumen_renderer* renderer, v2 a, v2 b){
	lumen_render_draw_line(renderer, a.x, a.y, b.x, b.y);
}

void lumen_render_set_color_rgb(lumen_renderer* renderer, uint8_t r, uint8_t g, uint8_t b){
	uint8_t a = renderer->render_color & 0xff;
	renderer->render_color = (r << 24) + (g << 16) + (b << 8) + a;
}

void lumen_render_set_color_hex(lumen_renderer* renderer, uint32_t color){
	uint8_t alpha = renderer->render_color & 0xff;
	renderer->render_color = (color << 8)+alpha;
}

void lumen_render_set_alpha(lumen_renderer* renderer, uint8_t alpha){
	renderer->render_color &= 0xffffff00;
	renderer->render_color += alpha;
}

v4 v4_v2(v2 a, v2 b){
	v4 rect = {a.x, a.y, b.x, b.y};
	return rect;
}

uint8_t check_image_file_header(char* bytes, uint8_t* header, size_t len){
	for (uint32_t i = 0;i<len;++i){
		if (header[i] != bytes[i]){
			return 0;
		}
	}
	return 1;
}

lumen_texture lumen_texture_load(const char* src){
	lumen_texture texture;
	texture.pixels = NULL;
	texture.w = 0;
	texture.h = 0;
	texture.origin_x = 0;
	texture.origin_y = 0;
	texture.scale_x = 1;
	texture.scale_y = 1;
	texture.angle = 0;
	FILE* file = fopen(src, "r");
	if (file == NULL){
		fprintf(stderr, "\033[1mLumen\033[0m tried to open file %s, but encountered an error", src);
		return texture;
	}
	char* data = malloc(READ_BUFFER_SIZE);
	uint32_t offset = 0;
	size_t bytes = 0;
	while((bytes = fread(data+offset, sizeof(*data), READ_BUFFER_SIZE, file)) == READ_BUFFER_SIZE){
		offset += READ_BUFFER_SIZE;
		data = realloc(data, offset+READ_BUFFER_SIZE);
	}
	if (file != stdin){
		fclose(file);
	}
	LUMEN_FILE_TYPE type = LUMEN_READ_NONE;
	uint8_t png_header[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
	size_t len = sizeof(png_header) / sizeof(uint8_t);
	if (check_image_file_header(data, png_header, len)){
		type = LUMEN_READ_PNG;
	}
	uint8_t bmp_header[] = {0x42, 0x4D};
	len = sizeof(png_header) / sizeof(uint8_t);
	if (check_image_file_header(data, bmp_header, len)){
		type = LUMEN_READ_BMP;
	}
	uint8_t ppm_header[] = {'P','6'};
	len = sizeof(ppm_header)/sizeof(uint8_t);
	if (check_image_file_header(data, ppm_header, len)){
		type = LUMEN_READ_PPM;
	}
	switch(type){
		case LUMEN_READ_PNG:{
			//TODO read data and store as png texture
			printf("lumen found a png\n");
			//uint32_t w = (data[16]<< 24) + (data[17]<<16) + (data[18]<<8) + data[19];
			//uint32_t h = (data[20]<< 24) + (data[21]<<16) + (data[22]<<8) + data[23];
		} break;
		case LUMEN_READ_BMP:{
			//TODO read data and store as bmp teexture
			printf("lumen has found a bmp\n");
		} break;
		case LUMEN_READ_PPM:{
			strtok(data, " \n\t");// p6
			int32_t w = atoi(strtok(NULL, " \n\t"));
			int32_t h = atoi(strtok(NULL, " \n\t"));
			char* maxval = strtok(NULL, " \n\t");
			size_t i, n = w*h;
			char* ptr;
			texture.pixels = malloc(sizeof(uint32_t)*n);
			texture.w = w;
			texture.h = h;
			texture.origin_x = w/2;
			texture.origin_y = h/2;
			for (ptr = maxval;*ptr != '\0';ptr++){}
			for (i = 0;i<n;++i){
				uint32_t pixel = 0;
				pixel += *(++ptr) << 24;
				pixel += *(++ptr) << 16;
				pixel += *(++ptr) << 8;
				pixel += 0xff;
				texture.pixels[i] = pixel;
			}
		}break;
		default:{
			fprintf(stderr, "\033[1mLumen\033[0m did not find a suitable image format for %s\n", src);
		} break;
	}
	free(data);
	data = NULL;
	return texture;
}

void lumen_texture_free(lumen_texture* texture){
	free(texture->pixels);
	texture->pixels = NULL;
}

void lumen_render_draw_texture(lumen_renderer* renderer, lumen_texture texture, int32_t x, int32_t y){
	int32_t xx, yy;
	int32_t xc = x+texture.origin_x;
	int32_t yc = y+texture.origin_y;
	size_t m = y+texture.h;
	size_t n = x+texture.w;
	for (yy=y;yy<m;++yy){
		for (xx=x;xx<n;++xx){
			int32_t x0 = (xx-xc)*texture.scale_x;
			int32_t y0 = (yy-yc)*texture.scale_y;
			int32_t xa = (x0*cos(texture.angle)) - (y0*sin(texture.angle));
			int32_t ya = (y0*cos(texture.angle)) + (x0*sin(texture.angle));
			lumen_render_set_pixel(renderer, xc+xa, yc+ya, texture.pixels[((yy-y)*texture.w)+(xx-x)]);
			int32_t i, k;
			for (i = 0;i<ceil(texture.scale_x);++i){
				for (k = 0;k<ceil(texture.scale_y);++k){
					lumen_render_set_pixel(renderer, xc+xa+i, yc+ya+k, texture.pixels[((yy-y)*texture.w)+(xx-x)]);
				}
			}
		}
	}
}

void lumen_input_init(lumen_input* input){
	input->term_saved = 0;
	lumen_input_new_frame(input);
	memset(input->key_held, 0, KEY_READ_COUNT);
	if ((input->file_d = open(INPUT_EVENT_FILE, O_RDONLY)) == -1){
		fprintf(stderr, "\033[1mLumen\033[0m could not open input event file\n");
		return;
	}
	if (tty_raw(input, 0) == -1) fprintf(stderr, "\033[1mLumen\033[0m could not set terminal to noncanonical mode\n");
}

void lumen_input_close(lumen_input* input){
	if (tty_reset(input, 0) == -1) fprintf(stderr, "\033[1mLumen\033[0m could not reset terminal to canonical mode\n");
}

void lumen_input_new_frame(lumen_input* input){
	memset(input->key_pressed, 0, KEY_READ_COUNT);
	memset(input->key_released, 0, KEY_READ_COUNT);
}

void lumen_input_event_parse(lumen_input* input){
	switch(input->event.value){
		case LUMEN_INPUT_RELEASED:{
			input->key_released[input->event.code] = 1;
			input->key_held[input->event.code] = 0;
		}break;
		case LUMEN_INPUT_PRESSED:{
			input->key_pressed[input->event.code] = 1;
			input->key_held[input->event.code] = 1;
		}break;
		default:return;
	}
}

void lumen_input_poll(lumen_input* input){
	lumen_input_new_frame(input);
	if (read(input->file_d, &input->event, sizeof(input->event)) > 0){
		if (input->event.type == EV_KEY){
			lumen_input_event_parse(input);
		}
	}
}

int32_t tty_raw(lumen_input* input, int32_t fd){
	struct termios buf;
	if (tcgetattr(fd, &input->save_termios) < 0) return -1;
	buf = input->save_termios;
	buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);	
	buf.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON);
	buf.c_cflag &= ~(CSIZE | PARENB);
	buf.c_cflag |= CS8;
	buf.c_oflag &= ~(OPOST);
	buf.c_cc[VMIN] = 0;
	buf.c_cc[VTIME] = 0;
	if (tcsetattr(fd, TCSAFLUSH, &buf) < 0) return -1;
	input->term_saved = 1;
	return 0;
}

int32_t tty_reset(lumen_input* input, int32_t fd){
	if (input->term_saved){
		if (tcsetattr(fd, TCSAFLUSH, &input->save_termios) < 0){
			return -1;
		}
	}
	return 0;
}

void lumen_render_draw_triangle_wireframe(lumen_renderer* renderer, v2 a, v2 b, v2 c){
	lumen_render_draw_line_v2(renderer, a, b);
	lumen_render_draw_line_v2(renderer, b, c);
	lumen_render_draw_line_v2(renderer, c, a);
}

void lumen_render_draw_triangle(lumen_renderer* renderer, v2 p1, v2 p2, v2 p3){
	float y1 = p1.y;
	float y2 = p2.y;
	float y3 = p3.y;
	float x1 = p1.x;
	float x2 = p2.x;
	float x3 = p3.x;
	int32_t minx = (int32_t)fmin(fmin(x1, x2), x3);
	int32_t maxx = (int32_t)fmax(fmax(x1, x2), x3);
	int32_t miny = (int32_t)fmin(fmin(y1, y2), y3);
	int32_t maxy = (int32_t)fmax(fmax(y1, y2), y3);
	int32_t x, y;
	for (y = miny;y<maxy;++y){
		for (x = minx;x<maxx;++x){
			if (
				(x1-x2) * (y-y1) - (y1-y2) * (x-x1) > 0 &&
				(x2-x3) * (y-y2) - (y2-y3) * (x-x2) > 0 && 
				(x3-x1) * (y-y3) - (y3-y1) * (x-x3) > 0
			){
				lumen_render_set_pixel(renderer, x, y, renderer->render_color);
			}
		}
	}
}

void rotate_v2(v2 origin, v2* point, float angle){
	float x0 = point->x - origin.x;
	float y0 = point->y - origin.y;
	float xa = (x0*cos(angle)) - (y0*sin(angle));
	float ya = (y0*cos(angle)) + (x0*sin(angle));
	point->x = origin.x + xa;
	point->y = origin.y + ya;
}

float* mat_mult(float* a, uint32_t w, uint32_t h, float* b, uint32_t ww, uint32_t hh){
	// assume matricies are ordered column by column: [c0r0,c0r1,...]
	uint32_t i, k, n, m;
	float* c = malloc(sizeof(float)*h*ww);
	for (i = 0;i<ww;++i){
		for (k = 0;k<h;++k){
			float val = 0;
			for (n = 0;n<hh;++n){
				val += a[(n*h)+k] * b[(i*hh)+n];
			}
			c[(k*hh)+i] = val;
		}
	}
	return 0;
}
