/* framework.c - Jon Mayo - PUBLIC DOMAIN - February 2009
 * framework for running various applications */
#include <cairo.h>
#include <cairo-xlib.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/select.h>
#include <unistd.h>
#include <X11/extensions/shape.h>
#include <X11/keysym.h>
#include <X11/X.h>
#include "framework.h"

#define DEFAULT_EVENT_MASK StructureNotifyMask|ExposureMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask

static const char *argv0, *xdisplay_str, *module_name;
static Atom atom_wm_delete_window;
static sig_atomic_t keep_going=1;
static struct module_configuration default_configuration = {
	0, 0, 0, 600, 600, 0, 1, "Lucida", -1, 0, 0, 0, 0,
};
static struct module_configuration current_configuration; /* */
static LIST_HEAD(module_list, module_entry) module_list;

static Display *display;
static Window main_window;
static cairo_surface_t *main_surface;

/* for shape extensions */
static int shape_supported, shape_event_base, shape_error_base;

static void (*module_load)(struct module_configuration *mc);
static void (*module_post_press)(struct module_configuration *mc, cairo_surface_t *cs, double x, double y);
static void (*module_post_action)(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action);
static void (*module_paint)(struct module_configuration *mc, cairo_surface_t *cs);
static void (*module_redraw_on_next_paint)(struct module_configuration *mc);

static void event_resize(Display *display __attribute__((unused)), cairo_surface_t *cs, XEvent *xev) {
	if(current_configuration.board_width!=(unsigned)xev->xconfigure.width || current_configuration.board_height!=(unsigned)xev->xconfigure.height) { /* resize event */
		current_configuration.board_width=xev->xconfigure.width;
		current_configuration.board_height=xev->xconfigure.height;
		cairo_xlib_surface_set_size(cs, current_configuration.board_width, current_configuration.board_height);
		if(module_redraw_on_next_paint) {
			module_redraw_on_next_paint(&current_configuration);
		}
		module_paint(&current_configuration, cs);
		surface_flush(cs);
	}
}

static void event_keypress(Display *display, cairo_surface_t *cs, XEvent *xev) {
	KeySym sym;
	enum framework_action action;

	sym=XKeycodeToKeysym(display, xev->xkey.keycode, 0);

	/* XConvertCase(sym, &sym, &sym_upper); */

	if(current_configuration.verbose) fprintf(stderr, __FILE__ ":key press sym=0x%x\n", (unsigned)sym);

	switch(sym) {
		case XK_Left:
			action=ACT_LEFT;
			break;
		case XK_Prior:
		case XK_Up:
			action=ACT_UP;
			break;
		case XK_Right:
			action=ACT_RIGHT;
			break;
		case XK_Next:
		case XK_Down:
			action=ACT_DOWN;
			break;
		case XK_F1:
		case XK_Return:
		case XK_KP_5:
		case XK_F24:
			action=ACT_SELECT;
			break;
		default:
			action=ACT_NONE;
	}

	if(action!=ACT_NONE) {
		module_post_action(&current_configuration, cs, action);
	}
}

static void do_expose(cairo_surface_t *cs) {
	if(!cs) {
		if(current_configuration.verbose) {
			fprintf(stderr, "%s:ignoring paint because we are not mapped.\n", argv0);
		}
		return;
	}

	if(current_configuration.verbose) {
		fprintf(stderr, "%s:painting.\n", argv0);
	}

	if(module_redraw_on_next_paint) {
		module_redraw_on_next_paint(&current_configuration);
	}

	module_paint(&current_configuration, cs);
	surface_flush(cs);
}

