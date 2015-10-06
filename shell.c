#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BUFFER 1024                        // max line buffer
#define MAX_ARGS 64                            // max # args
#define SEPARATORS " \t\n"                     // token sparators
#define DEFAULT_PATH "/home/za643519"

// Global Variables
char buf[MAX_BUFFER];                      // line buffer
char * args[MAX_ARGS];                     // pointers to arg strings
char ** arg;                               // working pointer thru args
char * prompt = "#";                       // shell prompt
char* cwd;
char history[MAX_BUFFER][MAX_ARGS];
int pids[MAX_BUFFER];
int historyIndex = 0;
int pidIndex = 0;

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
    cwd = malloc(MAX_BUFFER);
    getcwd(cwd, sizeof(cwd));
    // Initialize default path
    //strcpy(cwd, DEFAULT_PATH); // TODO I don't think we'll need this default path anymore
    // initialize history array
    for (i=0; i<MAX_BUFFER; i++)
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

            arg = args;
            while (*arg){
                strcat(history[historyIndex],*arg++);
                strcat(history[historyIndex], " ");
            }
            historyIndex++;

            // Reset history if we reach the max. Shouldn't happen really ever.
            if(historyIndex >= MAX_BUFFER)
                historyIndex = 0;

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
                BackgroundRunCommand(0);
            }

            // Kill a particular process
            else if (!strcmp(args[0], "murder")){
                if(!kill(atoi(args[1]), SIGKILL))
                    printf("Success! The PID is dead.\n");
            }

            // Run a command multiple times
            else if (!strcmp(args[0], "repeat")){
                int n = atoi(args[1]);
                for (i = 0; i<n; i++)
                    BackgroundRunCommand(1);
            }

            else if (!strcmp(args[0], "murder")){
                if(!kill(atoi(args[1]), SIGKILL))
                    printf("Success! The PID is dead.\n");
                else
                    printf("Failure. Pid was not killed. It may not exist.\n");
            }

            else if (!strcmp(args[0], "murderall")){
                if (pidIndex > 0)
                    printf("Murdering %d processes", pidIndex);
                else
                    printf("No processes to murder");
                for (i = 0; i<pidIndex; i++){
                    kill(pids[i], SIGKILL);
                    printf(" %d", pids[i]);
                }
                printf("\n");
                pidIndex = 0;
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

        char *chptr = strrchr(cwd, '/');
        int index = chptr - cwd;
        temp[index] = '\0';
        strcpy(runpath, temp);
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
        for (i=0; i<MAX_BUFFER; i++)
            strcpy(history[i], "");
        historyIndex = 0;
        return;
    }
    for (i=0; i<historyIndex; i++){
        fprintf(stdout, "%s\n", history[i]);
    }
}

void RunCommand() {

    // Check that we have arguments
    if (args[1] == NULL){
        printf("Error, Expected Argument following run\n");
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
    pid_t child_pid;

    // Print error if fork fails
    if((child_pid = fork()) < 0 )
    {
      printf("Failure with Fork\n");
      return;
    }

    // If we have the child pid
    if(child_pid == 0){
        // Run the program
        execv(runpath, arguments);

        // We shouldn't get here ever, unless there was an error
        printf("Error, program failed to execute.\n");
        return;
    }

    // Parent process
    else
    {
        // Wait for child to finish
        wait(NULL);
    }
}

// The 'multiple' parameter determines if this method is called by background command
// or the repeat command. The arguments are in different indexies for each.
void BackgroundRunCommand(int multiple) {

    // Check that we have arguments
    if (args[1] == NULL){
        printf("Error, Expected Argument following command\n");
        return;
    }

    // Depending on if we get the command repeat n program or run program,
    // the argument indexies are different.
    char * arguments[MAX_ARGS];
    int i = 1;
    int j = 1;
    if(multiple == 1){
        i++;
        j++;
    }

    // We want all the arguments but the initial "run" command
    while (args[i]){
        arguments[i-j] = args[i];
        i++;
    }

    // Add a null terminatory so execv knows when to stop
    arguments[i-j] = '\0';

    char * runpath = malloc(sizeof(char) * (strlen(args[j]) + 1));
    strcpy(runpath, args[j]);

    AppendDirectory(runpath);

    // Child to run program
    pid_t child_pid;

    // Print an error if the fork failed
    if((child_pid = fork()) < 0 )
    {
        // We shouldn't get here ever, unless there was an error
        printf("Error, program failed to execute.\n");
        return;
    }

    if(child_pid == 0){
        execv(runpath, arguments);
        // We shouldn't get here unless it failed to execute.
        printf("Error: Program failed to execute.");
        return;
    }
    else{
        // Print the newly created Child Pid
        printf("Newly Created Pid: %d\n", child_pid);
        pids[pidIndex] = child_pid;
        pidIndex++;
    }
}
