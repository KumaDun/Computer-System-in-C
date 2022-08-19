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

int main(int argc, char **argv) {
    printf("Registering signal handler...\n");
    registerSignalHandlers();

    int timeout = 0;
    if (argc == 2) {
        timeout = atoi(argv[1]);
    }

    if (timeout < 0) {
        writeToStdout("Invalid input detected. Ignoring timeout value.\n");
        timeout = 0;
    }

    while (1) {
        /* Reset the alarm clock
        */
        alarm(0);
        executeShell(timeout);
    }

    return 0;
}

/* Sends SIGKILL signal to a child process.
 * Error checks for kill system call failure and exits program if
 * there is an error */
void killChildProcess() {
    if (kill(childPid, SIGKILL) == -1) {
        perror("Error in kill");
        exit(EXIT_FAILURE);
    }
}

/* Signal handler for SIGALRM. Catches SIGALRM signal and
 * kills the child process if it exists and is still executing.
 * It then prints out penn-shredder's catchphrase to standard output */
void alarmHandler(int sig) {
    writeToStdout("Bwahaha ... tonight I dine on turtle soup \n");
    killChildProcess();
}

/* Signal handler for SIGINT. Catches SIGINT signal (e.g. Ctrl + C) and
 * kills the child process if it exists and is executing. Does not
 * do anything to the parent process and its execution */
void sigintHandler(int sig) {
    printf("_sigintHandler childPid is %d...\n", childPid);
    if (childPid != 0) {
        killChildProcess();
    }
}


/* Registers SIGALRM and SIGINT handlers with corresponding functions.
 * Error checks for signal system call failure and exits program if
 * there is an error */
void registerSignalHandlers() {
    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        perror("Error in interrupt signal");
        exit(EXIT_FAILURE);
    } else {
    }
    if (signal(SIGALRM,alarmHandler)== SIG_ERR) {
        perror("Error in alarm signal");
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
    //command valid check
    if (command == NULL) {
        writeToStdout("command NULL check \n");
        free(command);
        return;
    }
    char commandFirstChar = command [0];
    if (commandFirstChar == ' ' || commandFirstChar == '\0'){
        writeToStdout("command leading space or leading NULL check \n");
        free(command);
        return;
    }
    
    //execute
    if (command != NULL) {
        childPid = fork();

        if (childPid < 0) {
            perror("Error in creating child process");
            exit(EXIT_FAILURE);
        }

        if (childPid == 0) {
            char *const envVariables[] = {NULL};
            char *const args[] = {command, NULL};
            if (execve(command, args, envVariables) == -1) {
                perror("invalid|No such file or directory");
                exit(EXIT_FAILURE);
            }
        } else {
            /*Note 1:

            The child process can finish by either being terminated naturally (WIFEXITED) or killed by a signal (WIFSIGNALED). 

            Note 2:

            The wait() returns -1 on all failures. When you get an error you should call a perror and exit the parent and the program should run.

            Note 3:

            The wait() system call returns a positive number as long as the child changes its state. The change is not necessary to be terminated or killed.
            To make sure the wait() returns because the child process was finished, we need to use WIFEXITED and WIFSIGNALED macros to verify.
            Therefore, you should put wait() in a do-while loop by checking WIFEXITED and WIFSIGNALED macros to make sure all the exited processes are collected.
            It prevents creating zombie processes.
            
            */

            do {
                /*Start the alarm
                */
                alarm(timeout);
                if (wait(&status) == -1) {
                    perror("Error in child process termination");
                    exit(EXIT_FAILURE);
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            
            /*
            Reset childPid to 0 for parent process so it will not send kill signal after wait() successed and quit
            */
            childPid = 0;
        }
    }
    free(command);
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
    char *inputBuffer = calloc(INPUT_SIZE,sizeof(char));
    if (inputBuffer == NULL) {
        writeToStdout("Memory allocation failure for inputBuffer \n");
        exit(EXIT_FAILURE);
    }

    int numBytes = read(STDIN_FILENO, inputBuffer, INPUT_SIZE);
    
    /*Check for blank input
    Also compatible with input Ctrl+D without text
    */
    if (numBytes == 0) {
        free(inputBuffer);
        writeToStdout("Nothing read \n");
        exit(EXIT_FAILURE);
    }
    
    /*Handle the space issue
    If encoutering space, skip this char and increment spaceCount.
    After loop, if spaceCount = string length of inputBuffer -1, print all space input error and exit
    String lengh include \n at end of inputBuffer, spaceCount does not, -1 to match
    */
    int i = 0;
    int spaceCount = 0;
    for (i=0;i<strlen(inputBuffer);i++) {
        if (inputBuffer[i]==' ') {
            spaceCount++;
        }
    }
    if (spaceCount == strlen(inputBuffer)-1) {
        free(inputBuffer);
        return NULL;
    }

    //replace \n with \0
    i=0;
    for (i=0;i<INPUT_SIZE;i++) {
        if (inputBuffer[i]=='\n') {
            inputBuffer[i] = '\0';
        }
        if (inputBuffer[i] == '\0') {
            break;
        }
    }
    
    //trim leading empty space
    char *bufferStrAddr = inputBuffer;
    while (*bufferStrAddr == ' ') {bufferStrAddr++;}

    //trim ending empty space
    char *bufferEndAddr = inputBuffer+strlen(inputBuffer)-1;
    while (bufferEndAddr>inputBuffer) {
        if (*bufferEndAddr == ' ') {
            *bufferEndAddr = '\0';
            bufferEndAddr--;
        }
        else if (*bufferEndAddr == '\0') {
            bufferEndAddr--;
        }
        else {
            break;
        }
    }
    

    /*Copy char to char from inputBuffer to command
    command does not keep the '\0' string terminator from buffer
    command does not include leading or ending space from buffer
    */
    char *command = malloc((strlen(bufferStrAddr)+1)*sizeof(char));
        //printf("bufferInput valid str is %lu",strlen(bufferStrAddr));
        if (command == NULL) {
            writeToStdout("Memory allocation failure for command \n");
            exit(EXIT_FAILURE);
    }
    i=0;
    while(i < strlen(bufferStrAddr) ){
        command[i] = bufferStrAddr[i];
        i++;
    }
    /*Add NULL at end of the command
    */
    command[i]='\0';

    free(inputBuffer);
    return command;
}
