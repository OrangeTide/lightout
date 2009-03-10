#include <cairo.h>
#include <math.h>
#include "framework.h"

static void gradient_load(struct module_configuration *mc __attribute__((unused))) {
	/* NOTHING TO DO */
}

static void gradient_paint(struct module_configuration *mc, cairo_surface_t *cs) {
	cairo_t *c;
	cairo_pattern_t *cp, *cn;

	c=cairo_create(cs);

	cp=cairo_pattern_create_linear(0., 0., 0., mc->board_height);
	cairo_pattern_set_filter(cp, CAIRO_FILTER_GAUSSIAN);
	cairo_pattern_add_color_stop_rgb(cp, 0, 0., 0., 0.);
	cairo_pattern_add_color_stop_rgb(cp, 1, 1., 1., 1.);

	cn=cairo_pattern_create_linear(0., 0., 0., mc->board_height);
	cairo_pattern_set_filter(cn, CAIRO_FILTER_NEAREST);
	cairo_pattern_add_color_stop_rgb(cn, 0, 0., 0., 0.);
	cairo_pattern_add_color_stop_rgb(cn, 1, 1., 1., 1.);

	cairo_rectangle(c, 0., 0., mc->board_width/2., mc->board_height);
	cairo_set_source(c, cp);
	cairo_fill(c);

	cairo_rectangle(c, mc->board_width/2., 0., mc->board_width/2., mc->board_height);
	cairo_set_source(c, cn);
	cairo_fill(c);

	cairo_show_page(c);
	cairo_destroy(c);
	cairo_pattern_destroy(cp);
}

static void gradient_post_press(struct module_configuration *mc, cairo_surface_t *cs, double x, double y) {
	/* just repaint on clicks */
	gradient_paint(mc, cs);
}

static void gradient_post_action(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action) {
	/* NOTHING TO DO */
}

static void init(void) __attribute__((constructor));
static void init(void) {
	module_register("gradient", "Gradient", gradient_load, gradient_post_press, gradient_post_action, gradient_paint);
}
