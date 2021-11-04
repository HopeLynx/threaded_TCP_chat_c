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
#define HOST_PORT 1236
#define MAX_CLIENTS_NUM 25
#define EXIT_PHRASE "exit\n"

struct client {
    char name[MAX_NAME_LEN];
    int socket;
};

pthread_mutex_t mutex;
int clients[MAX_CLIENTS_NUM];
int n=0,non_exit_flag=1;

void send_to_all(char *msg,int curr){
    int i;
    pthread_mutex_lock(&mutex);
    printf("%s",msg);
    for(i = 0; i < n; i++) {
        if(clients[i] != curr) {
            if(send(clients[i],msg,strlen(msg),0) < 0) {
                printf("sending failure \n");
                continue;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

void *recieve_message(void *client_socket){
    int sock = *((int *)client_socket);
    char msg[MAX_MSG_LEN+MAX_NAME_LEN];
    int len;
    while((len = recv(sock,msg,MAX_MSG_LEN+MAX_NAME_LEN,0)) > 0) {
        msg[len] = '\0';
        //TODO - proper parcer

        send_to_all(msg,sock);
    }
    if (len == 0){
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

    if(listen( main_socket ,MAX_CLIENTS_NUM ) == -1) printf("listening failed \n");
    pthread_create(&stdin_thread, NULL, &read_stdin, NULL);
    while(non_exit_flag){
        if( (client_socket = accept(main_socket, (struct sockaddr *)NULL,NULL)) < 0 )
            printf("accept failed  \n");
        pthread_mutex_lock(&mutex);
        clients[n]= client_socket;
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
            pthread_join(clients[i], NULL);
            }
    pthread_join(recieve_thread,NULL);
    pthread_mutex_unlock(&mutex);
    printf("Have a nice day!");
    return 0;

}
