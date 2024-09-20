/*******************************************************************************
 * Name        : minishell.c
 * Author      : Ryan Eshan
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System. 
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/wait.h>
#include <pwd.h>

#define BLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"

#define BUFFER_SIZE 1024 // Used to store string
#define SIGINT 2 // Interrupt the process

// Macros for error messages
#define ERROR_MSG_PASSWD_ENTRY "Error: Cannot get passwd entry. %s.\n"
#define ERROR_MSG_CHANGE_DIR "Error: Cannot change directory to %s. %s.\n"
#define ERROR_MSG_TOO_MANY_ARGS_CD "Error: Too many arguments to cd.\n"
#define ERROR_MSG_READ_STDIN "Error: Failed to read from stdin. %s.\n"
#define ERROR_MSG_FORK "Error: fork() failed. %s.\n"
#define ERROR_MSG_EXEC "Error: exec() failed. %s.\n"
#define ERROR_MSG_WAIT "Error: wait() failed. %s.\n"
#define ERROR_MSG_MALLOC "Error: malloc() failed. %s.\n"
#define ERROR_MSG_CWD "Error: Cannot get current working directory. %s.\n"
#define ERROR_MSG_SIGNAL_HANDLER "Error: Cannot register signal handler. %s.\n"

// Struct for process info 
typedef struct {
    int pid; // PID 
    char user[BUFFER_SIZE]; // User stuff related to the process 
    char commands[BUFFER_SIZE]; // Commands that are executed by the proces 
} ProcesssInformation;

volatile sig_atomic_t interrupted;

void signal_handler_function(int signaler) {
    interrupted = 1;
    printf("\n");
}

void print_prompt() {
    char cwd[BUFFER_SIZE]; // Store the cwd in the buffer
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s[%s]%s>", BLUE, cwd, DEFAULT);
    } else {
        fprintf(stderr, ERROR_MSG_CWD, strerror(errno));
    }
}

void mini_cd(char *path) {
    if (path == NULL || strcmp(path, "~") == 0) {
        char *home_directory = getenv("HOME");
        if (home_directory == NULL) {
            fprintf(stderr, ERROR_MSG_CWD, "HOME environment variable not set");
            return;
        }
        path = home_directory; 
    } else if (path[0] == '~') {
        char *home_directory = getenv("HOME");
        if (home_directory == NULL) {
            fprintf(stderr, ERROR_MSG_CWD, "HOME environment variable not set");
            return;
        }
        // full path with the home directory
        char full_path[BUFFER_SIZE];
        char *p = full_path;
        while (*home_directory) {
            *p++ = *home_directory++;
        }
        home_directory = path + 1;
        while (*home_directory) {
            *p++ = *home_directory++;
        }
        *p = '\0';
        char fuller_path[BUFFER_SIZE];
        if (realpath(full_path, fuller_path) == NULL) {
            fprintf(stderr, ERROR_MSG_CHANGE_DIR, path, strerror(errno));
            return;
        }
        path = fuller_path;
    }
    if (chdir(path) == -1) {
        fprintf(stderr, ERROR_MSG_CHANGE_DIR, path, strerror(errno));
    }
}

void mini_exit(){
    exit(EXIT_SUCCESS); 
}

void mini_pwd() {
    char cwd[BUFFER_SIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        fprintf(stderr, ERROR_MSG_CWD, strerror(errno));
    }
}

void mini_lf() {
    DIR *dir;
    struct dirent *dire;
    if ((dir = opendir(".")) == NULL) {
        perror("Error");
        return;
    }
    while ((dire = readdir(dir)) != NULL) {
        if (strcmp(dire->d_name, ".") != 0 && strcmp(dire->d_name, "..") != 0) {
            printf("%s\n", dire->d_name);
        }
    }
    closedir(dir);
}

// Function to print process info 
void print_process_information(ProcesssInformation *processes, int count) {
    for (int i = 0; i < count; i++) {
        printf("%d %s %s\n", processes[i].pid, processes[i].user, processes[i].commands);
    }
}

// Compare processes based on PID 
int compare_processes(const void *a, const void *b) {
    const ProcesssInformation *pa = (const ProcesssInformation *)a;
    const ProcesssInformation *pb = (const ProcesssInformation *)b;
    return pa->pid - pb->pid; 
}

void mini_lp() {
    // Open /proc directory 
    DIR *proc = opendir("/proc");
    if (!proc) {
        perror("Error trying to open the /proc directory");
        return;
    }
    
    ProcesssInformation processes[1000]; // Array to store the process info 
    int count = 0; // Count for the # of processes that are found 

    struct dirent *entry;
    // iterate each entry of the /proc directory 
    while ((entry = readdir(proc)) != NULL) {
        // check if the entry given is a directory and its name is a PID 
        if (entry->d_type == DT_DIR && atoi(entry->d_name) != 0) { // DT_DIR is a directory 
            char *pid_string = entry->d_name; // PID in a string 
            int pid = atoi(pid_string); // convert PID string to integer 
            processes[count].pid = pid; // store PID integer into the array 

            // read user and command
            char user[BUFFER_SIZE];
            char cmd_path[BUFFER_SIZE];
            strcpy(cmd_path, "/proc/");
            strcat(cmd_path, pid_string);
            strcat(cmd_path, "/cmdline");
            FILE *cmd_file = fopen(cmd_path, "r");
            if (cmd_file) {
                if (fgets(processes[count].commands, sizeof(processes[count].commands), cmd_file)) {
                    // remove newline character from the command
                    processes[count].commands[strcspn(processes[count].commands, "\n")] = 0;
                }
                fclose(cmd_file);
            }

            // read user from /proc/<PID>/status
            strcpy(cmd_path, "/proc/");
            strcat(cmd_path, pid_string);
            strcat(cmd_path, "/status");
            FILE *status_file = fopen(cmd_path, "r");
            if (status_file) {
                char line[BUFFER_SIZE];
                while (fgets(line, sizeof(line), status_file)) {
                    if (strncmp(line, "Uid:", 4) == 0) { // UID: user identifier 
                        int uid;
                        sscanf(line, "Uid:\t%d", &uid); // scan the UID 
                        struct passwd *pwd = getpwuid(uid);
                        // get user name that relates to the UID 
                        if (pwd != NULL) {
                            strcpy(processes[count].user, pwd->pw_name); // store user name 
                        } else {
                            strcpy(processes[count].user, "Unknown User Name"); // a case if user name is not found
                        }
                        break;
                    }
                }
                fclose(status_file); // file close 
            }

            count++; // count increment for next process 
        }
    }
    closedir(proc); // close /proc 

    // sort processes 
    qsort(processes, count, sizeof(ProcesssInformation), compare_processes);

    // print 
    print_process_information(processes, count);
}

void execute_command(char **user_input) {
    pid_t forker_pid = fork();

    if (forker_pid == -1){
        fprintf(stderr, ERROR_MSG_FORK, strerror(errno));
        return; 
    }

    // a child process is made (pid value is a child) and new subprocess is made
    if (forker_pid == 0){
        execvp(user_input[0], user_input);
        fprintf(stderr, ERROR_MSG_EXEC, strerror(errno));
        exit(EXIT_FAILURE); 
    } else {
        int status = 0; 
        if (waitpid(forker_pid, &status, 0) == -1){
            if (interrupted) {
            interrupted = 0; 
        } else {
            fprintf(stderr, ERROR_MSG_WAIT, strerror(errno));
            } 
        }
    }
    // a parent process is waits for child 
}

int main() {
    struct sigaction signaler = {0}; 
    signaler.sa_handler = signal_handler_function;

    if (sigaction(SIGINT, &signaler, NULL) == -1) {
        fprintf(stderr, ERROR_MSG_SIGNAL_HANDLER, strerror(errno));
        return 1;
    }

    char user_input[BUFFER_SIZE];
    size_t input_buffer_size = BUFFER_SIZE;

    while (1) {        
        print_prompt();

        if (fgets(user_input, BUFFER_SIZE, stdin) == NULL){
            if (interrupted) {
            interrupted = 0; 
            continue; 
        } else {
            fprintf(stderr, ERROR_MSG_READ_STDIN, strerror(errno));
            continue;
        }
        }

        if (user_input[strlen(user_input) - 1] == '\n') {
            user_input[strlen(user_input) - 1] = '\0';
        }

        char *token = strtok(user_input, " ");
        int mini_command;

        char *args[BUFFER_SIZE]; 

        int i = 0; 
        while (token != NULL) {
            args[i] = token;
            i++; 
            token = strtok(NULL, " ");
        }
            args[i] = NULL; 
            

        if (token == NULL && i != 0) {
            if (strcmp(args[0], "cd") == 0) {
                if (i > 2){
                    fprintf(stderr, ERROR_MSG_TOO_MANY_ARGS_CD);
                    continue;
                }
                mini_command = 0;
            } else if (strcmp(args[0], "exit") == 0) {
                mini_command = 1;
            } else if (strcmp(args[0], "pwd") == 0) {
                mini_command = 2;
            } else if (strcmp(args[0], "lf") == 0) {
                mini_command = 3;
            } else if (strcmp(args[0], "lp") == 0) {
                mini_command = 4;
            } else {
                mini_command = 5; // For other commands
            }
        }
        switch (mini_command) {
            case 0: // CD Command
                mini_cd(args[1]);
                break;
            case 1: // Exit Command
                mini_exit();
                break;
            case 2: // PWD Command
                mini_pwd();
                break;
            case 3: // LF Command
                mini_lf();
                break;
            case 4: // LP Command
                mini_lp();
                break;
            case 5: // Other commands
                execute_command(args);
                break;
        }
    }
    return 0;
}
