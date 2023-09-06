#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#define SOCKMAX 30
#define READ_BUFFER 1024
#define MAX_EVENTS 30
void setnonblocking(int fd)
{
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

}



/*
 * Add epoll to support multi-io.
 *
 * */
int main()
{
  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(8888);
  server_addr.sin_family = AF_INET;

  int sockfd = socket(AF_INET,SOCK_STREAM,0);

  errif(bind(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) == -1, "bind failed!\n");

  errif(listen(sockfd, SOCKMAX) == -1, "listen failed!\n");

  //struct sockaddr_in client_addr;
  //bzero(&client_addr,sizeof(client_addr));
  //socklen_t len = sizeof(client_addr);
  //int client_fd = accept(sockfd, (struct sockaddr*)&client_addr,&len);
  //errif(client_fd == -1, "accept failed !\n");
  //printf("The client fd  is %d, the client ip is %s the client port is %d\n", client_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

  int epfd = epoll_create1(0);
  errif(epfd == -1, "epoll create error\n");

  struct epoll_event events[MAX_EVENTS], ev;
  bzero(&events, sizeof(events));

  bzero(&ev, sizeof(ev));

  ev.data.fd = sockfd;   //register sockfd as the observed file descriptor.
  ev.events = EPOLLIN | EPOLLET;
  setnonblocking(sockfd);
  epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
 
  while(1)
  {
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
    errif(nfds == -1, "epoll wait error\n");
    for(int i = 0; i < nfds; i++)
    {
      if(events[i].data.fd == sockfd)    //we got the new client.
      {
        struct sockaddr_in client_addr;
        bzero(&client_addr,sizeof(client_addr));
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(sockfd, (struct sockaddr*)&client_addr,&len);
        errif(client_fd == -1, "accept failed !\n");
        printf("The client fd  is %d, the client ip is %s the client port is %d\n", client_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        bzero(&ev, sizeof(ev));
        ev.data.fd = client_fd;
        ev.events = EPOLLIN | EPOLLET;
        setnonblocking(client_fd);
        epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
      }    
    
    else if(events[i].events & EPOLLIN)     //can read!!!
    {
      char buf[1024] = {0};
      while(true)
      {
        bzero(buf, sizeof(buf));
        ssize_t read_bytes = read(events[i].data.fd, buf, sizeof(buf));
        if(read_bytes > 0)
        {
          printf("The data read out from client %d and the msg are %s\n", events[i].data.fd, buf);
          write(events[i].data.fd, buf, sizeof(buf));
        }
        else if(read_bytes == 0)
        {
        printf("the client is disconnected\n");
        close(events[i].data.fd);
        break;
        }
        else if((read_bytes == -1) && (errno == EINTR))
        {
          printf("continue reading.....\n");
          continue;
        }
        else if((read_bytes == -1) && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
          printf("finish reading once errno: %d\n", errno);
          break;
        }
      }
    }
    else {
      printf("Something else happened!\n");
    }
  }
  }
  close(sockfd);
}
