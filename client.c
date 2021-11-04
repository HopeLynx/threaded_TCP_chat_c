//gcc client.c -pthread -o client
//./client.out client_name

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define NULL ((void *)0)

//char msg[500];
#define MAX_MSG_LEN 500
#define MAX_NAME_LEN 100
#define HOST_IP "127.0.0.1"
#define HOST_PORT 1243

void *recieve_message(void *my_sock){
    int sock = *((int *)my_sock);
    int len;char msg[MAX_MSG_LEN+MAX_NAME_LEN];
    // client thread always ready to receive message
    while((len = recv(sock,msg,MAX_MSG_LEN+MAX_NAME_LEN,0)) > 0) {
        msg[len] = '\0';
        fputs(msg,stdout);
    }
    //
    return NULL;
}

int main(){
    pthread_t recieve_thread;
    int len, sock;

    struct sockaddr_in ServerIp;
//    char client_name[MAX_NAME_LEN];
//    strcpy(client_name, argv[1]);
    sock = socket( AF_INET, SOCK_STREAM,0);
    ServerIp.sin_port = htons(HOST_PORT);
    ServerIp.sin_family= AF_INET;
    ServerIp.sin_addr.s_addr = inet_addr(HOST_IP);
    if( (connect( sock ,(struct sockaddr *)&ServerIp,sizeof(ServerIp))) == -1 )
        printf("\n connection to socket failed \n");

    //creating a client thread which is always waiting for a message
    pthread_create(&recieve_thread,NULL,(void *)recieve_message,&sock);

    //ready to read a message from console
    char tmp_msg[MAX_MSG_LEN];
    while(fgets(tmp_msg,MAX_MSG_LEN,stdin) > 0) {
        len = write(sock,tmp_msg,strlen(tmp_msg));
        if(len < 0)
            printf("\n message not sent \n");
    }

    //thread is closed
    pthread_join(recieve_thread,NULL);
    close(sock);
    return 0;
}
