#ifndef FRAMEWORK_H
#define FRAMEWORK_H
#include <sys/queue.h>

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef NR
#define NR(x) (sizeof(x)/sizeof*(x))
#endif

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
	unsigned disable_fonts;
};

struct module_entry {
	const char *name;
	const char *win_title;
	void (*module_load)(struct module_configuration *mc);
	void (*module_post_press)(struct module_configuration *mc, cairo_surface_t *cs, double x, double y);
	void (*module_post_action)(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action);
	void (*module_paint)(struct module_configuration *mc, cairo_surface_t *cs);
	LIST_ENTRY(module_entry) list;
};

int module_register(const char *name, const char *win_title, void (*module_load)(struct module_configuration *mc), void (*module_post_press)(struct module_configuration *mc, cairo_surface_t *cs, double x, double y), void (*module_post_action)(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action), void (*module_paint)(struct module_configuration *mc, cairo_surface_t *cs));
#endif
