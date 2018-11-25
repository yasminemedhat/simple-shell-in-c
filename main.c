#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>
#define MAXLINE 500  //max number of characters for user input
#define MAXTOKENS 20 // max number of tokens for a command

//handle the signal SIGCHLD when a child process is terminated
void signalHandler(int sig_num)
{
    FILE *fp;
    // open my .log file
    fp=fopen("shell_logfile.log","a+");
    if(fp == NULL)
    {
        printf("\nerror:couldn't open file");
    }
    else fprintf(fp,"Child process was terminated\n");
    fclose(fp);

}
// print the current directory with each input
void printCurrentDirectory()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\nDir: %s :~$ ", cwd);
}

//change directory either to home or to the specified path
void changeDirectory(char * args[])
{
    // only 'cd'
    if (args[1] == NULL)
        chdir(getenv("HOME"));
    // or change it to the specified or error
    else
    {

        if (chdir(args[1]) == -1)
            printf(" %s: no such directory\n", args[1]);
    }
}
//handle the input of the user
void commandHandler(char * args[], int args_count)
{
    int count=args_count;
    char cwd[1024];
    // status of the child
    int status;
    int background=0;

    //check if it's a background process or not
    if(!(strcmp(args[count-1],"&")))
    {
        //SET background flag to zero
        background=1;
        //remove the "&" so it could be executed
        args[count-1]='\0';
        count--;
    }

    // print the current directory
    if (strcmp(args[0],"pwd") == 0)
    {
        printf("%s\n", getcwd(cwd, sizeof(cwd)));
    }
    // change directory
    else if (strcmp(args[0],"cd") == 0)
    {
        changeDirectory(args);
    }
    //execute the command
    else
    {
        //process id
        pid_t pid=fork();

        // no error in forking
        if (pid >= 0)
        {
            // if it's a child process
            if(pid== 0)
            {
                execvp(args[0],args);

            }
            //if it's the parent process
            else
            {
                if(background==1) return;
                else
                {
                    //suspends execution until the child specified by pid change state
                    do
                    {
                    waitpid(pid,&status,WUNTRACED);
                   }
                   while(!WIFEXITED(status) && !WIFSIGNALED(status));
                    //wait(&status);
                }
            }
        }
        else
            printf("\nfailed to fork!");
    }
}
//parse the input ine and tokenize it by space
int tokenize(char line[], char * tokens[])
{
    int i=0;
    //if the user input nothing return 1 to continue
    if((tokens[i] = strtok(line," \n\t"))==NULL) return 0;
    i=1;
    //tokenize the input line by space
    while((tokens[i] = strtok(NULL, " \n\t")) != NULL) i++;
    tokens[i]= NULL;
    return i;
}
int main()
{
    char line[MAXLINE]; // buffer for the user input
    char * tokens[MAXTOKENS]; // array for the different tokens in the command
    int token_count=0;


    //handling child signals
    struct sigaction sa;

    sa.sa_handler = signalHandler; // to call my signal handling functions
    sigemptyset(&sa.sa_mask);  //set sa with zeros
    sigaction(SIGCHLD,&sa,NULL); //call my signal handler when a child terinates
    while(1)
    {
        //prints the current directory after each loop
        printCurrentDirectory();

        //reads user input
        fgets(line, MAXLINE, stdin);

        //tokenize the input of the user
        token_count=tokenize(line,tokens);
        //t=1 if no input so we loop again
        if(token_count==0)
        {
            continue;
        }
        //exit the shell
        if(strcmp(tokens[0],"exit") == 0) exit(0);

        //handle the input
        commandHandler(tokens,token_count);

    }

    return 0;
}
