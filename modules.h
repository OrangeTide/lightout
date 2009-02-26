#ifndef MODULES_H
#define MODULES_H
void lightout_game_load(struct module_configuration *mc);
void lightout_game_post_press(struct module_configuration *mc, cairo_surface_t *cs, double x, double y);
void lightout_paint(struct module_configuration *mc, cairo_surface_t *cs);
void gradient_game_load(struct module_configuration *mc);
void gradient_game_post_press(struct module_configuration *mc, cairo_surface_t *cs, double x, double y);
void gradient_paint(struct module_configuration *mc, cairo_surface_t *cs);
#endif
