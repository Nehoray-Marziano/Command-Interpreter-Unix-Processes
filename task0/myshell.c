#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "LineParser.h"

int execute(cmdLine *pCmdLine){
    execvp(pCmdLine->arguments[0], pCmdLine->arguments);
    perror("execvp");
    exit(EXIT_FAILURE);
}

int main(int argc, char ** argv) {
    int shouldQuit = -1;
    while(shouldQuit != 0) {
        char path[PATH_MAX];
        getcwd(path, sizeof(path));
        printf("%s>", path);
        char mystring [2048];
        fgets (mystring , 2048 , stdin);
        cmdLine* mycommand;
        mycommand = parseCmdLines(mystring);
        shouldQuit = strcmp(mycommand->arguments[0], "quit");
        if (shouldQuit != 0) {
            execute(mycommand);
        }
        freeCmdLines(mycommand);
    }
    return 0;
}