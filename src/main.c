#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

int getConnectionInfo(const char *node, struct addrinfo **result) {
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

  return getaddrinfo(node, service, &hints, result);
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
  char sendData[1024];
  snprintf(sendData, sizeof(sendData), "HEAD /%s HTTP/1.1\r\nHost: %s\r\n\r\n", "input/testInput.csv", node);
  printf("sending data\n");
  if(send(sfd, sendData, strlen(sendData), 0) == -1) {
    return -1;
  }

  return 0;
}

int receiveData(int sfd, char recvData[]) {
  size_t bytes_receieved;
  while((bytes_receieved = recv(sfd, recvData, 1024, 0)) > 0) {
    if(bytes_receieved == -1) {
      return -1;
    }

    if(bytes_receieved < 1024) break;
  }
  
  return 0;
}

long parseContentLength(const char* recvData) {
  char* strToFind = "Content-Length: ";
  char* result = strstr(recvData, strToFind);
  result += strlen(strToFind);
  
  char numbers[strlen(result)];
  int index = 0;
  while(*result != '\r') {
    numbers[index] = *result;

    ++index;
    ++result;
  }

  return atol(numbers);
 }

long getContentLength() {
  time_t now;
  time(&now);
  printf("Start getting content length: %s\n", ctime(&now));

  const char *node = "logsquaredn.s3.amazonaws.com";
  struct addrinfo *result;
  if(getConnectionInfo(node, &result) < 0) {
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

  char recvData[1024];
  if(receiveData(sfd, recvData) < 0) {
    printf("error receiving data");
    return -1;
  }
  long contentLength = parseContentLength(recvData);
  printf("content length: %d\n", parseContentLength(recvData));

  close(sfd);

  time(&now);
  printf("Finished getting content length: %s\n", ctime(&now));

  return contentLength;
}

void* go(void* var1) {
  int* threadId = (int*) var1;
  printf("threadId: %d\n", threadId);
}

int main(int argc, const char *argv[]) {
  long contentLength = getContentLength();

  int nProcs = get_nprocs();
  printf("number of processors: %d\n", nProcs);
  int chunkSize = contentLength / nProcs;
  for(int i = 0; i < nProcs; ++i) {
    pthread_t thread;
    pthread_create(&thread, NULL, go, (void *) &thread);
  }

  pthread_exit(NULL);

  return 0;
}
