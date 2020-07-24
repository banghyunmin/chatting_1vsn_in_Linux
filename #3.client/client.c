#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <dirent.h>
#include <netdb.h>
#include <errno.h>

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

  /* peer Variable */
  struct sockaddr_in p_addr, p_clientaddr;
  int i;
  socklen_t p_addrlen, p_clilen;
  int p_sockfd, p_new_fd;
  int nSockOpt;
 
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
			  // P2P FILE MODE
			  else if(0 == strcmp(buf, "[FILE]\n"))
			  {
				  char recvAddr[20];
				  recv(socketfd, recvAddr, 20, 0);
				  printf("Wait...\n");
				  sleep(3);
				  /* peer Variable */
				  int p_sockfd;
				  struct sockaddr_in p_sockaddr;
				  int p_socklen;

				  p_sockfd = socket(AF_INET, SOCK_STREAM, 0);
				  p_sockaddr.sin_family = AF_INET;
				  p_sockaddr.sin_addr.s_addr = inet_addr(recvAddr);
				  p_sockaddr.sin_port = htons(4091);
				  p_socklen = sizeof(p_sockaddr);
				  if(-1 == connect(p_sockfd, (struct sockaddr *)&p_sockaddr, p_socklen))
				  {
					  perror("connect error ");
				  }
				  // connect

				  // get file list
				  char buf[1024];
				  DIR *d;
				  struct dirent *dir;
				  d = opendir(".");
				  if(d)
				  {
					  while((dir = readdir(d)) != NULL)
					  {
						  write(p_sockfd, dir->d_name, strlen(dir->d_name));
						  sleep(1);
					  }
					  sprintf(buf, "%s", "end");
					  write(p_sockfd, buf, strlen(buf));
				  }
				  // get file name
				  memset(buf, 0x00, 1024);
				  printf("wait recv file name...\n");
				  recv(p_sockfd, buf, 1024, 0);
				  printf("%s\n", buf);

				  // find dest file
				  FILE *source;
				  source = fopen(buf, "r");
				  if(source == NULL)
				  {
					  perror("file open error ");
					  exit(0);
				  }

				  memset(buf, 0x00, 1024);
				  char c = 0;
				  int cn = 0;
				  // send file
				  while((c = fgetc(source)) != EOF)
				  {
					  if(c == '\n')
					  {
						  buf[cn] = '\0';
						  write(p_sockfd, buf, strlen(buf));
						  sleep(1);
						  cn = 0;
						  memset(buf, 0x00, 1024);
					  }
					  else
					  {
						  buf[cn++] = c;
					  }
				  }
				  sprintf(buf, "%s", "end");
				  write(p_sockfd, buf, strlen(buf));
				  fclose(source);
				  printf("Send File : Done!\n");
				  fprintf(stderr, "%s", ">>");
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
	  if (FD_ISSET(0, &readfds))
	  {
		  // RECV FROM CONSOLE
		  memset(buf, 0x00, MAXLINE);
		  readn = read(0, buf, MAXLINE);
		  if (flag)
		  {
			  fprintf(stderr, "%s", ">>");
		  }
		  if (readn <= 0)
		  {
			  return 1;
		  }
		  // WRITE TO SERVER
		  writen = write(socketfd, buf, readn);
		  if (readn != writen)
		  {
			  return 1;
		  }
		  // P2P FILE MODE
		  // check FILE MODE
		  char filestr[20];
		  for (int fn = 0; fn < strlen(buf); fn++)
		  {
			  if (fn < 5)
			  {
				  filestr[fn] = buf[fn];
			  }
			  else if (buf[fn] == ']')
			  {
				  filestr[5] = ']';
				  filestr[6] = '\0';
				  break;
			  }
		  }

		  // command check FILE MODE
		  if (0 == strcmp(filestr, "[FILE]"))
		  {
			  // return username check
			  char start[20];
			  //recv(socketfd, start, 20, 0);
			  //if(0 != strcmp(start, "start"))continue;

			  // start FILE MODE
			  p_sockfd = socket(AF_INET, SOCK_STREAM, 0);
			  if (p_sockfd == -1)
			  {
				  perror("socket error ");
				  exit(0);
			  }

			  p_addrlen = sizeof(p_addr);
			  p_addr.sin_family = AF_INET;
			  p_addr.sin_port = htons(4091);
			  p_addr.sin_addr.s_addr = htonl(INADDR_ANY);

			  nSockOpt = 1;
			  setsockopt(p_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)& nSockOpt, sizeof(nSockOpt));
			  if (-1 == bind(p_sockfd, (struct sockaddr*) & p_addr, p_addrlen))
			  {
				  perror("bind error : ");
				  exit(0);
			  }
			  if (-1 == listen(p_sockfd, 5))
			  {
				  printf("listen error\n");
			  }
			  p_clilen = sizeof(struct sockaddr);
			  printf("Wait...\n");
			  p_new_fd = accept(p_sockfd, (struct sockaddr*) & p_clientaddr, &p_clilen);

			  if (p_new_fd == -1)
			  {
				  printf("accept error\n");
			  }
			  // connect

			  char buf[1024];
			  // get file list
			  int dn = 1;
			  while (1)
			  {
				  memset(buf, 0x00, 1024);
				  read(p_new_fd, buf, 1024);
				  if (0 == strcmp(buf, "end"))
				  {
					  break;
				  }
				  printf("%d. %s\n", dn++, buf);
			  }
			  // send file name
			  char filename[20];
			  printf("input file name : ");
			  scanf("%s", filename);
			  write(p_new_fd, filename, strlen(filename));
			  // wrtie file
			  FILE* file = fopen(filename, "w");
			  if (file == NULL)
			  {
				  perror("file open error ");
			  }

			  while (1)
			  {
				  memset(buf, 0x00, 1024);
				  read(p_new_fd, buf, 1024);
				  if (0 == strcmp(buf, "end"))
				  {
					  break;
				  }
				  fprintf(file, "%s\n", buf);
			  }
			  fclose(file);
			  printf("Recv File : Done!\n");
			  fprintf(stderr, "%s", ">>");
		  }
	  }
  }
}
