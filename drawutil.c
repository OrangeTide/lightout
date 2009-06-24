/* drawutil.c - Jon Mayo - PUBLIC DOMAIN - February 2009
 */
#include <assert.h>
#include <stddef.h>
#include <cairo.h>
#include <math.h>
#include "drawutil.h"

void drawutil_round_rectangle(cairo_t *c, double x, double y, double w, double h, double r) {

	/* radius at most half the width or height */
	if((r>h/2.))
		r=h/2.;
	if((r>w/2.))
		r=w/2.;

	cairo_save(c);
	cairo_move_to(c, x, y+r);
	cairo_arc(c, x+r, y+r, r, M_PI, -M_PI/2.);
	cairo_line_to(c, x+w-r, y);
	cairo_arc(c, x+w-r, y+r, r, -M_PI/2., 0.);
	cairo_line_to(c, x+w, y+h-r);
	cairo_arc(c, x+w-r, y+h-r, r, 0., M_PI/2.);
	cairo_line_to(c, x+r, y+h);
	cairo_arc(c, x+r, y+h-r, r, M_PI/2., M_PI);
	cairo_close_path(c);
	cairo_restore(c);
}

void drawutil_curved_rectangle(cairo_t *c, double x, double y, double w, double h) {

	cairo_save(c);
	cairo_move_to(c, x, y+h/2.);
	cairo_curve_to(c, x, y, x, y, x+w/2., y);
	cairo_curve_to(c, x+w, y, x+w, y, x+w, y+h/2.);
	cairo_curve_to(c, x+w, y+h, x+w, y+h, x+w/2., y+h);
	cairo_curve_to(c, x, y+h, x, y+h, x, y+h/2.);
	cairo_restore(c);
}

void drawutil_set_source_rgb(cairo_t *c, const double *rgb) {
	assert(c != NULL);
	assert(rgb != NULL);
	cairo_set_source_rgb (c, rgb[0], rgb[1], rgb[2]);
}
