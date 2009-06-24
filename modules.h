#ifndef MODULES_H
#define MODULES_H
void lightout_load(struct module_configuration *mc);
void lightout_post_press(struct module_configuration *mc, cairo_surface_t *cs, double x, double y);
void lightout_paint(struct module_configuration *mc, cairo_surface_t *cs);
void lightout_post_action(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action);

void gradient_load(struct module_configuration *mc);
void gradient_post_press(struct module_configuration *mc, cairo_surface_t *cs, double x, double y);
void gradient_paint(struct module_configuration *mc, cairo_surface_t *cs);
void gradient_post_action(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action);

void graphictest_load(struct module_configuration *mc);
void graphictest_post_press(struct module_configuration *mc, cairo_surface_t *cs, double x, double y);
void graphictest_paint(struct module_configuration *mc, cairo_surface_t *cs);
void graphictest_post_action(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action);
#endif
