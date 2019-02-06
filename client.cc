#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h> //Eva
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "Protocol.h"

constexpr size_t bufferSize = 1024;
bool			 clientSideTCP(const std::string &port, const std::string &login, const std::string &hostName, protocolType type);

int main(int argc, char **argv)
{
	if (argc < 6)
	{
		errno = EINVAL;
		perror("Missing arguments. Start program like: ./ipk-client -h host -p port [-n|-f|-l] login");
		return EXIT_FAILURE;
	}
	int			 argument;
	std::string  hostName;
	std::string  port;
	std::string  login("");
	protocolType type;

	while ((argument = getopt(argc, argv, "h:p:n:f:l::")) != -1)
	{
		switch (argument)
		{
			case 'h':
				hostName = optarg;
				break;

			case 'p':
				port = optarg;
				break;

			case 'n':
				login = optarg;
				type  = UINFO;
				break;

			case 'f':
				login = optarg;
				type  = HOMEDIR;
				break;

			case 'l':
				login = optind < argc ? argv[optind] : "";
				type  = LIST;
				break;

			default:
				exit(EXIT_FAILURE);
		}
	}

	if (port.empty() || hostName.empty())
	{
		errno = EINVAL;
		perror("Missing arguments. Start program like: ./ipk-client -h host -p port [-n|-f|-l] login");
		return EXIT_FAILURE;
	}

	if (clientSideTCP(port, login, hostName, type))
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}

bool clientSideTCP(const std::string &port, const std::string &login, const std::string &hostName, protocolType type)
{
	int				   clientSocket;
	struct sockaddr_in serverAdress;
	struct hostent *   server;
	int				   bytesIn = 0, bytesOut;
	std::string		   buffer("");
	std::string		   recvMsg;
	char			   buf[bufferSize] = "";

	if ((server = gethostbyname(hostName.c_str())) == nullptr)
	{
		perror("No host");
		exit(EXIT_FAILURE);
	}
	if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	{
		perror("Socket fault");
		exit(EXIT_FAILURE);
	}

	serverAdress.sin_family = AF_INET;
	memcpy(&serverAdress.sin_addr.s_addr, server->h_addr, server->h_length);
	serverAdress.sin_port = htons(std::stoi(port));

	if (connect(clientSocket, (struct sockaddr *)&serverAdress, sizeof(serverAdress)) < 0)
	{
		perror("Connection fault");
		close(clientSocket);
		exit(EXIT_FAILURE);
	}

	Protocol clientProtocol(login, type, CLIENT, true);
	buffer   = clientProtocol.prepToSend();
	bytesOut = send(clientSocket, buffer.c_str(), std::strlen(buffer.c_str()), 0);
	if (bytesOut < 0)
		perror("Send fault");

	size_t pBytes = 0;

	//Get the header of the message
	int inB = recv(clientSocket, buf, bufferSize, 0);
    if(inB == -1)
        perror("Error getting header");

    bytesIn += inB;
	recvMsg.append(buf);
	pBytes = std::stoul(recvMsg.substr(recvMsg.find_first_of('K') + 1, recvMsg.find_first_of(':')));//Calculation message data size
    pBytes += recvMsg.find_first_of(':') + 1;//Calculating full message size(with header)

	if (bytesIn < 0)
		perror("Recv fault");

//Start getting the other message parts
	while (bytesIn < static_cast<int>(pBytes))
	{

		inB = recv(clientSocket, buf, bufferSize - 24, 0); //Get current message
        if(inB == -1)
            perror("Error receiving");

		recvMsg.append(buf);
		if (bytesIn < 0)
			perror("Recv fault");
        bytesIn += inB;
	}

	close(clientSocket);
    recvMsg = recvMsg.substr(recvMsg.find_first_of(':') + 1);

	if (buf[Protocol::successByte] == 'F')
	{
		std::cerr << recvMsg << std::endl;
		return false;
	}
    else if (!recvMsg.empty())
		std::cout << recvMsg << std::endl;
    else
        std::cout << recvMsg;

	return true;
}
