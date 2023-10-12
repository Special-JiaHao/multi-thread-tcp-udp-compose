/*
  Copyright (c) 2023 Value
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef TELECOM
#define TELECOM

#include <netinet/in.h>
#include <iostream>
#include <unordered_map>
#include <queue>

typedef std::pair<sa_family_t, in_addr_t> Tele_Addr;
typedef std::pair<int, struct sockaddr_in> Tele_Socket;
struct Message{
	char language[100];
	char inventor[100];
	int age;
	Message(){};
	Message(char *language, char * inventor, int age);
	Message(std::string language, std::string inventor, int age);
};
typedef struct Message Packet;
extern std::ostream& operator<<(std::ostream& out, Message& message);

/* create a socket descriptor */
extern int Tele_CreateSocketDescriptor(int family, int type, int protocol);
extern sockaddr_in *Tele_CreateSocketAddress(Tele_Addr addr, in_port_t port);
extern sockaddr_in *Tele_CreateSocketAddress(int family, std::string addr, in_port_t port);
extern int Tele_Bind(int sockfd, const struct sockaddr_in *addr, socklen_t addrlen);
extern int Tele_Listen(int sockfd, int backlog);
extern int Tele_BindAndListen(Tele_Addr addr, in_port_t port, int backlog);
extern int Tele_Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern int Tele_Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
extern int Tele_Receive(int sockfd, void *buff, size_t nbytes, int flags);
extern int Tele_UDPReceive(int sockfd, void *buff, size_t nbyte, int flags, struct sockaddr_in* from, socklen_t *addrlen);
extern int Tele_Send(int sockfd, void *buff, size_t nbytes, int flags);
extern int Tele_UDPSend(int sockfd, void *buff, size_t nbytes, int flags, const struct sockaddr_in *to, socklen_t addrlen);
extern int Tele_GetAvailablePort();


class BaseCompose{
public:
	BaseCompose();
	BaseCompose(sa_family_t sin_family, std::string addr, std::string tag = "");
	void setAddress(sa_family_t sin_family, std::string addr);
	void setTag(std::string tag);
	Tele_Addr getAddress();
	std::string getTag();
	bool AddressIsAvailable();
	~BaseCompose();
private:
	Tele_Addr addr;
	bool addrState;
	std::string tag;
};


class Client: public BaseCompose{
public:
	Client();
	Client(sa_family_t sin_family, std::string addr, std::string tag);
	int TCPConnect(struct sockaddr_in serverAddr, in_port_t clientPort);
	int push_backTCPSendBuf(int sockfd, Packet message);
	int TCPSend(int sockfd, int flags = MSG_NOSIGNAL);
	int UDPSend(in_port_t UDP_Port, sockaddr_in* serverAddr, socklen_t addrlen, void *buff, size_t nbytes, int flags = MSG_NOSIGNAL);
	~Client();
	std::unordered_map<int, std::queue<Message> > TCPSendBuf;
	std::unordered_map<int, struct sockaddr_in> TCPConnection;
};




class Server: public Client{
public:
	Server();
	Server(sa_family_t sin_family, std::string addr, std::string tag);
	int openTCP_Port(in_port_t TCP_Port, int backlog = 10);
	int openUDP_Port(in_port_t UDP_Port, int backlog = 10);
	~Server();
private:
};

	











#endif 