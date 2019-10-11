#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libnotify/notify.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define PID_STR_LEN 7 //max value:2^22
#define OUT_STR_LEN 20
#define CMD_STRT 0
#define CMD_HALT 1
#define CMD_INFO 2
#define CMD_KILL 3

typedef enum {POMO, LBRK, BBRK} pomo_state;

typedef struct {
  unsigned int tmr;
  char *cmt;
} Timers;

static Timers timers[] = {
  /* timer(s)  comment */
  {     1500,  "Time to start working!"},
  {      300,  "Time to start resting!"},
  {     1000,  "Time to take some nap!"},
};

const char pomodlock[] = "/tmp/pomod.lock";
const char pomodout[] = "/tmp/pomod.out";
const char pomodtmp[] = "/tmp/pomod.tmp";

struct timespec remaining;
pomo_state curr_state = POMO;
unsigned int pomo_count = 0;

static volatile sig_atomic_t running;

/* functions implementations */
void die(const char *errstr, ...)
{
  va_list ap;
  va_start(ap, errstr);
  vfprintf(stderr, errstr, ap);
  va_end(ap);
  exit(EXIT_FAILURE);
}

void usage()
{
  die("usage: pomod [start|halt|info|kill]\n");
}

void notify_send(char *cmt)
{
  notify_init("pomod");
  NotifyNotification *n = notify_notification_new("pomod", cmt, "dialog-information");
  notify_notification_show(n, NULL);
  g_object_unref(G_OBJECT(n));
  notify_uninit();
}

void cleanup(void)
{
  unlink(pomodout);
  unlink(pomodtmp);
  unlink(pomodlock);
}

void cmdinforeply_handler(int sig){};

void cmdkill_handler(int sig)
{
  cleanup();
  _exit(EXIT_SUCCESS); //stdio cleanup needed?
}

void cmdstart_handler(int sig)
{
  running = 1;
}

void cmdhalt_handler(int sig)
{
  running = 0;
}

void cmdinfo_handler(int sig, siginfo_t *siginfo, void *ucontext)
{
  char buf[OUT_STR_LEN + 1];
  int tmp_fd, msg_len;
  msg_len = snprintf(buf, OUT_STR_LEN + 1, "%d;%u;%u;%ld",
                     curr_state, running, pomo_count, remaining.tv_sec);
  tmp_fd = open(pomodtmp, O_CREAT | O_WRONLY, 0644);
  if(write(tmp_fd, buf, msg_len + 1) != (msg_len + 1)) _exit(EXIT_FAILURE); //TODO: add loging
  close(tmp_fd);
  if(rename(pomodtmp, pomodout) == -1) _exit(EXIT_FAILURE); //TODO: add logging
  kill(siginfo->si_pid, SIGPWR); //let client know output is ready
}

void daemonize(void)
{
  pid_t sid, pid = fork();
  if(pid < 0) exit(EXIT_FAILURE);
  if(pid > 0) exit(EXIT_SUCCESS);
  umask(0);
  sid = setsid();
  if(sid < 0) {
    exit(EXIT_FAILURE);
  }
  if (chdir("/") < 0) {
    exit(EXIT_FAILURE);
  }
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}

