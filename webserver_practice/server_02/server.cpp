#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util.h"
#define SOCKMAX 30
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

  struct sockaddr_in client_addr;
  bzero(&client_addr,sizeof(client_addr));
  socklen_t len = sizeof(client_addr);
  int client_fd = accept(sockfd, (struct sockaddr*)&client_addr,&len);
  errif(client_fd == -1, "accept failed !\n");
  printf("The client fd  is %d, the client ip is %s the client port is %d\n", client_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
 
  while(1)
  {
    char buf[1024] = {0};
    size_t read_bytes = read(client_fd, buf, sizeof(buf));
    if(read_bytes > 0)
    {
      printf("The data read out from client %d and the msg are %s\n", sockfd, buf);
      write(client_fd, buf, read_bytes);
    }
    else if(read_bytes == 0)
    {
     printf("the client is disconnected\n");
     close(client_fd);
     break;
    }
    else if(read_bytes < 0)
    {
      close(client_fd);
      errif(true, "socket read failed!!\n");
    }
  }
  close(sockfd);
}
