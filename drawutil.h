#ifndef DRAWUTIL_H
#define DRAWUTIL_H

#include <cairo.h>

#define BLACK (0.)
#define GREY1 (1/3.)
#define GREY2 (2/3.)
#define WHITE (1.)

void drawutil_round_rectangle(cairo_t *c, double x, double y, double w, double h, double r);
void drawutil_curved_rectangle(cairo_t *c, double x, double y, double w, double h);
void drawutil_set_source_rgb(cairo_t *c, const double *rgb);
#endif
