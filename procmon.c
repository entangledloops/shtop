#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
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
      init_pair(1, COLOR_GREEN, COLOR_BLACK);
      init_pair(2, COLOR_YELLOW, COLOR_BLACK);

      unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
      FILE *cpu = fopen("/proc/stat", "r");
      fscanf(cpu, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow, &lastTotalSys, &lastTotalIdle);
      fclose(cpu);

      while (true)
      {
        unsigned long long totalUser, totalUserLow, totalSys, totalIdle;
        FILE *cpu = fopen("/proc/stat", "r");
        fscanf(cpu, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow, &totalSys, &totalIdle);
        fclose(cpu);

        unsigned long long cpuTotal = ( (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) + (totalSys - lastTotalSys) );
        double cpuPercent = cpuTotal;
        cpuTotal += (totalIdle - lastTotalIdle);
        cpuPercent /= cpuTotal;
        cpuPercent *= 100;

        struct sysinfo memInfo;
        sysinfo(&memInfo);
        long long totalVirtualMem = memInfo.totalram;
        long long totalUsedMem = memInfo.totalram - memInfo.freeram;
        totalUsedMem += memInfo.totalswap - memInfo.freeswap;
        totalUsedMem *= memInfo.mem_unit;
        double memPercent = (double) totalUsedMem / (double) totalVirtualMem;

        attron(COLOR_PAIR(1));
        {
          mvwprintw(stdscr, 20, 40, "CPU: %lf\n", cpuPercent);
        }
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(2));
        {
          mvwprintw(stdscr, 21, 40, "MEM: %lf\n", memPercent);
        }
        attroff(COLOR_PAIR(2));
        
        refresh();
      }
  }

  return 0;
}
