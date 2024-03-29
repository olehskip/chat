#ifndef SERVER_H
#define SERVER_H

#include "session.h"

class Server
{
public:
	Server() noexcept;
private:
	boost::asio::io_context io_context;
	tcp::acceptor tcpAcceptor;
	std::vector<std::shared_ptr<Session>> sessions;
	
	void acceptConnections() noexcept;
};

#endif // SERVER_H
