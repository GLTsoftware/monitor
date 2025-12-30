#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>

struct page_descriptions {
    char cmd;		/* The char one types to get that page */
    char num_slots;	/* the number of 1/4 line units. 0 means unused cmd */
    char *desc;		/* A short description the user will recognize */
} pd[] = {
    {'a', 1, "Antenna summary (Home page)"},
    {'h', 1, "or \"?\" This page"},
    {'m', 1, "Metrology data"},
    {'M', 1, "Maser data"},
    {'P', 1, "Current pointing model"},
    {'q', 1, "Exit"},
};
#define NUM_PD (sizeof(pd) / sizeof(pd[0]))

void help(int count) {
  int line, col;
  int pdn;			/* page description number */

  /* The help page only changes when the structure above is changed
   * and this file recompiled, so there is no need to recalculate the
   * display except when it is being redrawn.
   */
  if ((count % 120) != 1) return;

  /* Initialize Curses Display */
  initscr();
#ifdef LINUX
  clear();
#endif
  move(1,1);
  refresh();
  move(4,28);
  printw("\"gltmonitor\" Help");
  move(7,28);
  printw("Commands:");

  line = 9;
  col = 28;
  for(pdn = 0; pdn < NUM_PD; pdn++) {
  move(line,col);
  printw("\"%c\" %s", pd[pdn].cmd, pd[pdn].desc);
  line++;
  }
/*
  for(pdn = 0; pdn < NUM_PD; pdn++) {
    if(pd[pdn].num_slots == 0) {
    } else {
      if(pd[pdn].num_slots > 1 && col > 40) {
	col = 28;
	line++;
      }
      move(line, col);
      printw("\"%c\" %s", pd[pdn].cmd, pd[pdn].desc);
      col += 20 * pd[pdn].num_slots;
      if(col >= 80) {
	col = 0;
	line++;
      }
    }
  }
*/

  move(23, 0);
  refresh();
}
