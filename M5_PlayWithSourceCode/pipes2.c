#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ARRAY_SIZE 10 // assume args array has less than 10 arguments

int main(int argc, char *argv[]) {
  // create a pipe for communication between child processes
  int fd[2];
  pipe(fd);
  char line[20];
  int n;

  // create one child process for ls
  pid_t pid1 = fork();
  if (pid1 == 0) {
    close(fd[0]); // close read end
    dup2(fd[1], STDOUT_FILENO); // pipe output redirection
    close(fd[1]); // close duplicate write end
    char * args[ARRAY_SIZE];
    args[0] = "ls";
    args[1] = "-l";
    args[2] = NULL; // arguments array should be NULL terminated
    /*write(STDOUT_FILENO,"Write to pid2 \n",15);
    exit(0);*/
    execvp(args[0], args);
  }

  // create another child process for more
  pid_t pid2 = fork();
  if (pid2 == 0) {
    close(fd[1]); // close write end
    dup2(fd[0], STDIN_FILENO);  // pipe input redirection
    close(fd[0]); // close duplicate read end
    n = read(STDIN_FILENO, line, 20);
    //write(STDOUT_FILENO, line, n);
    char *args[ARRAY_SIZE];
    args[0] = "more";
    args[1] = NULL;
    execvp(args[0], args);
    //exit(EXIT_SUCCESS);
  }

  // close both pipe ends in parent process
  close(fd[0]);
  close(fd[1]);

  // wait for all child processes to finish
  int status;
  //-1 means waiting for any child process
  while (waitpid(-1, &status, 0) > 0) {
    // do nothing
  }

  printf("Done!\n");
  return 0;
}
