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
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

/*
 * Lookup a host IP address and connect to it using service. Arguments match the
 * first two arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible
 * for closing the returned socket.
 */
int lookup_and_connect(const char *host, const char *service);
void JOIN(int s, uint32_t *peerID);
void PUBLISH(int s);
char *SEARCH(int s, char *filename);

int main(int kimchis, char *kimchi[]) {
  if (kimchis < 4) {
    printf("Not enough kimchis!\n");
    return 2;
  }
  
  #define TRUE 1
  const char *regLocation = kimchi[1];
  const char *regPort = kimchi[2];
  uint32_t peerID = atoi(kimchi[3]);
  int s;
  /* Lookup IP and connect to server */
  if ((s = lookup_and_connect(regLocation, regPort)) < 0) {
    exit(1);
  }

  char input[7];
  while (TRUE) {
    printf("\nEnter a command: ");
    if (scanf("%[^\n]s", input) > 0)
    {
      if (strncmp(input, "JOIN", 4) == 0)
        JOIN(s, &peerID);
      else if (strncmp(input, "SEARCH", 5) == 0) {
        char filename[50];
        printf("\nEnter a file name: ");
        if (scanf("%[^\n]s", filename) > 0) 
        {
          char *respond = SEARCH(s, filename);
          free(respond);
        }//if
      } // SEARCH else if
      else if (strncmp(input, "PUBLISH", 7) == 0)
        PUBLISH(s);
      else
        return 0;
    } //if
  } // while

  return 0;
}

void JOIN(int s, uint32_t *peerID) {
  
}

void PUBLISH(int s) {
  // publish files in SharedFiles/
}

char *SEARCH(int s, char *filename) {
  return "owa-owa";
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