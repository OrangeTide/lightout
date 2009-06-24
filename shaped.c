/* shaped.c - Jon Mayo
 */
#include <assert.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/X.h>
#include "drawutil.h"
#include "framework.h"

enum {
	COLOR1,
	COLOR2,
};

static cairo_surface_t *mask_surface;

static double color_table[][3] = {
	{GREY2, GREY2, GREY2}, /* donut */
	{GREY1, GREY1, GREY1}, /* donut's outline */
};

static double mask_table[NR(color_table)][3] = {
	{0., 0., 0.}, /* donut */
	{0., 0., 0.}, /* donut's outline */
};

static void shaped_load(struct module_configuration *mc __attribute__((unused))) {
	/* nothing to do here */
}

static void shaped_redraw_on_next_paint(struct module_configuration *mc __attribute__((unused))) {
	/* nothing to do here */
}

static void draw(cairo_surface_t *dest, unsigned width, unsigned height, double palette[][3]) {
	cairo_t *c;

	c=cairo_create(dest);

	/* make certain the destination has no junk on it */
	cairo_set_operator(c, CAIRO_OPERATOR_CLEAR);
	cairo_fill(c);

	cairo_set_line_width(c, 10.);

	/* draw the donut */
	cairo_arc(c,
		width*0.5, height*0.5,
		(width<height?width:height)/2.2,
		0, 2*M_PI
	);

	drawutil_set_source_rgb(c, palette[COLOR1]); /* donut */
	cairo_set_operator(c, CAIRO_OPERATOR_OVER);
	cairo_fill_preserve(c);
	drawutil_set_source_rgb(c, palette[COLOR2]); /* donut's outline */
	cairo_stroke(c);

	/* carve out a hole */
	cairo_arc(c,
		width*0.5, height*0.5,
		(width<height?width:height)/5,
		0, 2*M_PI
	);
	cairo_set_operator(c, CAIRO_OPERATOR_CLEAR);
	cairo_fill_preserve(c);
	cairo_set_operator(c, CAIRO_OPERATOR_OVER);

	/* draw the outline around the hole */
	drawutil_set_source_rgb(c, palette[COLOR2]); /* donut's outline */
	cairo_stroke(c);

	// cairo_show_page(c);
	cairo_destroy(c);
}

/* deal with the mask surface and make sure it is the correct size */
static void mask_surface_check(cairo_surface_t *cs, unsigned width, unsigned height) {
	/* deal with the mask surface */
	if(!mask_surface) {
		mask_surface=make_mask_surface(cs, width, height);
	} else {
		/* detect if mask should be resized and recreate it */
		if((int)height!=cairo_xlib_surface_get_height(mask_surface) || (int)width!=cairo_xlib_surface_get_width(mask_surface)) {
			destroy_mask_surface(mask_surface);
			mask_surface=make_mask_surface(cs, width, height);
		}
	}
}

static void shaped_paint(struct module_configuration *mc, cairo_surface_t *cs) {
	unsigned width=mc->board_width, height=mc->board_height;

	if(!cs) {
		if(mc->verbose) {
			fprintf(stderr, "%s():ignoring paint because surface is NULL.\n", __func__);
		}
		return;
	}

	mask_surface_check(cs, width, height);

	assert(mask_surface != NULL);

	/* construct a mask */
	draw(mask_surface, width, height, mask_table);

	/* uses XShapeCombineMask */
	mask_set(cs, mask_surface);

	/* draw the data */
	draw(cs, width, height, color_table);
}

/* called on a mouse click */
static void shaped_post_press(struct module_configuration *mc, cairo_surface_t *cs, double x __attribute__((unused)), double y __attribute__((unused))) {
	shaped_paint(mc, cs);
}

/* called on a button press */
static void shaped_post_action(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action) {
	switch(action) {
		case ACT_RIGHT:
			break;
		case ACT_LEFT:
			break;
		case ACT_UP:
			break;
		case ACT_DOWN:
			break;
		case ACT_SELECT:
			break;
		case ACT_NONE: ;
	}
	shaped_paint(mc, cs);
}

static void init(void) __attribute__((constructor));
static void init(void) {
	module_register("shaped", "Shape Demo", shaped_load, shaped_post_press, shaped_post_action, shaped_paint, shaped_redraw_on_next_paint);
}
