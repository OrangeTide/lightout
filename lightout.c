/* lightout.c - Jon Mayo - PUBLIC DOMAIN - February 2009
 * classic lights-out game */
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

#define BOARD_W 5
#define BOARD_H 5

static int board_state[BOARD_W][BOARD_H];
static unsigned current_level;

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

static void _draw_round_rectangle(cairo_t *c, double x, double y, double w, double h, double r) {

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

static void _draw_curved_rectangle(cairo_t *c, double x, double y, double w, double h) {

	cairo_save(c);
	cairo_move_to(c, x, y+h/2.);
	cairo_curve_to(c, x, y, x, y, x+w/2., y);
	cairo_curve_to(c, x+w, y, x+w, y, x+w, y+h/2.);
	cairo_curve_to(c, x+w, y+h, x+w, y+h, x+w/2., y+h);
	cairo_curve_to(c, x, y+h, x, y+h, x, y+h/2.);
	cairo_restore(c);
}


static void _paint_head(cairo_t *c) {
	char buf[64];
	cairo_text_extents_t te;
	double w=base_configuration.game_board_width+base_configuration.game_offset_x;

	cairo_rectangle(c, 0., 0., w, base_configuration.game_header_h);
	cairo_set_source_rgb(c, 0., 0., 0.);
	cairo_fill(c);

	snprintf(buf, sizeof buf, "L%02u", current_level+1);

	cairo_set_source_rgb(c, 1., 1., 1.);
	cairo_select_font_face(c, base_configuration.default_font_name, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(c, base_configuration.game_header_h);
	cairo_text_extents(c, buf, &te);
	cairo_move_to(c, w/2-te.width/2-te.x_bearing, base_configuration.game_header_h/2-te.height/2-te.y_bearing);
	cairo_show_text(c, buf);

}

static void _paint_board(cairo_t *c) {
	double button_w, button_h, x_ofs, y_ofs;
	unsigned x, y;

	cairo_translate(c, base_configuration.game_offset_x, base_configuration.game_header_h);

	cairo_rectangle(c, 0., 0., base_configuration.game_board_width, base_configuration.game_board_height);
	cairo_set_source_rgb(c, 0., 0., 0.);
	cairo_fill(c);

	button_w=base_configuration.game_board_width/(double)BOARD_W;
	button_h=base_configuration.game_board_height/(double)BOARD_H;
	x_ofs=button_w*.2/2;
	y_ofs=button_h*.2/2;

	for(y=0;y<BOARD_H;y++) {
		for(x=0;x<BOARD_W;x++) {
			switch(base_configuration.board_style) {
				case 1:
					_draw_curved_rectangle(c, x_ofs+button_w*x, y_ofs+button_h*y, button_w*.8, button_h*.8);
					break;
				case 2:
					_draw_round_rectangle(c, x_ofs+button_w*x, y_ofs+button_h*y, button_w*.8, button_h*.8, button_w/16.);
					break;
				default:
					cairo_rectangle(c, x_ofs+button_w*x, y_ofs+button_h*y, button_w*.8, button_h*.8);
			}

			if(board_state[x][y]) {
				cairo_set_source_rgb(c, 1.0, 0.7, 0.0);
			} else {
				cairo_set_source_rgb(c, 0.2, 0.2, 0.6);
			}
			cairo_fill(c);

		}
	}

	/*
	cairo_move_to(c, 10., 10.);
	cairo_set_source_rgb(c, 1., 1., 1.);
	cairo_show_text(c, "Hello World");
	*/

}

void paint(cairo_surface_t *cs) {
	cairo_t *c;
	c=cairo_create(cs);

	_paint_head(c);

	_paint_board(c);

	cairo_show_page(c);
	cairo_destroy(c);
}

static void game_clear(void) {
	memset(board_state, 0, sizeof board_state);
}

static void game_invert(long x, long y) {
	/* fprintf(stderr, "invert %ld, %ld\n", x, y); */
	if(x>=0 && x<BOARD_W && y>=0 && y<BOARD_H) {
		board_state[x][y]=!board_state[x][y];
	}
}

void game_load(unsigned board) {
	unsigned x, y;
	current_level=board;
try_again:
	if(board>sizeof(level_data)/sizeof(*level_data)) {
		board=0; /* start from the beginning */
	}
	game_clear();
	for(y=0;y<BOARD_H;y++) {
		for(x=0;x<BOARD_W;x++) {
			char ch;
			ch=level_data[board][x+y*BOARD_W];
			if(!ch) {
				fprintf(stderr, "Malformed level data for #%d\n", board);
				board++;
				goto try_again;
			}
			if(ch!='.' && ch!=' ')
				game_invert(x, y);
		}
	}
	printf("You are now playing level #%d\n", board+1);
}

static void game_win_pattern(unsigned frame) {
	unsigned x, y, dx, dy;
	game_clear();

	if(BOARD_W>BOARD_H) {
		frame%=BOARD_H/2;
	} else {
		frame%=BOARD_W/2;
	}

	dy=2*frame>BOARD_H?BOARD_H/2:frame;
	dx=2*frame>BOARD_W?BOARD_W/2:frame;

	for(y=0;y<BOARD_H-2*dy;y++) {
		board_state[dx][y+dy]=1;
		board_state[BOARD_W-1-dx][y+dy]=1;
	}

	for(x=0;x<BOARD_W-2*dx;x++) {
		board_state[x+dx][dy]=1;
		board_state[x+dx][BOARD_H-1-dy]=1;
	}
}

/* state = 0 - classic game, turn all the lights out */
static int game_check_win(int state) {
	unsigned x, y;
	for(y=0;y<BOARD_H;y++) {
		for(x=0;x<BOARD_W;x++) {
			if(board_state[x][y]!=state) return 0;
		}
	}
	return 1; /* you win */
}

void game_post_press(cairo_surface_t *cs, double x, double y) {
	x*=(double)BOARD_W;
	y*=(double)BOARD_H;

	/* check that value is in range - ignore out of range */
	if(x>=0. && x<=(double)BOARD_W && y>=0. && y<=(double)BOARD_H) {
		/* fprintf(stderr, "Push %d, %d\n", (int)x, (int)y); */
		game_invert((int)x, (int)y);
		game_invert((int)x, (int)y-1);
		game_invert((int)x, (int)y+1);
		game_invert((int)x-1, (int)y);
		game_invert((int)x+1, (int)y);
	}

	if(game_check_win(0)) {
		unsigned i;

		printf("You win!\n");

		for(i=0;i<5;i++) {
			game_win_pattern(i);
			paint(cs);
			XSync(cairo_xlib_surface_get_display(cs), False);
			sleep(1);
		}
		game_load(++base_configuration.current_level);
	}
	paint(cs);
}
