#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#define SOCKMAX 30
int main()
{
  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(8888);
  server_addr.sin_family = AF_INET;

  int sockfd = socket(AF_INET,SOCK_STREAM,0);

  bind(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr));

  listen(sockfd, SOCKMAX);

  struct sockaddr_in client_addr;
  bzero(&client_addr,sizeof(client_addr));
  socklen_t len = sizeof(client_addr);
  int client_fd = accept(sockfd, (struct sockaddr*)&client_addr,&len);

  printf("The client fd  is %d, the client ip is %s the client port is %d\n", client_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));


}
