/* display.c
 * $Id: display.c,v 1.6.2.5 2005/07/28 12:35:20 becker Exp $
 * Ralf Becker <nctop@web.de>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ncurses.h>
#include <term.h>
#include <sys/ioctl.h>
#include "globals.h"
#include "slist.h"
#include "display.h"

#ifndef HAVE_STRUCT_LDAT
struct ldat {
        chtype  *text;            /* text of the line */
        NCURSES_SIZE_T firstchar; /* first changed character in the line */
        NCURSES_SIZE_T lastchar;  /* last changed character in the line */
        NCURSES_SIZE_T oldindex;  /* index of the line at last update */
};
#endif


static char cvsid[] = "$Id: display.c,v 1.6.2.5 2005/07/28 12:35:20 becker Exp $";

/* there are three windows
 * the headerlines
 * the table itself
 * the msgline
 */
WINDOW *win;
WINDOW *hdrwin;
WINDOW *htwin;
WINDOW *msgwin;

/* the sizes of the windows */
int HLINES,  HCOLS;
int HTLINES, HTCOLS;
int MLINES,  MCOLS;

/* set to 0 */
int resize = 0;

/* scroll indikator */
int scrlpos = 0;

/* the messages */
char *msg[] = { "q,Q-Quit j,KEY_UP-up k,KEY_DOWN-down" ,
	"unknown command"};

/* hoide the cursor */
void hidecursor(void) {

	leaveok(hdrwin,TRUE);
	leaveok(msgwin,TRUE);
	leaveok(htwin,TRUE);
	curs_set(0);
}

/* initialize display:
 * initialize curses structures and allocate windows
 * set terminal line
 */
int display_init() {

	/* initialize curses display & window */
	if ((win = initscr()) == NULL) {
		display_kill();
		fprintf(stderr, "curses error (initscr)\n");
		return -1;
	}
	/* allocate windows */
	HLINES = 2;
	MLINES = 1;
	HTLINES = LINES - HLINES -MLINES;
	if ((hdrwin = newwin(HLINES, 0, 0, 0)) == NULL) {
		display_kill();
		fprintf(stderr, "curses error (newwin, hdrwin)\n");
		return -1;
	}	
	if ((htwin = newwin(HTLINES, 0, HLINES, 0)) == NULL) {
		display_kill();
		fprintf(stderr, "curses error (newwin, htwin)\n");
		return -1;
	}
	if ((msgwin = newwin(MLINES, 0, LINES-1, 0)) == NULL) {
		display_kill();
		fprintf(stderr, "curses error (newwin, msgwin)\n");
		return -1;
	}

	/* initialize terminal */
	noecho();
	cbreak();
	hidecursor();

	/* setting of message window */
	nodelay(msgwin,TRUE);
	keypad(msgwin,TRUE);
	nodelay(hdrwin,TRUE);
	keypad(hdrwin,TRUE);
	nodelay(htwin,TRUE);
	keypad(htwin,TRUE);

	/* enable scrolling of table */
	scrollok(htwin, TRUE);

	/* print header and initial message */
	display_header();
	display_msg(0);
	
	/* check scrlpos, only usefull for resize */
	if (scrlpos > slist_size(hlist)-HTLINES) {
		scrlpos = slist_size(hlist) - HTLINES;
		if (scrlpos < 0) {
			scrlpos = 0;
		}
	}

	resize = 0;

	return 0;
}

/* remove curses structures */
void display_kill() {

	if (hdrwin != NULL) {
		delwin(hdrwin);
		hdrwin = NULL;
	}
	if (htwin != NULL) {
		delwin(htwin);
		htwin = NULL;
	}
	if (msgwin != NULL) {
		delwin(msgwin);
		msgwin = NULL;
	}
	win = NULL; 
	endwin();
}

/* get new window size and set curses variables */
void display_resize(void) {

	struct winsize ws;

	if (ioctl(1,TIOCGWINSZ, &ws) != -1) {
		LINES = ws.ws_row;
		COLS  = ws.ws_col;
	}
	display_kill();
	refresh();
	display_init();
	display_refresh();
}

/* display the header lines */
void display_header() {

	wattron(hdrwin,A_REVERSE);
	mvwprintw(hdrwin,0,0,"%-10s %-20s %-17s %-26s %3s",
		"Node","Load Avg.","CPU states (%)", \
		"MEM(KB)","#");
	mvwprintw(hdrwin,1,0,"%-10s %-20s %5s %5s %5s %8s %8s %8s %3s",
		" "," ","user","sys","idle","used","free","total"," ");
	wattroff(hdrwin,A_REVERSE);
}

void display_msg(int n) {

	wprintw(msgwin,"%-*s\r",COLS-1,msg[n]);
}
	
