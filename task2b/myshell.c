#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "LineParser.h"



typedef struct cmdNode{
    struct cmdLine *cmdata;
    struct cmdNode *prev;
    struct cmdNote *next;
}cmdNode;

int listSize;

/*DECLARATIONS*/
void tildaCase(cmdLine* line);
void historyCase(cmdNode* head,cmdLine* cmd);
char replacing(char *replaceIn, char *replaceWhat, char *replaceWith);
cmdNode* newNode(cmdLine* myCmd); /*creates a new node*/
cmdNode* add_cmd(cmdNode *originalHead, cmdLine* newCmd);
cmdLine *copy_cmd_line(cmdLine *command);
void printHistory(cmdNode* head);
void makeHistory(char buffer[],cmdNode *node);
void free_linked_list(cmdNode* start);
cmdNode* HistoryDeleteCase(cmdNode *head, cmdLine *cmd);
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
            waitpid(-1,&status,0);
        return status;
    }
}

int main(int argc,char **argv){
    int shouldquit=-1;
    int firstIteration=1;
    listSize=1;
    cmdNode* head=NULL;
    cmdLine* command;

    while(shouldquit!=0){/*the main endless loop*/
        char mypath[PATH_MAX];
        getcwd(mypath,sizeof(mypath));
        printf("%s>",mypath); /*prints the pathin each loop , just like in a real shell*/
        char str[2048];
        fgets(str,2048,stdin);/*fill str from whats in stdin*/
        command=parseCmdLines(str); /*creating a new command using what was written on stdin*/

        if(firstIteration==1){
            head=newNode(command);
            tildaCase(command);
            firstIteration++;
            if(strcmp(command->arguments[0],"cd")==0){
                if(chdir(command->arguments[1])==-1){/* if directory transfer failed*/
                    _exit(EXIT_FAILURE);
                }
            }
            else if(strcmp(command->arguments[0],"history")==0){
                historyCase(head,command);
            }
            else if(shouldquit!=0){
                execute(command);
            }

        }
        else{
            head=add_cmd(head,command);
            shouldquit=(strcmp(command->arguments[0],"quit"));
            if(strcmp(command->arguments[0],"cd")==0){
                head=add_cmd(head,command);
                if(chdir(command->arguments[1])==-1){
                    _exit(EXIT_FAILURE);
                }
            }
            else if(strcmp(command->arguments[0],"history")==0){
                historyCase(head,command);
            }
            else if(shouldquit!=0){
                execute(command);
            }
        }
    }
    free_linked_list(head);
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

cmdNode* newNode(cmdLine* myCmd){ /*creates a new node*/
    cmdNode *node= (cmdNode*) malloc(sizeof(cmdNode));
    node->cmdata=copy_cmd_line(myCmd);
    node->prev=NULL;
    node->next=NULL;
    return node;
}


cmdNode* add_cmd(cmdNode *originalHead, cmdLine* newCmd){
    cmdNode *newHead=newNode(newCmd);
    /*connecting the newHead:*/
    originalHead->prev=newHead;
    newHead->next=originalHead;
    listSize++;/*updating the global variable representing the length of the linkedlist*/
    return newHead;
}

void freeNode(cmdNode* toFree){ /*frees a node*/
    cmdNode* next = toFree->next;
    freeCmdLines(toFree->cmdata);
    free(toFree);
}

void free_linked_list(cmdNode* start){
    cmdNode *curr=start;
    cmdNode* temp; /*the one which is going to be freed*/
    while(curr!=NULL){
        temp=curr;
        curr=curr->next;
        freeNode(temp);
    }
}


void historyCase(cmdNode* head,cmdLine* cmd){
    if ((cmd->argCount > 2) && (strcmp(cmd->arguments[0],"history") == 0) &&  (strcmp(cmd->arguments[1],"-d") == 0)){ /*meaning we are in the deleting case*/
        head= HistoryDeleteCase(head,cmd);
    }
    else if(strcmp(cmd->arguments[0],"history")==0)/*meaning it is indeed the history case*/
        printHistory(head);
}

cmdNode* HistoryDeleteCase(cmdNode *head, cmdLine *cmd) {
    long int whereToDelete=atoi(cmd->arguments[2]);
    cmdNode *tempPrev;
    cmdNode *tempNext;
    cmdNode *curr=head;
    int i=0;
    while(curr->next!=NULL){/*going all the way to the end of the list (which is actually where the first command is)*/
        curr=curr->next;
    }
    while(i<whereToDelete){/*going to the node we want to delete*/
        curr=curr->prev;
        i++;
    }
    tempNext=curr->next;/*connecting his right and left 'hands'*/
    tempPrev=curr->prev;
    tempPrev->next=tempNext;
    tempNext->prev=tempPrev;
    freeCmdLines(curr->cmdata);
    free(curr);
    listSize--;
    return tempPrev;
}

void printHistory(cmdNode* head){/*where we actually print in one by one*/
    char buffer[1024];
    int i;
    cmdNode*  curr=head;
    i=listSize-1;
    while(curr!=NULL&&i<10){ /*while there are still nodes to print+ we know there are at most 10*/
        makeHistory(buffer,curr);
        printf("%d. %s\n",i--,buffer);
        curr=curr->next;
    }
}
void makeHistory(char buffer[],cmdNode *node){ /*fill the buffer*/
    memset(buffer, '\0',1024);/*filling it with \0*/

    int i;
    for(i=0;i<node->cmdata->argCount;i++){/*copies all of the command to the buffer*/
        strcat(buffer,node->cmdata->arguments[i]);
        strcat(buffer, " ");
    }
}


