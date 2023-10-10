#include <unistd.h>
#include "telecom.h"
int main()
{
	Client client(AF_INET, "127.0.0.3", "Client");
	struct sockaddr_in *serverAddr = Tele_CreateSocketAddress(AF_INET, "127.0.0.10", 8080);
	int sockfd = client.TCPConnect(*serverAddr, 6678);
	Message message((char*)"C++", (char *)"Value", 25);
	while(true)
	{
		client.push_backTCPSendBuf(sockfd, message);
		if(client.TCPSend(sockfd) <= 0)	break;
		sleep(5);
	}
	close(sockfd);
}