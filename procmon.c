#define _GNU_SOURCE
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ncurses.h>

void interrupt_handler(int signal)
{
  endwin();
  exit(1);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "usage: /path/to/executable [arguments]\n");
    return -1;
  }

  struct sigaction handler;
  handler.sa_handler = interrupt_handler;
  sigemptyset(&handler.sa_mask);
  handler.sa_flags = 0;


  int status;
  int pipefd[2];
  FILE *output;
  char buffer[1024];

  if (pipe(pipefd) < 0)
  {
    perror("failed to pipe output from command\n");
    return -1;
  }

  pid_t pid = fork();
  switch (pid)
  {
    case -1:
      fprintf(stderr, "failed to spawn child process\n");
      return -1;

    case 0:
      dup3(pipefd[1], STDOUT_FILENO, O_CLOEXEC);
      close(pipefd[0]);
      close(pipefd[1]);

      execl(argv[1], argv[1], argv[2], (char*) NULL);
      fprintf(stderr, "failed to execute\n");
      return -1;

    default:
      sigaction(SIGINT, &handler, NULL);

      close(pipefd[1]);
      output = fdopen(pipefd[0], "r");

      initscr();
      start_color();
      
      int counter = 0;
      while (true)
      {
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        attron(COLOR_PAIR(1));
        {
          //if (fgets(buffer, sizeof buffer, output)) mvwprintw(stdscr, 0, 10, "%s", buffer);
          mvwprintw(stdscr, 0, 10, "%d", ++counter);
          refresh();
        }
        attroff(COLOR_PAIR(1));
      }
  }

  return 0;
}
