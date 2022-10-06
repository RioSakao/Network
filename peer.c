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
void JOIN(int s);
u PUBLISHUtil(char *request, u *Count);
void PUBLISH(int s);
void SEARCH(int s, char *filename, char *response);
u peerID;

int main(int kimchis, char *kimchi[]) {
  if (kimchis < 4) {
    printf("Not enough kimchis!\n");
    return 2;
  }
  
  #define TRUE 1
  char *end;
  const char *regLocation = kimchi[1];
  const char *regPort = kimchi[2];
  peerID = strtoul(kimchi[3],&end,10);
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
          JOIN(s);
          Join++;
        }//if
        else 
          perror("\nMultiple JOIN requests are prohibited");
      } else if (strncmp(input, "SEARCH", 5) == 0) {
        char filename[50], response[50];
        printf("\nEnter a file name: ");
        if (scanf("\n%s", filename) > 0) {
          SEARCH(s, filename, response);
          printf("\nFile found at");
          for(u i = 0; i < sizeof response; i++) {
            if(i != 0 && i < 2){
              goto PeerID;
            } else if (i > 3 && i < 8){
              printf("\n");
              goto IPaddress; 
            } else if (i > 7 && i < 10) {
              goto PortNumber;
            }//if
            
            printf("\n Peer ");
              PeerID:
                printf("%d", response[i]);
              IPaddress:
                printf("%d", response[i]);
              PortNumber:
                printf("%d", response[i]);
          }
          /**
          printf("\n Peer %.4d", response[0]);
          printf("\n %.4d:", response[4]);
          printf("%.2d", response[8]);
          **/
        }//if
        else 
          perror("\nSEARCH: Scanf() error");
      } //SEARCH else if
      else if (strncmp(input, "PUBLISH", 7) == 0)
        PUBLISH(s);
      else if (strncmp(input, "EXIT", 4) == 0)
        break;;
    } //if
  } //while
  return 0;
}

void JOIN(int s) {
  char request[5]; int bytes_sent=0; const u len = (u)5;
  request[0] = (u)0;//Action
  u Peer_net = htonl((uint32_t)peerID);

  memcpy(&request[1], &Peer_net, sizeof Peer_net);
  bytes_sent = send(s, request, len, (u)0);
  if(bytes_sent == -1) {
    perror("\nsend() error");
  }
}
u PUBLISHUtil(char *request, u *Count) {
  /* publish files in SharedFiles/ */
  u index = 0;
  DIR *dir;
  
  dir = opendir("SharedFiles");
  struct dirent *ptr; char *filename = '\0';
  
  while((ptr = readdir(dir))) {
    filename = ptr->d_name;
    
    if(strncmp(filename, ".", 1) == 0) {
      goto end;
    } else if (strncmp(filename, "..", 2) == 0){
        goto end; 
    }

    u file_len = strlen(filename)+1;
    (*Count)++;
    memcpy(&request[index], filename, file_len);
    index += file_len;//end index
    end:
  }//while
  closedir(dir);
  return index;
}

void PUBLISH(int s) {
  u Count = 0; int bytes_sent = 0; u index = 0; u ret = 0;
  char request[1200];

  ret = PUBLISHUtil(request, &Count);
  index = ret;

  char new_request[5+index];
  u Count_net = htonl((uint32_t)Count); 
  
  new_request[0] = (u)1;//Action
  memcpy(&new_request[1], &Count_net, 4);
  memcpy(&new_request[5], &request, index); 

  bytes_sent = send(s, new_request, sizeof new_request, 0);
  if(bytes_sent == -1) {
    perror("\nsend() error");
  }
    
}

void SEARCH(int s, char *filename, char *response) {
  int bytes_sent = 0, bytes_received = 0; u len = 0;
  
  for(u i = 0; i < sizeof filename; i++) {
     if(filename[i] == '\0'){
       len = i + 1;
       break;
     }
  }

  char request[1+len]; //request[0] = '2'; //Action
  
  request[0] = (u)2;
  memcpy(&request[1], &filename, len);
  
  bytes_sent = send(s, request, 1+len, 0);
  if(bytes_sent == -1) {
    perror("\nsend() error");
  }
  
  bytes_received = recv(s, &response, 50, 0);
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
