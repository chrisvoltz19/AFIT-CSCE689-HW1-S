#include "TCPServer.h"

TCPServer::TCPServer() 
{
	// initalize the memory for the pollfd struct
	memset(this->fds, 0, sizeof(this->fds));
	this->timeout = 2 * 60 * 1000; // set timeout value to 2 minutes 
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
		perror("Failed to create server socket");
		shutdown();
	}
	
	// Set the Socket as nonblocking 
	if(fcntl(this->lSocket, F_SETFD, O_NONBLOCK) == -1)
	{
		perror("Failed to set up nonblocking");
	}

	// Bind the socket to an IP and port
	sockaddr_in hint;
	hint.sin_family = AF_INET; //sets it to IPv4
	hint.sin_port = htons(port); //uses port passed into method
	inet_pton(AF_INET, ip_addr, &hint.sin_addr); 
	if(bind(this->lSocket, (sockaddr*)&hint, sizeof(hint)) == -1)
	{
		perror("Failed to bind to server socket");
		shutdown();
	}

	// Mark the socket as listening 
	if(listen(this->lSocket, 20) == -1) // limits the socket to 20 connections
	{
		perror("Unable to currently listen, please try again later");
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
 * Polling code and layout referenced from: ibm.com/support/knowledgecenter/en/ssw_ibm_i_71/rzab6/poll.htm
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::listenSvr() 
{
	int result; // will be used to differentiate between different polling and receiving errors
	int newConnection = 0; // used for checking if a new connection is attempting to connect
	int shutdown = 0; // variable to track if the server had an error within one of the loops
	char buffer[150]; // will be used to hold from receive
	sockaddr_in client;
	socklen_t clientSize = sizeof(client);
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
			perror("Timeout reached... Disconecting server");
			break;
		}

		// while loop to handle new connections 
		for(int i = 0; i < this->nfds; i++)
		{
			// Loop through and see if any file descriptors 
			if(this->fds[i].revents == 0) // nothing happened in this case
			{
				//std::cout << this->fds[i].fd << " had no action this iteration" << std::endl;
				continue;
			}		
			// Deal with bad case of revents not being POLLIN
			if(this->fds[i].revents != POLLIN)
			{
				perror("Error: revents is not POLLIN");
				shutdown = 1;
				break;
			}

			if(this->fds[i].fd == this->lSocket) // Case that we are at the listening (server) socket
			{
				// accept all new incoming connections that are waiting
				//while(newConnection != -1) 
				//{
					newConnection = accept(this->lSocket, (sockaddr*)&client, &clientSize);
					if(newConnection < 0) // error on exception
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
					this->fds[nfds].events = POLLIN; 
					nfds++;
					std::cout << "Received new connection. NFDS: " <<  nfds << " File Descriptor: " << this->nfds <<std::endl;
				//}
			}
			// Not the listening server socket so must be a different connection
			else
			{
				memset(buffer, '\0', 150);
				result = recv(this->fds[i].fd, buffer, sizeof(buffer) - 1, 0); 
				if(result < 0)
				{
					if(errno != EWOULDBLOCK && errno != EAGAIN) // these two mean valid connection but no new data
					{
						perror("Error on receive"); 
						closeConnection(i); 
					}
					break;
				}
				if(result == 0) // client closed connection
				{
					closeConnection(i); 
					break;
				}
				// received data to use 
				// parse data 
				
				while(buffer[148] != '\0')
				{
					memset(buffer, '\0', 150);
					result = recv(this->fds[i].fd, buffer, sizeof(buffer) - 1, 0); 
					
				}
				shutdown = parseData(buffer, result, i); 
			}
		}	
	}
	
		

}

/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() 
{
	// close all the fds here 
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
 * Returns 1 on error in sending
 *
 * *********************************************************************************************/
int TCPServer::parseData(char* buffer, int length, int fd)
{
	std::string input = buffer; 
	std::stringstream inputStream;

	inputStream << input;

	if(buffer != NULL)
	{
		while(std::getline(inputStream, input))
		{
			//perform functionality based on command
			if(decision(input, fd) == 1) // there was an error
			{
				return 1;
			}
		}
	}
	return 0;
}
/************************************************************************************************
 * decision - takes a string and file descriptor, does the appropriate command and sends it back
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 * 
 * Returns 1 on error on sending
 *
 ************************************************************************************************/

int TCPServer::decision(std::string input, int fd)
{      
	std::string result;

	if(input.compare("hello") == 0)
	{
		result.assign("Hello new user. I hope you have a great day!");
	}
	else if(input.compare("1") == 0)
        {
                result.assign("You should watch The Counte of Monte Cristo or Gladiator");
        }
        else if(input.compare("2") == 0)
        {
                result.assign("You should read Call Sign Chaos: Learning to Lead");
        }
        else if(input.compare("3") == 0)
        {
                result.assign("If you like fps games, try the Halo series");
        }
        else if(input.compare("4") == 0)
        {
                result.assign("Squirrels usually forget where they hide about half of their nuts");
        }
        else if(input.compare("5") == 0)
        {
                result.assign("Sea Otters hold hands when they sleep so they don't drift apart");
        }
        else if(input.compare("passwd") == 0)
        {
		result.assign("To be implemented at a later date");
        }
        else if(input.compare("menu" ) == 0)
        {
		//result.assign("MENU: \n");
                result.assign(options("MENU: \n"));

        }
        else
        {
                result.assign("Invalid command. Please use command \"menu\" for assistance.\n");
        }
	
	// string to buffer conversion (referenced geeksforgeeks.org/onvert-string-char-array-cpp)
	int len = result.length();
	char buffer[len+1];
	strcpy(buffer, result.c_str());
	

	// send the command back to the client
	if(send(fds[fd].fd, buffer, len, 0) < 0)
	{	
		perror("Failed to send information back");
		return 1;
	}
	else 
	{
		return 0;
	}
}


/************************************************************************************************
 *  options - This is a method that prints out the current options for users 
 *  It is put in a method for cleaner programming practices so one change affects all instances 
 *
 ************************************************************************************************/
std::string TCPServer::options(std::string s)
{
        s.append("hello: Greeting \n"
		 "1: Movie Recommendation \n" 
                 "2: Book Recommendation \n" 
                 "3: Game Recommendation \n" 
                 "4: Fun fact \n"
                 "5: Cute fun fact \n"
                 "passwd: Change your password\n"
                 "exit: Disconnect from server\n" 
                 "menu: Displays this menu\n");

	return s;


}

