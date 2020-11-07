/***************************************************************************
 * Author: Carlos Gonzalez
 * Assignment 3 - Smallsh
 * 
 * Date: Oct. 20 - Nov. 3, 2020
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // pid_t
#include <unistd.h> //getpid, getppid
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "dynarray.h"


int fg_mode = 0; //Initialize foreground-only to off (0=off, 1=on)

struct cmdvector{

    char* cmd;
    int argcount;
    char **argvec; //temporarily store whole string, will parse later. change in parse_cmdl() when ready.
    char* inFile;
    char* outFile;
    char bckgrndproc;
};

/***********************
 * Function: ignore_sgnt()
 * 
 * This function sets the smallsh and background processes to ignore SIGINT.
 * */
void ignore_sgnt(){

    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = SIG_IGN;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);

}

/***********************
 * Function: handle_SIGTSTP()
 * 
 * This function handles the SIGTSTP signal. toggles forground-only mode and prints message.
 * */
void handle_SIGTSTP(int signo){
    if(fg_mode == 0){
        fg_mode = 1; 
        char *message = "Entering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, message, 50);
        fflush(stdout);
    }else if(fg_mode == 1){
        fg_mode = 0;
        char *message = "Exiting foreground-only mode\n";
        write(STDOUT_FILENO, message, 30);
        fflush(stdout);
    }
}

/***********************
 * Function: ignore_sgtp()
 * 
 * This function sets the foreground and background processes to ignore SIGTSTP
 * */
void ignore_sgtp(){

    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = SIG_IGN;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

}

/***********************
 * Function: set_sgtp()
 * 
 * This function sets the smallsh to handle SIGTSTP according to handle_SIGTSTP().
 * Toggles foreground-only mode.
 * */
void set_sgtp(){

    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

}

/***********************
 * Function: parse_cmdl
 * 
 * This function accepts a string. Breaks the string into tokens using a space char and "\n" as a delimiter.
 * Allocates memory for the command vector struct and its variables. Stores the token into appropriate locations.
 * Returns cmdvector struct. 
 * */
struct cmdvector* parse_cmdl(char* cmdl){
    
    //create variable struct for command 
    struct cmdvector* v = malloc(sizeof(struct cmdvector));
    v->cmd = NULL;
    v->inFile = NULL;
    v->outFile = NULL;
    v->bckgrndproc = '\0';
    int count = 0; //track arg count

    char* saveptr;

    char *token = strtok_r(cmdl, " \n", &saveptr); //first extract command
    
    v->cmd = malloc(1+strlen(token)*sizeof(char));
    strcpy(v->cmd, token);
   
    v->argvec = malloc(512*sizeof(char*)); //at most 512 args
    v->argvec[count] = malloc(1+strlen(token)*sizeof(char)); //also store command as first arg. this is for execvp()
    strcpy(v->argvec[count], token);
    count++;

    token = strtok_r(NULL, " \n", &saveptr); //extract next token
    while(token != NULL){
        if( strcmp(token, "<") == 0){ //if input file detected
            token = strtok_r(NULL, " \n", &saveptr);
            v->inFile = malloc(1+strlen(token)*sizeof(char)); //next token will be input file string
            strcpy(v->inFile, token);
        }
        else if( strcmp(token, ">") == 0){ //if outputfile detected
            token = strtok_r(NULL, " \n", &saveptr);
            v->outFile = malloc(1+strlen(token)*sizeof(char)); //next string will be output file string
            strcpy(v->outFile, token);
        }
        else if( strcmp(token, "&") == 0){ 
            v->bckgrndproc = '&';
        }else{
            v->argvec[count] = malloc(1+strlen(token)*sizeof(char)); //everything else will be an arg
            strcpy(v->argvec[count], token);
            count++;
        }
        token = strtok_r(NULL, " \n", &saveptr);
    }
    //v->argvec[count] = malloc(5*sizeof(char));
    v->argvec[count] = NULL;
    v->argcount = count;

    return v;
}

/***********************
 * Function: free_cmdvec()
 * 
 * This function accepts a cmdvector struct and frees all memory 
 * */
void free_cmdvec(struct cmdvector *v){

    if(v->cmd){
        free(v->cmd);
    }
    for(int i=0; i < v->argcount; i++){
        free(v->argvec[i]);
    }
    if(v->argvec)
        free(v->argvec);
    if(v->inFile)
        free(v->inFile);
    if(v->outFile)
        free(v->outFile);
    free(v);
}

/***********************
 * Function: replace_inst()
 * 
 * This function takes in a string, and parent process id. 
 * Replaces each instance of '$$' with the pid.
 * return new string.
 * */
