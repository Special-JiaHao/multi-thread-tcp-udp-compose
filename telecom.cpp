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

#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <queue>
#include <pthread.h>
#include "telecom.h"


Message::Message(char *language, char *inventor, int age)
{
	this->age = age;
	for(int i = 0; i < 99 && language[i]; i ++ )
	{
		this->language[i] = language[i];
		if(language[i + 1] == '\0')	this->language[i + 1] = '\0';
	}
	for(int i = 0; i < 99 && inventor[i] != '\0'; i ++ )
	{
		this->inventor[i] = inventor[i];
		if(inventor[i + 1] == '\0')	this->inventor[i + 1] = '\0';
	}	
}

Message::Message(std::string language, std::string inventor, int age)
{
	this->age = age;
	for(int i = 0; i < language.size(); i ++ )
	{
		this->language[i] = language[i];
		if(i == language.size() - 1)	this->language[i + 1] = '\0';
	}	
	for(int i = 0; i < inventor.size(); i ++ )
	{
		this->inventor[i] = inventor[i];
		if(i == inventor.size() - 1)	this->inventor[i + 1] = '\0'; 
	}	
}

std::ostream& 
operator<<(std::ostream& out, Message& message){
	std::string language, inventor;
	for(int i = 0; i < 100 && message.language[i]; i ++ )	language += message.language[i];
	for(int i = 0; i < 100 && message.inventor[i]; i ++ )	inventor += message.inventor[i];
	out << "  |----------------------------" << std::endl;
	out << "  |Struct[Message]: " << std::endl;
	out << "  |  language: " << language << std::endl;
	out << "  |  inventor: " << inventor << std::endl;
	out << "  |  age: " << message.age << std::endl;
	out << "  |----------------------------";
    return out;
}

int
Tele_CreateSocketDescriptor(int family, int type, int protocol)
{
	int socketDescriptor = socket(family, type, protocol);
	if(socketDescriptor == -1)
	{
		perror("socket error ");
		return -1;
	}
	return socketDescriptor;
}

sockaddr_in*
Tele_CreateSocketAddress(Tele_Addr addr, in_port_t port)
{
	struct sockaddr_in *socketAddr = new sockaddr_in();
	socketAddr->sin_family = addr.first;
	socketAddr->sin_addr.s_addr = addr.second;
	socketAddr->sin_port = port;
	return socketAddr;
}

sockaddr_in*
Tele_CreateSocketAddress(int family, std::string addr, in_port_t port)
{
	struct sockaddr_in *socketAddr = new sockaddr_in();
	socketAddr->sin_family = family;
	int res = inet_pton(family, addr.c_str(), &socketAddr->sin_addr.s_addr);
	if(res == -1 || res == 0)
	{
		printf("Illegal address, please check your address.\n");
		return nullptr;
	}
	socketAddr->sin_port = htons(port);
	return socketAddr;
}


int 
Tele_Bind(int sockfd, const struct sockaddr_in *addr, socklen_t addrlen)
{
	if(bind(sockfd, (struct sockaddr*)addr, addrlen) == -1)
	{
		perror("bind error ");
		close(sockfd);
		return -1;
	}
	return 0;
}

int 
Tele_Listen(int sockfd, int backlog)
{
	if(listen(sockfd, backlog) == -1)
	{
		perror("listen error ");
		close(sockfd);
		return -1;
	}
	return 0;
}

int
Tele_BindAndListen(Tele_Addr addr, in_port_t port, int backlog)
{
	int socketDescriptor = Tele_CreateSocketDescriptor(addr.first, SOCK_STREAM, 0);
	if(socketDescriptor == -1)
	{
		printf("TCP Port open failed.\n");
		return -1;
	}
	struct sockaddr_in* socketAddr = Tele_CreateSocketAddress(addr, port);
	if(Tele_Bind(socketDescriptor, socketAddr, sizeof (struct sockaddr_in)) == -1)
	{
		printf("TCP Port open failed.\n");
		delete socketAddr;
		return -1;
	}
	if(Tele_Listen(socketDescriptor, backlog) == -1)
	{
		printf("TCP Port open failed.\n");
		delete socketAddr;
		return -1;
	}
	delete socketAddr;
	return socketDescriptor;
}

int 
Tele_Accept(int sockfd, struct sockaddr_in *addr, socklen_t *addrlen)
{
	int connectSockfd = accept(sockfd,  (struct sockaddr*)addr, addrlen);
	if(connectSockfd == -1)
	{
		perror("accept error ");
		delete addr;
		// close(sockfd);
		return -1;
	}
	return connectSockfd;
}

