#include <ncurses.h>
#include <string.h>

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage: /path/to/executable [arguments]");
    return -1;
  }

  initscr();
  
  start_color();      /* Start color      */
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  attron(COLOR_PAIR(1));

  mvwprintw(stdscr, 0, 10, "%s", argv[1]);
  refresh();
  
  attroff(COLOR_PAIR(1));

  getch();
  endwin();

  return 0;
}
