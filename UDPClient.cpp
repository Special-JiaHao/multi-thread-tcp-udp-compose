#include <unistd.h>
#include "telecom.h"
int main()
{
	Client client(AF_INET, "127.0.0.2", "Client");
	Message message((char *)"PHP", (char *)"Value", 25);
	sockaddr_in *serverAddr = Tele_CreateSocketAddress(AF_INET, "127.0.0.10", 8080);
	int n = 3;
	while(n -- )
	{
		client.UDPSend(6677, serverAddr, sizeof (struct sockaddr_in), (void *)&message, sizeof (struct Message));
		sleep(5);
	}
	return 0;
}