char* replace_inst(char* s, char* pid){

    char temp[256];
    int temp_idx = 0;
    int c=0;
    for(int i=0; i < strlen(s)-c+1; i++){
        if((s[i+c] == '$') && (s[i+1+c] == '$')){
            c++;
            for(int j=0; j < strlen(pid); j++){
                temp[temp_idx] = pid[j];
                temp_idx++;
            }
        }else{
            temp[temp_idx] = s[i+c];
            temp_idx++;
        }
    }
    
    char* new_s = malloc(1+strlen(temp)*sizeof(char));
    strcpy(new_s, temp);
    return new_s;
    
}

/***********************
 * Function: expand_inst()
 * 
 * This function gets the pid. and for each variable in the cmdvector struct, calls replace_inst() to get a new string.
 * */
void expand_inst(struct cmdvector* v){

    char shellpid[10];
    sprintf(shellpid, "%d", getpid());
    
    if(v->cmd){
        char* n1 = replace_inst(v->cmd, shellpid);
        v->cmd = realloc(v->cmd, 1+strlen(n1)*sizeof(char));
        strcpy(v->cmd, n1);
        free(n1);
    }
    if(v->inFile){
        char *n2 = replace_inst(v->inFile, shellpid);
        v->inFile = realloc(v->inFile, 1+strlen(n2)*sizeof(char));
        strcpy(v->inFile, n2);
        free(n2);
    }
    if(v->outFile){
        char *n3 = replace_inst(v->outFile, shellpid);
        v->outFile = realloc(v->outFile, 1+strlen(n3)*sizeof(char));
        strcpy(v->outFile, n3);
        free(n3);
    }
    for(int i=0; i < v->argcount; i++){
        char* n4 = replace_inst(v->argvec[i], shellpid);
        v->argvec[i] = realloc(v->argvec[i], 1+strlen(n4)*sizeof(char));
        strcpy(v->argvec[i], n4);
        free(n4);
    }
    
}

/***********************
 * Function: exit_shell()
 * 
 * This function accepts a dynamic array of background pids. Then kills each pid. 
 * */
void exit_shell(struct dynarray* pids){


    for(int i = dynarray_length(pids)-1; i >= 0; i--){
        pid_t curr_p = dynarray_get(pids, i);
        int killed = kill(curr_p, SIGKILL);
    }

}

/***********************
 * Function: change_dir()
 * 
 * This function gets the home directory. then calls chdir() using the dir indicated by the user.
 * */
void change_dir(int argcount, char* argvec[]){

    char *homedir = getenv("HOME");
 
    if(argcount == 1){
        if(chdir(homedir)==-1){
            perror("Change directory failed.\n");
        }
    }else{
        char new_dir[256];
        if(getcwd(new_dir, sizeof(new_dir))==NULL){
            perror("getcwd() error.\n");
        }else{
            strcat(new_dir, "/");
            strcat(new_dir, argvec[1]);
            if(chdir(new_dir)==-1){
                perror("Change directory failed.\n");
            }
        }
    }
}

/***********************
 * Function: check_status()
 * 
 * This function accepts the foreground child status and an error status variable. 
 * Prints the status of last foreground child unless previous command produced an error. 
 * */
void check_status(int fgchildstat, int stat){

    if(stat == 0){ //no foreground command run yet.
        printf("exit value 0\n");
        fflush(stdout);
    }else if(stat == -1){ //error in command execution
        printf("exit value 1\n");
        fflush(stdout);
    }else if(WIFEXITED(fgchildstat)){ //if terminated normally
        printf("exit value %d\n", WEXITSTATUS(fgchildstat));
        fflush(stdout);
    }else if(WIFSIGNALED(fgchildstat)){// if terminated abnormally
        printf("terminated by signal %d\n", WTERMSIG(fgchildstat));
        fflush(stdout);
    }

}

/***********************
 * Function: new_fgproc()
 * 
 * This function accepts an int pointer that stores address of foreground child status, cmdvector struct, and status pointer.
 * forks a new child. if file redirection, handle it. set signal handling. wait for child. 
 * */
