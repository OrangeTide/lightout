/* lightout.c - Jon Mayo - PUBLIC DOMAIN - February 2009
 * classic lights-out game */
#include <assert.h>
#include <X11/X.h>
#include <cairo-xlib.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "drawutil.h"
#include "framework.h"

#define BOARD_W 5
#define BOARD_H 5
#define HEADER_H_SCALE .08

static unsigned char board_state[BOARD_W][BOARD_H];
static unsigned char board_dirty[BOARD_W][BOARD_H];
static int board_header_dirty=1, board_all_dirty=1;
static cairo_surface_t *mask_surface;

enum {
	BACKGROUND_COLOR,
	TILE_OFF_COLOR,
	TILE_ON_COLOR,
	HIGHLIGHT_COLOR,
	TEXT_COLOR,
	OUTLINE_COLOR,
};

static double color_table[][3] = {
	{WHITE, WHITE, WHITE}, /* background */
	{GREY1, GREY1, GREY1}, /* button off */
	{GREY2, GREY2, GREY2}, /* button on */
	{BLACK, BLACK, BLACK}, /* button highlight */
	{BLACK, BLACK, BLACK}, /* text */
	{WHITE, WHITE, WHITE}, /* text outline */
};

static const char *level_data[] = {
	"....."
	"....."
	"X.X.X"
	"....."
	"....."
	,
	"X.X.X"
	"X.X.X"
	"....."
	"X.X.X"
	"X.X.X"
	,
	".X.X."
	"XX.XX"
	"XX.XX"
	"XX.XX"
	".X.X."
	,
	"....."
	"XX.XX"
	"....."
	"X...X"
	"XX.XX"
	,
	"XXXX."
	"XXX.X"
	"XXX.X"
	"...XX"
	"XX.XX"
	,
	"....."
	"....."
	"X.X.X"
	"X.X.X"
	".XXX."
	,
	"XXXX."
	"X...X"
	"X...X"
	"X...X"
	"XXXX."
	,
	"....."
	"..X.."
	".X.X."
	"X.X.X"
	".X.X."
	,
	".X.X."
	"XXXXX"
	".XXX."
	".X.XX"
	"XXX.."
	,
	".XXX."
	".XXX."
	".XXX."
	"....."
	"....."
	,
	"X.X.X"
	"X.X.X"
	"X.X.X"
	"X.X.X"
	".XXX."
	,
	"XXXXX"
	".X.X."
	"XX.XX"
	".XXX."
	".X.X."
	,
	"...X."
	"..X.X"
	".X.X."
	"X.X.."
	".X..."
	,
	"....."
	"....."
	".X..."
	".X..."
	".X..."
	,
	"....."
	".X..."
	"....."
	".X..."
	"....."
	,
	"X...."
	"X...."
	"X...."
	"X...."
	"XXXXX"
	,
	"....."
	"....."
	"..X.."
	".XXX."
	"XXXXX"
	,
	"..X.."
	".X.X."
	"X.X.X"
	".X.X."
	"..X.."
	,
	"X.X.X"
	"....."
	"X.X.X"
	"....."
	"X.X.X"
	,
	"....."
	"....."
	"X...X"
	"....."
	"....."
	,
	".XXXX"
	".X..."
	".XXX."
	".X..."
	".X..."
	,
	".XXX."
	"X...X"
	"X...X"
	"X...X"
	".XXX."
	,
	"....."
	"....."
	"..XXX"
	"..XX."
	"..X.."
	,
	"....."
	"....."
	"X...X"
	"XXXXX"
	".X..X"
	,
	"X...."
	"XX..."
	"XXX.."
	"XXXX."
	".XXXX"
	,
	"X...X"
	"X...X"
	"XXXXX"
	"X...X"
	"X...X"
	,
	"..X.."
	".XXX."
	"..X.."
	"..X.."
	"..X.."
	,
	"....."
	"....."
	"..XXX"
	"..XXX"
	"..XXX"
	,
	"....."
	".X..."
	"....."
	"....."
	"....."
	,
	"....."
	"....."
	"..X.."
	"....."
	"....."
	,
	"X...X"
	"XX..X"
	"X.X.X"
	"X..XX"
	"X...X"
	,
	"XXXXX"
	"...X."
	"..X.."
	".X..."
	"XXXXX"
	,
	"...X."
	"...X."
	"X.X.X"
	"X...X"
	"X..XX"
	,
	"..X.X"
	"X...X"
	"X...X"
	".XX.X"
	".XXXX"
	,
	"...XX"
	".X.X."
	"X...X"
	"X.X.X"
	"....."
	,
	"..X.."
	".X.X."
	"X...X"
	"XXXXX"
	"X...X"
	,
	"....."
	".XXX."
	".XXX."
	".XXX."
	"....."
	,
	"X.X.X"
	".X.X."
	"X.X.X"
	".X.X."
	"X.X.X"
	,
	".X.X."
	"X...."
	"XX..."
	"..XX."
	".X.X."
	,
	"....."
	"....."
	".X.X."
	"....."
	"....."
	,
	"X...X"
	".X.X."
	"..X.."
	"..X.."
	"..X.."
	,
	"XXX.."
	"X..X."
	"XXX.."
	"X..X."
	"XXX.."
	,
	"X...X"
	"XX.X."
	"XXX.."
	".X..."
	".XXX."
	,
	"....."
	"XX.XX"
	"XXXXX"
	"..X.."
	".XXX."
	,
	".XXX."
	"X.X.."
	"..XXX"
	"XXXX."
	"X.X.X"
	,
	"..X.."
	".XXX."
	"XXXXX"
	".XXX."
	"..X.."
	,
	"..X.."
	"XXXXX"
	"X.X.."
	".X..X"
	"....X"
	,
	"....."
	"X...X"
	"..X.."
	"X...X"
	"....."
	,
	"X...X"
	".X.X."
	"..X.."
	".X.X."
	"X...X"
	,
	"XXXXX"
	"XXXXX"
	"XXXXX"
	"XXXXX"
	"XXXXX"
};

