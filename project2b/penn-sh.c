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
#define REDIRECTION_SIZE 10
#define PIPE_STAGE_NUM 2

pid_t pid = 0;

void executeShell();

void writeToStdout(char *text);

void alarmHandler(int sig);

void sigintHandler(int sig);

char *getCommandFromInput();

void registerSignalHandlers();

void killChildProcess();

void freeArgsList(char* list[], int len);

void treatTok(TOKENIZER * tokenizer, char* tok, char* stdoutList[], char*stdinList[], char* commandArgs[],
    int* stdoutListLen, int* stdinListLen, int* commandArgsLen);

int main(int argc, char **argv) {
    //printf("Registering signal handler...\n");
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
    int status;
    char minishell[] = "penn-sh> ";

    TOKENIZER * tokenizer;
    char* tok;    
    
    //Initialize stdout, stdin, commandargs arrays
    char * stdoutList[PIPE_STAGE_NUM][REDIRECTION_SIZE];
    int stdoutListLen [PIPE_STAGE_NUM];
    for (int i = 0; i<PIPE_STAGE_NUM;i++){
        stdoutList[i][0]=NULL;
        stdoutListLen[i]=0;
    }

    char * stdinList[PIPE_STAGE_NUM][REDIRECTION_SIZE];
    int stdinListLen [PIPE_STAGE_NUM];
    for (int i = 0; i<PIPE_STAGE_NUM;i++){
        stdinList[i][0]=NULL;
        stdinListLen[i]=0;
    }
    
    char * commandArgs[PIPE_STAGE_NUM][INPUT_SIZE];
    int commandArgsLen [PIPE_STAGE_NUM];
    for (int i = 0; i<PIPE_STAGE_NUM;i++){
        commandArgs[i][0]=NULL;
        commandArgsLen[i]=0;
    }

    int flag=0;

    writeToStdout(minishell);
    
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
        //printf("Tok is -> %s \n ", tok);
        if (*tok == '|') {
            flag = 1;
            free(tok);
            continue;
        }
        else {
            treatTok(tokenizer,tok, stdoutList[flag],stdinList[flag],commandArgs[flag],
            &stdoutListLen[flag],&stdinListLen[flag],&commandArgsLen[flag]);
            free (tok);
        }
    }
    free_tokenizer(tokenizer);

    //Print the arrays
    /*for (int i = 0; i<PIPE_STAGE_NUM;i++){
        for (int j=0;j<stdoutListLen[i];j++){
            printf("STDOUTLIST %d at %d is %s \n", i, j, stdoutList[i][j]);
        }
        for (int j=0;j<stdinListLen[i];j++){
            printf("STDINLIST %d at %d is %s \n", i, j, stdinList[i][j]);
        }
        for (int j=0;j<commandArgsLen[i];j++){
            printf("COMMANDLIST %d at %d is %s \n", i, j, commandArgs[i][j]);
        }
    }*/

    //Add NULL to the end of command args array for execvp usage
    for (int i = 0; i<PIPE_STAGE_NUM;i++){
        commandArgs[i][commandArgsLen[i]] = NULL;
        commandArgsLen[i] ++;
    }

    //execute

    //If no pipeline
    if (flag == 0){
        pid = fork();

        if (pid < 0) {
            perror("Error in creating child process");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            //printf("In child process \n");
            //child process

            //Check if there are one more redirection, if yes then error
            if(stdoutListLen[0]>1) {
                perror("Invalid: Multiple standard output redirects");
                exit(EXIT_FAILURE);
            }
            if(stdinListLen[0]>1) {
                perror("Invalid: Multiple standard input redirects");
                exit(EXIT_FAILURE);
            }

            //If there is redirection out, dup2
            if (stdoutList[0][0] != NULL) {
                printf("New stdout is %s \n", stdoutList[0][0]);
                char* newStdoutName = stdoutList[0][0];
                int newStdout = open(newStdoutName, O_WRONLY | O_TRUNC | O_CREAT, 0644);
                if (newStdout == -1) {
                    perror("Invalid, Open out redirection file error\n");
                    exit(EXIT_FAILURE);
                }
                dup2(newStdout,STDOUT_FILENO);
            }
            //If there is redirection in, dup2
            if (stdinList[0][0] != NULL) {
                printf("New stdin is %s \n", stdinList[0][0]);
                char* newStdinName = stdinList[0][0];
                int newStdin = open(newStdinName, O_RDONLY);
                 if (newStdin == -1) {
                    perror("Invalid, Open in redirection file error\n");
                    exit(EXIT_FAILURE);
                }
                dup2(newStdin,STDIN_FILENO);
            }

            //Execute
            if (execvp(commandArgs[0][0], commandArgs[0]) == -1) {
                perror("invalid|No such file or directory");
                exit(EXIT_FAILURE);
            }
        }
        else {
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

    //If there is pipeline    
    if (flag == 1){
        int fd[2];
        pipe(fd);

        //child process #1
        pid_t pid1 = fork();
        if (pid1 == 0) {

            //close read
            close(fd[0]);
            
            //Check if there are one more redirection, if yes then error
            if(stdoutListLen[0]>1) {
                perror("Invalid: Multiple standard output redirects");
                exit(EXIT_FAILURE);
            }
            if(stdinListLen[0]>1) {
                perror("Invalid: Multiple standard input redirects");
                exit(EXIT_FAILURE);
            }

            //If there is stdin redirection in the process, allowed
            if (stdinList[0][0]!= NULL) {
                char* newStdinName = stdinList[0][0];
                int newStdin = open(newStdinName, O_RDONLY);
                if (newStdin == -1) {
                    perror("Invalid, Open in redirection file error\n");
                    exit(EXIT_FAILURE);
                }
                dup2(newStdin,STDIN_FILENO);
            }
            
            //Build stdout pipeline
            if (stdoutList[0][0] == NULL) {
                dup2(fd[1],STDOUT_FILENO);
                close(fd[1]);
            }
            //If there is redirection out, overlapping with pipeline not allowed so error
            else{
                perror("Invalid output redirection \n");
                exit(EXIT_FAILURE);
            }
            
            //Execute
            if (execvp(commandArgs[0][0], commandArgs[0]) == -1) {
                perror("invalid|No such file or directory");
                exit(EXIT_FAILURE);
            }
        }
        
        //child process #2
        pid_t pid2 = fork();
        if (pid2 == 0) {
            close(fd[1]);

            //Check if there are one more redirection, if yes then error
            if(stdoutListLen[1]>1) {
                perror("Invalid: Multiple standard output redirects");
                exit(EXIT_FAILURE);
            }
            if(stdinListLen[1]>1) {
                perror("Invalid: Multiple standard input redirects");
                exit(EXIT_FAILURE);
            }

            //If there is stdout redirection in the process, allowed
            if (stdoutList[1][0] != NULL){
                //printf("New stdout is %s \n", stdoutList[0]);
                char* newStdoutName = stdoutList[1][0];
                int newStdout = open(newStdoutName, O_WRONLY | O_TRUNC | O_CREAT, 0644);
                if (newStdout == -1) {
                    perror("Invalid, Open out redirection file error\n");
                    exit(EXIT_FAILURE);
                }
                dup2(newStdout,STDOUT_FILENO);
            }

            //Build stdin pipeline
            if (stdinList[1][0] == NULL) {
                dup2(fd[0],STDIN_FILENO);
                close(fd[0]);
            }
            //If there is redirection in, overlapping with pipeline not allowed so error
            else {
                //printf("New stdin is %s \n", stdinList[0]);
                perror("Invalid output redirection \n");
                exit(EXIT_FAILURE);
            }
            
            if (execvp(commandArgs[1][0], commandArgs[1]) == -1) {
                perror("invalid|No such file or directory");
                exit(EXIT_FAILURE);
            }
        }
        //printf("In parent process \n");

        //close pipeline for parent
        close(fd[0]);
        close(fd[1]);
        /*do{
            if (waitpid (-1,&status,0) == -1){
                perror ("Error in child process termination\n");
                exit(EXIT_FAILURE);
            }
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));*/
        while (1) {
            //wait all child process
            //-1 in parameter waits all children
            if (waitpid (-1,&status , 0) == -1){
                perror ("Error in child process termination\n");
                exit(EXIT_FAILURE);
            }
            if (waitpid (-1,&status , 0) > 0){
                if (!WIFEXITED(status) && !WIFSIGNALED(status)){
                    continue;
                }
                else {
                    break;
                }
            }
        }
        
        /*
        Reset childPid to 0 for parent process so it will not send kill signal after wait() successed and quit
        */
        pid = 0;
    }     
    
    free(userinput);
    for (int i=0;i<PIPE_STAGE_NUM;i++) {
        freeArgsList(commandArgs[i],commandArgsLen[i]);
        freeArgsList(stdinList[i],stdinListLen[i]);
        freeArgsList(stdoutList[i],stdoutListLen[i]);
    }
    
}
    //printf("Quit parent do-while loop \n");

/* Treat the tok */
void treatTok(TOKENIZER * tokenizer, char* tok, char* stdoutList[], char*stdinList[], char* commandArgs[],
    int* stdoutListLen, int *stdinListLen, int* commandArgsLen) {
    char* nextTok;
    if (*tok == '<' ) {
        if (( nextTok = get_next_token(tokenizer)) !=NULL) {
            //printf("NextTok is %s \n", nextTok);
            char* stdinArg = malloc((strlen(nextTok)+1)*sizeof(char));
            strcpy(stdinArg,nextTok);
            stdinList[*stdinListLen] = stdinArg;
            *stdinListLen = *stdinListLen+1;
            free(nextTok);
        }
    }
    else if (*tok == '>' ) {
        if (( nextTok = get_next_token(tokenizer)) !=NULL) {
            //printf("NextTok is %s \n", nextTok);
            char* stdoutArg = malloc((strlen(nextTok)+1)*sizeof(char));
            strcpy(stdoutArg,nextTok);
            stdoutList[*stdoutListLen] = stdoutArg;
            *stdoutListLen=*stdoutListLen+1;
            free(nextTok);
        }
    }
    else{
        char* command = malloc((strlen(tok)+1)*sizeof(char));
        strcpy(command,tok);
        //printf("Command is %s \n", command);
        commandArgs[*commandArgsLen] = command;
        *commandArgsLen = *commandArgsLen +1;
    }
}

/* Free argslist */
void freeArgsList(char* list[], int len) {
    len--;
    while (len>=0) {
        free(list[len]);
        len--;
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
    //printf("Userinput is %s \n", userinput);
    return userinput;
}