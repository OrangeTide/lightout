#include <stddef.h>
#include <cairo.h>
#include <math.h>
#include "framework.h"

static void graphictest_load(struct module_configuration *mc __attribute__((unused))) {
	/* NOTHING TO DO */
}

static void graphictest_paint(struct module_configuration *mc, cairo_surface_t *cs) {
	cairo_t *c;
	cairo_pattern_t *cp, *cn;
	double smaller=MIN(mc->board_width, mc->board_height);

	c=cairo_create(cs);

	cairo_set_line_width(c, smaller*.02);

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

	cairo_arc(c, mc->board_width/2., mc->board_height/2., smaller/3., 0., 2*M_PI);
	cairo_set_source_rgb(c, 1., 1., 1);
	cairo_fill_preserve(c);
	cairo_set_source_rgb(c, 0., 0., 0);
	cairo_stroke(c);

	cairo_show_page(c);
	cairo_destroy(c);
	cairo_pattern_destroy(cp);
}

static void graphictest_post_press(struct module_configuration *mc, cairo_surface_t *cs, double x __attribute__((unused)), double y __attribute__((unused))) {
	/* just repaint on clicks */
	graphictest_paint(mc, cs);
}

static void graphictest_post_action(struct module_configuration *mc __attribute__((unused)), cairo_surface_t *cs __attribute__((unused)), enum framework_action action __attribute__((unused))) {
	/* NOTHING TO DO */
}

static void init(void) __attribute__((constructor));
static void init(void) {
	module_register("graphictest", "Graphic Test", graphictest_load, graphictest_post_press, graphictest_post_action, graphictest_paint, NULL);
}
