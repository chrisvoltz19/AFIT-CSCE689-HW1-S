#include "TCPClient.h"


/**********************************************************************************************
 * TCPClient (constructor) - Creates a Stdin file descriptor to simplify handling of user input. 
 *
 **********************************************************************************************/

TCPClient::TCPClient() 
{

}

/**********************************************************************************************
 * TCPClient (destructor) - No cleanup right now
 *
 **********************************************************************************************/

TCPClient::~TCPClient() {

}

/**********************************************************************************************
 * connectTo - Opens a File Descriptor socket to the IP address and port given in the
 *             parameters using a TCP connection.
 *
 * I referenced https://www.youtube.com/watch?v=fmn-pRvNaho to make sure I was implementing correctly.
 *
 *    Throws: socket_error exception if failed. socket_error is a child class of runtime_error
 **********************************************************************************************/

void TCPClient::connectTo(const char *ip_addr, unsigned short port) 
{
	this->failedConnect = 0;	
	// create a socket
	if((this->clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Failed to create socket");
		//closeConn();
		this->failedConnect = 1;
	}
	
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ip_addr, &hint.sin_addr);

	// connect to the server on the socket
	if(connect(this->clientSocket, (sockaddr*)&hint, sizeof(sockaddr_in)) == -1)
	{
		perror("Failed to connect to the server");
		//closeConn();
		this->failedConnect = -1;
	}

	
}

/**********************************************************************************************
 * handleConnection - Performs a loop that checks if the connection is still open, then 
 *                    looks for user input and sends it if available. Finally, looks for data
 *                    on the socket and sends it.
 * 
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::handleConnection() 
{
	if(this->failedConnect == 0)
	{
		char buffer[4096];
		std::string command;
		int close = 0; 
		while(close == 0)
		{
			// get input command from user
			std::cout << "Command: "; 
			getline(std::cin, command);
			if(command.compare("exit") == 0)
			{
				close = 1;
				break;
			}
			if(send(this->clientSocket, command.c_str(), command.size() + 1, 0) == 1)
			{
				perror("Failed to send command");
				continue;
			}
	
			// wait for response
			memset(buffer, 0, 4096); 
			int bytesReceived = recv(this->clientSocket, buffer, 4096, 0);

			// display results
			std::cout << "Result: " << std::string(buffer, bytesReceived) << std::endl;
		}
	}
}


/**********************************************************************************************
 * closeConnection - Your comments here
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::closeConn() 
{
	close(this->clientSocket);
}


