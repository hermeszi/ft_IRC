/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jngew <jngew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 20:18:48 by jngew             #+#    #+#             */
/*   Updated: 2026/01/29 21:03:25 by jngew            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <csignal>

bool	g_stop = false;

void	handle_signal(int sig)
{
	(void)sig;
	g_stop = true;
}

Server::Server() : _port(6667), _server_fd(-1), _password("default") {}

Server::Server(int port, std::string password) : _port(port), _server_fd(-1), _password(password) {}

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
	std::cout << "\nDestructing Server..." << std::endl;
	for (size_t x = 0; x < _pollfds.size(); x++)
	{
		if (_pollfds[x].fd > 0)
		{
			close (_pollfds[x].fd);
		}
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
	signal(SIGINT, handle_signal);
	struct pollfd pfd;
	pfd.fd = _server_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
	std::cout << "Server listening on port " << _port << "..." << std::endl;
}

void	Server::run()
{
	std::cout << "Waiting for connections... (Press Ctrl+C to stop)" << std::endl;
	while (g_stop == false)
	{
		int	poll_count = poll(&_pollfds[0], _pollfds.size(), -1);
		if (poll_count < 0 && g_stop == false)
		{
			std::cerr << "Error: poll failed" << std::endl;
			break ;
		}
		if (g_stop == true)
			break ;
		for (size_t x = 0; x < _pollfds.size(); x++)
		{
			if (_pollfds[x].revents & POLLIN)
			{
				if (_pollfds[x].fd == _server_fd)
				{
					struct sockaddr_in client_addr;
					socklen_t client_len = sizeof(client_addr);
					int	new_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &client_len);
					if (new_fd < 0)
					{
						if (g_stop)
							break ;
						std::cerr << "Error: accept failed" << std::endl;
						continue ;
					}
					if (fcntl(new_fd, F_SETFL, O_NONBLOCK) < 0)
					{
						std::cerr << "Error: fcntl on client failed" << std::endl;
						close(new_fd);
						continue;
					}
					struct pollfd client_pfd;
					client_pfd.fd = new_fd;
					client_pfd.events = POLLIN;
					client_pfd.revents = 0;
					_pollfds.push_back(client_pfd);
					std::cout << "New client connected! FD: " << new_fd << std::endl;
				}
				else
				{
					char	buffer[1024];
					memset(buffer, 0, sizeof(buffer));
					int	bytes_received = recv(_pollfds[x].fd, buffer, sizeof(buffer) - 1, 0);
					if (bytes_received <= 0)
					{
						std::cout << "Client disconnected. FD: " << _pollfds[x].fd << std::endl;
						close(_pollfds[x].fd);
						_pollfds.erase(_pollfds.begin() + x);
						x--;
					}
					else
					{
						std::cout << "Received from FD " << _pollfds[x].fd << ": " << buffer << std::endl;
					}
				}
			}
		}
	}
}
