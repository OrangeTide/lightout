/* framework.c - Jon Mayo - PUBLIC DOMAIN - February 2009
 * framework for running various applications */
#include <X11/X.h>
#include <X11/keysym.h>
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
static struct module_configuration default_configuration = {
	0, 0, 0, 500, 500*1.08, 0, 1, "Times", -1, 0,
};
static struct module_configuration current_configuration; /* */

static struct {
	const char *name;
	const char *win_title;
	void (*module_load)(struct module_configuration *mc);
	void (*module_post_press)(struct module_configuration *mc, cairo_surface_t *cs, double x, double y);
	void (*module_post_action)(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action);
	void (*module_paint)(struct module_configuration *mc, cairo_surface_t *cs);
} module_list[] = {
	{"lightout", "Lights-Out", lightout_load, lightout_post_press, lightout_post_action, lightout_paint},
	{"gradient", "gradient", gradient_load, gradient_post_press, gradient_post_action, gradient_paint},
	{"graphictest", "graphictest", graphictest_load, graphictest_post_press, graphictest_post_action, graphictest_paint},
};

static void (*module_load)(struct module_configuration *mc);
static void (*module_post_press)(struct module_configuration *mc, cairo_surface_t *cs, double x, double y);
static void (*module_post_action)(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action);
static void (*module_paint)(struct module_configuration *mc, cairo_surface_t *cs);

static void event_loop(Display *display, cairo_surface_t *cs) {
	while(1) {
		XEvent xev;
		XNextEvent(display, &xev);
		if(xev.type==Expose) {
			if(xev.xexpose.count<1) {
				module_paint(&current_configuration, cs);
			}
		} else if(xev.type==ClientMessage && xev.xclient.format==32 && (Atom)xev.xclient.data.l[0]==atom_wm_delete_window) {
			/* DONE */
			break;
		} else if(xev.type==ConfigureNotify) {
			if(current_configuration.board_width!=(unsigned)xev.xconfigure.width || current_configuration.board_height!=(unsigned)xev.xconfigure.height) { /* resize event */
				current_configuration.board_width=xev.xconfigure.width;
				current_configuration.board_height=xev.xconfigure.height;
				cairo_xlib_surface_set_size(cs, current_configuration.board_width, current_configuration.board_height);
				module_paint(&current_configuration, cs);
			}
		} else if(xev.type==ButtonPress) {
			module_post_press(
				&current_configuration,
				cs,
				xev.xbutton.x/(double)current_configuration.board_width,
				xev.xbutton.y/(double)current_configuration.board_height
			);
		} else if(xev.type==KeyPress) {
			KeySym sym;
			enum framework_action action;
			sym=XKeycodeToKeysym(display, xev.xkey.keycode, 0);

			/* XConvertCase(sym, &sym, &sym_upper); */

			switch(sym) {
				case XK_Left:
					action=ACT_LEFT;
					break;
				case XK_Up:
					action=ACT_UP;
					break;
				case XK_Right:
					action=ACT_RIGHT;
					break;
				case XK_Down:
					action=ACT_DOWN;
					break;
				case XK_F1:
				case XK_Return:
				case XK_KP_5:
					action=ACT_SELECT;
					break;
				default:
					action=ACT_NONE;
			}

			if(action!=ACT_NONE) {
				module_post_action(&current_configuration, cs, action);
			}
		} else if(xev.type==ButtonRelease || xev.type==ReparentNotify || xev.type==KeyRelease) {
			/* Ignore */
		} else {
			fprintf(stderr, "Unknown event %d\n", xev.type);
		}
	}
}

static Bool wait_for_MapNotify(Display *_dpy_unused __attribute__((unused)), XEvent *xev, XPointer arg) {
	return (xev->type==MapNotify) && (xev->xmap.window==(Window)arg);
}

static void map_and_wait(Display *display, Window w) {
	XEvent xev;
	XMapRaised(display, w);
	XIfEvent(display, &xev, wait_for_MapNotify, (XPointer)w);
}

