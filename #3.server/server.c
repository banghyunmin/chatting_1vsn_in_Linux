#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
 
#include <string.h>
#include <stdio.h>
 
#define PORT_NUM 4090
#define EPOLL_SIZE 20
#define MAXLINE 1024

#define USER1_ID "user1"
#define USER1_PW "passwd1"
#define USER2_ID "user2"
#define USER2_PW "passwd2"
 
// user data struct
struct udata
{
    int fd;
	char id[20];
	char pw[20];
};
struct ipAddr
{
	char ip[20];
	char id[20];
};

int user_fds[1024];
struct ipAddr ipA[1024];
 
void send_msg(struct epoll_event ev, char *msg, int flag);
 
int main(int argc, char **argv)
{
	/* Variable */
    struct sockaddr_in addr, clientaddr;
    struct epoll_event ev, *events;
    struct udata *user_data;
    
    int listenfd;
    int clientfd;
    int i;
    socklen_t addrlen, clilen;
    int readn;
    int eventn;
    int epollfd;
    char buf[MAXLINE];
	int one_flag = 0;
 
	/* Create Socket */
    events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
    if((epollfd = epoll_create(100)) == -1)
        return 1;
 
    addrlen = sizeof(addr);
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return 1;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_NUM);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind (listenfd, (struct sockaddr *)&addr, addrlen) == -1)
        return 1;
 
    listen(listenfd, 5);
 
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);
    memset(user_fds, -1, sizeof(int) * 1024);            

	printf("Wait Connecting...\n");
    while(1)
    {
        // Wait Event
        eventn = epoll_wait(epollfd, events, EPOLL_SIZE, -1); 
        if(eventn == -1)
        {
            return 1;
        }
        for(i = 0; i < eventn ; i++)
        {
			/*====== Listen Event ==========*/
            if(events[i].data.fd == listenfd)    
            {
                memset(buf, 0x00, MAXLINE);
                clilen = sizeof(struct sockaddr);
                clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clilen);
                user_fds[clientfd] = 1;        
            
                user_data = malloc(sizeof(user_data));
                user_data->fd = clientfd;

				sprintf(ipA[clientfd].ip, "%s", inet_ntoa(clientaddr.sin_addr));

				/*========== LOGIN ============*/
				// id
				char* tmp = "ID :";
				write(user_data->fd, tmp, 5);

				sleep(1);
				read(user_data->fd, user_data->id, sizeof(user_data->id));
				user_data->id[strlen(user_data->id) - 1] = 0;
				// pw
				char* tmp2 = "PW :";
				write(user_data->fd, tmp2, 5);

				sleep(1);
				read(user_data->fd, user_data->pw, sizeof(user_data->pw));
				user_data->pw[strlen(user_data->pw) - 1] = 0;

				/*========== Auth ==============*/
				if (0 == strcmp(user_data->id, USER1_ID))
				{
					if (0 == strcmp(user_data->pw, USER1_PW))
					{
						sprintf(buf, "Log-in Success!![%s] *^^*\n", user_data->id);
					}
					else
					{
						sprintf(buf, "Log-in fail: incorrect password...\n");
						write(user_data->fd, buf, 40);
                    	close(user_data->fd);
						continue;
					}
				}
				else if (0 == strcmp(user_data->id, USER2_ID))
				{
					if (0 == strcmp(user_data->pw, USER2_PW))
					{
						sprintf(buf, "Log-in Success!![%s] *^^*\n", user_data->id);
					}
					else
					{
						sprintf(buf, "Log-in fail: incorrect password...\n");
						write(user_data->fd, buf, 40);
                    	close(user_data->fd);
						continue;
					}
				}
				else
				{
					sprintf(buf, "Log-in fail: incorrect id...\n");
					write(user_data->fd, buf, 40);
                    close(user_data->fd);
					continue;
				}
				write(user_data->fd, buf, 40);

				// for file mode
				sprintf(ipA[user_data->fd].id, "%s", user_data->id);

				// START CHAT MODE
				if(one_flag == 0)
				{
					printf("===============Chating Room================\n>>");
					one_flag++;
				}
				printf("Welcome [%s]  \n>>", user_data->id);
            
                sleep(1);
 
                ev.events = EPOLLIN;
                ev.data.ptr = user_data;
 
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
            }
			/*=========== Event ============*/
            else
            {
				// RECV FROM CLIENT
                user_data = events[i].data.ptr;
                memset(buf, 0x00, MAXLINE);
                readn = read(user_data->fd, buf, MAXLINE);
                if(readn <= 0)
                {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->fd, events);
                    close(user_data->fd);
                    user_fds[user_data->fd] = -1;
                    //free(user_data);

					printf("Goodbye [%s] \n>>", user_data->id);
                }
                else
                {
					/* check [FILE] MODE and usrename */
					char filestr[20];
					char username[20];
					for(int fn = 0; fn < strlen(buf); fn++)
					{
						if(fn < 5)
						{
							filestr[fn] = buf[fn];
						}
						else if(fn == 5)
						{
							continue;
						}
						else
						{
							if(buf[fn] != ']')
							{
								username[fn - 6] = buf[fn];
							}
							else
							{
								username[fn - 6] = '\0';

								filestr[5] = ']';
								filestr[6] = '\n';
								filestr[7] = '\0';
								break;
							}
						}
					}
					// send [FILE]mode and ipaddress
					if(0 == strcmp(filestr, "[FILE]\n"))
					{
						printf("[FILE]\n>>");

						// find user name
						for(int un = 0; un < 1024; un++)
						{
							if(user_fds[un] == 1)
							{
								if(0 == strcmp(ipA[un].id, username))
								{
									write(un, filestr, 7);
									sleep(1);
									write(un, ipA[user_data->fd].ip, 20);
									break;
								}
							}
						}
					}
					else
					{
						printf("[%s] %s>>", user_data->id, buf);
                    	send_msg(events[i], buf, 0);
					}
                }
            }
        }
    }
}
 
// SEND TO ALL CLIENT
void send_msg(struct epoll_event ev, char *msg, int flag)
{
    int i;
    char buf[MAXLINE+24];
    struct udata *user_data;
    user_data = ev.data.ptr;
    for(i =0; i < 1024; i++)
    {
        memset(buf, 0x00, MAXLINE+24);
		if(flag)
		{
        	sprintf(buf, "%s", msg);
		}
		else
		{
        	sprintf(buf, "[%s] %s", user_data->id, msg);
		}
        if((user_fds[i] == 1))
        {
			if(user_data->fd != i)
			{
            	write(i, buf, MAXLINE+24);
			}
        }
    }
}
