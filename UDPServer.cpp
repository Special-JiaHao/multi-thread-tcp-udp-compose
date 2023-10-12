#include <unistd.h>
#include "telecom.h"
int main()
{
	Server server(AF_INET, "127.0.0.10", "Server");
	server.openUDP_Port(8080);
	return 0;
}