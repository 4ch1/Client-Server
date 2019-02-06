#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h> //Eva
#include <string>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Protocol.h"

constexpr size_t bufferSize = 1024;
bool			 serverSideTCP(const std::string &port);
Protocol		 fillProtocol(const char *query);
static bool		 getUserInfo(const std::string &info, std::string &storage);
static bool		 getHomeDir(const std::string &info, std::string &storage);//Function below is a code duplicate... Bad programming practices for the sake of speed.
static bool		 getList(const std::string &info, std::string &storage);       
static bool		 getUNameContains(const std::string &uName, const std::string &substr);

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		errno = EINVAL;
		perror("Missing arguments. Start program like: ./ipk-server -p port");
		return EXIT_FAILURE;
	}
	int			argument;
	std::string port;

	while ((argument = getopt(argc, argv, "p:")) != -1)
	{
		switch (argument)
		{
			case 'p':
				port = optarg;
				break;

			case '?':
				errno = EINVAL;
				perror("Unknown argument");
				return EXIT_FAILURE;

			default:
				break;
		}
	}
	if (port.empty())
	{
		errno = EINVAL;
		perror("Missing arguments. Start program like: ./ipk-server -p port");
		return EXIT_FAILURE;
	}

	if (serverSideTCP(port))
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}

bool serverSideTCP(const std::string &port)
{
	int					welocmeSocket;
	int					rc;
	struct sockaddr_in serverAdress;
	struct sockaddr_in clientAdress;
	socklen_t			clientAdressSize = sizeof(clientAdress);

	if ((welocmeSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket fault");
		exit(EXIT_FAILURE);
	}

	memset(&serverAdress, '\0', sizeof(serverAdress));
	serverAdress.sin_family = AF_INET;
	serverAdress.sin_addr.s_addr   = INADDR_ANY;
	serverAdress.sin_port   = htons(std::stoi(port));

	if ((rc = bind(welocmeSocket, (struct sockaddr *)&serverAdress, sizeof(serverAdress)) < 0))
	{
		perror("Bind fault");
		exit(EXIT_FAILURE);
	}
	if (listen(welocmeSocket, 1) < 0)
	{
		perror("Listening fault");
		exit(EXIT_FAILURE);
	}
	while (true)
	{
		int acceptingSocket = accept(welocmeSocket, (struct sockaddr *)&clientAdress, &clientAdressSize);
		if (acceptingSocket > 0)
		{
			std::string buffer;
			char		buff[bufferSize] = "";
			memset(buff, '\0', bufferSize);

			int bytesIn;
		    bytesIn = recv(acceptingSocket, buff, bufferSize, 0);

			Protocol servProtocol = fillProtocol(buff);
			buffer				  = servProtocol.prepToSend();

            size_t outBytes = 0;
            size_t bytesLeft = buffer.size();
            while(outBytes < buffer.size())
            {
                int currentOut = send(acceptingSocket, buffer.c_str() + outBytes , bytesLeft, 0);
                if(currentOut == -1)
                {
                    perror("Sending error");
                    exit(EXIT_FAILURE);
                }
                outBytes += currentOut;
                bytesLeft -= currentOut;
            }
            
		}
		close(acceptingSocket);
	}
}

Protocol fillProtocol(const char *query)
{
	protocolType type;
	std::string  inMsg(query);
    inMsg = inMsg.substr(inMsg.find_first_of(':') + 1);
	std::string  response;
	bool		 success = false;
	switch (query[Protocol::typeByte])
	{
		case 'U':
			type	= UINFO;
			success = getUserInfo(inMsg, response);
			break;
		case 'H':
			type	= HOMEDIR;
			success = getHomeDir(inMsg, response);
			break;
		case 'L':
			type	= LIST;
			success = getList(inMsg, response);
			break;
	}

	return {response, type, SERVER, success};
}

static bool getUNameContains(const std::string &uName, const std::string &substr) //Function is not general, written so in purpose for this project
{																				  //function makes full subseq match with seq(e.g. "xmano" won't match "xmanoi00")
	if (uName.empty())															  //I'm not sure if that was requested by the project task...
		return false;

	return uName.substr(0, uName.find_first_of(':')) == substr;
}
bool getUserInfo(const std::string &info, std::string &storage)
{
	std::ifstream passwd;
	passwd.open("/etc/passwd");

	if (!passwd.is_open())
	{
		storage = std::string("Can't open /etc/passwd");
		return false;
	}

	while (!passwd.eof())
	{
		std::string buffString;
		std::getline(passwd, buffString);
		if (getUNameContains(buffString, info))
		{
			int firstColon  = buffString.find_last_of(':');
			int secondColon = buffString.find_last_of(':', firstColon - 1);
			firstColon		= buffString.find_last_of(':', secondColon - 1);
			storage			= buffString.substr(firstColon + 1, secondColon - firstColon - 1);
			return true;
		}
	}
	storage = std::string("No user " + info + " found.");
	return false;
}
bool getHomeDir(const std::string &info, std::string &storage)
{
	std::ifstream passwd;
	passwd.open("/etc/passwd");

	if (!passwd.is_open())
	{
		storage = std::string("Can't open /etc/passwd");
		return false;
	}

	while (!passwd.eof())
	{
		std::string buffString;
		std::getline(passwd, buffString);
		if (getUNameContains(buffString, info))
		{
			int firstColon  = buffString.find_last_of(':');
			int secondColon = buffString.find_last_of(':', firstColon - 1);
			storage			= buffString.substr(secondColon + 1, firstColon - secondColon - 1);
			return true;
		}
	}
	storage = std::string("No user " + info + " found.");
	return false;
}
bool getList(const std::string &info, std::string &storage)
{

	std::ifstream passwd;
	passwd.open("/etc/passwd");

	if (!passwd.is_open())
	{
		storage = std::string("Can't open /etc/passwd");
		return false;
	}

	while (!passwd.eof())
	{
		std::string buffString;
		std::getline(passwd, buffString);
		size_t position = buffString.find(info);
		if (position == 0 && !buffString.empty())
			storage.append(buffString.substr(0, buffString.find_first_of(':')) + "\n");
	}
	if (!storage.empty()) //Remove the last newline character
		storage.pop_back();

	return true;
}
