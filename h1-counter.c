/**************************************************************
 **************** Rio Sakao  **********************************
 **************** Nissa Lieu **********************************
 **************** FALL 2022  **********************************
 **************** EECE 446   **********************************
 **************************************************************/

/* This code is an updated version of the sample code from "Computer Networks: A
 * Systems Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some
 * code comes from man pages, mostly getaddrinfo(3).*/
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * Lookup a host IP address and connect to it using service. Arguments match the
 * first two arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible
 * for closing the returned socket.
 */
int lookup_and_connect(const char *host, const char *service);

int sendall(int s, char *msg, int len);
int recvall(int s, char *buff, int *len_received);
void TagCount(char *buff, int *h1_count);
int chunk_size;

int main(int kimchis, char *kimchi[]) {
  if(kimchis < 2)
  {
    printf("Not enough kimchis!\n");
    return 2;
  }          

  int s;
  chunk_size = atoi(kimchi[1]);
  const char *host = "www.ecst.csuchico.edu";
  const char *port = "80";
  char *msg = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";
  //char *buff = malloc(chunk_size);
  char buff[4485];
  if (buff == NULL)
    return -1;
  int len, len_received = 0, bytes_sent , bytes_received = 0, h1_count = 0;

  /* Lookup IP and connect to server */
  if ((s = lookup_and_connect(host, port)) < 0) {
    exit(1);
  }
  // sends "GET /~kkredo/file.html HTTP/1.0\r\n\r\n" to the server
  len = strlen(msg);
  bytes_sent = sendall(s, msg, len);

  if (bytes_sent == -1) { // sendall() error
    perror("sendall");
    printf("We only sent %d bytes because of the error!\n", len);
  } else { // sendall() success
    bytes_received = recvall(s, buff, &len_received);
    if (bytes_received == -1) { // recvall() error
      perror("recvall");
      printf("We only received %d bytes because of the error!\n", len_received);
    }
  }
  TagCount(buff, &h1_count);
  printf("Number of <h1> tags: %d", h1_count);
  printf("\n");
  printf("Number of bytes: %d", len_received);
  printf("\n");

  close(s);

  return 0;
}

int sendall(int s, char *msg, int len) {
  int total = 0;       // how many bytes we've sent
  int bytesleft = len; // how many we have left to send
  int n;

  while (total < len) {
    n = send(s, msg + total, bytesleft, 0);
    if (n == -1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }

  len = total; // return number actually sent here

  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

int recvall(int s, char *buff, int *len_received) {
  int n = 0, total = 0;
  char a_buff[chunk_size];
  n = recv(s, buff, chunk_size, 0);

  while (n != 0 && n != -1) {
    (*len_received) += n;
    total += n;
    if(total < 4485)  
      n = recv(s, &buff[total], chunk_size, 0);
    else 
      n = recv(s, a_buff, chunk_size, 0);
  }//while
  

  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

void TagCount(char *buff, int *h1_count) {
  const int h1tag_bytes = 4;
  int buff_itr = 0;
  int i, k = 0;
  int cs_index = 1;
  char a_buff[4] = { 0 };

  while(buff_itr < 4485 - h1tag_bytes) {
    a_buff[0] = buff[buff_itr];
    a_buff[1] = buff[buff_itr + 1];
    a_buff[2] = buff[buff_itr + 2];
    a_buff[3] = buff[buff_itr + 3];
      for (i = k; i < chunk_size * cs_index - h1tag_bytes; i++){
        if (strncmp(a_buff, "<h1>", h1tag_bytes) == 0) {
          (*h1_count)++;
        }//if  
        /**
        printf("%c%c%c%c", a_buff[0], a_buff[1], a_buff[2], a_buff[3]);
        printf("\n");
        **/
        a_buff[0] = buff[i+1];
        a_buff[1] = buff[i+2];
        a_buff[2] = buff[i+3];
        a_buff[3] = buff[i+4];
      }//for
      i += h1tag_bytes;
      k = i;
      cs_index += 1;
      buff_itr = k;
  }//while
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

