#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//the -e option indicates that the next argument is the command to be executed.
//(aka ttt 123456789)
int main(int argc, char *argv[]) {
    // check if the number of arguments is correct and if the second argument is "-e"
    if (argc != 3 || strcmp(argv[1], "-e") != 0) {
        printf("Error\n");
        exit(1);
    }

    // split the command line into arguments
    char *args[256];
    int i = 0;
    char *token = strtok(argv[2], " ");// split the command string by space 
    while (token != NULL && i < 255) {// continue until there are no more tokens or the array is full
        args[i] = token;// store the token in the args array
        i++;// increment the index
        token = strtok(NULL, " ");// Get the next token
    }
    args[i] = NULL; // // set null to terminate the args array

     // Execute the command using execvp
    // execvp replaces the current process with a new process running the specified command
    if (execvp(args[0], args) == -1) {
        perror("execvp failed");// print an error message if execvp fails
        exit(1);  // exit with an error code
    }

    return 0;
}
