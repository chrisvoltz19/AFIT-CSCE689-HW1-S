#include "TCPServer.h"

TCPServer::TCPServer() 
{
	// initalize the memory for the pollfd struct
	memset(this->fds, 0, sizeof(this->fds));
	this->timeout = 1000 * 19;// 2 * 60 * 1000; // set timeout value to 2 minutes 
}


TCPServer::~TCPServer() {

}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port) 
{
	// referenced youtube video: https://www.youtube.com/watch?v==cNdlrbZSkyQ
	this->lSocket = socket(AF_INET, SOCK_STREAM, 0);
	// check if socket creation failed, shutdown server if it did 
	if(this->lSocket == -1)
	{
		std::cerr << "Failed to create server socket" << std::endl;
		shutdown();
	}
	
	// Set the Socket as nonblocking 
	if(fcntl(this->lSocket, F_SETFD, O_NONBLOCK) == -1)
	{
		std::cerr << "Failed to set up nonblocking" << std::endl;
	}

	// Bind the socket to an IP and port
	sockaddr_in hint;
	hint.sin_family = AF_INET; //sets it to IPv4
	hint.sin_port = htons(port); //uses port passed into method
	inet_pton(AF_INET, ip_addr, &hint.sin_addr); 
	if(bind(this->lSocket, (sockaddr*)&hint, sizeof(hint)) == -1)
	{
		std::cerr << "Failed to bind to server socket" << std::endl;
		shutdown();
	}

	// Mark the socket as listening 
	if(listen(this->lSocket, 20) == -1) // limits the socket to 20 connections
	{
		std::cerr << "Unable to currently listen, please try again later" << std::endl;
		shutdown();
	}
	
	// add the lSocket to the fds
	this->fds[0].fd = lSocket;
	this->fds[0].events = POLLIN;
	this->nfds = 1;
}

/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::listenSvr() 
{
	// loop until exit condition is reached. TODO: What is the exit condition??
	/*while(true)
	{
		sockaddr_in client;
		int clientSocket = accept4(this->lSocket, (sockaddr*)&client, sizeof(client), O_NONBLOCK));
		
		//accept an incoming connection, if there is no error (need to check for other errors?)
		if(clientSocket != -1)
		{
			// add to the list of file descriptors 
			this->clients.emplace_back(clientSocket);
		}
	}*/

	int result; // will be used to differentiate between different polling and receiving errors
	int newConnection = 0; // used for checking if a new connection is attempting to connect
	int shutdown = 0; // variable to track if the server had an error within one of the loops
	char buffer[150]; // will be used to hold from receive
	sockaddr_in client;
	std::string subString;
	std::string commands; // string to hold the input from clients
	// while loop to handle everything 
	while(shutdown == 0)
	{
		// call poll and wait the timeout or a file descriptor is ready
		result = poll(this->fds, this->nfds, this->timeout);
		// check result for failures 
		if(result < 0) // poll call actually had an error
		{
			perror("Poll failed"); 
			break;
			
		}
		else if(result == 0) // timeout limit reached
		{
			std::cout << "Timeout reached... Disconecting server" << std::endl;
			break;
		}

		// while loop to handle new connections 
		for(int i = 0; i < this->nfds; i++)
		{
			// Loop through and see if any file descriptors 
			if(this->fds[i].revents == 0) // nothing happened in this case
			{
				std::cout << this->fds[i].fd << " had no action this iteration" << std::endl;
				continue;
			}		
			// Deal with bad case of revents not being POLLIN
			if(this->fds[i].revents != POLLIN)
			{
				std::cout << "Error: revents is not POLLIN" << std::endl;
				shutdown = 1;
				break;
			}
			if(this->fds[i].fd = this->lSocket) // Case that we are at the listening (server) socket
			{
				// accept all new incoming connections that are waiting
				while(newConnection != -1) 
				{
					newConnection = accept(this->lSocket, (sockaddr*)&client, sizeof(client));
					if(new_connection < 0) // error on exception
					{
						if(errno != EWOULDBLOCK && errno != EAGAIN) // these two errors just mean no connections
						{
							perror("Failed to accept an inbound client");
							shutdown = 1; // shutdown server if it goes wrong (Note may not want this in some cases
						}
						break;
					}
					// new connection has been accepted
					this->fds[nfds].fd = newConnection;
					this->fds[nfds].events = POLLIN; //TODO: if errors sending back check here
					nfds++
				}
			}
			// Not the listening server socket so must be a different connection
			else
			{
				// the current server still has information it is sending
				while(true)
				{
					result = recv(this->fds[i].fd, buffer, sizeof(buffer)); 
					if(result < 0)
					{
						if(errno != EWOULDBLOCK && errno != EAGAIN) // these two mean valid connection but no new data
						{
							perror("Error on receive"); 
							closeConnection(i); 
							break;
						}
					}
					if(result == 0) // client closed connection
					{
						closeConnection(i); 
						break;
					}
					// received data to use 
					// parse data 
					parseData(buffer, result); 
											

		
		// while loop to handle commands sent/requested by client



	}
	
		

}

/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() 
{
	// close all the fds here or in the deconstructor
		for(int i=0; i < this->nfds; i++)
	{
		if(this->fds[i].fd >= 0)
		{
			close(this->fds[i].fd);
		}
	}
}

/***********************************************************************************************
 * closeConnection - Cleanly closes connections with client if their connection fails or
 *                   is closed on its end
 *
 ***********************************************************************************************/
void TCPServer::closeConnection(int connection)
{
	close(fds[connection].fd);
	fds[connection].fd = -1; // set file descriptor back to -1 to indicate not used
	for(int i = 0; i < this->nfds; i++)
	{
		// clean up fds if connection closed
		if(this->fds[i].fd == -1)
		{
			for(int j = i; j < this->nfds; j++) // starts at current iteration and moves the rest down
			{
				fds[j].fd = this->fds[j+1].fd;
			}
			i--; // repeat current i to check connection after one just fixed
			this->nfds--; // decrement nfds
		}
	}
}

/***********************************************************************************************
 * ParseData - takes in buffer data and runs commands and then returns when text has been used
 *
 * I referenced https://stackoverflow.com/questions/13172158/c-split-string-by-line
 *
 * *********************************************************************************************/
void TCPServer::parseData(char* buffer, int length)
{
	std::string input; 
	std::stringstream inputStream(buffer);

	if(buffer != NULL)
	{
		while(std::getline(ss, input))
		{
			//perform functionality based on command
			decision(input);
		}
	}
}


