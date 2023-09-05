#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util.h"
int main()
{
  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(8888);
  server_addr.sin_family = AF_INET;

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  errif(connect(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) == -1, "connect failed\n");
  while(true)
  {
    char buf[1024] = {0};
    scanf("%s", buf);
    size_t write_bytes = write(sockfd, buf, sizeof(buf));
    if(write_bytes == -1)
    {
     printf("the socket has been broken!\n");
     break;
    }
    
    bzero(buf, sizeof(buf));
    size_t read_bytes = read(sockfd, buf, sizeof(buf));
    if(read_bytes > 0)
    {
      printf("the message read from server is %s\n", buf);
    }
    else if(read_bytes == -1)
    {
     close(sockfd);
     errif(true, "read failed!!\n");
    }
    else if(read_bytes == 0)
    {
      printf("the client is disconnected\n");
      break;
    }
  }
  return 0;


}
