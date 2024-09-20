/*******************************************************************************
 * Name        : server.c
 * Author      : Ryan Eshan
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System. 
 ******************************************************************************/
#include <stdio.h> 
#include <ctype.h>
#include <unistd.h> // for getopts
#include <stdlib.h> // for exit
#include <sys/socket.h> 
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
//#include <bits/getopt_core.h>

#define USAGE_MSG "Usage: %s [-f question_file] [-i IP_address] [-p port_number] [-h]\n"
#define QUESTION "questions.txt"
#define QUESTION_MSG "Question file: %s\n"
#define IP_ADD "127.0.0.1"
#define IP_MSG "IP address: %s\n"
#define PORT 25555
#define PORT_MSG "Port number: %d\n"
#define ERROR_MSG_UNKNOWN "Error: Unknown option '-%c' received.\n"
#define ERROR_FLAG_UNKNOWN "Error: Cannot use multiple flags.\n"
#define ERROR_MISS_UNKNOWN "Error: Missing an arguement for '-%c'.\n"
#define ERROR_F_UNKNOWN "Error: f option invalid.\n"
#define ERROR_I_UNKNOWN "Error: i option invalid.\n"
#define ERROR_P_UNKNOWN "Error: p invalid.\n"
#define WELCOME_MSG "Welcome to 392 Trivia!\n"
#define MAX_CONN 3 
#define MAXER 9000 
#define QUESTIONER "Question %d: %s\n"
#define PRESS1 "Press 1: %s\n"
#define PRESS2 "Press 2: %s\n"
#define PRESS3 "Press 3: %s\n"
#define OPTION1 "1: %s\n"
#define OPTION2 "2: %s\n"
#define OPTION3 "3: %s\n"
#define NEW_CONNECTION "New connection detected!\n"
#define NAMER_PLEASE "Please type your name:"
#define LOST_CONNECTION "Lost connection!\n"
#define MAX_CONNECTION "Max connection reached\n"
#define HI_MSG "Hi %s\n"
#define CONGRATS_MSG "Congrats, %s!\n"
#define CORRECT_ANSWER_FMT "Correct answer: %d - %s\n"
#define GAME_START_MSG "The game starts now!\n"


struct Entry {
    char prompt[1024];
    char options[3][50];
    int answer_idx; 
};

struct Player {
    int fd;
    int score;
    char name[128];
}; 

// prints the flags 
void printer(const char *filename) {
    printf(USAGE_MSG, filename);
    printf("\n");
    printf("  -f question_file   Default to: \"question.txt\";\n");
    printf("  -i IP_address      Default to: \"127.0.0.1\";\n");
    printf("  -p port_number     Default to: 25555;\n");
    printf("  -h                 Display this help info.\n");
}


int read_question(struct Entry* arr, char* filename){
    // open file in read mode, if successful it loads into memory 
    FILE *filer;

    if ((filer = fopen(filename, "r")) == NULL){
        return -1;
    }  

    if (filer == NULL){
        perror("error");
        exit(EXIT_FAILURE);
    }

    int num_question;

    for (int num_question = 0; num_question < 50; num_question++){
        char question_line[1024];
        char options_line[1024];
        char answer_line[1024];
        char d_line[MAXER];

        fgets(question_line,sizeof(question_line),filer);
        fgets(options_line,sizeof(options_line),filer);
        fgets(answer_line,sizeof(answer_line),filer);

        question_line[strcspn(question_line,"\n")] = '\0';
        options_line[strcspn(options_line,"\n")] = '\0';
        answer_line[strcspn(answer_line,"\n")] = '\0';

        sprintf(arr[num_question].prompt,"%s",question_line);

        sscanf(options_line,"%s %s %s",arr[num_question].options[0],arr[num_question].options[1],arr[num_question].options[2]);

        for(int i = 0; i < 3; i++){
            if(strcmp(answer_line,arr[num_question].options[i]) == 0){
                arr[num_question].answer_idx = i;
                break;
            }
        }

        if(fgets(d_line,sizeof(d_line),filer) == NULL){
            fclose(filer);
            return (num_question+1);
        }

    }

    fclose(filer);
    return num_question;
}

// // Using the geeks for geeks 

