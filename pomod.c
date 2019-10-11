#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libnotify/notify.h>
#include <time.h>
#include <stdio.h>

void notify_send(char *cmt)
{
  notify_init("pomod");
  NotifyNotification *n = notify_notification_new("pomod", cmt, "dialog-information");
  notify_notification_show(n, NULL);
  g_object_unref(G_OBJECT(n));
  notify_uninit();
}


int main(int argc, char *argv[])
{
  //get params
  //set role
  //is server alive
  //let's create a daemon
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
  //start the actual program
  return 1;
}
