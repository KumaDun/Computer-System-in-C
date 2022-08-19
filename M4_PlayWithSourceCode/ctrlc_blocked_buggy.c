#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>

static sigset_t intmask;
/*
The buggy code (ctrc_blocked_buggy.c) cannot work because:

if sigprocmask() is called in a signal handler, returning from the handler may undo the work of sigprocmask() by restoring the original pending signal mask. 
Please check this page: https://www.mkssoftware.com/docs/man3/sigprocmask.3.asp

In particular, in the buggy code, within the sig_handler function, we call sigprocmask to unblock the SIGINT signal.
Unfortunately, when it reaches the end of this sig_handler function and returns back to the main function, this unblocking is undone by the OS, which means the SIGINT signal is blocked again. 

As a result, when we raise SIGALRM, it goes to the sig_handler function and unblock the SIGINT.
Then the OS check if there is any suspended SIGINT. If there is, SIGINT will be executed, i.e. the program will exit. Otherwise, it reaches the end of sig_handler and returns back. At this time, the work of sigprocmask() is undone, i.e. SIGINT is still blocked.

If you send SIGINT before raising alarm, SIGINT is blocked and saved.
The program exits after unblock and execute SIGINT in the sig_handler, which is expected behaviour.

However, if you send SIGINT after raising alarm, SIGINT is still blocked, the sig_handler does not  work, and program never exits. 

The solution is to avoid calling sigprocmask() in a signal handler.
The correct code (ctrlc_blocked.c) works because we set a flag in the signal handler and check its value in the main function. 
*/


void sig_handler(int signo)
{
  if (signo == SIGINT) {
    printf("Receives SIGINT signal in handler\n");
    exit(0);
  }

  if (signo == SIGALRM) {
    if (sigprocmask(SIG_UNBLOCK, &intmask, NULL) == -1) {
      perror("Failed to unblock SIGINT");
    } else {
      printf("SIGINT signal unblocked\n");      
    } 
  }
}


int main(void)
{
  //sigemptyset to initialize a signal set
  //sigaddset to add signal into a signal set
  if ((sigemptyset(&intmask) == -1) || 
      (sigaddset(&intmask, SIGINT) == -1)) {
    perror("Failed to initialize the signal mask");
    return 1;
  }

  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    perror("Unable to catch SIGINT\n");
  }

  if (signal(SIGALRM, sig_handler) == SIG_ERR) {
    perror("Unable to catch SIGALRM\n");
  }

  //sigprocmask to add signal in intmask to SIG_BLOCk or SIG_UNBLOCK or to make SIG_BLOCKED equal to intmak by setting how parameter be SIG_SETMASK
  if (sigprocmask(SIG_BLOCK, &intmask, NULL) == -1) {
    perror("Unable to block SIGINT");
  } else {
    fprintf(stderr, "SIGINT signal blocked\n");
  }

  alarm(15);

  while (1) {
    sleep(1);
  }
  return 0;
}