static void do_mapnotify(Window win) {
	XWindowAttributes wattr;

	if(current_configuration.verbose) {
		fprintf(stderr, "%s:MapNotify seen - configuring\n", argv0);
	}

	XGetWindowAttributes(display, win, &wattr);

	current_configuration.board_width=wattr.width;
	current_configuration.board_height=wattr.height;

	if(current_configuration.verbose) {
		switch(wattr.map_state) {
			case IsUnmapped:
				fprintf(stderr, "%s:window 0x%lx is Unmapped.\n", argv0, win);
				break;
			case IsViewable:
				fprintf(stderr, "%s:window 0x%lx is Viewable.\n", argv0, win);
				break;
			case IsUnviewable:
				fprintf(stderr, "%s:window 0x%lx is Unviewable.\n", argv0, win);
				break;
			default:
				fprintf(stderr, "%s:window 0x%lx map_state=%d\n", argv0, win, wattr.map_state);
		}

		fprintf(stderr, "%s:window 0x%lx is %ux%u\n", argv0, win, current_configuration.board_width, current_configuration.board_height);
	}

	do_expose(main_surface);
}

static void event_loop(Display *display) {
	int fdmax, res;
	fd_set rfds;
	struct timeval tv;

	while(keep_going) {
		XEvent xev;

		XFlush(display); /* flush before going to sleep */

		FD_ZERO(&rfds);

		fdmax=ConnectionNumber(display);
		FD_SET(ConnectionNumber(display), &rfds);

		tv.tv_sec=3600; /* TODO: check timer queue */
		tv.tv_usec=0;

		if(current_configuration.verbose>1) {
			fprintf(stderr, "%s:waiting for events...\n", argv0);
		}
		res=select(fdmax+1, &rfds, 0, 0, &tv);
		if(res) {
			// while(XCheckMaskEvent(display, ~ExposureMask, &xev)) { }
			do {
				XNextEvent(display, &xev);
				if(current_configuration.verbose>1) {
					fprintf(stderr, "processing event.. %d\n", xev.type);
				}
				if(xev.type==Expose) {
					if(current_configuration.verbose) {
						fprintf(stderr, "%s:Expose seen (count=%d)\n", argv0, xev.xexpose.count);
					}
					if(xev.xexpose.count==0) {
						do_expose(main_surface);
					}
				} else if(xev.type==UnmapNotify) {
					if(current_configuration.verbose) {
						fprintf(stderr, "%s:UnmapNotify seen\n", argv0);
					}
					/* TODO: release the cairo surface and wait for a mapnotify */
				} else if(xev.type==DestroyNotify) {
					keep_going=0; /* terminate */
				} else if(xev.type==MapNotify && xev.xmap.window==main_window) {
					do_mapnotify(main_window);
				} else if(xev.type==ClientMessage && xev.xclient.format==32 && (Atom)xev.xclient.data.l[0]==atom_wm_delete_window) {
					keep_going=0; /* DONE */
					break;
				} else if(xev.type==ConfigureNotify) {
					event_resize(display, main_surface, &xev);
				} else if(xev.type==ButtonPress) {
					module_post_press(
						&current_configuration,
						main_surface,
						xev.xbutton.x/(double)current_configuration.board_width,
						xev.xbutton.y/(double)current_configuration.board_height
					);
				} else if(xev.type==KeyPress) {
					event_keypress(display, main_surface, &xev);
				} else if(xev.type==ButtonRelease || xev.type==ReparentNotify || xev.type==KeyRelease) {
					/* Ignore */
				} else {
					fprintf(stderr, "Unknown event %d\n", xev.type);
				}
			} while(XEventsQueued(display, QueuedAlready));
		}
		/* TODO: check timer queue */
	}
}

/* replaces some window instead of creating a subwindow */
static Window window_takeover(Display *display, const char *title, Window win, int capture_events_fl) {
	int screen;

	screen=DefaultScreen(display);

	XSetWMProtocols(display, win, &atom_wm_delete_window, 1);

	XStoreName(display, win, title);

	if(capture_events_fl) {
		XSetWindowAttributes swa;

		if(current_configuration.verbose>1) {
			fprintf(stderr, __FILE__ ":capturing events. mask=0x%lx\n", swa.event_mask);
		}

		swa.event_mask = DEFAULT_EVENT_MASK,
		swa.border_pixel=BlackPixelOfScreen(DefaultScreenOfDisplay(display));

		XChangeWindowAttributes(display, win, CWBorderPixel|CWEventMask, &swa);
	}

	return win;
}

