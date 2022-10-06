#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define STDIN 0
#define True 1
int main() {
  int ret = 0;
  struct timeval tv;
  fd_set readfds;
  char buffer[2000];
  size_t buffersize = 2000;
  size_t characters;
  char *b = buffer;

  //tv.tv_sec = 2;
  //tv.tv_usec = 0;

  FD_ZERO(&readfds);
  FD_SET(STDIN, &readfds);

  //ret = select(STDIN+1, &readfds, NULL, NULL, &tv);
  while(True){
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    ret = select(STDIN+1, &readfds, NULL, NULL, &tv);
    if(ret == 0) {
      printf("\nThe select return value is: %d", ret);
      printf("\nThe value of both timeval structures are: %d(sec) %d(usec)", tv.tv_sec, tv.tv_usec);
      if(FD_ISSET(STDIN, &readfds))
        printf("\nreadfd is in the set\n");
      else
        printf("\nreadfd is not in the set\n");
      break;
    }
    characters = getline(&b, &buffersize, stdin);
    printf("\nYou typed ");
    for(unsigned i = 0; i < sizeof buffer; i++) {
      if(buffer[i] == '\n')
        break;
      printf("%c", buffer[i]);
    }
    printf("\nThe select return value is: %d", ret);
    printf("\nThe value of both timeval structures are: %d(sec) %d(usec)", tv.tv_sec, tv.tv_usec);   
    if(FD_ISSET(STDIN, &readfds)) 
      printf("\nreadfd is in the set\n");
    else
      printf("\nreadfd is not in the set\n");
    fflush(stdin); 
    //FD_ZERO(&readfds); 
  }//while


  return 0;
}
