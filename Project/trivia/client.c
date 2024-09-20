#include <stdio.h> 
#include <ctype.h>
#include <unistd.h> // for getopts
#include <stdlib.h> // for exit
#include <sys/socket.h> 
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#include <bits/getopt_core.h>
//#include <bits/getopt_core.h>


#define USAGE_MSG "Usage: %s [-i IP_address] [-p port_number] [-h]\n"
#define QUESTION "questions.txt"
#define QUESTION_MSG "Question file: %s\n"
#define IP_ADD "127.0.0.1"
#define IP_MSG "IP address: %s\n"
#define PORT 25555
#define PORT_MSG "Port number: %d\n"
#define ERROR_MSG_UNKNOWN "Error: Unknown option '-%c' received.\n"
#define ERROR_FLAG_UNKNOWN "Error: Cannot use multiple flags.\n"
#define ERROR_MISS_UNKNOWN "Error: Missing an arguement for '-%c'.\n"
#define ERROR_I_UNKNOWN "Error: i option invalid.\n"
#define ERROR_P_UNKNOWN "Error: p invalid.\n"
#define STDIN_FILENO 0 
#define MAX_CONN 3 
#define MAXER 1024

void printer(const char *filename) {
    printf(USAGE_MSG, filename);
    printf("\n");
    printf("  -i IP_address      Default to: \"127.0.0.1\";\n");
    printf("  -p port_number     Default to: 25555;\n");
    printf("  -h                 Display this help info.\n");
}

void parse_connect(int argc, char** argv, int* server_fd){
    char *ip = IP_ADD;
    int port = PORT; 

    // task 1 establish server with getopt 
    int hflag = 0;
    int pflag = 0;
    int iflag = 0;
    
    int serverint;
    while ((serverint = getopt (argc,argv,":i:p:h")) != -1){
        switch (serverint){
        case 'i':
            if(optarg != NULL && strcmp(optarg,"-f") != 0 && strcmp(optarg,"-p") != 0 && strcmp(optarg,"-h") != 0){
                ip = optarg;
                iflag++;
            }
            else{
                fprintf(stderr,ERROR_I_UNKNOWN);
                exit(EXIT_FAILURE);
            }
            break;
        case 'p':
            if(optarg != NULL && strcmp(optarg,"-f") != 0 && strcmp(optarg,"-i") != 0 && strcmp(optarg,"-h") != 0){
                int per = atoi(optarg);
                if(per == 0 && strcmp(optarg,"0") != 0){
                    fprintf(stderr,ERROR_P_UNKNOWN);
                    exit(EXIT_FAILURE);
                }
                port = per;
                pflag++;
            }
            else{
                fprintf(stderr,ERROR_P_UNKNOWN);
                exit(EXIT_FAILURE);
            }
            break;
        case 'h':
            hflag += 1;
            break;
        case ':':
            fprintf(stderr,ERROR_MISS_UNKNOWN,optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr,ERROR_MSG_UNKNOWN,optopt);
            exit(EXIT_FAILURE);
        }
    }
    if(iflag > 1 || pflag > 1 || hflag > 1){
        fprintf(stderr,ERROR_FLAG_UNKNOWN);
        exit(EXIT_FAILURE);
    }
    if(hflag == 1){
        printer(argv[0]);
        exit(EXIT_SUCCESS);
    }
    
    struct sockaddr_in server_addr;
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd == -1) {
        perror("Socket error");
        exit(EXIT_FAILURE);
    }
        
    // create a socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
        
    int c = connect(*server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (c < 0){
        perror("Connect error");
        exit(EXIT_FAILURE); 
    }
}


int main (int argc, char*argv[]){
    int server_fd; 
    struct sockaddr_in server_addr;
    struct sockaddr_in in_addr; 
    socklen_t addr_size = sizeof(in_addr);
    int max_fd = server_fd;

    parse_connect(argc, argv, &server_fd);
    char buffer[MAXER];
    int cfds[MAX_CONN];
    
    cfds[0] = STDIN_FILENO;
    cfds[1] = server_fd; 

    memset(buffer, 0, sizeof(buffer));
    
    while (1) {
        fd_set myset; 
        FD_ZERO(&myset);
        FD_SET(server_fd, &myset);
        FD_SET(STDIN_FILENO, &myset); 
        
        
        if (server_fd > STDIN_FILENO){
            max_fd = server_fd;
        } else {
            max_fd = STDIN_FILENO;
        }
        
        if (select(max_fd+1, &myset, NULL, NULL, NULL) == -1){
            perror("");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_fd, &myset)){
            ssize_t byter = recv(server_fd, buffer, MAXER, 0);
            if (byter <= 0){
                if (byter == 0){
                    printf("Server closed\n");
                } else {
                    perror("error");
                }
                break;
            } 
            buffer[byter] = '\0';
            printf("%s", buffer); 
            fflush(stdout);
        }

        if (FD_ISSET(STDIN_FILENO, &myset)){
            fgets(buffer, MAXER, stdin);
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n'){
                buffer[len - 1] = '\n';
            }
            send(server_fd, buffer, strlen(buffer), 0);
        }
    }
    close(server_fd);
    return 0;
}