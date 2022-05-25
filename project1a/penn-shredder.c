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
        childPid = 100;
        printf("Exevuting command in a loop...\n");
        executeShell(timeout);
    }

    return 0;
}

/* Sends SIGKILL signal to a child process.
 * Error checks for kill system call failure and exits program if
 * there is an error */
void killChildProcess() {
    printf("_killChildProcess childPid is %d \n", childPid);
    if (kill(childPid, SIGKILL) == -1) {
        perror("Error in kill");
        exit(EXIT_FAILURE);
    }
}

/* Signal handler for SIGALRM. Catches SIGALRM signal and
 * kills the child process if it exists and is still executing.
 * It then prints out penn-shredder's catchphrase to standard output */
void alarmHandler(int sig) {
    printf("Alart handler\n");
}

/* Signal handler for SIGINT. Catches SIGINT signal (e.g. Ctrl + C) and
 * kills the child process if it exists and is executing. Does not
 * do anything to the parent process and its execution */
void sigintHandler(int sig) {
    printf("_sigintHandler childPid is %d...\n", childPid);
    /*if (childPid > 0 || childPid < 0) {
        printf("_sigintHandler if childPid not 0, before _killChildProcess childPid is %d...\n", childPid);
        printf("Killing child process with Pid %d \n",childPid); 
        killChildProcess();
    }*/
    if (childPid == 0) {
        printf("_sigintHandler if childPid equal 0, before _killChildProcess childPid is %d...\n", childPid);
        killChildProcess();
    }
}


/* Registers SIGALRM and SIGINT handlers with corresponding functions.
 * Error checks for signal system call failure and exits program if
 * there is an error */
void registerSignalHandlers() {
    printf("_registerSignalHandler childPid is %d...\n", childPid);
    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        perror("Error in signal");
        exit(EXIT_FAILURE);
    } else {
        printf("No error in signal check. Continue...\n");
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
    printf("Command: %s ...\n", command);
    if (command == NULL) {
        free(command);
        return;
    }
    char commandFirstChar = command [0];
    if (commandFirstChar == ' ' || commandFirstChar == '\0'){
        free(command);
        return;
    }
    if (strlen(command) < 5) {
        perror("invalid|No such file or directory");
        free(command);
        return;
    }
    char validCommandStr[] = {'/','b','i','n','/','\0'};
    char commandStr[] = {command[0],command[1],command[2],command[3],command[4],'\0'};
    if (strcmp(validCommandStr,commandStr) != 0){
        perror("invalid|No such file or directory");
        free(command);
        return;
    } 

    if (command != NULL) {
        printf("_executeShell childPid before fork is %d... \n",childPid);
        childPid = fork();

        printf("_executeShell childPid after fork is : %d\n", childPid);

        if (childPid < 0) {
            perror("Error in creating child process");
            exit(EXIT_FAILURE);
        }

        if (childPid == 0) {
            char *const envVariables[] = {NULL};
            char *const args[] = {command, NULL};
            printf("_executeShell childPid==0 part before execve childPid is : %d\n", childPid);
            if (execve(command, args, envVariables) == -1) {
                perror("invalid|No such file or directory");
                exit(EXIT_FAILURE);
            }
            printf("_executeShell childPid==0 part after execve childPid is %d \n",childPid);
        } else {
            printf("_executeShell childPid>0 part before dowhile childPid is %d \n",childPid);
            do {
                if (wait(&status) == -1) {
                    perror("Error in child process termination");
                    exit(EXIT_FAILURE);
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            printf("_executeShell childPid>0 part after dowhile childPid is %d \n",childPid);
            printf("parent process stop wait...\n");
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
    for (i=0;i<numBytes;i++) {
        if (inputBuffer[i]==' ') {
            spaceCount++;
        }
    }
    if (spaceCount == strlen(inputBuffer)-1) {
        printf("All space");
        free(inputBuffer);
        return NULL;
    }

    
    //trim leading empty space
    char *bufferStrAddr = inputBuffer;
    while (*bufferStrAddr == ' ') {bufferStrAddr++;}

    //trim ending empty space
    char *bufferEndAddr = inputBuffer+strlen(inputBuffer)-1;
    while (bufferEndAddr>inputBuffer && (*bufferEndAddr == ' ' || *bufferEndAddr == '\n')) {bufferEndAddr--;}

    /*Copy char to char from inputBuffer to command
    command does not keep the '\0' string terminator from buffer
    command does not include leading or ending space from buffer
    */
    char *command = malloc((bufferEndAddr-bufferStrAddr+2)*sizeof(char));
        if (command == NULL) {
            writeToStdout("Memory allocation failure for command \n");
            exit(EXIT_FAILURE);
    }
    i=0;
    while(i<=(bufferEndAddr-bufferStrAddr)){
        command[i] = bufferStrAddr[i];
        printf("%d : %c \n", i, command[i]);
        i++;
    }
    command[bufferEndAddr-bufferStrAddr+2] = '\0';
    
    free(inputBuffer);
    return command;
}
