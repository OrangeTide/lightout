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
#include "modules.h"

#define BOARD_W 5
#define BOARD_H 5

static int board_state[BOARD_W][BOARD_H];

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

static void _paint_head(struct module_configuration *mc, cairo_t *c, double header_h) {
	char buf[64];
	cairo_text_extents_t te;
	double w=mc->game_board_width;

	cairo_rectangle(c, 0., 0., w, header_h);
	cairo_set_source_rgb(c, 0., 0., 0.);
	cairo_fill(c);

	snprintf(buf, sizeof buf, "L%02u", mc->current_level+1);

	cairo_set_source_rgb(c, 1., 1., 1.);
	cairo_select_font_face(c, mc->default_font_name, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(c, header_h);
	cairo_text_extents(c, buf, &te);
	cairo_move_to(c, w/2-te.width/2-te.x_bearing, header_h/2-te.height/2-te.y_bearing);
	cairo_show_text(c, buf);

}

static void _paint_board(struct module_configuration *mc, cairo_t *c, double board_width, double board_height) {
	double button_w, button_h, x_ofs, y_ofs;
	unsigned x, y;

	cairo_rectangle(c, 0., 0., board_width, board_height);
	cairo_set_source_rgb(c, 0., 0., 0.);
	cairo_fill(c);

	button_w=board_width/(double)BOARD_W;
	button_h=board_height/(double)BOARD_H;
	x_ofs=button_w*.2/2;
	y_ofs=button_h*.2/2;

	for(y=0;y<BOARD_H;y++) {
		for(x=0;x<BOARD_W;x++) {
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

static void game_clear(void) {
	memset(board_state, 0, sizeof board_state);
}

static void game_invert(long x, long y) {
	/* fprintf(stderr, "invert %ld, %ld\n", x, y); */
	if(x>=0 && x<BOARD_W && y>=0 && y<BOARD_H) {
		board_state[x][y]=!board_state[x][y];
	}
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

void lightout_game_load(struct module_configuration *mc) {
	unsigned x, y;
try_again:
	if(mc->current_level>sizeof(level_data)/sizeof(*level_data)) {
		mc->current_level=0; /* start from the beginning */
	}
	game_clear();
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
				game_invert(x, y);
		}
	}
	printf("You are now playing level #%d\n", mc->current_level+1);
}

void lightout_game_post_press(struct module_configuration *mc, cairo_surface_t *cs, double x, double y) {
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
			lightout_paint(mc, cs);
			XSync(cairo_xlib_surface_get_display(cs), False);
			sleep(1);
		}
		mc->current_level++;
		lightout_game_load(mc);
	}
	lightout_paint(mc, cs);
}

void lightout_paint(struct module_configuration *mc, cairo_surface_t *cs) {
	double header_h=mc->game_board_height*.08;
	cairo_t *c;
	c=cairo_create(cs);

	_paint_head(mc, c, header_h);

	cairo_translate(c, 0., header_h);

	_paint_board(mc, c, mc->game_board_width, mc->game_board_height-header_h);

	cairo_show_page(c);
	cairo_destroy(c);
}
