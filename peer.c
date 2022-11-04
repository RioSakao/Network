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
int lookup_and_connect(const char *host, const char *service);
void JOIN(int s);
uint32_t PUBLISHUtil(char *request, long *Count);
void PUBLISH(int s);
void SEARCH(int s, char *filename);
void SEARCHPrint(char *response);
void FETCH(int s, char *filename);
void FETCHUtil(char *response, char *filename);
long peerID;

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("Not enough arguments...>_<\n");
    return 2;
  }
  
  #define TRUE 1
  char *end;
  const char *regLocation = argv[1];
  const char *regPort = argv[2];
  peerID = strtoul(argv[3],&end,10);
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
        char filename[50];
        printf("\nEnter a file name: ");
        if (scanf("\n%s", filename) > 0) {
          SEARCH(s, filename);
        }//scanf
        else 
          perror("\nSEARCH: Scanf() error");
      } //SEARCH else if
      else if (strncmp(input, "PUBLISH", 7) == 0)
        PUBLISH(s);
      else if (strncmp(input, "FETCH", 5) == 0) {
        char filename[50];
        printf("\nEnter a file name: ");
        if (scanf("\n%s", filename) > 0) { 
          FETCH(s, filename);
        }//scanf
      }//else if
      else if (strncmp(input, "EXIT", 4) == 0)
        break;
    } //outer scanf
  } //while
  return 0;
}

void JOIN(int s) {
  char request[5]; int bytes_sent=0; const uint32_t len = 5;
  request[0] = 0;//Action

  uint32_t Peer_net = htonl(peerID);
  memcpy(&request[1], &Peer_net, sizeof Peer_net);

  bytes_sent = send(s, request, len, 0);
  if(bytes_sent == -1) {
    perror("\nsend() error");
  }
}

void FETCHUtil(char *response, char *filename) {
  long NETip;//IPv4
  short NETport; char char_port[2];//Port
  
  memcpy(&NETip, response+sizeof(uint32_t), sizeof(uint32_t));
  memcpy(&NETport, response+sizeof(uint32_t)+sizeof(uint32_t), sizeof(uint16_t));

  uint16_t port_number = ntohs(NETport);

  struct in_addr addr;
  addr.s_addr = NETip;
  char *dot_ip = inet_ntoa(addr);

  sprintf(char_port,"%hu", port_number);
  
  const char *location = dot_ip;
  const char *port = char_port;

  int sp = lookup_and_connect(location, port);
  
  if(sp < 0){
    perror("\nlookup_and_connect()");
  }else{
    uint32_t bytes_received = 0 ,bytes_sent = 0, file_len = strlen(filename) + 1;
    char fetch_request[1+file_len];
    char fetch_response[99999];
    fetch_request[0] = 3;
    memcpy(&fetch_request[1], filename, file_len);
    bytes_sent = send(sp, fetch_request, sizeof fetch_request, 0);
    if(bytes_sent == -1)
        perror("\nsend() error");

    uint32_t index = 0, chunk_size = 1000;
    bytes_received = recv(sp, fetch_response, chunk_size, 0);
    while(bytes_received != 0) {
      if(bytes_received == -1){
        perror("\nrecv() error");
        break;
      }//if
      index += bytes_received;
      bytes_received = recv(sp, &fetch_response[index], chunk_size, 0);
    }//While

    if(fetch_response[0] == 0) {//SUCCESS
      uint32_t buff_len = index-1;
      char buff[buff_len];
      memcpy(buff, &fetch_response[1], buff_len);
      
      FILE *file = fopen(filename, "w");
      fwrite(buff, buff_len, 1, file);
      fclose(file);
    } else
        perror("FETCH() error");

  }//outer if
}

void FETCH(int s, char *filename){
  int bytes_sent = 0, bytes_received = 0; uint32_t file_len = 0;
  file_len = strlen(filename)+1;
  const uint32_t request_len = 1 + file_len;
  char request[request_len];
  char response[10];

  request[0] = 2;//SEARCH REQUEST
  memcpy(&request[1], filename, file_len);

  
  bytes_sent = send(s, request, request_len, 0);
  if(bytes_sent == -1) {
    perror("\nsend() error");
  }//VERIFIED

  bytes_received = recv(s, response, sizeof(long)+sizeof(long)+sizeof(short), 0);
  if(bytes_received == -1) {
    perror("\nrecv() error");
  }//VERIFIED
  
  FETCHUtil(response, filename);  
}


uint32_t PUBLISHUtil(char *request, long *Count) {
  /* publish files in SharedFiles/ */
  uint32_t index = 0;
  DIR *dir;
  
  dir = opendir("SharedFiles");
  struct dirent *ptr; char *filename = '\0';
  
  while((ptr = readdir(dir))) {
    filename = ptr->d_name;
    
    if(strncmp(filename, ".", 1) == 0) {
      continue;
    } else if (strncmp(filename, "..", 2) == 0){
      continue;
    }

    uint32_t file_len = strlen(filename)+1;
    (*Count)++;
    memcpy(&request[index], filename, file_len);
    index += file_len;//end index
  }//while
  closedir(dir);
  return index;
}

void PUBLISH(int s) {
  long Count = 0; uint32_t index = 0,  ret = 0;
  int bytes_sent = 0;
  char request[1200];

  ret = PUBLISHUtil(request, &Count);
  index = ret;
  
  const uint32_t request_len = 5 + index;
  char new_request[request_len];
  uint32_t Count_net = htonl(Count); 
  
  new_request[0] = 1;//Action
  memcpy(&new_request[1], &Count_net, 4);
  memcpy(&new_request[5], &request, index); 

  bytes_sent = send(s, new_request, sizeof new_request, 0);
  if(bytes_sent == -1) {
    perror("\nsend() error");
  }
    
}
void SEARCHPrint(char *response) {
  long NETpeer,NETip;
  short NETport;

  memcpy(&NETpeer, response, sizeof(uint32_t));
  memcpy(&NETip, response+sizeof(uint32_t), sizeof(uint32_t));
  memcpy(&NETport, response+sizeof(uint32_t)+sizeof(uint32_t), sizeof(uint16_t));
  
  uint32_t peer_id = ntohl(NETpeer);
  uint16_t port_number = ntohs(NETport);
  
  struct in_addr addr;
  addr.s_addr = NETip;
  char *dot_ip = inet_ntoa(addr);

  if(peer_id == 0){
    printf("\nFile not indexed by registry");
  }else{
    printf("\nFile found at");
    printf("\n Peer %u", peer_id);
    printf("\n %s:", dot_ip);
    printf("%d", port_number);
  }//else

}

void SEARCH(int s, char *filename) {
  int bytes_sent = 0, bytes_received = 0; uint32_t file_len = 0;
  file_len = strlen(filename)+1; 
  const uint32_t request_len = 1 + file_len;
  char request[request_len];
  char response[10];
  
  request[0] = 2;
  memcpy(&request[1], filename, file_len);
  
  bytes_sent = send(s, request, request_len, 0);
  if(bytes_sent == -1) {
    perror("\nsend() error");
  }
  
  bytes_received = recv(s, response, sizeof(long)+sizeof(long)+sizeof(short), 0);
  if(bytes_received == -1) {
    perror("\nrecv() error");
  }

  SEARCHPrint(response);
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
