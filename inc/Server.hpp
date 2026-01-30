/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jngew <jngew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 20:00:55 by jngew             #+#    #+#             */
/*   Updated: 2026/01/30 16:12:23 by jngew            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP
# include <iostream>
# include <vector>
# include <map>
# include <sys/socket.h> // To use socket(), bind(), listen()
# include <netinet/in.h> // To use sockaddr_in
# include <arpa/inet.h> // To use inet_ntoa
# include <fcntl.h> // To use fcntl()
# include <unistd.h> // To use close()
# include <cstdlib> // To use exit()
# include <poll.h>
# include "Client.hpp"

class	Server
{
	public:
				Server();
				Server(int port, std::string password);
				~Server();

				void	init();
				void	run();
				void	closeClient(int fd);
	private:
				int			_port;
				int			_server_fd;
				std::string	_password;
				std::vector<struct pollfd> _pollfds;
				std::map<int, Client *> _clients;

				Server(const Server &src);
				Server &operator=(const Server &src);
};

#endif
