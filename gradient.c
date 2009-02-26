#include <cairo.h>
#include "framework.h"
#include "modules.h"

void gradient_game_load(struct module_configuration *mc) {
	/* NOTHING TO DO */
}

void gradient_game_post_press(struct module_configuration *mc, cairo_surface_t *cs, double x, double y) {
	/* just repaint on clicks */
	gradient_paint(mc, cs);
}

void gradient_paint(struct module_configuration *mc, cairo_surface_t *cs) {
	cairo_t *c;
	cairo_pattern_t *cp;

	c=cairo_create(cs);

	cp=cairo_pattern_create_linear(0., 0., mc->game_board_width, mc->game_board_height);

	cairo_pattern_add_color_stop_rgb(cp, 0, 0., 0., 0.);
	cairo_pattern_add_color_stop_rgb(cp, 1, 1., 1., 1.);

	cairo_rectangle(c, 0., 0., mc->game_board_width, mc->game_board_height);

	cairo_set_source(c, cp);

	cairo_fill(c);

	cairo_show_page(c);
	cairo_destroy(c);
	cairo_pattern_destroy(cp);
}
