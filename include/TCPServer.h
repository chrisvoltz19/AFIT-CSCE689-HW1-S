#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "Server.h"
// added necessary header files
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>11
#include <poll.h>
#include <stdlib.h>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>

class TCPServer : public Server 
{
public:
   TCPServer();
   ~TCPServer();

   void bindSvr(const char *ip_addr, unsigned short port);
   void listenSvr();
   void shutdown();
   void closeConnection(int connection);
   void parseData(char* buffer, int length);

private:
   int lSocket; // File Descriptor of the listening server socket
   //std::vector<int> clients; // vector of the file descriptors of the clients 
   struct pollfd fds[21]; // struct used in the poll command, which will be used instead of select
   int nfds; // variable to hold the number of file descriptors in the above fds
   int timeout; // variable to work with poll, stored in ms

};


#endif
