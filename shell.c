#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BUFFER 1024                        // max line buffer
#define MAX_ARGS 64                            // max # args
#define MAX_HISTORY 15
#define SEPARATORS " \t\n"                     // token sparators
#define DEFAULT_PATH "/home/za643519"

// Global Variables
char buf[MAX_BUFFER];                      // line buffer
char * args[MAX_ARGS];                     // pointers to arg strings
char ** arg;                               // working pointer thru args
char * prompt = "#";                       // shell prompt
char* cwd;
char history[MAX_HISTORY][MAX_ARGS];
int historyIndex = 0;

//Method Prototypes
void AppendDirectory(char * runpath);
void ChangeDirectoryCommand();
void HistoryCommand();
void RunCommand();
void BackgroundRunCommand();

int main (int argc, char ** argv)
{
    int i;
    // Initialize Current Working Directory
    cwd = malloc(strlen("/home/za643519"));
    // Initialize default path
    strcpy(cwd, DEFAULT_PATH);
    // initialize history array
    for (i=0; i<MAX_HISTORY; i++)
        strcpy(history[i], "");

    // keep reading input until "quit" command or eof of redirected input
    while (!feof(stdin)) {

        // Get command line from input
        fputs (prompt, stdout);                // write prompt
        if (fgets (buf, MAX_BUFFER, stdin )) { // read a line

            //Tokenize the input into args array
            arg = args;
            *arg++ = strtok(buf,SEPARATORS);   // tokenize input
            while ((*arg++ = strtok(NULL,SEPARATORS))); // last entry will be NULL

            // Only need to continue if anything is here
            if (!args[0])
                break;

// TODO What happens when history commands goes over 15?
            arg = args;
            while (*arg){
                strcat(history[historyIndex],*arg++);
                strcat(history[historyIndex], " ");
            }
            historyIndex++;

            // Check if the command is a recognized command:

            // Clear Command
            if (!strcmp(args[0],"clear")) {
                system("clear");
            }

            // Quit Command
            else if (!strcmp(args[0],"quit"))
                break; // break out of 'while' loop

            // Change Directory
            else if (!strcmp(args[0], "cd")){
                ChangeDirectoryCommand();
            }

            // Print Working Directory
            else if (!strcmp(args[0], "pwd")){
                printf("Current Directory: %s\n", cwd);
            }

            // History Command
            else if (!strcmp(args[0], "history")){
                HistoryCommand();
            }

            // Run command
            else if (!strcmp(args[0], "run")){
                RunCommand();
            }

            // Run in background command
            else if (!strcmp(args[0], "background")){
                BackgroundRunCommand();
            }

            // Kill a particular process
            else if (!strcmp(args[0], "murder")){
                if(!kill(atoi(args[1]), SIGKILL))
                    printf("Success! The PID is dead.");
            }

            // Command is unknown
            else {
                arg = args;
                printf("Invalid command: ");
                while (*arg) fprintf(stdout,"%s ",*arg++);
                fputs ("\n", stdout);
            }
        }
    }
    return 0;
}

// If the path doesn't start with a '/' then we are appending to the exisiting path.
// Otherwise we are replacing the path
void AppendDirectory(char * runpath){

    // Check if we want to go up a level
    if (!strcmp(runpath, "..")){
        char * temp = malloc(strlen(cwd) + 1);
        strcpy(temp, cwd);

        printf("We got here with runpath |%s|\n", runpath);
        char *chptr = strrchr(cwd, '/');
        int index = chptr - cwd;
        temp[index] = '\0';
        strcpy(runpath, temp);
        printf("Yo: %s\n", runpath);
    }

    // Check if we are appending to directory, or replacing it
    else if (runpath[0] != '/'){
        // Store cwd in a temporary variable
        char * temp = malloc(strlen(cwd) + 1);
        strcpy(temp, cwd);

        // Check if we need to add a trailing slash
        if (temp[(strlen(temp)-1)] != '/'){
            temp = realloc(temp, strlen(temp) + 1);
            strcat(temp, "/");
        }

        // Add current directory to the end of the runpath
        temp = realloc(temp, strlen(temp) + strlen(runpath));
        strcat(temp, runpath);
        strcpy(runpath, temp);
    }
}

void ChangeDirectoryCommand(){

    // Make sure we have a path thats not null
    if (args[1] == NULL){
        printf("Error, Expected Path Argument following cd\n");
        return;
    }

    // Store args[1] in a temporary path
    char * path = malloc(sizeof(char) * (strlen(args[1]) + 1));
    strcpy(path, args[1]);

    AppendDirectory(path);

    // Check if Directory Exists
    DIR* dir = opendir(path);
    if (!dir){
        printf ("Error: Invalid Directory for %s\n", path);
        return;
    }

    // If its made it here, its a valid directory
    // Update internal variable for current directory
    cwd = malloc(strlen(path) + 1);
    strcpy(cwd, path);
    printf("Directory changed to %s\n", cwd);
}

