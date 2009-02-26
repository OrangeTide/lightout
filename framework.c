/* framework.c - Jon Mayo - PUBLIC DOMAIN - February 2009
 * framework for running various applications */
#include <X11/X.h>
#include <cairo-xlib.h>
#include <cairo.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "framework.h"
#include "modules.h"

static const char *argv0, *xdisplay_str, *module_name;
static Atom atom_wm_delete_window;
static struct module_configuration default_module_configuration = {
	500, 500, 0, 40, 0, 1, "Charter"
};
static struct module_configuration current_module_configuration; /* */

static void (*game_load)(struct module_configuration *mc);
static void (*game_post_press)(struct module_configuration *mc, cairo_surface_t *cs, double x, double y);
static void (*paint)(struct module_configuration *mc, cairo_surface_t *cs);

static void event_loop(Display *display, cairo_surface_t *cs) {
	while(1) {
		XEvent xev;
		XNextEvent(display, &xev);
		if(xev.type==Expose) {
			if(xev.xexpose.count<1) {
				paint(&current_module_configuration, cs);
			}
		} else if(xev.type==ClientMessage && xev.xclient.format==32 && xev.xclient.data.l[0]==atom_wm_delete_window) {
			/* DONE */
			break;
		} else if(xev.type==ConfigureNotify) {
			if(current_module_configuration.game_board_width!=xev.xconfigure.width || current_module_configuration.game_board_height!=xev.xconfigure.height) { /* resize event */
				current_module_configuration.game_board_width=xev.xconfigure.width-current_module_configuration.game_offset_x;
				current_module_configuration.game_board_height=xev.xconfigure.height-current_module_configuration.game_header_h;
				cairo_xlib_surface_set_size(cs, current_module_configuration.game_board_width+current_module_configuration.game_offset_x, current_module_configuration.game_board_height+current_module_configuration.game_header_h);
				paint(&current_module_configuration, cs);
			}
		} else if(xev.type==ButtonPress) {
			game_post_press(
				&current_module_configuration,
				cs,
				(xev.xbutton.x-(double)current_module_configuration.game_offset_x)/(double)current_module_configuration.game_board_width,
				(xev.xbutton.y-(double)current_module_configuration.game_header_h)/(double)current_module_configuration.game_board_height
			);
		} else if(xev.type==ButtonRelease || xev.type==ReparentNotify) {
			/* Ignore */
		} else {
			fprintf(stderr, "Unknown event %d\n", xev.type);
		}
	}
}

static Bool wait_for_MapNotify(Display *dpy, XEvent *xev, XPointer arg) {
	return (xev->type==MapNotify) && (xev->xmap.window==(Window)arg);
}

static void map_and_wait(Display *display, Window w) {
	XEvent xev;
	XMapRaised(display, w);
	XIfEvent(display, &xev, wait_for_MapNotify, (XPointer)w);
}

static cairo_surface_t *window_setup(Display *display, const char *title, unsigned w, unsigned h) {
	Window win;
	int screen;
	XSetWindowAttributes swa = {
		.event_mask = StructureNotifyMask|ExposureMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask,
		.border_pixel = 0,
	};

	screen=DefaultScreen(display);

	win=XCreateWindow(display, DefaultRootWindow(display), 0, 0, w, h, 1, CopyFromParent, InputOutput, CopyFromParent, CWBorderPixel|CWEventMask, &swa);
	XSetWMProtocols(display, win, &atom_wm_delete_window, 1);
	map_and_wait(display, win);
	XStoreName(display, win, title);

	return cairo_xlib_surface_create(display, win, DefaultVisual(display, screen), w, h);
}

static Display *display_setup(void) {
	Display *display;
	display=XOpenDisplay(xdisplay_str);
	if(!display) {
		exit(EXIT_FAILURE);
	}

	atom_wm_delete_window=XInternAtom(display, "WM_DELETE_WINDOW", False);
	return display;
}

static void usage(void) {
	fprintf(stderr, "usage: %s [-display :0.0] [-font <name>] [-level <level>] [-style 0|1|2]\n", argv0);
	exit(EXIT_FAILURE);
}

static void process_args(int argc, char **argv) {
	int i;
	if(!(argv0=strrchr(argv[0], '/'))) argv0=argv[0];

	for(i=1;i<argc;i++) {
		if(!strcasecmp(argv[i], "-display") && i+1<argc) {
			xdisplay_str=argv[++i];
		} else if(!strcasecmp(argv[i], "-font") && i+1<argc) {
			default_module_configuration.default_font_name=argv[++i];
		} else if(!strcasecmp(argv[i], "-level") && i+1<argc) {
			default_module_configuration.current_level=strtoul(argv[++i], 0, 10)-1;
		} else if(!strcasecmp(argv[i], "-style") && i+1<argc) {
			default_module_configuration.board_style=strtoul(argv[++i], 0, 10);
		} else if(!strcasecmp(argv[i], "-help")) {
			usage();
		} else if(!strcasecmp(argv[i], "-module") && i+1<argc) {
			module_name=argv[++i];
		} else {
			printf("Unknown or invalid option '%s'\n", argv[i]);
			usage();
		}
	}
}

int main(int argc, char **argv) {
	cairo_surface_t *cs;
	Display *display;
	const char *win_title;

	process_args(argc, argv);

	display=display_setup();

	current_module_configuration=default_module_configuration;

	if(!module_name || !strcmp(module_name, "lightout")) {
		win_title="Lights-Out";
		game_load=lightout_game_load;
		game_post_press=lightout_game_post_press;
		paint=lightout_paint;
	} else if(!strcmp(module_name, "gradient")) {
		win_title="gradient";
		game_load=gradient_game_load;
		game_post_press=gradient_game_post_press;
		paint=gradient_paint;
	} else {
		usage();
	}

	cs=window_setup(display, win_title, current_module_configuration.game_board_width+current_module_configuration.game_offset_x, current_module_configuration.game_board_height+current_module_configuration.game_header_h);

	game_load(&current_module_configuration);

	event_loop(display, cs);

	XCloseDisplay(display);
	return 0;
}
