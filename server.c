//gcc server.c -pthread -o server
//./server.out

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define NULL ((void *)0)
#define MAX_MSG_LEN 500
#define MAX_NAME_LEN 100
#define HOST_IP "127.0.0.1"
#define HOST_PORT 1243
#define MAX_CLIENTS_NUM 25
#define EXIT_PHRASE "exit\n"

struct client {
    char name[MAX_NAME_LEN];
    int socket;
};

pthread_mutex_t mutex;
struct client clients[MAX_CLIENTS_NUM];
int n=0,non_exit_flag=1;

void send_to_all(char *msg,int curr){
    int i;
    pthread_mutex_lock(&mutex);
    printf("%s",msg);
    for(i = 0; i < n; i++) {
        if(clients[i].socket != curr) {
            if(send(clients[i].socket,msg,strlen(msg),0) < 0) {
                printf("sending failure \n");
                continue;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

void send_to_one(char *msg,int curr){
    pthread_mutex_lock(&mutex);
    printf("%s",msg);
    if(send(curr,msg,strlen(msg),0) < 0) {
        printf("sending failure \n");
    }
    pthread_mutex_unlock(&mutex);
}

int parse_command(char* cmd, char** params) { //split cmd into array of params
    int i,m=-1;
    for(i=0; i<2; i++) {
        params[i] = strsep(&cmd, " ");
        m++;
        if(params[i] == NULL) break;
    }
    return(m);
};

void mystrcpy(char* dest, char* source){
    int i = 0,m = 0;
        while (1){
            if (source[i]!='\n'){
            dest[m] = source[i];
            m++;
            }
            if (dest[m] == '\0')break;

            i++;
        }
    }

void *recieve_message(void *client_socket){
    int sock = *((int *)client_socket);
    int ind = 0;
    for(int i = 0; i < n; i++) if(clients[i].socket == sock) {ind = i;break;}
    char msg[MAX_MSG_LEN];
    int len;
    while((len = recv(sock,msg,MAX_MSG_LEN,0)) > 0) {
        msg[len] = '\0';
        //TODO - proper parcer

        char* tmp = strchr(msg,' ');
        msg[tmp-msg]='\0';
        tmp++;
        /*
            char* params[2];
            parse_command(msg, params); //split cmd into array of params
            if (strcmp(params[0], "name") == 0) {
                strcpy(clients[ind].name,params[1]);
                */
        char send_msg[MAX_MSG_LEN+MAX_NAME_LEN];
        if (strcmp(msg, "name") == 0) {
            strcpy(clients[ind].name,tmp);
            strcpy(send_msg,"new user:");
            strcat(send_msg,clients[ind].name);
            //                strcat(msg,"\n");
            send_to_all(send_msg,sock);

        } else if (strcmp(msg, "message") == 0){
            if (strlen(clients[ind].name)==0) {
                send_to_one("add name first\n",sock);
            } else {
                strcpy(send_msg,"message:");
                strcat(send_msg,tmp);
                strcat(send_msg,"| from: ");
                strcat(send_msg,clients[ind].name);
                send_to_all(send_msg,sock);
            }

        } else if (strcmp(msg, "quit") == 0){
            strcpy(send_msg,"oh, ");
            strcat(send_msg,clients[ind].name);
            strcat(send_msg,"must have left\n");
            send_to_all(send_msg,sock);

        } else {
            printf("wtf: %s ; %s",msg,tmp);
            send_to_one("unknown command\n",sock);
        }


    }
    if (len == 0){
        //желательно закрыть бы этот сокет и подвинуть массив
        n--;
        printf("The client closed the connection. Clients left: %d\n",n);
    }
}

void* read_stdin(){
    char s[MAX_NAME_LEN];
    while ((fgets(s,MAX_MSG_LEN,stdin) > 0) && non_exit_flag == 1){
        printf("%s",s);
        if (strcmp(s,EXIT_PHRASE)==0) {printf("terminating....\n");non_exit_flag = 0;}
    }
    return NULL;

}

int main(){
    struct sockaddr_in server_id;
    pthread_t recieve_thread,stdin_thread;
    int main_socket=0 , client_socket=0;

    server_id.sin_family = AF_INET;
    server_id.sin_port = htons(HOST_PORT);
    server_id.sin_addr.s_addr = inet_addr(HOST_IP);
    main_socket = socket( AF_INET , SOCK_STREAM, 0 );
    (bind( main_socket, (struct sockaddr *)&server_id, sizeof(server_id)) == -1) ? printf("cannot bind, error!! \n") : printf("Server Started \n");

    if(listen(main_socket ,MAX_CLIENTS_NUM ) == -1) printf("listening failed \n");
    pthread_create(&stdin_thread, NULL, &read_stdin, NULL);
    while(non_exit_flag){
        if( (client_socket = accept(main_socket, (struct sockaddr *)NULL,NULL)) < 0 )
            printf("accept failed  \n");
        pthread_mutex_lock(&mutex);
        clients[n].socket= client_socket;
        n++;
        // creating a thread for each client
        pthread_create(&recieve_thread,NULL,(void *)recieve_message,&client_socket);
        pthread_mutex_unlock(&mutex);
    }

    //good shutdown
    pthread_join(stdin_thread, NULL);
    pthread_mutex_lock(&mutex);
    for(int i = 0; i < n; i++) {
        printf("closing clients!\n");
        pthread_join(clients[i].socket, NULL);
    }
    pthread_join(recieve_thread,NULL);
    pthread_mutex_unlock(&mutex);
    printf("Have a nice day!");
    return 0;

}
