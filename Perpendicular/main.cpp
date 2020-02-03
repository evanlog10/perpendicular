#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int createConnection(struct addrinfo *result) {
    int sfd = -1;
       struct addrinfo *rp;
       for(rp = result; rp != NULL; rp = rp->ai_next) {
         sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

         if(sfd == -1) {
           continue;
         }

         if(connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
           printf("connected successfully\n");
           break;
         }
       }

       if(rp == NULL) {
         printf("error connecting\n");
         return -1;
       }
    
    return sfd;
}

int sendData(const char *node, int sfd) {
    char send_data[1024];
    snprintf(send_data, sizeof(send_data), "HEAD /%s HTTP/1.1\r\nHost: %s\r\n\r\n", "input/test.csv", node);
    printf("%s\n", send_data);
    if(send(sfd, send_data, strlen(send_data), 0) == -1) {
        printf("error sending");
        return -1;
    }
    
    return 0;
}

void* go(void *vargp) {
    time_t now;
    time(&now);
    int *threadId = (int *) vargp;
    printf("Thread %d Start: %s\n", *threadId, ctime(&now));
    
    const char *node = "logsquaredx.s3.amazonaws.com";
    const char *service = "80";

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    struct addrinfo *result;
    int s = getaddrinfo(node, service, &hints, &result);
    if(s != 0) {
      printf("getaddrinfo failed: %s\n", gai_strerror(s));
      return NULL;
    }
    
    int sfd = createConnection(result);
    freeaddrinfo(result);
    if(sfd == -1) {
        return NULL;
    }
    
    if(sendData(node, sfd) == -1) {
        return NULL;
    }
    
    size_t bytes_receieved;
    char recv_data[1024];
    while((bytes_receieved = recv(sfd, recv_data, 1024, 0)) > 0) {
        if(bytes_receieved == -1) {
            printf("error reading");
            return NULL;
        }
        
        printf("bytes received: %zu\n", bytes_receieved);
        printf("data received: %s\n", recv_data);
        
        if(bytes_receieved < 1024) break;
    }
    
    close(sfd);
    
    time(&now);
    printf("Thread %d End: %s\n", *threadId, ctime(&now));
    
    return NULL;
}

int main(int argc, const char *argv[]) {
    pthread_t threadId1;
    pthread_create(&threadId1, NULL, go, (void *) &threadId1);
    //pthread_t threadId2;
    //pthread_create(&threadId2, NULL, go, (void *) &threadId2);
    
    pthread_exit(NULL);
    return 0;
}
