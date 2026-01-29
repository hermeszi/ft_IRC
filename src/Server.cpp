/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jngew <jngew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 20:18:48 by jngew             #+#    #+#             */
/*   Updated: 2026/01/29 20:25:46 by jngew            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <cstring>
#include <cerrno>

Server::Server() : _port(6667), _password("default"), _server_fd(-1) {}

Server::Server(int port, std::string password) : _port(port), _password(password), _server_fd(-1) {}

Server::Server(const Server &src) { *this = src; }

Server &Server::operator=(const Server &src)
{
	if (this != &src)
	{
		_port = src._port;
		_password = src._password;
		_server_fd = -1;
	}
	return (*this);
}

Server::~Server()
{
	if (_server_fd != -1)
	{
		std::cout << "Closing server socket..." << std::endl;
		close (_server_fd);
	}
}

void	Server::init()
{
	// 1. Create the socket, AF_INET = IPv4, SOCK_STREAM = TCP
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0)
	{
		std::cerr << "Error: socket creation failed" << std::endl;
		exit(1);
	}
	// 2. Set socket options, this allow us to restart the server immediately without waiting for the port to "cool down"
	int	opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "Error: setsockopt failed" << std::endl;
		exit(1);
	}
	// 3. Set Non-blocking mode
	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error: fcntl failed" << std::endl;
		exit(1);
	}
	// 4. Configure the address
	struct	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(_port);
	// 5. Bind the socket to the port
	if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		std::cerr << "Error: bind failed" << std::endl;
		exit(1);
	}
	// 6. Start listening
	if (listen(_server_fd, 3) < 0)
	{
		std::cerr << "Error: listen failed" << std::endl;
		exit(1);
	}
	std::cout << "Server listening on port " << _port << "..." << std::endl;
}

void	Server::run()
{
	std::cout << "Waiting for connections... (Press Ctrl+C to stop)" << std::endl;
	while (true)
	{
		// Poll
	}
}