static cairo_surface_t *window_setup(Display *display, const char *title, int x, int y, unsigned w, unsigned h) {
	Window win;
	int screen;
	XSetWindowAttributes swa = {
		.event_mask = StructureNotifyMask|ExposureMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask,
		.border_pixel = 0,
	};

	screen=DefaultScreen(display);

	win=XCreateWindow(display, DefaultRootWindow(display), x, y, w, h, 1, CopyFromParent, InputOutput, CopyFromParent, CWBorderPixel|CWEventMask, &swa);
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

static void usage(void) __attribute__((noreturn));
static void usage(void) {
	unsigned i;
	fprintf(stderr, "usage: %s [-display :0.0] [-font <name>] [-nofonts] [-level <level>] [-style 0|1|2]\n", argv0);
	for(i=0;i<NR(module_list);i++) {
		fprintf(stderr, "  -module %s\n", module_list[i].name);
	}
	exit(EXIT_FAILURE);
}

static void process_args(int argc, char **argv) {
	int i;
	if(!(argv0=strrchr(argv[0], '/'))) argv0=argv[0]; else argv0++;

	if(!module_name)
		module_name=argv0;

	for(i=1;i<argc;i++) {
		if(!strcasecmp(argv[i], "-display") && i+1<argc) {
			xdisplay_str=argv[++i];
		} else if(!strcasecmp(argv[i], "-font") && i+1<argc) {
			default_configuration.default_font_name=argv[++i];
		} else if(!strcasecmp(argv[i], "-level") && i+1<argc) {
			default_configuration.current_level=strtoul(argv[++i], 0, 10)-1;
		} else if(!strcasecmp(argv[i], "-style") && i+1<argc) {
			default_configuration.board_style=strtoul(argv[++i], 0, 10);
		} else if(!strcasecmp(argv[i], "-fullscreen")) {
			default_configuration.fullscreen_fl=1;
		} else if(!strcasecmp(argv[i], "-geometry")) {
			int x, y;
			unsigned w, h;
			XParseGeometry(argv[++i], &x, &y, &w, &h);
			default_configuration.board_width=w;
			default_configuration.board_height=h;
			default_configuration.window_x=x;
			default_configuration.window_y=y;
		} else if(!strcasecmp(argv[i], "-help")) {
			usage();
		} else if(!strcasecmp(argv[i], "-nofonts")) {
			default_configuration.disable_fonts=1;
		} else if(!strcasecmp(argv[i], "-module") && i+1<argc) {
			module_name=argv[++i];
		} else {
			printf("Unknown or invalid option '%s'\n", argv[i]);
			usage();
		}
	}
}

static int module_find(const char *module_name, const char **win_title) {
	unsigned i;
	for(i=0;i<NR(module_list);i++) {
		if(!strcmp(module_name, module_list[i].name)) {
			*win_title=module_list[i].win_title;
			module_load=module_list[i].module_load;
			module_post_press=module_list[i].module_post_press;
			module_post_action=module_list[i].module_post_action;
			module_paint=module_list[i].module_paint;
			return 1; /* success */
		}
	}
	return 0;
}

int main(int argc, char **argv) {
	cairo_surface_t *cs;
	Display *display;
	const char *win_title;

	process_args(argc, argv);

	display=display_setup();

	current_configuration=default_configuration;
	
	if(!module_find(module_name, &win_title)) {
		usage();
	}

	if(current_configuration.fullscreen_fl) {
		Window rootwin;
		int x, y;
		unsigned border_width, depth;
		XGetGeometry(display, DefaultRootWindow(display), &rootwin, &x, &y, &current_configuration.board_width, &current_configuration.board_height, &border_width, &depth);
	}

	cs=window_setup(display, win_title, current_configuration.window_x, current_configuration.window_y, current_configuration.board_width, current_configuration.board_height);

	module_load(&current_configuration);

	event_loop(display, cs);

	XCloseDisplay(display);
	return 0;
}
