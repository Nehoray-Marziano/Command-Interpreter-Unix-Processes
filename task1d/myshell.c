#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "LineParser.h"


/*DECLARATIONS*/
void tildaCase(cmdLine* line);
char replacing(char *str,char* replaceWhat,char *replaceWith);


int execute(cmdLine *pCmdline){
    pid_t pid_child;
    int status;
    pid_child=fork(); /*this is information to the parent- it's child id*/

    if(pid_child==0){ /*meaning it is a child*/
        execvp(pCmdline->arguments[0],pCmdline->arguments);
        perror("execvp");/*if the code reached here, it means the execution did not work*/
        _exit(EXIT_FAILURE);
    }
    else{ /*parent proccess*/
        if(pCmdline->blocking==1)
            waitpid(pid_child,&status,0);
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
if(command!=NULL){
        tildaCase(command);
        shouldquit=strcmp(command->arguments[0],"quit");/*check if we should end the loop and end the program*/
        if(strcmp(command->arguments[0],"cd")==0){/*meaning the command is cd*/
            if(chdir(command->arguments[1])==-1)/*if cd failed*/
                fprintf( stderr, "Error using cd! Check the directory you entered! \n");
        }
        else if(shouldquit!=0){/*meaning the user sent us a command he wants to execute*/
            execute(command);
        }
        if(command!=NULL){
            freeCmdLines(command);
        }
}
    }
    return 0;   
}

void tildaCase(cmdLine *line){
    char temp[1024];
    char *myhome = getenv("HOME"); /*home directory*/
    int i;

    for (i = 0; i < line->argCount; i++)
    {
        memset(temp, '\0', strlen(temp)); /*fills up temp with the '\0'. You'll see why in 'replacing' */
        /*char *current_argument = line->arguments[i];*/
        strcpy(temp, line->arguments[i]);/*copies the argument to temp*/

        if (replacing(temp, "~", myhome))
            replaceCmdArg(line, i, temp);
    }
}

char replacing(char *replaceIn, char *replaceWhat, char *replaceWith)
{
    char *accurrence = strstr(replaceIn, replaceWhat); /*to find the first accurrence on '~' in temp (stops when it reaches '\0'!)*/
    if (!accurrence)/*no accurrence*/
        return 0;

    char buffer[1024];
    memset(buffer, '\0', strlen(buffer));/*same reason as in 'tildaCase'*/

    if (replaceIn == accurrence) /* if 'replaceWhat' was found at the beggining of 'replaceIn' */
    {
        strcpy(buffer, replaceWith);
        strcat(buffer, accurrence + strlen(replaceWhat));/*adds 'accurrence' except for the 'replaceWhat' to the end of buffer*/
    }
    else /*there is an accurrence further in 'replaceIn'*/
    {
        strncpy(buffer, replaceIn, strlen(replaceIn) - strlen(accurrence));/*copies the string until the first accurrence*/
        strcat(buffer, replaceWith); /*adds 'replaceWith*/
        strcat(buffer, accurrence + strlen(replaceWhat));/*adds the rest of 'replaceIn' after 'replaceWhat'*/
    }

    memset(replaceIn, '\0', strlen(replaceIn));/*'empties' 'replaceIn'*/
    strcpy(replaceIn, buffer); /*copies the updated string to 'replaceIn'*/
    return 1;
}
