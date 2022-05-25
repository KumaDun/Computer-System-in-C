#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_SIZE 1024

pid_t childPid = 0;

void executeShell(int timeout);

void writeToStdout(char *text);

void alarmHandler(int sig);

void sigintHandler(int sig);

char *getCommandFromInput();

void registerSignalHandlers();

void killChildProcess();
char trim(char *inputBuffer);


int main(int argc, char **argv) {
    registerSignalHandlers();

    int timeout = 10;
    if (argc == 2) {
        timeout = atoi(argv[1]);
    }

    if (timeout < 0) {
        writeToStdout("Invalid input detected. Ignoring timeout value.\n");
        timeout = 0;
    }

    while (1) {
      //reset childPid to 0
        childPid=0;
        executeShell(timeout);
        alarmHandler(timeout);
        sigintHandler(timeout);
    }

    return 0;
}

/* Sends SIGKILL signal to a child process.
 * Error checks for kill system call failure and exits program if
 * there is an error */
void killChildProcess() {
    printf("killChildProcess func, PID is %s \n",childPid);
    if (kill(childPid, SIGKILL) == -1) {
        perror("Error in kill");
        exit(EXIT_FAILURE);
    }
}

/* Signal handler for SIGALRM. Catches SIGALRM signal and
 * kills the child process if it exists and is still executing.
 * It then prints out penn-shredder's catchphrase to standard output */
void alarmHandler(int sig) {
  if (childPid > 0||childPid<0) {
    writeToStdout("Bwahaha ... tonight I dine on turtle soup");
    killChildProcess();
  }
}

/* Signal handler for SIGINT. Catches SIGINT signal (e.g. Ctrl + C) and
 * kills the child process if it exists and is executing. Does not
 * do anything to the parent process and its execution */
void sigintHandler(int sig) {
  printf("sigintHandler PID is %s \n",childPid);
    if (childPid >0||childPid< 0) {
        killChildProcess();
    }
}


/* Registers SIGALRM and SIGINT handlers with corresponding functions.
 * Error checks for signal system call failure and exits program if
 * there is an error */
void registerSignalHandlers() {
    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        perror("Error in signal");
        exit(EXIT_FAILURE);
    }
}

/* Prints the shell prompt and waits for input from user.
 * Takes timeout as an argument and starts an alarm of that timeout period
 * if there is a valid command. It then creates a child process which
 * executes the command with its arguments.
 *
 * The parent process waits for the child. On unsuccessful completion,
 * it exits the shell. */
void executeShell(int timeout) {
    char *command;
    int status;
    char minishell[] = "penn-shredder# ";
    writeToStdout(minishell);

    command = getCommandFromInput();

    // if the first character of command is empty or null return
    char firstChar = command[0];
    if(firstChar==0x20||firstChar=='\0'){
      free(command);
      return;
    }

    if (command != NULL) {
        childPid = fork();

        if (childPid < 0) {
            perror("Error in creating child process");
            exit(EXIT_FAILURE);
        }

        if (childPid == 0) {
            printf("executeShell if childPID = 0 check, PID is %s \n",childPid);
            char *const envVariables[] = {NULL};
            char *const args[] = {command, NULL};
            if (execve(command, args, envVariables) == -1) {
                perror("Error in execve");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("executeShell if childPID > 0 check, PID is %s \n",childPid);
            do {
                if (wait(&status) == -1) {
                    perror("Error in child process termination");
                    exit(EXIT_FAILURE);
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
}

/* Writes particular text to standard output */
void writeToStdout(char *text) {
    if (write(STDOUT_FILENO, text, strlen(text)) == -1) {
        perror("Error in write");
        exit(EXIT_FAILURE);
    }
}

/* Reads input from standard input till it reaches a new line character.
 * Checks if EOF (Ctrl + D) is being read and exits penn-shredder if that is the case
 * Otherwise, it checks for a valid input and adds the characters to an input buffer.
 *
 * From this input buffer, the first 1023 characters (if more than 1023) or the whole
 * buffer are assigned to command and returned. An \0 is appended to the command so
 * that it is null terminated */
char *getCommandFromInput() {

    char *inputBuffer = (char *) malloc(INPUT_SIZE);

    if(read(STDIN_FILENO,inputBuffer,INPUT_SIZE)==0){
            free(inputBuffer);
            exit(0);
      }
      //insert a null
      int count = 0;
      for(count=0;count<1024;count++){
        if(inputBuffer[count]==0xA){
          inputBuffer[count]='\0';
          if(inputBuffer[count]=='\0'){
            break;
          }
        }
      }
      //remove all leading space
      //point to the begining of inputBuffer
      char *leadingSpace = inputBuffer;
      // remove space until first is not space
      while(1){
        if(*leadingSpace==0x20){
          //move to next
          leadingSpace++;
        }
        else if(*leadingSpace!=0x20){
          break;
        }
      }
      char *inputBufferTrim = malloc(strlen(leadingSpace)+1);
      strcpy(inputBufferTrim,leadingSpace);
      free(inputBuffer);
      //remove all trailing soace
      //point to the end of inputBuffer
      char *trailingSpace = inputBufferTrim+strlen(inputBufferTrim)-1;
      //remove space until last is not space
      while(1){
          if(*trailingSpace==0x20){
            //set as null and go to previous
            *trailingSpace='\0';
            trailingSpace--;
          }
          else if(*trailingSpace=='\0'){
            //set as null and go to previous
            *trailingSpace='\0';
            trailingSpace--;
          }
        else{
          break;
        }
      }

    return inputBufferTrim;
}
