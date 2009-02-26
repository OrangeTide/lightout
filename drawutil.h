#ifndef DRAWUTIL_H
#define DRAWUTIL_H
#include <cairo.h>
void drawutil_round_rectangle(cairo_t *c, double x, double y, double w, double h, double r);
void drawutil_curved_rectangle(cairo_t *c, double x, double y, double w, double h);
#endif