void new_fgproc(int *cs, struct cmdvector* v, int *stat){

    *stat= -5; //if process works, status variable should not activate if statement in check_status()
    int childst;
    pid_t child = fork();
    if(child == -1){
        perror("fork() failed.\n");
        return;
    }else if(child == 0){
        int sourceFD;
        int targetFD;
        if(v->inFile){
            sourceFD = open(v->inFile, O_RDONLY);
            if(sourceFD == -1){
                perror("source open()");
                *stat = -1;
                return;
            }
        }
        if(v->outFile){
            targetFD = open(v->outFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if(targetFD==-1){
                perror("target open()");
                *stat=-1;
                return;
            }
        }
        int result;
        if(v->inFile){
            result = dup2(sourceFD,0);
            if(result==-1){
                perror("source dup2()");
                *stat = -1;
                return;
            }
        }
        if(v->outFile){
            result = dup2(targetFD, 1);
            if(result==-1){
                perror("target dup2()");
                *stat = -1;
                return;
            }
        }
        signal(SIGINT, SIG_DFL);
        ignore_sgtp();
        *stat = execvp(v->cmd, v->argvec);
        printf("%s: no such file or directory.\n", v->cmd);
        fflush(stdout);
    }else{
        child = waitpid(child, &childst, 0);
        if(WIFSIGNALED(childst)){
            printf("terminated by signal %d\n", WTERMSIG(childst));
            fflush(stdout);
        }
        *cs = childst;
        return;
    }

}

/***********************
 * Function: new_backproc()
 * 
 * Accepts array of pids, cmdvector struct, and status pointer.
 * forks new process, handles redirection, stores pid in array instead of waiting on child. 
 * */
void new_backproc(struct dynarray* pids, struct cmdvector* v, int *stat){

    *stat = -5; //set as -5 to not trigger if statement in check_status
    pid_t spawnpid = fork();
    if(spawnpid == -1){
        perror("fork() failed.\n");
        return;
    }else if(spawnpid == 0){
        int sourceFD;
        int targetFD;
        if(v->inFile){
            sourceFD = open(v->inFile, O_RDONLY);
            if(sourceFD == -1){
                perror("source open()");
                *stat = -1; //-1 will display error message in check_status
                return;
            }
        }else{
            sourceFD = open("/dev/null", O_RDONLY);
        }
        if(v->outFile){
            targetFD = open(v->outFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if(targetFD==-1){
                perror("target open()");
                *stat=-1;
                return;
            }
        }else{
            targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC);
        }
        int result;
        if(v->inFile){
            result = dup2(sourceFD,0);
            if(result==-1){
                perror("source dup2()");
                *stat = -1;
                return;
            }
        }
        if(v->outFile){
            result = dup2(targetFD, 1);
            if(result==-1){
                perror("target dup2()");
                *stat = -1;
                return;
            }
        }
        ignore_sgnt();
        ignore_sgtp();
        //signal(SIGTSTP, SIG_IGN);
        *stat = execvp(v->cmd, v->argvec);
        perror("execvp error.\n");
    }else{
        printf("background pid is %d\n", spawnpid);
        fflush(stdout);
        dynarray_insert(pids,-1, spawnpid); //store pid
        return;
    }

}

/***********************
 * Function: check_pids()
 * 
 * Accepts array of pids. Checks to see if any child had terminated yet. If so, remove from array.
 * */
void check_pids(struct dynarray* pids){
    
    for(int i=0; i<dynarray_length(pids); i++){
        int childstatus;
        pid_t child_pid = dynarray_get(pids, i);
        pid_t stat = waitpid(child_pid, &childstatus, WNOHANG);
        if(stat != 0){
            printf("background pid %d is done: ", child_pid);
            check_status(childstatus, -5);
            dynarray_remove(pids, i);
        }
    }
}

/***********************
 * Function: smallsh()
 * 
 * The smallsh itself. prints prompt, and takes user input. parses input. and calls appropriate function.
 * free cmdline and struct each time before returning input access to user. exit if user enters "exit".
 * kill all background pids before exiting.
 * */
int smallsh(){

    int exit_stat = 1;
    struct dynarray* pids = dynarray_create();
    int FGchild_stat = 0;
    int stat = 0;
    do{
        char *cmdline; size_t cmdsize = 2048;
        cmdline = malloc(cmdsize*sizeof(char));
        do{
            printf(": "); //symbol for command line prompt
            fflush(stdout);
            size_t nbytes = getline(&cmdline, &cmdsize, stdin); //user command
            if(nbytes==-1){
                clearerr(stdin);
            }
        }while((cmdline[0] == '#') || strcmp(cmdline, "") == 0);
        
        struct cmdvector *cv = parse_cmdl(cmdline);
        expand_inst(cv);
        if(strcmp(cv->cmd, "exit")==0){
            exit_shell(pids);
            exit_stat = 0;
        }
        else if(strcmp(cv->cmd, "cd")==0){
            change_dir(cv->argcount, cv->argvec);
        }
        else if(strcmp(cv->cmd, "status")==0){
            check_status(FGchild_stat, stat);
        }
        else if( (fg_mode == 0) && (cv->bckgrndproc == '&')){
            new_backproc(pids, cv, &stat);
        }else{
            new_fgproc(&FGchild_stat, cv, &stat);
        }
        
        free(cmdline);
        free_cmdvec(cv);
        check_pids(pids);
    }while(exit_stat == 1);
    
    dynarray_free(pids);
    return 0;
}


int main(void){

    ignore_sgnt();
    set_sgtp();
    printf("$ smallsh\n");
    fflush(stdout);
    int start = smallsh();
    printf("$\n");

    return 0;
}