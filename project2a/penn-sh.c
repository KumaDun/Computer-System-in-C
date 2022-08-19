#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tokenizer.h"
#include <fcntl.h>

#define INPUT_SIZE 1024
#define LIST_SIZE 10

pid_t pid = 0;

void executeShell();

void writeToStdout(char *text);

void alarmHandler(int sig);

void sigintHandler(int sig);

char *getCommandFromInput();

void registerSignalHandlers();

void killChildProcess();

int main(int argc, char **argv) {
    printf("Registering signal handler...\n");
    registerSignalHandlers();

    while (1) {
        executeShell();
    }

    return 0;
}

/* Sends SIGKILL signal to a child process.
 * Error checks for kill system call failure and exits program if
 * there is an error */
void killChildProcess() {
    if (kill(pid, SIGKILL) == -1) {
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
    printf("_sigintHandler childPid is %d...\n", pid);
    if (pid != 0) {
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
void executeShell() {
    char *userinput;
    char * commandArgs[LIST_SIZE];
    int commandArgsLen = 0;
    int status;
    char minishell[] = "penn-sh> ";
    TOKENIZER * tokenizer;
    char* tok;
    char* nextTok;

    writeToStdout(minishell);
    char * stdoutList[LIST_SIZE];
    stdoutList[0] = NULL;
    char * stdinList[LIST_SIZE];
    stdinList[0] = NULL;
    int stdoutListLen = 0;
    int stdinListLen = 0;

    int i = 0;

    userinput = getCommandFromInput();
    //command valid check
    if (userinput == NULL) {
        writeToStdout("userinput NULL check \n");
        free(userinput);
        return;
    }
    char inputFirstChar = userinput [0];
    if (inputFirstChar == ' ' || inputFirstChar == '\0'){
        writeToStdout("userinput leading space or leading NULL check \n");
        free(userinput);
        return;
    }
    
    //Token search
    tokenizer = init_tokenizer(userinput);
    while( (tok = get_next_token(tokenizer) ) != NULL) {
        //printf("Tok is %s -> ", tok);
        if (*tok == '<' ) {
            if (( nextTok = get_next_token(tokenizer)) !=NULL) {
                //printf("NextTok is %s \n", nextTok);
                char* stdinArg = malloc((strlen(nextTok)+1)*sizeof(char));
                strcpy(stdinArg,nextTok);
                stdinList[stdinListLen] = stdinArg;
                stdinListLen++;
                free(nextTok);
            }
            else{
                continue;
            }
        }
        else if (*tok == '>' ) {
            if (( nextTok = get_next_token(tokenizer)) !=NULL) {
                //printf("NextTok is %s \n", nextTok);
                char* stdoutArg = malloc((strlen(nextTok)+1)*sizeof(char));
                strcpy(stdoutArg,nextTok);
                stdoutList[stdoutListLen] = stdoutArg;
                stdoutListLen++;
                free(nextTok);
            }
            else{
                continue;
            }
        }
        else{
            char* command = malloc((strlen(tok)+1)*sizeof(char));
            strcpy(command,tok);
            //printf("Command is %s \n", command);
            commandArgs[commandArgsLen] = command;
            commandArgsLen ++;
        }
        free(tok);
    }
    free_tokenizer(tokenizer);
    commandArgs[commandArgsLen] = NULL;
    commandArgsLen ++;

    //execute
    if (userinput != NULL) {
        pid = fork();

        if (pid < 0) {
            perror("Error in creating child process");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            //printf("In child process \n");
            //child process
            if(stdoutListLen>1) {
                perror("Invalid: Multiple standard output redirects");
                exit(EXIT_FAILURE);
            }
            if(stdinListLen>1) {
                perror("Invalid: Multiple standard input redirects");
                exit(EXIT_FAILURE);
            }

            /*
            printf("stdout List 0 is %s \n", stdoutList[0]);
            printf("stdout List 1 is %s \n", stdoutList[1]);
            printf("stdout List 2 is %s \n", stdoutList[2]);
            printf("stdin List 0 is %s \n", stdinList[0]);
            printf("stdin List 1 is %s \n", stdinList[1]);
            printf("stdin List 2 is %s \n", stdinList[2]);
            */

            if (stdoutList[0] != NULL) {
                //printf("New stdout is %s \n", stdoutList[0]);
                char* newStdoutName = stdoutList[0];
                int newStdout = open(newStdoutName, O_WRONLY | O_TRUNC | O_CREAT, 0644);
                dup2(newStdout,STDOUT_FILENO);
            }
            if (stdinList[0] != NULL) {
                //printf("New stdin is %s \n", stdinList[0]);
                char* newStdinName = stdinList[0];
                int newStdin = open(newStdinName, O_RDONLY);
                dup2(newStdin,STDIN_FILENO);
            }
            while(i<commandArgsLen) {
                //printf("command args list at %d is %s \n", i, commandArgs[i]);
                i++;
            }

            if (execvp(commandArgs[0], commandArgs) == -1) {
                perror("invalid|No such file or directory");
                exit(EXIT_FAILURE);
            }
        }
        else {
            //printf("In parent process \n");
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
                if (wait(&status) == -1) {
                    perror("Error in child process termination");
                    exit(EXIT_FAILURE);
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            
            /*
            Reset childPid to 0 for parent process so it will not send kill signal after wait() successed and quit
            */
            pid = 0;
        }
    }
    //printf("Quit parent do-while loop \n");
    //printf("command Args lens is %d \n", commandArgsLen);
    commandArgsLen --;
    while (commandArgsLen >= 0 ) {
        free(commandArgs[commandArgsLen]);
        commandArgsLen--;
    }
    free(userinput);

    //printf("stdout list lens is %d \n", stdoutListLen);
    stdoutListLen --;
    while (stdoutListLen >= 0 ) {
        free(stdoutList[stdoutListLen]);
        stdoutListLen--;
    }

    //printf("stdin list lens is %d \n", stdinListLen);
    stdinListLen --;
    while (stdinListLen >= 0 ) {
        free(stdinList[stdinListLen]);
        stdinListLen--;
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

    inputBuffer[numBytes-1] = '\0';
    
    /*If all input is space, return NULL command
    NULL command will be caught in execute*/
    int i = 0;
    int spaceCount = 0;
    for (i=0;i<strlen(inputBuffer);i++) {
        if (inputBuffer[i]==' ') {
            spaceCount++;
        }
    }
    if (spaceCount == strlen(inputBuffer)) {
        free(inputBuffer);
        return NULL;
    }

    /*copy the command to string command*/
    char *userinput = malloc((strlen(inputBuffer)+1)*sizeof(char));
    strcpy(userinput,inputBuffer);

    free(inputBuffer);
    printf("Userinput is %s \n", userinput);
    return userinput;
}