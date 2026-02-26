/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jngew <jngew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 20:00:55 by jngew             #+#    #+#             */
/*   Updated: 2026/02/26 21:15:10 by jngew            ###   ########.fr       */
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
# include "Channel.hpp"

class	Server
{
	public:
				Server();
				Server(int port, std::string password);
				~Server();

				void	init();
				void	run();
	private:
				int			_port;
				int			_server_fd;
				std::string	_password;
				std::vector<struct pollfd> _pollfds;
				std::map<int, Client *> _clients;
				std::map<std::string, Channel *> _channels;

				void	closeClient(int fd);
				void	parseMessage(std::string message, int fd);

				void	_executePASS(Client *client, std::string arg);
				void	_executeNICK(Client *client, std::string arg);
				void	_executeUSER(Client *client, std::string arg);
				void	_executePING(int fd, std::string arg);
				void	_executePRIVMSG(Client *client, std::string arg);
				void	_executeQUIT(Client *client, std::string arg);
				void	_executeJOIN(Client *client, std::string arg);
				void	_executeKICK(Client *client, std::string arg);
				Client	*_getClientByNick(std::string nick);

				Server(const Server &src);
				Server &operator=(const Server &src);
};

#endif
