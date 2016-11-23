#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage: /path/to/executable [arguments]\n");
    return -1;
  }

  pid_t pid = fork();
  switch (pid)
  {
    case -1:
      fprintf(stderr, "failed to spawn child process\n");
      return -1;

    case 0:
      execl(argv[1], argv[1], argv[2], (char*) NULL);
      fprintf(stderr, "failed to execute\n");
      return -1;

    default:
      initscr();
      
      int counter = 0;
      int status;
      while (!WIFEXITED(status))
      {
        start_color();      /* Start color      */

        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        attron(COLOR_PAIR(1));
        {
          mvwprintw(stdscr, 0, 10, "%d", counter);
          refresh();
        }
        attroff(COLOR_PAIR(1));
      }
      
      getch();
      endwin();
  }
  return 0;
}
