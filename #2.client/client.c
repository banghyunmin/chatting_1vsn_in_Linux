#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 1024
 
int main(int argc, char **argv)
{
  int socketfd;
  struct sockaddr_in sockaddr;
  int socklen;
  fd_set readfds, oldfds;
  int maxfd;
  int fd_num;
  char buf[MAXLINE];
  int readn, writen;
  int flag = 0;
 
  if( (socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
  {
      return 1;
  }
 
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = inet_addr("220.149.128.100");
  sockaddr.sin_port = htons(4090);
 
  socklen = sizeof(sockaddr);
  
  if(connect(socketfd, (struct sockaddr *)&sockaddr, socklen) == -1)
  {        perror("connect error");
         return 1;
  }
  
  // readfd set 0 
  FD_ZERO(&readfds);
  // readfds's [socketfd] index bit set 1
  FD_SET(socketfd, &readfds);
  // readfds's [0] index bit set 1
  FD_SET(0, &readfds);
 
  maxfd = socketfd;
  oldfds = readfds;
 
  while(1)
  {
      readfds = oldfds;
      fd_num = select(maxfd + 1, &readfds, NULL, NULL, NULL);
      if(FD_ISSET(socketfd, &readfds))
      {
		  // RECV FROM SERVER
          memset(buf, 0x00, MAXLINE);
          readn = read(socketfd, buf, MAXLINE);
          if(readn <= 0)
          {
              return 1;
          }
		  // WRITE TO CONSOLE
          writen = write(1, buf, readn);
          if(writen != readn)
          {
              return 1;
          }
		  if(1)
		  {
			  // LOGIN AUTH
			  if(0 == strcmp(buf, "ID :"))
			  {
			  }
			  else if(0 == strcmp(buf, "PW :"))
			  {
			  }
			  else if(0 == strcmp(buf, "Log-in fail: incorrect password...\n"))
			  {
			  }
			  else if(0 == strcmp(buf, "Log-in fail: incorrect id...\n"))
			  {
			  }
			  // CHAT MODE
			  else
			  {
				  if(0 == flag)
				  {
					  printf("====================Chating Room====================\n");
				  }
				  if(0 == flag%2)
				  {
				  	fprintf(stderr, "%s", ">>");
				  }
				  flag++;
				  if(0 == flag) flag = 2;
			  }
		  }

      }
      if(FD_ISSET(0, &readfds))   
      {
		  // RECV FROM CONSOLE
          memset(buf, 0x00, MAXLINE);
          readn = read(0, buf, MAXLINE);
		  if(flag)
		  {
			  fprintf(stderr, "%s", ">>");
		  }
          if(readn <= 0)
          {
              return 1;
          }
		  // WRITE TO SERVER
          writen = write(socketfd, buf, readn);
          if(readn != writen)
          {
              return 1;
          }
      }
  }
}
