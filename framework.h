#ifndef FRAMEWORK_H
#define FRAMEWORK_H
struct module_configuration {
	unsigned fullscreen_fl;
	int window_x, window_y;
	unsigned game_board_width, game_board_height;
	unsigned current_level;
	unsigned board_style;
	const char *default_font_name;
};

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#endif