int 
Tele_Receive(int sockfd, void *buff, size_t nbytes, int flags)
{
	int recvlen = recv(sockfd, buff, nbytes, flags);
	if(recvlen < 0)
	{
		perror("recv error ");
		close(sockfd);
		delete (Packet*)buff;
		return -1;
	}else if(recvlen == 0)
	{
		printf("connect interrupt.\n");
		close(sockfd);
		delete (Packet*)buff;
		return 0;
	}
	return recvlen;
}

int 
Tele_Connect(int sockfd, const struct sockaddr_in *addr, socklen_t addrlen)
{
	if(connect(sockfd, (struct sockaddr *)addr, addrlen) == -1)
	{
		perror("connect error ");
		close(sockfd);
		return -1;
	}
	return 0;
}

int 
Tele_Send(int sockfd, void *buff, size_t nbytes, int flags)
{
	int sendlen = send(sockfd, buff, nbytes, flags);
	if(sendlen < 0)
	{
		perror("send error ");
		close(sockfd);
		return -1;
	}else if(sendlen == 0)
	{
		printf("connect interrupt.\n");
		close(sockfd);
		return 0;
	}
	return sendlen;
}


void*
correspondence(void *arg)
{
	std::pair<Server*, Tele_Socket*> *tmp = (std::pair<Server*, Tele_Socket*>*)arg;
	Tele_Socket *clientSocket = tmp->second;
	Server* server = tmp->first;
	void *recvBuf = (void *)new Packet();
	char ip[32];
	inet_ntop(clientSocket->second.sin_family, &clientSocket->second.sin_addr.s_addr, ip, sizeof ip);
	int port = ntohs(clientSocket->second.sin_port);
	server->TCPSendBuf[clientSocket->first] = std::queue<Packet>();
	server->TCPConnection[clientSocket->first] = clientSocket->second;
	while(true)
	{
		if(Tele_Receive(clientSocket->first, recvBuf, sizeof (Packet), 0) <= 0)
		{
			printf("connection with %s:%u interrupt.\n", ip, port);
			break;
		}	
		printf("received from %s:%u message:\n", ip, port);
		std::cout << (*(Packet*)recvBuf) << std::endl;

		server->push_backTCPSendBuf(clientSocket->first, *(Packet *)recvBuf);
		if(server->TCPSend(clientSocket->first) < 0)	break;
	}
	server->TCPSendBuf.erase(clientSocket->first);
	server->TCPConnection.erase(clientSocket->first);
	delete clientSocket;
	delete tmp;
	return NULL;
}


BaseCompose::BaseCompose(sa_family_t sin_family, std::string addr, std::string tag)
{
	this->addr.first = sin_family;
	int res = inet_pton(sin_family, addr.c_str(), &this->addr.second);
	if(res == -1 || res == 0)
	{
		printf("Illegal address, please check your address\n");
		return ;
	}	
	this->addrState = true;
	this->tag = tag;
}

void 
BaseCompose::setAddress(sa_family_t sin_family, std::string addr)
{
	this->addr.first = sin_family;
	int res = inet_pton(sin_family, addr.c_str(), &this->addr.second);
	if(res == -1 || res == 0)
	{
		printf("Illegal address, please check your address\n");
		return ;
	}	
	this->addrState = true;
}

void 
BaseCompose::setTag(std::string tag)
{
	this->tag = tag;
}

Tele_Addr 
BaseCompose::getAddress()
{
	return this->addr;
}

std::string
BaseCompose::getTag()
{
	return this->tag;
}

bool
BaseCompose::AddressIsAvailable()
{
	return this->addrState;
}

BaseCompose::~BaseCompose()	{}


Server::Server(sa_family_t sin_family, std::string addr, std::string tag) : Client::Client(sin_family, addr, tag){}


int 
Server::openTCP_Port(in_port_t TCP_Port, int backlog)
{
	if(!this->AddressIsAvailable())
	{
		printf("server address hasn’t been setted.\n");
		return -1;
	}
	Tele_Addr addr = this->getAddress();
	int sockfd = Tele_BindAndListen(addr, htons(TCP_Port), backlog);
	if(sockfd == -1)	return -1;
	char ip[32];
	std::string tag = this->getTag();
	if(tag != "")	std::cout << "(" << tag << ")";
	printf("%s:%u open success.\n", inet_ntop(addr.first, &addr.second, ip, sizeof ip), TCP_Port);
	while(true)
	{ 
		Tele_Socket* clientSocket = new Tele_Socket();
		socklen_t addrlen = sizeof (struct sockaddr_in);
		clientSocket->first = Tele_Accept(sockfd, &clientSocket->second, &addrlen);
		if(clientSocket->first == -1)	continue;
		printf("accept connection with %s:[%u] open success.\n", inet_ntop(clientSocket->second.sin_family, &clientSocket->second.sin_addr.s_addr, ip, sizeof ip), ntohs(clientSocket->second.sin_port));
		pthread_t correspondenceTid;
		std::pair<Server*, Tele_Socket*> *tmp = new std::pair<Server*, Tele_Socket*>({this, clientSocket});
		pthread_create(&correspondenceTid, NULL, correspondence, tmp);
		pthread_detach(correspondenceTid);
	}
	return 0;
}

