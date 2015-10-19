/* display.h
 * $Id: display.h,v 1.2 2004/09/23 11:11:55 becker Exp $
 * Ralf Becker <nctop@web.de>
 */


#ifndef _DISPLAY_H_

int display_init(void);

void display_kill(void);
	
void display(void);

void display_header(void);

void display_msg(int n);

void display_refresh(void);

void display_scroll(int n);

/* handle keyboard events */
int key_handler(void);

void sig_resize(int signal);

void printlist(void);

#define _DISPLAY_H_
#endif 