/* print line without WRAPPING */
void display_line(WINDOW *win, int i, int n, char *line) {

	int k;

	if (i > win->_maxy) {
		return;
	}
	for (k = 0; (k <= win->_maxx) && (k < n); k++) {
		win->_line[i].text[k] = line[k];
	}
	win->_line[i].firstchar = 0;
	win->_line[i].lastchar  = k;
}

/* print hoststatus line */
int print_hostline(WINDOW *win, int i, struct hostlist_t *p, int COLS,
	int scroll) {

	char line[81];

	if (p->error != NULL) {
		snprintf(line,81,"%-10.10s %-69s",p->name,p->error);
		display_line(win,i,81,line);
		return -1;
	}
	if (p->act == 0) {
		snprintf(line,81,"%-10.10s %-69s",p->name,
			"Resource temporarily unavailable");
		display_line(win,i,81,line);
		return -1;
	}
	snprintf(line,81,
	"%-10.10s %6.2f %6.2f %6.2f %5.1f %5.1f %5.1f %8d %8d %8d %3d",
		p->name,
		p->hoststat.loadavg[0],
		p->hoststat.loadavg[1],
		p->hoststat.loadavg[2],
		(double)p->hoststat.cpu[0]/10.0,
		(double)p->hoststat.cpu[2]/10.0,
		(double)p->hoststat.cpu[3]/10.0,
		p->hoststat.mem[1],
		p->hoststat.mem[2],
		p->hoststat.mem[0],
		p->hoststat.nusers);
	display_line(win,i,81,line);
	p->act-=scroll;
	return 0;
}

/* print host status table */
void display(void) {

	struct hostlist_t *p;
	int i,k;

	if (resize) {
		display_resize();
	}

	i = -1;
	k = 0;
	slist_foreach(hlist, p) {
		i++;
		if (i < scrlpos) {
			continue;
		}
 		if (i >= scrlpos + HTLINES) {
			break;
		} 
		print_hostline(htwin, k, p,COLS,1);
		k++;
	}
}

/* scroll table */
void display_scroll(int n) {

	struct hostlist_t *p;
	int i,k,l;
	int lbound, ubound;

	scrlpos += n;
	if (scrlpos < 0) {
		scrlpos = 0;
		return;
	}
	if (HTLINES >= slist_size(hlist)) {
		scrlpos = 0;
		return;
	}
	if (scrlpos > slist_size(hlist)-HTLINES) {
		scrlpos = slist_size(hlist) - HTLINES;
		return;
	}
	wscrl(htwin,n);
	if (n > 0) {
		k = HTLINES - n;
		ubound = scrlpos + HTLINES;
		lbound = ubound - n;
	} else {
		k = 0;
		lbound = scrlpos;
		ubound = scrlpos - n;
	}
	i = -1;
	l=0;
	slist_foreach(hlist, p) {
		i++;
		if (i < lbound) {
			continue;
		}
		if (i > ubound) {
			break;
		}
		print_hostline(htwin,k,p,COLS,0);
		k++;
	}
	wrefresh(htwin);
}

void display_refresh(void) {

	wnoutrefresh(hdrwin);
	wnoutrefresh(htwin);
	wnoutrefresh(msgwin);
	doupdate();
}

/* handle keyboard events */
int key_handler(void) {

	int ch;

	switch (ch = wgetch(msgwin)) {
		case 'q': { /* fall through */
		}
		case 'Q': {
			return 0;
			break;
		}
		case KEY_DOWN: { /* fall through */
		}
		case 'k': {
			display_scroll(1);
			break;
		}
		case KEY_UP: { /* fall through */
		}
		case 'j': {
			display_scroll(-1);
			break;
		}
		case KEY_RESIZE: { /* resize event */
			break;
		}
		default: {
			display_msg(1);
			display_refresh();
		}
	}
	return 1;
}

void sig_resize (int signal) {

	resize = 1;

}

/* print host status table in batch mode */
void printlist(void) {

	struct hostlist_t *p;

	slist_foreach(hlist, p) {
		if (p->error != NULL) {
			printf("%-10.10s %-69s\n", p->name,p->error);
			continue;
		}
		if (p->act == 0) {
			printf("%-10.10s %-69s\n",
				p->name,"Resource temporarily unavailable");
			continue;
		}
		printf(
		"%-10.10s %6.2f %6.2f %6.2f %5.1f %5.1f %5.1f %8d %8d %8d %3d\n",
			p->name,
			p->hoststat.loadavg[0],
			p->hoststat.loadavg[1],
			p->hoststat.loadavg[2],
			(double)p->hoststat.cpu[0]/10.0,
			(double)p->hoststat.cpu[2]/10.0,
			(double)p->hoststat.cpu[3]/10.0,
			p->hoststat.mem[1],
			p->hoststat.mem[2],
			p->hoststat.mem[0],
			p->hoststat.nusers);
	}
}
