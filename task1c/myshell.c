#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "LineParser.h"

int execute(cmdLine *pCmdline){
    pid_t pid_child;
    int status;
    pid_child=fork(); /*this is information to the parent- it's child id*/

    if(pid_child==0){ /*meaning it is a child*/
        execvp(pCmdline->arguments[0],pCmdline->arguments);
        perror("EXECVP FAILED!");/*if the code reached here, it means the execution did not work*/
        _exit(EXIT_FAILURE);
    }
    else{ /*parent proccess*/
        wait(&status);
        return status;
    }
}

int main(int argc,char **argv){
    int shouldquit=-1;
    while(shouldquit!=0){/*the main endless loop*/
        char mypath[PATH_MAX];
        getcwd(mypath,sizeof(mypath));
        printf("%s>",mypath); /*prints the pathin each loop , just like in a real shell*/
        char str[2048];
        fgets(str,2048,stdin);/*fill str from whats in stdin*/
        cmdLine* command;
        command=parseCmdLines(str); /*creating a new command using what was written on stdin*/
        shouldquit=strcmp(command->arguments[0],"quit");/*check if we should end the loop and end the program*/
        if(strcmp(command->arguments[0],"cd")==0){/*meaning the command is cd*/
            if(chdir(command->arguments[1])==-1)/*if cd failed*/
                fprintf( stderr, "ERROR USING CD! CHECK THE DIRECTORY YOU ENTERED! \n");
        }
        else if(shouldquit!=0){/*meaning the user sent us a command he wants to execute*/
            execute(command);
        }
        if(command!=NULL){
            freeCmdLines(command);
        }
    }
    return 0;
}
