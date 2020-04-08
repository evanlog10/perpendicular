#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int getConnectionInfo(const char* node, struct addrinfo *result) {
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

  return getaddrinfo(node, service, &hints, &result);
}

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
    return -1;
  }

  return sfd;
}

int sendData(const char *node, int sfd) {
  char send_data[1024];
  snprintf(send_data, sizeof(send_data), "HEAD /%s HTTP/1.1\r\nHost: %s\r\n\r\n", "input/testInput.csv", node);
  printf("sending data\n")
  if(send(sfd, send_data, strlen(send_data), 0) == -1) {
    return -1;
  }

  return 0;
}

u_long getContentSize() {
  time_t now;
  time(&now);
  printf("Start getting content size: %s\n", ctime(&now));

  const char *node = "logsquaredn.s3.amazonaws.com";
  struct addrinfo *result;
  int status = getConnectionInfo(node, result);
  if(status != 0) {
    printf("error getting connection info\n");
    return -1;
  }

  int sfd = createConnection(result);
  freeaddrinfo(result);
  if(sfd < 0) {
    printf("error creating connection\n");
    return -1;
  }

  if(sendData(node, sfd) == -1) {
    printf("error sending data\n");
    return -1;
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
  printf("Finished getting content size: %s\n", ctime(&now));

  return NULL;
}

// void* go(void *vargp) {
//   int *threadId = (int *) vargp;
// }

int main(int argc, const char *argv[]) {
  // pthread_t threadId1;
  // pthread_create(&threadId1, NULL, go, (void *) &threadId1);
  // //pthread_t threadId2;
  //pthread_create(&threadId2, NULL, go, (void *) &threadId2);

  //pthread_exit(NULL);
  return 0;
}
