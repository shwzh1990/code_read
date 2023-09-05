#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
int main()
{
  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(8888);
  server_addr.sin_family = AF_INET;

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  connect(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr));

  return 0;


}