// Method that prints out the history of recent commands or clears it
void HistoryCommand() {
    int i  = 0;

    if (args[1] && !strcmp(args[1], "-c")){
        for (i=0; i<MAX_HISTORY; i++)
            strcpy(history[i], "");
        historyIndex = 0;
        return;
    }
    for (i=0; i<historyIndex; i++){
        fprintf(stdout, "%s\n", history[i]);
    }
}

void RunCommand() {

    char * arguments[MAX_ARGS];
    int i = 1;

    // We want all the arguments but the initial "run" command
    while (args[i]){
        arguments[i-1] = args[i];
        i++;
    }
    // Add a null terminatory so execv knows when to stop
    arguments[i-1] = '\0';

    char * runpath = malloc(sizeof(char) * (strlen(args[1]) + 1));
    strcpy(runpath, args[1]);

    AppendDirectory(runpath);

    // Child to run program
    pid_t my_pid, parent_pid, child_pid;
    int status;

    /* get and print my pid and my parent's pid. */

    my_pid = getpid();    parent_pid = getppid();
    printf("\n Parent: my pid is %d\n\n", my_pid);
    printf("Parent: my parent's pid is %d\n\n", parent_pid);

    /* print error message if fork() fails */
    if((child_pid = fork()) < 0 )
    {
      perror("fork failure");
      exit(1);
    }

    /* fork() == 0 for child process */

    if(child_pid == 0){
        printf("\nChild: I am a new-born process!\n\n");
        my_pid = getpid();    parent_pid = getppid();
        printf("Child: my pid is: %d\n\n", my_pid);
        printf("Child: my parent's pid is: %d\n\n", parent_pid);

        printf("Child: Now, I woke up and am executing given command \n\n");
        execv(runpath, arguments);
        //execl("/home/za643519/myfolder/helloworld", "helloworld", 0, 0, NULL);
        perror("execl() failure!\n\n");

        printf("This print is after execl() and should not have been executed if execl were successful! \n\n");

        _exit(1);
    }
    /*
    * parent process
    */
    else
    {
        printf("\nParent: I created a child process.\n\n");
        printf("Parent: my child's pid is: %d\n\n", child_pid);
        //system("ps -acefl | grep ercal");  printf("\n \n");
        wait(&status); /* can use wait(NULL) since exit status from child is not used. */
        printf("\n Parent: my child is dead. I am going to leave.\n \n ");
    }
}

void BackgroundRunCommand() {

    if (args[1] == NULL){
        printf("Error, Expected Argument following cd\n");
        return;
    }

    char * arguments[MAX_ARGS];
    int i = 1;

    // We want all the arguments but the initial "run" command
    while (args[i]){
        arguments[i-1] = args[i];
        i++;
    }
    // Add a null terminatory so execv knows when to stop
    arguments[i-1] = '\0';

    char * runpath = malloc(sizeof(char) * (strlen(args[1]) + 1));
    strcpy(runpath, args[1]);

    AppendDirectory(runpath);

    // Child to run program
    pid_t my_pid, parent_pid, child_pid;
    int status;

    /* get and print my pid and my parent's pid. */

    my_pid = getpid();    parent_pid = getppid();
    printf("\n Parent: my pid is %d\n\n", my_pid);
    printf("Parent: my parent's pid is %d\n\n", parent_pid);

    /* print error message if fork() fails */
    if((child_pid = fork()) < 0 )
    {
      perror("fork failure");
      exit(1);
    }

    /* fork() == 0 for child process */

    if(child_pid == 0){
        printf("\nChild: I am a new-born process!\n\n");
        my_pid = getpid();    parent_pid = getppid();
        printf("Child: my pid is: %d\n\n", my_pid);
        printf("Child: my parent's pid is: %d\n\n", parent_pid);

        printf("Child: Now, I woke up and am executing given command \n\n");
        execv(runpath, arguments);
        //execl("/home/za643519/myfolder/helloworld", "helloworld", 0, 0, NULL);
        perror("execl() failure!\n\n");

        printf("This print is after execl() and should not have been executed if execl were successful! \n\n");

        _exit(1);
    }
    /*
    * parent process
    */
    else
    {
        printf("\nParent: I created a child process.\n\n");
        printf("Parent: my child's pid is: %d\n\n", child_pid);
        //system("ps -acefl | grep ercal");  printf("\n \n");
        //wait(&status); /* can use wait(NULL) since exit status from child is not used. */
        printf("\n Parent: my child is dead. I am going to leave.\n \n ");
    }
}
