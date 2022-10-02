/**************************************************************
 **************** Rio Sakao  **********************************
 **************** Nissa Lieu **********************************
 **************** FALL 2022  **********************************
 **************** EECE 446   **********************************
 **************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

/*
 * Lookup a host IP address and connect to it using service. Arguments match the
 * first two arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible
 * for closing the returned socket.
 */
#define u uint32_t
int lookup_and_connect(const char *host, const char *service);
void JOIN(int s, u *peerID);
void PUBLISHUtil(char *request, u *Count);
void PUBLISH(int s);
void SEARCH(int s, char *filename, char *response);

int main(int kimchis, char *kimchi[]) {
  if (kimchis < 4) {
    printf("Not enough kimchis!\n");
    return 2;
  }
  
  #define TRUE 1
  char *end;
  const char *regLocation = kimchi[1];
  const char *regPort = kimchi[2];
  u peerID = strtoul(kimchi[3],&end,2);
  int s, Join = 0;
  /* Lookup IP and connect to server */
  if ((s = lookup_and_connect(regLocation, regPort)) < 0) {
    exit(1);
  }

  char input[7];
  while (TRUE) {
    printf("\nEnter a command: ");
    if (scanf("%s", input) > 0) {
      if (strncmp(input, "JOIN", 4) == 0) {
        if(Join < 1) {
          JOIN(s, &peerID);
          Join++;
        }//if
        else 
          perror("\nMultiple JOIN requests are prohibited");
      } else if (strncmp(input, "SEARCH", 5) == 0) {
        char filename[50], response[10];
        printf("\nEnter a file name: ");
        if (scanf("\n%s", filename) > 0) {
          SEARCH(s, filename, response);
          printf("\nFile found at");
          printf("\n Peer %.4s", response);
          printf("\n %.4d:", response[4]);
          printf("%.2d", response[8]);
        }//if
        else 
          perror("\nSEARCH: Scanf() error");
      } //SEARCH else if
      else if (strncmp(input, "PUBLISH", 7) == 0)
        PUBLISH(s);
      else if (strncmp(input, "EXIT", 4) == 0)
        return 0;
    } //if
  } //while
  return 0;
}

void JOIN(int s, u *peerID) {
  char request[5]; int bytes_sent=0; const int len = 5;
  request[0] = 0; //Action = 0
  u Peer_net = htonl((uint32_t)(uintptr_t)peerID);

  memcpy(&request[1], &Peer_net, sizeof Peer_net);
  bytes_sent = send(s, request, len, 0);
  if(bytes_sent == -1) {
    perror("\nsend() error");
  }
}
void PUBLISHUtil(char *request, unsigned *Count) {
  unsigned f_bytes = 0, index = 5;
  DIR *dir;
  
  dir = opendir("SharedFiles");
  struct dirent *ptr; char *filename;
  
  while((ptr = readdir(dir)) != NULL) {
    filename = ptr->d_name;
    f_bytes += sizeof(&filename);
    (*Count)++;
    memcpy(&request[index], filename, f_bytes);
    index += f_bytes;
  }
  closedir(dir);
}

void PUBLISH(int s) {
  /* publish files in SharedFiles/ */
  u Count = 0; int bytes_sent = 0;
  const unsigned len = 1200;
  char request[1200];
  request[0] = 1; //Action

  PUBLISHUtil(request, &Count);
  
  u Count_net = htonl(Count); 
  memcpy(&request[1], &Count_net, sizeof Count_net);
  bytes_sent = send(s, request, len, 0);
  if(bytes_sent == -1) {
    perror("\nsend() error");
  }
    
}

void SEARCH(int s, char *filename, char *response) {
  int bytes_sent = 0, bytes_received = 0; int len = sizeof(&filename);
  char request[1+len]; request[0] = 2; //Action
  
  memcpy(&request[1], &filename, len);
  
  bytes_sent = send(s, request, len, 0);
  if(bytes_sent == -1) {
    perror("\nsend() error");
  }
  
  bytes_received = recv(s, &response, 10, 0);
  if(bytes_received == -1) {
    perror("\nrecv() error");
  }
}

int lookup_and_connect(const char *host, const char *service) {
  struct addrinfo hints;
  struct addrinfo *rp, *result;
  int s;

  /* Translate host name into peer's IP address */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  if ((s = getaddrinfo(host, service, &hints, &result)) != 0) {
    fprintf(stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* Iterate through the address list and try to connect */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
      continue;
    }

    if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1) {
      break;
    }

    close(s);
  }
  if (rp == NULL) {
    perror("stream-talk-client: connect");
    return -1;
  }
  freeaddrinfo(result);

  return s;
}