/* creates a subwindow of parent_win */
static Window window_setup(Display *display, const char *title, int x, int y, unsigned w, unsigned h, Window parent_win) {
	Window win;
	int screen;
	XSetWindowAttributes swa = {
		.event_mask = DEFAULT_EVENT_MASK,
		.border_pixel = 0,
	};

	screen=DefaultScreen(display);

	win=XCreateWindow(display, parent_win, x, y, w, h, 1, CopyFromParent, InputOutput, CopyFromParent, CWBorderPixel|CWEventMask, &swa);
	XSetWMProtocols(display, win, &atom_wm_delete_window, 1);

	XStoreName(display, win, title);

	XMapRaised(display, win);

	return win;
}

static Display *display_setup(void) {
	Display *display;
	display=XOpenDisplay(xdisplay_str);
	if(!display) {
		fprintf(stderr, "Cannot open display %s\n", xdisplay_str);
		exit(EXIT_FAILURE);
	}

	atom_wm_delete_window=XInternAtom(display, "WM_DELETE_WINDOW", False);
	return display;
}

static void usage(void) __attribute__((noreturn));
static void usage(void) {
	struct module_entry *curr;
	fprintf(stderr, "usage: %s [-verbose] [-display :0.0] [-font <name>] [-nofonts] [-level <level>] [-style 0|1|2] [-window-id <id>] [-nocapture] [-shaped]\n", argv0);
	for(curr=module_list.lh_first;curr;curr=curr->list.le_next) {
		fprintf(stderr, "  -module %s\n", curr->name);
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
		} else if(!strcasecmp(argv[i], "-nocapture")) {
			default_configuration.capture_window=0;
		} else if(!strcasecmp(argv[i], "-module") && i+1<argc) {
			module_name=argv[++i];
		} else if(!strcasecmp(argv[i], "-verbose")) {
			default_configuration.verbose++;
		} else if(!strcasecmp(argv[i], "-window-id")) {
			default_configuration.parent_win=strtoul(argv[++i], 0, 0);
			default_configuration.capture_window=1;
		} else if(!strcasecmp(argv[i], "-shaped")) {
			default_configuration.use_shaped=1;
		} else {
			printf("Unknown or invalid option '%s'\n", argv[i]);
			usage();
		}
	}
}

static int module_get(const char *module_name, const char **win_title) {
	struct module_entry *curr;
	for(curr=module_list.lh_first;curr;curr=curr->list.le_next) {
		if(!strcmp(module_name, curr->name)) {
			*win_title=curr->win_title;
			module_load=curr->module_load;
			module_post_press=curr->module_post_press;
			module_post_action=curr->module_post_action;
			module_paint=curr->module_paint;
			module_redraw_on_next_paint=curr->module_redraw_on_next_paint;
			return 1; /* success */
		}
	}
	return 0;
}

cairo_surface_t *make_mask_surface(cairo_surface_t *mainwin, unsigned width, unsigned height) {
	Window current_win;
	GC gc;
	XGCValues xgcv;
	Pixmap mask_pixmap;

	if(current_configuration.verbose) {
		fprintf(stderr, "%s:creating mask pixmap %ux%u\n", argv0, width, height);
	}

	current_win=cairo_xlib_surface_get_drawable(mainwin);
	mask_pixmap=XCreatePixmap(display, current_win, width, height, 1);

	xgcv.foreground=0;
	xgcv.background=0;
	gc=XCreateGC(display, mask_pixmap, GCForeground|GCBackground, &xgcv);
	XFillRectangle(display, mask_pixmap, gc, 0, 0, width, height);
	XFreeGC(display, gc);

	return cairo_xlib_surface_create_for_bitmap(display, mask_pixmap, DefaultScreenOfDisplay(display), width, height);
}

