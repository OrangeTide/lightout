#ifndef FRAMEWORK_H
#define FRAMEWORK_H
struct base_configuration {
	unsigned game_board_width, game_board_height, game_offset_x, game_header_h;
	unsigned current_level;
	unsigned board_style;
	const char *default_font_name;
};
extern struct base_configuration base_configuration;
#endif
