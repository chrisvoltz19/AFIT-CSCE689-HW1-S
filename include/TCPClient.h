#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <string>
#include "Client.h"
// added necessary header files
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>
#include <stdlib.h>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sstream>

// The amount to read in before we send a packet
const unsigned int stdin_bufsize = 50;
const unsigned int socket_bufsize = 100;

class TCPClient : public Client
{
public:
   TCPClient();
   ~TCPClient();

   virtual void connectTo(const char *ip_addr, unsigned short port);
   virtual void handleConnection();

   virtual void closeConn();

private:
	int clientSocket;

};


#endif