int main (int argc, char*argv[]){
    
    char *ques = QUESTION;
    char *ip = IP_ADD;
    int port = PORT; 

    int fflag = 0; 
    int iflag = 0;
    int pflag = 0; 
    int hflag = 0; 


    // task 1 establish server with getopt 
    int serverint;
    while ((serverint = getopt (argc,argv,"f:i:p:h")) != -1){
        switch (serverint){
        case 'f':
            if(optarg != NULL && strcmp(optarg,"-i") != 0 && strcmp(optarg,"-p") != 0 && strcmp(optarg,"-h") != 0){
                ques = optarg;
                fflag++; 
            } else{
                fprintf(stderr, ERROR_F_UNKNOWN);
                exit(EXIT_FAILURE);
            }
            break; 
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
    if(fflag > 1 || iflag > 1 || pflag > 1 || hflag > 1){
        fprintf(stderr,ERROR_FLAG_UNKNOWN);
        exit(EXIT_FAILURE);
    }
    if(hflag == 1){
        printer(argv[0]);
        exit(EXIT_SUCCESS);
    }

    // printf(QUESTION_MSG, ques);
    // printf(IP_MSG, ip);
    // printf(PORT_MSG, port);

    // read question
    struct Entry rques[50];
    int numques = read_question(rques, QUESTION);

    int server_fd;
    int client_fd; 
    struct sockaddr_in server_addr;
    struct sockaddr_in in_addr; 
    socklen_t addr_size = sizeof(in_addr);

    // create a server socket 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket error");
        exit(EXIT_FAILURE);
    }

    // server address struct 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);


    // this binds the ip and port to the socket 
    int b = bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (b < 0){
        perror("Bind error");
        exit(EXIT_FAILURE);
    }

    // 3 connections 
    if (listen(server_fd, MAX_CONN) == -1){
        perror("Listen error");
        exit(EXIT_FAILURE); 
    }

    printf(WELCOME_MSG);
    fd_set myset;
    FD_SET(server_fd, &myset);
    int max_fd = server_fd;
    int n_conn = 0;
    int player_count = 0;
    struct Player clienters[MAX_CONN];
    for(int i = 0; i < MAX_CONN; i++) {
        clienters[i].fd = -1;
        clienters[i].score = 0; 
        memset(clienters[i].name,0,sizeof(clienters[i].name));
    }
    char clienters_response[MAX_CONN][MAXER];


    int recvbytes = 0;
    char buffer[128];
    // accept TODO
    while (1) {
        // Re-initalization 
        FD_ZERO(&myset);
        FD_SET(server_fd, &myset);
        max_fd = server_fd; 

        for (int i = 0; i < MAX_CONN; i++){
            if (clienters[i].fd != -1){
                FD_SET(clienters[i].fd, &myset);
                if (clienters[i].fd > max_fd) max_fd = clienters[i].fd;
            }
        }
        // monitoring fds
        if (select(max_fd+1, &myset, NULL, NULL, NULL) == -1){
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_fd, &myset)){
            client_fd = accept(server_fd, (struct sockaddr*)&in_addr, &addr_size);
            if (n_conn < MAX_CONN){
                printf(NEW_CONNECTION);
                char namer[] = NAMER_PLEASE;
                n_conn++;

                for(int i = 0; i < MAX_CONN; i++){
                    if(clienters[i].fd == -1){
                        clienters[i].fd = client_fd;
                        break;
                    }
                }

                write(client_fd, namer, strlen(namer));
            }
            else{
                printf(MAX_CONNECTION);
                close(server_fd);
                break;
                }
        } 
            for(int j = 0; j < MAX_CONN; j++){
                    if(clienters[j].fd != -1 && FD_ISSET(clienters[j].fd,&myset)){
                        recvbytes = read(clienters[j].fd, buffer, sizeof(buffer));
                        if (recvbytes == 0){
                            printf(LOST_CONNECTION);
                            close(clienters[j].fd);
                            clienters[j].fd = -1;
                            n_conn--;
                            if(clienters[j].name[0] != 0){
                                memset(clienters[j].name,0,sizeof(clienters[j].name));
                                player_count--;
                            }
                        } 
                        else {
                            if(clienters[j].name[0] == 0){
                                buffer[recvbytes] = 0;
                                buffer[strcspn(buffer,"\n")] = '\0';
                                printf(HI_MSG, buffer); 
                                sprintf(clienters[j].name,"%s",buffer);
                                player_count++;
                            }
                        }
                    }
            }
            if(player_count == MAX_CONN){
                printf(GAME_START_MSG);
                break;
            }
    }
    while(1){
                int current_question_index = 0;
                int num_players = n_conn;
                
                while (current_question_index < numques) {
                    FD_ZERO(&myset);
                    for (int i = 0; i < MAX_CONN; i++){
                        if (clienters[i].fd != -1){
                        FD_SET(clienters[i].fd, &myset);
                        if (clienters[i].fd > max_fd) max_fd = clienters[i].fd;
                        }
                    }

                    struct Entry current_question = rques[current_question_index];
                    char ques_prompt[MAXER];
                    sprintf(ques_prompt, QUESTIONER, current_question_index + 1, current_question.prompt);
                    char optioner1[MAXER];
                    sprintf(optioner1, PRESS1, current_question.options[0]);
                    char optioner2[MAXER];
                    sprintf(optioner2, PRESS2, current_question.options[1]);
                    char optioner3[MAXER];
                    sprintf(optioner3, PRESS3, current_question.options[2]);
                    char optioner1_dis[MAXER];
                    sprintf(optioner1_dis,OPTION1,current_question.options[0]);
                    char optioner2_dis[MAXER];
                    sprintf(optioner2_dis,OPTION2,current_question.options[1]);
                    char optioner3_dis[MAXER];
                    sprintf(optioner3_dis,OPTION3,current_question.options[2]);
                    
                    for (int i = 0; i < MAX_CONN; i++) {
                        if (clienters[i].fd != -1) {
                            write(clienters[i].fd, ques_prompt, strlen(ques_prompt));
                            write(clienters[i].fd, optioner1, strlen(optioner1));
                            write(clienters[i].fd, optioner2, strlen(optioner2));
                            write(clienters[i].fd, optioner3, strlen(optioner3));
                        }
                    } 
                    printf("%s",ques_prompt);
                    printf("%s",optioner1_dis);
                    printf("%s",optioner2_dis);
                    printf("%s",optioner3_dis);

                    if (select(max_fd+1, &myset, NULL, NULL, NULL) == -1){
                        perror("select");
                        exit(EXIT_FAILURE);
                    }
                    for (int i = 0; i < MAX_CONN; i++) {
                        if (clienters[i].fd != -1 && FD_ISSET(clienters[i].fd,&myset)) {
                            char chipi_chipi[2];
                            int byter = read(clienters[i].fd, chipi_chipi, sizeof(chipi_chipi));

                            if (byter > 0) {
                                chipi_chipi[byter] = '\0';
                                int choser = atoi(chipi_chipi);
                                if(current_question.answer_idx == (choser - 1)){
                                    clienters[i].score++;
                                }
                                else{
                                    clienters[i].score--;
                                }
                            } else {
                                printf(LOST_CONNECTION);
                                for(int i = 0; i < MAX_CONN; i++){
                                    close(clienters[i].fd);
                                    clienters[i].fd = -1;
                                }
                                close(server_fd);
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                     int correcter = current_question.answer_idx;
                     char answerer[MAXER];
                     sprintf(answerer, CORRECT_ANSWER_FMT, correcter+1, current_question.options[correcter]);

                    for (int i = 0; i < MAX_CONN; i++) {
                        if (clienters[i].fd != -1) {
                            write(clienters[i].fd, answerer, strlen(answerer));
                        }
                    }
                    printf("%s",answerer);

                    current_question_index++;
                }

                int highscorer = clienters[0].score;

                for (int i = 0; i < MAX_CONN; i++) {
                    if (clienters[i].fd != -1 && clienters[i].score > highscorer) {
                        highscorer = clienters[i].score;
                    }
                }
                char loader[MAXER];
                for(int i = 0; i < MAX_CONN; i++){
                    if(clienters[i].fd != -1 && clienters[i].score == highscorer){
                         sprintf(loader, CONGRATS_MSG, clienters[i].name);
                         printf("%s",loader);
                    }
                }

                printf("Game over. Here are the scores: \n");
                for (int i = 0; i < MAX_CONN; i++){
                    if (clienters[i].fd != -1){
                        printf("%s: %d\n", clienters[i].name, clienters[i].score);
                    }
                }

                for (int i = 0; i < MAX_CONN; i++) {
                    if (clienters[i].fd != -1) {
                    close(clienters[i].fd);
                    }
                }
                close(server_fd);
                return 0; 
            }
    close(server_fd);
    return 0;
    }