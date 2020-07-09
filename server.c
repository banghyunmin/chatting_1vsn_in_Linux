#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERV_IP "220.149.128.100"
#define SERV_PORT 4090
#define BACKLOG 10

#define INIT_MSG "========================\nHello! I'm P2P File Sharing Server...\nPlease, LOG_IN!\n==========================\n"
#define USER1_ID "user1"
#define USER1_PW "passwd1"
#define USER2_ID "user2"
#define USER2_PW "passwd2"

int main(void) {
	/* listen on sock_fd, new connection on new_fd */
	int sockfd, new_fd;

	/* my address information, address where I run this program */
	struct sockaddr_in my_addr;

	/* remote address information */
	struct sockaddr_in their_addr;
	unsigned int sin_size;

	/* buffer */
	int rcv_byte;
	char buf[512];

	char id[20];
	char pw[20];

	char msg[512];

	int val = 1;

	/* process id */
	pid_t pid;


	/* socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) {
		perror("Server-socket() lol!");
		exit(1);
	}
	else printf("Server-socket() sockfd is OK...\n");

	/* host byte order */
	my_addr.sin_family = AF_INET;

	/* short, network byte order */
	my_addr.sin_port = htons(SERV_PORT);
	// my_addr.sin_addr.s_addr = inet_addr(SREV_IP);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	/* zero the rest of the struct */
	memset(&(my_addr.sin_zero), 0, 8);

	/* to prevent 'Address already in use...' */
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val)) < 0) {
		perror("setsockopt");
		close(sockfd);
		return -1;
	}

	/* bind */
	if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("Server-bind() error lol!");
		exit(1);
	}
	else printf("Server-bind() is OK...\n");

	/* listen */
	if(listen(sockfd, BACKLOG) == -1) {
		perror("listen() error lol!");
		exit(1);
	}
	else printf("listen() is OK...\n");

	while(1) {
		/* ...other codes to read the received data... */
		sin_size = sizeof(struct sockaddr_in);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if(new_fd == -1) {
			perror("accept() error lol!");
			exit(1);
		}
		else printf("accept() is OK...\n\n");

		/* fork process */
		pid = fork();

		/* Child Process */
		if(pid == 0) {	
			/* send INIT_MSG */
			send(new_fd, INIT_MSG, strlen(INIT_MSG) + 1, 0);

			/* receive data */
			rcv_byte = recv(new_fd, id, sizeof(id), 0);
			rcv_byte = recv(new_fd, pw, sizeof(pw), 0);
			/* print data */
			printf("=========================\n");
			printf("User Information\n");
			printf("ID: %s, PW: %s\n", id, pw);
			printf("=========================\n");
			/* check login */
			/* check user1 */
			if(!strcmp(USER1_ID, id)) {
				if(strcmp(USER1_PW, pw)) {
					printf("Log-in fail: Incorrect password...\n");
					strcpy(buf, "Log-in fail: incorrect password...\n");
				}
				else {
					printf("Log-in Success!! [%s]*^^*\n", id);
					strcpy(buf, "Log-in Sucess!! [");
					strcat(buf, id);
					strcat(buf, "]*^^*\n");
				}
			}
			/* check user2 */
			else if(!strcmp(USER2_ID, id)) {
				if(strcmp(USER2_PW, pw)) {
					printf("Log-in fail: Incorrect password...\n");
					strcpy(buf, "Log-in fail: incorrect password...\n");
				}
				else {
					printf("Log-in Success!! [%s]*^^*\n", id);
					strcpy(buf, "Log-in Sucess!! [");
					strcat(buf, id);
					strcat(buf, "]*^^*\n");
				}
			}
			else {
				printf("Log-in fail: Incorrect id...\n");
				strcpy(buf, "Log-in fail: Incorrect id...\n");
			}		

			/* send data */
			send(new_fd, buf, strlen(buf) + 1, 0);

			/* close fd and process */
			close(new_fd);
			close(sockfd);
			exit(0);
		}
		/* Parent Process */
		else if(pid > 0) {
			close(new_fd);
		}
		/* Error */
		else {
			fprintf(stderr, "can't fork, error %d\n", errno);
			exit(1);
		}
	} 

	return 0;
}