int 
Server::openUDP_Port(in_port_t UDP_Port)
{
	if(!this->AddressIsAvailable())
	{
		printf("server address hasn’t been setted.\n");
		return -1;
	}
	Tele_Addr addr = this->getAddress();
	return 0;
}



Server::~Server() {}	


void *
recvFromServer(void *arg)
{
	std::pair<Client*, Tele_Socket>* tmp = (std::pair<Client*, Tele_Socket>*)arg;
	Client* client = tmp->first;
	Tele_Socket& serverSocket = tmp->second;
	void *recvBuf = (void *)new Packet();
	char ip[32];
	inet_ntop(serverSocket.second.sin_family, &serverSocket.second.sin_addr.s_addr, ip, sizeof ip);
	int port = ntohs(serverSocket.second.sin_port);
	while(true)
	{
		if(Tele_Receive(serverSocket.first, recvBuf, sizeof (Packet), 0) <= 0)
		{
			printf("connection with %s:%u interrupt.\n", ip, port);
			break;
		}	
		printf("received from %s:%u message(ack):\n", ip, port);
		std::cout << (*(Packet*)recvBuf) << std::endl;
	}
	client->TCPConnection.erase(serverSocket.first);
	client->TCPSendBuf.erase(serverSocket.first);
	delete tmp;
	return NULL;
}


Client::Client(sa_family_t sin_family, std::string addr, std::string tag) : BaseCompose::BaseCompose(sin_family, addr, tag){}

int
Client::TCPConnect(struct sockaddr_in serverAddr, in_port_t clientPort)
{
	if(!this->AddressIsAvailable())
	{
		printf("client address hasn’t been setted.\n");
		return -1;
	}	
	int sockfd = Tele_CreateSocketDescriptor(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		printf("TCP Port open failed.\n");
		return -1;
	}
	struct sockaddr_in* addr = Tele_CreateSocketAddress(this->getAddress(), htons(clientPort));
	if(Tele_Bind(sockfd, addr, sizeof (struct sockaddr_in)) == -1)
	{
		printf("TCP Bing local host error.\n");
		return -1;
	}
	if(Tele_Connect(sockfd, &serverAddr, sizeof (struct sockaddr_in)) == -1)
	{
		printf("TCP Connect failed.\n");
		return -1;
	}
	this->TCPSendBuf[sockfd] = std::queue<Packet>();
	this->TCPConnection[sockfd] = serverAddr;
	char ip[32];
	inet_ntop(serverAddr.sin_family, &serverAddr.sin_addr.s_addr, ip, sizeof ip);
	printf("successfully connect with %s:%u\n", ip, ntohs(serverAddr.sin_port));
	pthread_t recvFromServerTid;
	std::pair<Client*, Tele_Socket>* tmp = new std::pair<Client*, Tele_Socket>(this, {sockfd, serverAddr});
	pthread_create(&recvFromServerTid, NULL, recvFromServer, tmp);
	// pthread_detach(recvFromServerTid);	
	return sockfd;
}

int 
Client::push_backTCPSendBuf(int sockfd, Packet message)
{
	if(this->TCPSendBuf.find(sockfd) == this->TCPSendBuf.end())
	{
		printf("push_back TCPBuf error. please check tthe validity of the sockfd.\n");
		return -1;
	}
	this->TCPSendBuf[sockfd].push(message);
	return 0;
}

int 
Client::TCPSend(int sockfd, int flags)
{
	if(this->TCPSendBuf.find(sockfd) == this->TCPSendBuf.end())
	{
		printf("send error. please check tthe validity of the sockfd.\n");
		return -1;
	}
	int cnt = 0;
	std::queue<Packet> &qu = this->TCPSendBuf[sockfd];
	while(!qu.empty())
	{
		Packet cur = qu.front();
		qu.pop();
		cnt ++ ;
		if(Tele_Send(sockfd, (void *)&cur, sizeof (Packet), flags) <= 0)
		{
			char ip[32];
			inet_ntop(TCPConnection[sockfd].sin_family, &TCPConnection[sockfd].sin_addr.s_addr, ip, sizeof ip);
			printf("interrupt connection with %s:%u\n", ip, ntohs(TCPConnection[sockfd].sin_port));
			this->TCPSendBuf.erase(sockfd);
			this->TCPConnection.erase(sockfd);
			return -1;
		}
	}
	return cnt;
}

Client::~Client() {}


