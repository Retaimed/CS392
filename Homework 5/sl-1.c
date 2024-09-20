/*******************************************************************************
 * Name        : sl.c
 * Author      : Ryan Eshan
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System. 
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h> //stat function 
#include <errno.h> // EONENT 
#include <sys/types.h> // pid 
#include <sys/wait.h> // waitpid function

// Macro for error messages
#define MSG_FOR_ARGS "Usage: %s <DIRECTORY>" 
#define MSG_FOR_TOTAL_FILES "Total files: %d\n"
#define ERROR_MSG_FOR_DIRECTORY_EXISTENCE "Error: Directory does not exist."
#define ERROR_MSG_FOR_FAILED "Error: Opening the directory."
#define ERROR_MSG_FOR_FIRST_ARG_DIR "The first argument has to be a directory."
#define ERROR_MSG_FOR_DIRECTORY_READABLE "Permission denied. %s cannot be read."
#define ERROR_MSG_FOR_OPENING_PIPE "Error: Failed to open the pipe."
#define ERROR_MSG_FOR_CHILD_PROCESS "Error: Failed waiting for a child process."
#define ERROR_MSG_FOR_FORK_CREATION "Error: Fork "
#define ERROR_MSG_FOR_LS "Error: ls failed."
#define ERROR_MSG_FOR_SORT "Error: sort failed."
#define ERROR_MSG_FOR_CLOSE_PIPE1 "Error: Failed to close read end of pipe1."
#define ERROR_MSG_FOR_WRITE_END "Error: Failed to close write end of pipe1."
#define ERROR_MSG_FOR_REDIRECT "Error: Failed to redirect STDOUT."
#define ERROR_MSG_FOR_REDIRECT1 "Error: Failed to redirect STDIN in childer B."
#define ERROR_MSG_FOR_REDIRECT2 "Error: Failed to redirect STDOUT in childer B"
#define ERROR_MSG_FOR_WRITE_END1 "Error: Failed to close write end of pipe1 in childer B."
#define ERROR_MSG_FOR_WRITE_ENDER1 "Error: Failed to close write end of pipe2 in childer B"
#define ERROR_MSG_FOR_READ_END1 "Error: Failed to close read end of pipe1 in childer B."
#define ERROR_MSG_FOR_READ_END2 "Error: Failed to close read end of pipe2 in childer B."

#define BUFSIZE 4096
// make a function that 1 ) check the first command-line argument 
// first check if a directory is there 
// then check if its a directory 
// afterwards check if directory is readable 
// ALL ERROR msg should be printed to stderr with an exit code of 1 

// Function to check directory existence, type, and readability
void checking_directory(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, MSG_FOR_ARGS, argv[0]);
        exit(EXIT_FAILURE);  
    }

    // Check if directory exists
    struct stat directoryinfo;
    if (stat(argv[1], &directoryinfo) == -1) {
        if (ENOENT == errno) { // directory does not exist (used stackoverflow to help come up with it)
            fprintf(stderr, ERROR_MSG_FOR_DIRECTORY_READABLE, argv[1]);
            exit(EXIT_FAILURE);
        } else { // cannot open the directory 
            fprintf(stderr, ERROR_MSG_FOR_FAILED);
            exit(EXIT_FAILURE);
        }
    }

    // check if it's a directory
    if (!S_ISDIR(directoryinfo.st_mode)) {
        fprintf(stderr, ERROR_MSG_FOR_FIRST_ARG_DIR);
        exit(EXIT_FAILURE);
    }

    // check if directory is readable
    DIR *dir = opendir(argv[1]);
    if (dir == NULL) {
        fprintf(stderr, ERROR_MSG_FOR_DIRECTORY_READABLE, argv[1]);
        exit(EXIT_FAILURE);
    }
    closedir(dir);
}

void printer_info (int fd){
    char storage[BUFSIZE];
    ssize_t counter;
    int total_files = 0;
    while ((counter = read(fd, storage, sizeof(storage))) > 0) {
        for (int i = 0; i < counter; i++){
            if (storage[i] == '\n'){
                total_files++;
            }
            write(STDOUT_FILENO, &storage[i], 1);
        }
    }
    printf(MSG_FOR_TOTAL_FILES, total_files);
}

int waitforprocess (pid_t pid){
    int condition;
    if (waitpid(pid, &condition, 0) == -1){
        fprintf(stderr, ERROR_MSG_FOR_CHILD_PROCESS);
        exit(EXIT_FAILURE); 
    }

    return condition;
}

// a pipe (in memory file that has a buffer saved from memory where you can write and read from it) 
    //        Child Process
    // File Descriptor (number unique across a process)  File
    //       0                                           STDIN (read)
    //       1                                           STDOUT (write)
    //       2                                           STDERR
int main(int argc, char *argv[]) {
    checking_directory(argc, argv);
    int fd_pipe1[2]; // PIPE 1 connect STDOUT of LS -> STDIN of SORT 
    // fd[0] - read fd[1] - write 
    int fd_pipe2[2]; // PIPE 2 connect STDOUT of SORT -> READ of PARENT PROCESS 
    
    // file descriptor is a key to access to a file (a key to tell us where we want to read or write data)

    // check if equals to -1 that meaning the pipe is unsuccessful
    if (pipe(fd_pipe1) == -1){
        fprintf(stderr, ERROR_MSG_FOR_OPENING_PIPE);
        exit(EXIT_FAILURE);
    }

    if (pipe(fd_pipe2) == -1){
        fprintf(stderr, ERROR_MSG_FOR_OPENING_PIPE);
        exit(EXIT_FAILURE);
    }

    // pipe() takes in array of two integers (the two integers being the file descriptors)
    // so pipe(fd) carries the keys to the pipe


    // goal 1) create two child processes
    // - we use the fork() 
    // using stack overflow define two childs in PID

    pid_t CHILDER_A, CHILDER_B;
    // let CHILDER_A be for the ls -1ai <DIRECTORY> 
    // let CHILDER_B be for the sort
    (CHILDER_A = fork()) && (CHILDER_B = fork()); // Create two children

    // check for a case when the fork is not equal to 0 
    if (CHILDER_A < 0){ // remember fork for the children process return a value of 0 so if the is not 0 then error (when fork returns -1)
        fprintf(stderr, ERROR_MSG_FOR_FORK_CREATION);
        exit(EXIT_FAILURE); 
    }

    
    if (CHILDER_A == 0) {
        // close the ends of the pipes
        if (close(fd_pipe1[0]) == -1){ // close the read end
            fprintf(stderr, ERROR_MSG_FOR_CLOSE_PIPE1); 
            exit(EXIT_FAILURE); 
        }                                                 
        
        if (dup2(fd_pipe1[1], STDOUT_FILENO) == -1){ // STDOUT_FILENO 1 and redirects ls 
            fprintf(stderr, ERROR_MSG_FOR_REDIRECT);
            exit(EXIT_FAILURE); 
        } 

        if (close(fd_pipe1[1]) == -1){ // close the write end 
            fprintf(stderr, ERROR_MSG_FOR_WRITE_END);
            exit(EXIT_FAILURE);
        }

        execlp("ls", "ls", "-ai", argv[1], NULL);

        fprintf(stderr, ERROR_MSG_FOR_LS);
        exit(EXIT_FAILURE); 
    }

    // check for a case when the fork is not equal to 0 
    if (CHILDER_B < 0){ // remember fork for the children process return a value of 0 so if the is not 0 then error
        fprintf(stderr, ERROR_MSG_FOR_FORK_CREATION);
        exit(EXIT_FAILURE); 
    } 
    
    if (CHILDER_B == 0) {
        // close the ends of the pipes 
        if (close(fd_pipe1[1]) == -1){
            fprintf(stderr, ERROR_MSG_FOR_WRITE_END1);
            exit(EXIT_FAILURE);
        }
        
        if (dup2(fd_pipe1[0], STDIN_FILENO) == -1){ // STDIN_FILENO 0 and redirects sort
            fprintf(stderr, ERROR_MSG_FOR_REDIRECT1);
            exit(EXIT_FAILURE);
        } 

        if (close(fd_pipe1[0]) == -1){
            fprintf(stderr, ERROR_MSG_FOR_READ_END1);
            exit(EXIT_FAILURE);
        }

        if (close(fd_pipe2[0]) == -1){
            fprintf(stderr, ERROR_MSG_FOR_READ_END2);
            exit(EXIT_FAILURE);
        }

        if (dup2(fd_pipe2[1], STDOUT_FILENO) == -1){
            fprintf(stderr, ERROR_MSG_FOR_REDIRECT2);
            exit(EXIT_FAILURE);
            
        } 

        if (close(fd_pipe2[1]) == -1){ // close the fd write end (this file has been close nothing to read)
            fprintf(stderr, ERROR_MSG_FOR_WRITE_ENDER1);
            exit(EXIT_FAILURE);
        } 

        execlp("sort", "sort", NULL);

        fprintf(stderr, ERROR_MSG_FOR_SORT);
        exit(EXIT_FAILURE);
    } 
    // close filedescriptors 
    close(fd_pipe1[0]);
    close(fd_pipe1[1]);
    close(fd_pipe2[1]); 

    printer_info(fd_pipe2[0]);

    close(fd_pipe2[0]);
    
    waitforprocess(CHILDER_A);
    waitforprocess(CHILDER_B);
    return EXIT_SUCCESS;
}