void talk2daemon(int cmd)
{
  int lock_fd, pid_len;
  char buf[PID_STR_LEN + 1];
  lock_fd = open(pomodlock, O_RDONLY);
  if(lock_fd == -1) die("error: open(O_RDONLY) lockfile.\n");
  pid_len = read(lock_fd, buf, PID_STR_LEN + 1);
  close(lock_fd);
  switch(pid_len)
  {
    case 0: die("error: read(). empty lockfile.\n"); break;
    case -1: die("error: read().\n"); break;
  }
  unsigned int srv_pid = strtoul(buf, NULL, 10);
  //TODO: verify that pid in lockfile exists and it's pomod.
  switch(cmd)
  {
    struct sigaction sa;
    sigset_t mask;
    case CMD_STRT: kill(srv_pid, SIGUSR1); break;
    case CMD_HALT: kill(srv_pid, SIGUSR2); break;
    case CMD_INFO:
      /* handle SIGPWR as INFO */
      sa.sa_handler = cmdinforeply_handler;
      sigfillset(&sa.sa_mask);
      sa.sa_flags = 0;
      if(sigaction(SIGPWR, &sa, NULL) == -1)
        exit(EXIT_FAILURE);
      sigemptyset(&mask);
      kill(srv_pid, SIGPWR);
      sleep(1); //workaround
      //sigsuspend(&mask); //TODO: sometimes hangs(overwritten pending signal on server). add a timeout
      //read pomod.out file
      char outbuf[OUT_STR_LEN + 1];
      int out_fd;
      out_fd = open(pomodout, O_RDONLY);
      if(out_fd == -1) die("error: open(O_RDONLY) outfile.\n");
      read(out_fd, outbuf, OUT_STR_LEN + 1);
      close(out_fd);
      printf("%s\n", outbuf);
      break;
    case CMD_KILL: kill(srv_pid, SIGINT); break;
    default: die("unexpected command\n");
  }
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
  int lock_fd;
  char buf[PID_STR_LEN + 1];
  int pid_len, cmd;

  if(argc == 1) usage();
  if(strcmp(argv[1], "start") == 0) cmd = CMD_STRT;
  else if(strcmp(argv[1], "halt") == 0) cmd = CMD_HALT;
  else if(strcmp(argv[1], "info") == 0) cmd = CMD_INFO;
  else if(strcmp(argv[1], "kill") == 0) cmd = CMD_KILL;
  else usage();
  //is server alive?
  lock_fd = open(pomodlock, O_CREAT | O_EXCL | O_WRONLY, 0644);
  if(lock_fd == -1) {
    if(errno != EEXIST) { //unexpected error
      die("error: open(O_CREAT) lockfile.\n");
    } else { //client
      talk2daemon(cmd);
    }
  } else { //server
    if(cmd != CMD_STRT) {
      cleanup();
      die("no daemon available. use 'start'\n");
    }

    daemonize();

    //start the actual program
    atexit(cleanup);
    pid_len = snprintf(buf, PID_STR_LEN + 1, "%u", getpid());
    if(write(lock_fd, buf, pid_len) != pid_len) die("error: write() lockfile.\n");
    close(lock_fd); //maybe move write() after daemon is ready to receive signals.

    struct sigaction sa, sa_info;
    sigset_t haltmask;
    curr_state = POMO;
    pomo_count = 0;
    remaining.tv_sec = timers[curr_state].tmr;
    remaining.tv_nsec = 0;
    running = 1;

    /* handle SIGINT as KILL*/
    sa.sa_handler = cmdkill_handler;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGINT, &sa, NULL) == -1)
      exit(EXIT_FAILURE);

    /* handle SIGUSR1 as START */
    sa.sa_handler = cmdstart_handler;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGUSR1, &sa, NULL) == -1)
      exit(EXIT_FAILURE);

    /* handle SIGUSR2 as HALT */
    sa.sa_handler = cmdhalt_handler;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGUSR2, &sa, NULL) == -1)
      exit(EXIT_FAILURE);

    /* handle SIGPWR as INFO */
    sa_info.sa_sigaction = cmdinfo_handler;
    sigfillset(&sa_info.sa_mask);
    sa_info.sa_flags = SA_SIGINFO;
    if(sigaction(SIGPWR, &sa_info, NULL) == -1)
      exit(EXIT_FAILURE);

    sigemptyset(&haltmask);
    while(1)
    {
      if(running) {
        if(nanosleep(&remaining, &remaining) == 0) {
          remaining.tv_nsec = 0;
          switch(curr_state)
          {
            case POMO:
              pomo_count++;
              curr_state = (pomo_count % 4) ? LBRK : BBRK;
              break;
            case LBRK:
            case BBRK:
              curr_state = POMO;
              break;
          }
          remaining.tv_sec = timers[curr_state].tmr;
          notify_send(timers[curr_state].cmt);
        }
      } else {
        sigsuspend(&haltmask);
        if(running) {
          curr_state = POMO;
          remaining.tv_sec = timers[curr_state].tmr;
        }
      }
    }
  }
}
