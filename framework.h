#ifndef FRAMEWORK_H
#define FRAMEWORK_H
enum framework_action {
	ACT_NONE,
	ACT_UP,
	ACT_DOWN,
	ACT_LEFT,
	ACT_RIGHT,
	ACT_SELECT,
};

struct module_configuration {
	unsigned fullscreen_fl;
	int window_x, window_y;
	unsigned board_width, board_height;
	unsigned current_level;
	unsigned board_style;
	const char *default_font_name;
	int current_selected_object;
};

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#endif