static void _paint_head(struct module_configuration *mc, cairo_t *c, double header_h, int mask_mode) {
	char buf[64];
	cairo_text_extents_t te;
	double w=mc->board_width;

	if(!board_header_dirty) {
		return; /* ignore */
	}

	if(!mc->disable_fonts) {
		cairo_rectangle(c, 0., 0., w, header_h);
		if(mask_mode && mc->use_shaped) {
			cairo_set_operator(c, CAIRO_OPERATOR_CLEAR);
		}
		drawutil_set_source_rgb(c, color_table[BACKGROUND_COLOR]);
		cairo_fill(c);

		cairo_set_operator(c, CAIRO_OPERATOR_OVER);

		snprintf(buf, sizeof buf, "L%02u", mc->current_level+1);

		cairo_select_font_face(c, mc->default_font_name, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(c, header_h);
		cairo_text_extents(c, buf, &te);
		cairo_move_to(c, w/2-te.width/2-te.x_bearing, header_h/2-te.height/2-te.y_bearing);

		if(mc->use_shaped) {
			cairo_set_line_width(c, 1.5);
			cairo_text_path(c, buf);
			drawutil_set_source_rgb(c, color_table[TEXT_COLOR]);
			cairo_fill_preserve(c);
			drawutil_set_source_rgb(c, color_table[OUTLINE_COLOR]);
			cairo_stroke(c);
		} else {
			drawutil_set_source_rgb(c, color_table[TEXT_COLOR]);
			cairo_show_text(c, buf);
		}
	}

	if(!mask_mode)
		board_header_dirty=0; /* header was painted */
}

static void _paint_board(struct module_configuration *mc, cairo_t *c, double board_width, double board_height, int mask_mode) {
	double button_w, button_h, x_ofs, y_ofs;
	unsigned x, y;

	if(board_all_dirty) {
		cairo_rectangle(c, 0., 0., board_width, board_height);
		drawutil_set_source_rgb(c, color_table[BACKGROUND_COLOR]);
		cairo_fill(c);
		if(!mask_mode)
			board_all_dirty=0;
	}

	button_w=board_width/(double)BOARD_W;
	button_h=board_height/(double)BOARD_H;
	x_ofs=button_w*.2/2;
	y_ofs=button_h*.2/2;

	cairo_set_line_width(c, 5);

	for(y=0;y<BOARD_H;y++) {
		for(x=0;x<BOARD_W;x++) {
			if(board_dirty[x][y]) {

				/* draw the button background */
				cairo_rectangle(c, button_w*x-0.5, button_h*y-0.5, button_w+1, button_h+1);
				if(mask_mode && mc->use_shaped) {
					cairo_set_operator(c, CAIRO_OPERATOR_CLEAR);
				}
				drawutil_set_source_rgb(c, color_table[BACKGROUND_COLOR]);
				cairo_fill(c);

				cairo_set_operator(c, CAIRO_OPERATOR_OVER);

				/* draw the button according to the current style */
				switch(mc->board_style) {
					case 1:
						drawutil_curved_rectangle(c, x_ofs+button_w*x, y_ofs+button_h*y, button_w*.8, button_h*.8);
						break;
					case 2:
						drawutil_round_rectangle(c, x_ofs+button_w*x, y_ofs+button_h*y, button_w*.8, button_h*.8, button_w/16.);
						break;
					default:
						cairo_rectangle(c, x_ofs+button_w*x, y_ofs+button_h*y, button_w*.8, button_h*.8);
				}

				if(board_state[x][y]) {
					drawutil_set_source_rgb(c, color_table[TILE_ON_COLOR]);
				} else {
					drawutil_set_source_rgb(c, color_table[TILE_OFF_COLOR]);
				}

				if(mc->current_selected_object==(int)(x+y*BOARD_W)) {
					cairo_fill_preserve(c);
					drawutil_set_source_rgb(c, color_table[HIGHLIGHT_COLOR]);
					cairo_stroke(c);
				} else {
					cairo_fill(c);
				}

				if(!mask_mode)
					board_dirty[x][y]=0; /* this element has been drawn */
			}
		}
	}

	/*
	cairo_move_to(c, 10., 10.);
	drawutil_set_source_rgb(c, color_table[TEXT_COLOR]);
	cairo_show_text(c, "Hello World");
	*/
}

/* mark entire board as dirty */
static void lightout_dirtyall(void) {
	board_header_dirty=1;
	memset(board_dirty, 1, sizeof board_dirty);
}

static void lightout_clear(void) {
	memset(board_state, 0, sizeof board_state);
	lightout_dirtyall();
	board_header_dirty=1; /* header needs to be repainted */
	board_all_dirty=1; /* board area needs to be repainted */
}

static void lightout_invert(long x, long y) {
	/* fprintf(stderr, "invert %ld, %ld\n", x, y); */
	if(x>=0 && x<BOARD_W && y>=0 && y<BOARD_H) {
		board_state[x][y]=!board_state[x][y];
		board_dirty[x][y]=1;
	}
}

static void lightout_win_pattern(unsigned frame) {
	unsigned x, y, dx, dy;
	lightout_clear();

	if(BOARD_W>BOARD_H) {
		frame%=BOARD_H/2;
	} else {
		frame%=BOARD_W/2;
	}

	dy=2*frame>BOARD_H?BOARD_H/2:frame;
	dx=2*frame>BOARD_W?BOARD_W/2:frame;

	for(y=0;y<BOARD_H-2*dy;y++) {
		board_state[dx][y+dy]=1;
		board_dirty[dx][y+dy]=1;
		board_state[BOARD_W-1-dx][y+dy]=1;
		board_dirty[BOARD_W-1-dx][y+dy]=1;
	}

	for(x=0;x<BOARD_W-2*dx;x++) {
		board_state[x+dx][dy]=1;
		board_dirty[x+dx][dy]=1;
		board_state[x+dx][BOARD_H-1-dy]=1;
		board_dirty[x+dx][BOARD_H-1-dy]=1;
	}
}

/* state = 0 - classic lightout, turn all the lights out */
static int lightout_check_win(int state) {
	unsigned x, y;
	for(y=0;y<BOARD_H;y++) {
		for(x=0;x<BOARD_W;x++) {
			if(board_state[x][y]!=state) return 0;
		}
	}
	return 1; /* you win */
}

static void lightout_load(struct module_configuration *mc) {
	unsigned x, y;
try_again:
	if(mc->current_level>sizeof(level_data)/sizeof(*level_data)) {
		mc->current_level=0; /* start from the beginning */
	}
	lightout_clear();
	for(y=0;y<BOARD_H;y++) {
		for(x=0;x<BOARD_W;x++) {
			char ch;
			ch=level_data[mc->current_level][x+y*BOARD_W];
			if(!ch) {
				fprintf(stderr, "Malformed level data for #%d\n", mc->current_level+1);
				mc->current_level++;
				goto try_again;
			}
			if(ch!='.' && ch!=' ')
				lightout_invert(x, y);
		}
	}
	printf("You are now playing level #%d\n", mc->current_level+1);
}

static void lightout_do_move(unsigned x, unsigned y) {
	if(x<BOARD_W && y<BOARD_H) {
		/* fprintf(stderr, "Push %d, %d\n", (int)x, (int)y); */
		lightout_invert((int)x, (int)y);
		lightout_invert((int)x, (int)y-1);
		lightout_invert((int)x, (int)y+1);
		lightout_invert((int)x-1, (int)y);
		lightout_invert((int)x+1, (int)y);
	}
}

static void lightout_redraw_on_next_paint(struct module_configuration *mc) {
	if(mc->verbose) {
		fprintf(stderr, "%s():called.\n", __func__);
	}
	lightout_dirtyall();
}

static void draw(struct module_configuration *mc, cairo_surface_t *cs, int mask_mode) {
	double header_h;
	cairo_t *c;

	if(mc->verbose>1) {
		fprintf(stderr, "%s():drawing with cairo surface %p\n", __func__, cs);
	}

	c=cairo_create(cs);

#if 0
	/* make certain the destination has no junk on it */
	cairo_set_operator(c, CAIRO_OPERATOR_CLEAR);
	drawutil_set_source_rgb(c, color_table[BACKGROUND_COLOR]);
	cairo_fill(c);
#endif

	cairo_set_operator(c, CAIRO_OPERATOR_OVER); /* return to normal drawing */

	if(!mc->disable_fonts) {
		header_h=mc->board_height*HEADER_H_SCALE;
	} else {
		header_h=0;
	}

	_paint_head(mc, c, header_h, mask_mode);

	cairo_translate(c, 0., header_h);

	_paint_board(mc, c, mc->board_width, mc->board_height-header_h, mask_mode);

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

static void lightout_paint(struct module_configuration *mc, cairo_surface_t *cs) {
	unsigned width=mc->board_width, height=mc->board_height;

	if(!cs) {
		if(mc->verbose) {
			fprintf(stderr, "%s():ignoring paint because surface is NULL.\n", __func__);
		}
		return;
	}

	if(mc->use_shaped) {
		mask_surface_check(cs, width, height);

		assert(mask_surface != NULL);

		/* construct a mask */
		if(mc->verbose) {
			fprintf(stderr, "%s():drawing the mask\n", __func__);
		}
		draw(mc, mask_surface, 1);

		/* uses XShapeCombineMask */
		mask_set(cs, mask_surface);
	}

	/* draw the real thing */
	if(mc->verbose) {
		fprintf(stderr, "%s():drawing the graphics\n", __func__);
	}
	draw(mc, cs, 0);
}

static void lightout_do_if_win(struct module_configuration *mc, cairo_surface_t *cs) {
	if(lightout_check_win(0)) {
		unsigned i;

		printf("You win!\n");

		for(i=0;i<5;i++) {
			lightout_win_pattern(i);
			lightout_paint(mc, cs);
			surface_flush(cs);
			sleep(1);
		}
		mc->current_level++;
		lightout_load(mc);
		lightout_paint(mc, cs);
	}
}

static void lightout_post_press(struct module_configuration *mc, cairo_surface_t *cs, double x, double y) {
	if(!mc->disable_fonts) {
		y-=HEADER_H_SCALE;
	}
	x*=(double)BOARD_W;
	y*=(double)BOARD_H;

	/* check that value is in range - ignore out of range */
	if(x>=0. && x<=(double)BOARD_W && y>=0. && y<=(double)BOARD_H) {
		lightout_do_move(x, y);

		/* mark old position as not being highlighted */
		if(mc->current_selected_object>=0)
			board_dirty[mc->current_selected_object%BOARD_W][mc->current_selected_object/BOARD_W]=1;

		mc->current_selected_object=-1; /* stop highlighting an object */
	}

	lightout_paint(mc, cs);

	lightout_do_if_win(mc, cs);
}

static void move_selection(struct module_configuration *mc, int dx, int dy) {
	const unsigned max_object=BOARD_W*BOARD_H;
	signed pos;

	pos=mc->current_selected_object;

	/* set old position as dirty */
	board_dirty[pos%BOARD_W][pos/BOARD_W]=1;

	pos+=dx;
	pos+=dy*BOARD_W;

	/* if the new position is valid, then copy it */
	if(pos>=0 && pos<(int)max_object) {
		mc->current_selected_object=pos;
	}

	/* set new position as dirty */
	board_dirty[mc->current_selected_object%BOARD_W][mc->current_selected_object/BOARD_W]=1;
}

static void lightout_post_action(struct module_configuration *mc, cairo_surface_t *cs, enum framework_action action) {
	const unsigned max_object=BOARD_W*BOARD_H;

	/* fprintf(stderr, "key = %d\n", action); */

	/* check that selection is in range */
	if(mc->current_selected_object<0 || (unsigned)mc->current_selected_object>=max_object) {
		mc->current_selected_object=BOARD_W*BOARD_H/2; /* highlight a default object */
	}

	switch(action) {
		case ACT_RIGHT:
			move_selection(mc, 1, 0);
			break;
		case ACT_LEFT:
			move_selection(mc, -1, 0);
			break;
		case ACT_UP:
			move_selection(mc, 0, -1);
			break;
		case ACT_DOWN:
			move_selection(mc, 0, 1);
			break;
		case ACT_SELECT: {
			unsigned x, y;
			x=mc->current_selected_object%BOARD_W;
			y=mc->current_selected_object/BOARD_W;
			lightout_do_move(x, y);
			break;
		}
		case ACT_NONE: ;
	}
	lightout_paint(mc, cs);

	lightout_do_if_win(mc, cs);
}

static void init(void) __attribute__((constructor));
static void init(void) {
	module_register("lightout", "Lights-Out", lightout_load, lightout_post_press, lightout_post_action, lightout_paint, lightout_redraw_on_next_paint);
}