/* destroy a mask surface */
void destroy_mask_surface(cairo_surface_t *mask_surface) {
	if(mask_surface) {
		Pixmap p=cairo_xlib_surface_get_drawable(mask_surface);
		if(current_configuration.verbose) {
			fprintf(stderr, "%s:freeing mask pixmap %ux%u (p=0x%lx)\n", argv0,
				cairo_xlib_surface_get_width(mask_surface),
				cairo_xlib_surface_get_height(mask_surface),
				p
			);
		}
		if(p!=None) {
			XFreePixmap(display, p);
		}
		cairo_surface_destroy(mask_surface);
	}

}

void mask_set(cairo_surface_t *mainwin, cairo_surface_t *mask) {
	Window win=cairo_xlib_surface_get_drawable(mainwin);
	Pixmap pix=cairo_xlib_surface_get_drawable(mask);

	/* ensure the cairo surface is sent before we use it from X */
	surface_flush(mask);

	XShapeCombineMask(display, win, ShapeBounding, 0, 0, pix, ShapeSet);
}

int module_register(const char *name, const char *win_title, void (*module_load)(struct module_configuration *mc), void (*module_post_press)(struct module_configuration *mc, cairo_surface_t *cs, double x, double y), void (*module_post_action)(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action), void (*module_paint)(struct module_configuration *mc, cairo_surface_t *cs), void (*module_redraw_on_next_paint)(struct module_configuration *mc)) {
	struct module_entry *e;

	e=calloc(1, sizeof *e);
	if(!e) {
		perror("calloc()");
		return 0; /* failure */
	}

	/* there are no plans to free/unregister these entries */
	e->name=name;
	e->win_title=win_title;
	e->module_load=module_load;
	e->module_post_press=module_post_press;
	e->module_post_action=module_post_action;
	e->module_paint=module_paint;
	e->module_redraw_on_next_paint=module_redraw_on_next_paint;
	LIST_INSERT_HEAD(&module_list, e, list);

	return 1; /* success */
}

void surface_flush(cairo_surface_t *cs) {
	cairo_surface_flush(cs);
	XFlush(display);
}

int main(int argc, char **argv) {
	const char *win_title;

	process_args(argc, argv);

	display=display_setup();

	current_configuration=default_configuration;

	if(!module_get(module_name, &win_title)) {
		usage();
	}

	shape_supported=XShapeQueryExtension(display, &shape_event_base, &shape_error_base);

	if(!shape_supported) {
		fprintf(stderr, "%s:Warning: X Shape Extension not supported\n", argv0);
	} else if(current_configuration.verbose) {
		fprintf(stderr, "%s:X Shape Extension event_base=%d error_base=%d\n", argv0, shape_event_base, shape_error_base);
	}

	/* if -window-id was not passed, use root and make a sub-window */
	if(!current_configuration.parent_win) {
		current_configuration.parent_win=DefaultRootWindow(display);
	} else {
		/* force "fullscreen" for swallowed windows
		 * it really means full size of parent window */
		current_configuration.fullscreen_fl=1;
	}

	if(current_configuration.fullscreen_fl) {
		Window rootwin;
		int x, y;
		unsigned border_width, depth, width, height;

		XGetGeometry(display, current_configuration.parent_win, &rootwin, &x, &y, &width, &height, &border_width, &depth);

		current_configuration.window_x=0;
		current_configuration.window_y=0;
		current_configuration.board_width=width;
		current_configuration.board_height=height;
	}

	if(current_configuration.capture_window) {
		XMapRaised(display, current_configuration.parent_win);

		main_window=window_takeover(display, win_title, current_configuration.parent_win, 1);
	} else {
		main_window=window_setup(display, win_title, current_configuration.window_x, current_configuration.window_y, current_configuration.board_width, current_configuration.board_height, current_configuration.parent_win);
	}

	main_surface=cairo_xlib_surface_create(display, main_window, DefaultVisual(display, DefaultScreen(display)), current_configuration.board_width, current_configuration.board_height);

	module_load(&current_configuration);

	do_expose(main_surface);

	event_loop(display);

	XCloseDisplay(display);
	return 0;
}
