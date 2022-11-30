/**************************************************************
 **************** Rio Sakao  **********************************
 **************** Nissa Lieu **********************************
 **************** FALL 2022  **********************************
 **************** EECE 446   **********************************
 **************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
struct peer_entry {
  uint32_t id; // ID of peer
  int s; // Socket descriptor for connection to peer
  char files[10][255]; // Files published by peer
  struct sockaddr_in address; // Contains IP address and port number
}peer[5];

int bind_socket(const char *port);
void JOIN(char *buf, int i); 
void PUBLISH(char *buf, int i);
void SEARCH(char *buf, int i);
void set_ip_and_port(int i);
void send_ip_and_port(int i, struct peer_entry peer);
void file_not_found(int i);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Not enough arguments...>_<\n");
    return 2;
  }
  
  fd_set master;    // master file descriptor list
  fd_set read_fds;  // temp file descriptor list for select()
  int fdmax;        // maximum file descriptor number
  int newfd;        // newly accept()ed socket descriptor
  int listener;
  struct sockaddr_storage remoteaddr; // client address
  socklen_t addrlen;
  int nbytes;
  int i;
  

  FD_ZERO(&master);    // clear the master and temp sets
  FD_ZERO(&read_fds);
  
  listener = bind_socket(argv[1]);
  if (listen(listener, 10) == -1) {
    perror("listen");
    exit(3);
  }//if 
 
  // add the listener to the master set
  FD_SET(listener, &master);

  // keep track of the biggest file descriptor
  fdmax = listener; // so far, it's this one 

  // main loop
  for(;;) {
    read_fds = master; // copy it
    if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(4);
    }//if

    // run through the existing connections looking for data to read
    for(i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, &read_fds)) { // we got one!!
        if (i == listener) {
          // handle new connections
          addrlen = sizeof remoteaddr;
          newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);
          if (newfd == -1) {
            perror("accept");
          } else {
            FD_SET(newfd, &master); // add to master set
            if (newfd > fdmax) {    // keep track of the max
              fdmax = newfd;
            }//if
          }//else
          } else {
            // handle data from a client
            char buf[1300];
            if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
              close(i);
              FD_CLR(i, &master); // remove from master set
            } else {
              // we got some data from a client
              if(buf[0] == 0) //JOIN REQUEST RECEIVED
                JOIN(buf, i);
              else if(buf[0] == 1) //PUBLISH REQUEST RECEIVED
                PUBLISH(buf, i);
              else if(buf[0] == 2) //SEARCH REQUEST RECEIVED
                SEARCH(buf, i);
            }
          }
      }
    }
  }

  return 0;
}

void JOIN(char *buf, int i) {
  const int max_peer = 5;
  for(int k = 0; k < max_peer; k++) {
    if(peer[k].s == '\0') {//add new peer
      uint32_t net_id;

      peer[k].s = i;
      memcpy(&net_id, &buf[1], sizeof net_id);
      peer[k].id = ntohl(net_id);

      printf("TEST] JOIN %u\n", peer[k].id);
      break;
    }//if
  }//for
}//JOIN()

void PUBLISH(char *buf, int i) {
  const int max_peer = 5;
  uint32_t target_peer;
  uint32_t rv=0, temp=0;
  memcpy(&temp, &buf[1], sizeof(long));
  rv = ntohl(temp);

  for(int k = 0; k < max_peer; k++) {
    if(peer[k].s == i){
      target_peer = k;
      const int max_files = 255;
      int m = 0,  s = 5;
      while(m != rv){
        char a_file[max_files];
        int q = 0, length = 0, flag = 0;
        while(buf[s] != '\0'){
          a_file[q] = buf[s];
          q++;
          s++;
          length++;
          flag++;
        }//while
        s += 1; //skip null
        if(flag != 0){
          memcpy(peer[k].files[m], a_file, length);
          m++;
        }//if
      }//while
    }//if
  }//outer for
  printf("TEST] PUBLISH %u", rv);
  for(int k=0; k < rv; k++){
    printf(" %s", peer[target_peer].files[k]);
  }//for
  printf("\n");
}//PUBLISH()

void SEARCH(char *buf, int i){
  const int max_peer = 5;
  const int max_file = 10;
  int flag = 0;
  char filename[255];
  //int file_len = sizeof(buf)+1;
  int file_len = 255;
  memcpy(&filename, &buf[1], file_len);

  for(int k = 0; k < max_peer; k++){
    if(flag != 0) break;
    for(int m = 0; m < max_file; m++){
      if(strncmp(peer[k].files[m], filename, file_len) == 0){
        set_ip_and_port(peer[k].s);
        printf("TEST] SEARCH %s ", filename);
        send_ip_and_port(i, peer[k]);//FILE FOUND
        flag++;
      }//if
    }//for
  }//outer for

  if(flag==0) {//FILE NOT FOUND
    printf("TEST] SEARCH %s ", filename);
    file_not_found(i);
  }//if

  printf("\n");

}//SEACH


void file_not_found(int i) {
  char response[10] = {'\0'};
  int bytes = 0;
  if ((bytes = send(i, response, sizeof response, 0) < 0)) {
    perror("send()");
  }//if
  printf("0 0.0.0.0:0");
}//file_not_found()

void send_ip_and_port(int i, struct peer_entry peer){
  char response[10];
  int bytes = 0;
  uint32_t net_id;
  uint32_t net_ip;
  uint16_t net_port;
  uint16_t host_port;

  net_id = htonl(peer.id);
  net_ip = peer.address.sin_addr.s_addr;
  net_port = peer.address.sin_port;
  memcpy(&response[0], &net_id, sizeof(net_id));
  memcpy(&response[4], &net_ip, sizeof(net_ip)); 
  memcpy(&response[8], &net_port, sizeof(net_port));
  
  if ((bytes = send(i, response, sizeof response, 0) < 0)) {
    perror("send()");
  }//if

  host_port = ntohs(net_port);
  
  struct in_addr addr;
  addr.s_addr = net_ip;
  char *dot_ip = inet_ntoa(addr);

  printf("%u %s:%hu", peer.id, dot_ip, host_port);  
}// send_ip_and_port 

void set_ip_and_port(int i) {
  const int max_peer = 5;
  for(int k = 0; k < max_peer; k++) {
    if(peer[k].s == i){
      socklen_t len = sizeof(peer[k].address);
      int ret;
      if((ret = getpeername(i, (struct sockaddr*)&peer[k].address, &len)) < 0) {
        perror("getpeername");
       }//if    
    }//outerif
    }//for
}//set_ip_and_port(peer[k].s)

int bind_socket(const char *port) {
  // get us a socket and bind it
  struct addrinfo hints, *ai, *p;
  int yes=1;        // for setsockopt() SO_REUSEADDR, below
  int rv, listener;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if ((rv = getaddrinfo(NULL, port, &hints, &ai)) != 0) {
    fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
    exit(1);
  }//if

  for(p = ai; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) {
      continue;
    }//if

  // lose the pesky "address already in use" error message
  setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
    close(listener);
    continue;
  }//if
 
  break;

  }//for


  // if we got here, it means we didn't get bound
  if (p == NULL) {
    fprintf(stderr, "selectserver: failed to bind\n");
    exit(2);
  }//if

  freeaddrinfo(ai); // all done with this
  
  return listener;
}//bind_socket()

