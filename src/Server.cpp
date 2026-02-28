/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myuen <myuen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 20:18:48 by jngew             #+#    #+#             */
/*   Updated: 2026/02/28 22:02:40 by myuen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <csignal>
#include <sstream>
#include <stdexcept>

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
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++ it)
	{
		delete it->second;
		close (it->first);
	}
	_clients.clear();
	for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		delete it->second;
	}
	_channels.clear();
	if (_server_fd != -1)
		close (_server_fd);
}

void	Server::init()
{
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0)
		throw std::runtime_error("Error: socket creation failed");
	int	opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Error: setsockopt failed");
	int	flags = fcntl(_server_fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Error: fcntl F_GETFL failed");
	if (fcntl(_server_fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("Error: fcntl F_SETFL failed");
	struct	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(_port);
	if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
		throw std::runtime_error("Error: bind failed");
	if (listen(_server_fd, 3) < 0)
		throw std::runtime_error("Error: listen failed");
	signal(SIGINT, handle_signal);
	signal(SIGQUIT, handle_signal);
	signal(SIGPIPE, SIG_IGN);
    struct	pollfd pfd;
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
					if (new_fd >= 0)
					{
						int	flags = fcntl(new_fd, F_GETFL, 0);
						if (flags == -1 || fcntl(new_fd, F_SETFL, flags | O_NONBLOCK) < 0)
						{
							std::cerr << "Error: fcntl on client failed" << std::endl;
							close(new_fd);
							continue;
						}
						Client *new_client = new Client(new_fd, inet_ntoa(client_addr.sin_addr));
						_clients[new_fd] = new_client;
						struct pollfd client_pfd;
						client_pfd.fd = new_fd;
						client_pfd.events = POLLIN;
						client_pfd.revents = 0;
						_pollfds.push_back(client_pfd);
						std::cout << "New client connected! FD: " << new_fd << std::endl;
					}
				}
				else
				{
					char	buffer[1024];
					memset(buffer, 0, sizeof(buffer));
					int	bytes_received = recv(_pollfds[x].fd, buffer, sizeof(buffer) - 1, 0);
					if (bytes_received <= 0)
					{
						closeClient(_pollfds[x].fd);
						_pollfds.erase(_pollfds.begin() + x);
						x--;
					}
					else
					{
						int	client_fd = _pollfds[x].fd;
						Client *client = _clients[client_fd];
						if (client)
						{
							client->appendBuffer(buffer);
							while (client->hasLine())
							{
								std::string msg = client->extractLine();
								parseMessage(msg, client_fd);
								if (_clients.find(client_fd) == _clients.end())
								{
									_pollfds.erase(_pollfds.begin() + x);
									x--;
									break ;
								}
							}
						}
					}
				}
			}
		}
	}
}

void	Server::closeClient(int fd)
{
	if (_clients.find(fd) != _clients.end())
	{
		Client	*client = _clients[fd];
		std::map<std::string, Channel *>::iterator it = _channels.begin();
		while (it != _channels.end())
		{
			Channel	*channel = it->second;
			if (channel->isMember(client))
			{
				channel->removeMember(client);
				if (channel->isEmpty())
				{
					delete channel;
					std::map<std::string, Channel *>::iterator temp = it;
					++it;
					_channels.erase(temp);
					continue ;
				}
			}
			++it;
		}
		std::cout << "Client disconnected: FD " << fd << std::endl;
		delete _clients[fd];
		_clients.erase(fd);
		close (fd);
	}
}

void	Server::parseMessage(std::string message, int fd)
{
	if (message.empty())
		return ;
	if (message.length() > 0 && message[message.length() - 1] == '\n')
		message.erase(message.length() - 1);
	if (message.length() > 0 && message[message.length() - 1] == '\r')
		message.erase(message.length() - 1);
	if (message.empty())
		return ;
	std::stringstream ss(message);
	std::string command;
	ss >> command;
	for (size_t x = 0; x < command.length(); x++)
		command[x] = std::toupper(static_cast<unsigned char>(command[x]));
	std::string	arg;
	std::getline(ss, arg);
	if (!arg.empty() && arg[0] == ' ')
		arg.erase(0, 1);
	Client	*client = _clients[fd];
	if (!client)
		return ;
	if (command == "PASS")
		_executePASS(client, arg);
	else if (command == "NICK")
		_executeNICK(client, arg);
	else if (command == "USER")
		_executeUSER(client, arg);
	else if (command == "PING")
		_executePING(fd, arg);
	else if (command == "PRIVMSG")
		_executePRIVMSG(client, arg);
	else if (command == "QUIT")
		_executeQUIT(client, arg);
	else if (command == "JOIN")
		_executeJOIN(client, arg);
	else if (command == "KICK")
		_executeKICK(client, arg);
	else if (command == "PART")
		_executePART(client, arg);
	else if (command == "TOPIC")
		_executeTOPIC(client, arg);
	else if (command == "MODE")
       _executeMODE(client, arg);
	else
	{
		std::string err = ":irc_server 421 " + command + " :Unknown command\r\n";
		send(fd, err.c_str(), err.length(), 0);
		std::cout << "Unknown Command: " << command << std::endl;
	}
}

void	Server::_executePASS(Client *client, std::string arg)
{
	if (client->isRegistered())
	{
		std::string err = ":irc_server 462 :You may not re-register\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	if (arg == _password)
	{
		client->setHasPassword(true);
		std::cout << "PASS Correct for FD " << client->getFd() << std::endl;
	}
	else
	{
		std::cout << "PASS Incorrect for FD " << client->getFd() << std::endl;
		std::string err = ":irc_server 464 :Password incorrect\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		closeClient(client->getFd());
	}
}

void	Server::_executeNICK(Client *client, std::string arg)
{
	if (arg.empty())
	{
		std::string err = ":irc_server 431 :No nickname given\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	if (arg.find(' ') != std::string::npos || arg[0] == '#' || arg[0] == ':' || arg[0] == '&' || std::isdigit(arg[0]))
	{
		std::string err = ":irc_server 432 * " + arg + " :Erroneous nickname\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second == client)
			continue;
		if (it->second->getNickname() == arg)
		{
			std::string err = ":irc_server 433 * " + arg + " :Nickname is already in used\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return ;
		}
	}
	client->setNickname(arg);
	if (client->hasPassword() && !client->getUsername().empty() && !client->isRegistered())
	{
		client->setRegistered(true);
		std::string welcome = ":irc_server 001 " + client->getNickname() + " :Welcome to the IRC Network\r\n";
		send(client->getFd(), welcome.c_str(), welcome.length(), 0);
	}
}

void	Server::_executeUSER(Client *client, std::string arg)
{
	if (client->isRegistered())
	{
		std::string err = ":irc_server 462 :You may not re-register\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	std::stringstream ss(arg);
	std::string username;
	ss >> username;
	if (!username.empty())
	{
		client->setUsername(username);
		if (client->hasPassword() && !client->getNickname().empty() && !client->isRegistered())
		{
			client->setRegistered(true);
			std::string welcome = ":irc_server 001 " + client->getNickname() + " :Welcome to the IRC Network\r\n";
			send(client->getFd(), welcome.c_str(), welcome.length(), 0);
		}
	}
}

void	Server::_executePING(int fd, std::string arg)
{
	std::string reply = "PONG " + arg + "\r\n";
	send(fd, reply.c_str(), reply.length(), 0);
}

Client	*Server::_getClientByNick(std::string nick)
{
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nick)
			return (it->second);
	}
	return (NULL);
}

void	Server::_executePRIVMSG(Client *client, std::string arg)
{
	if (arg.empty())
	{
		std::string err = ":irc_server 411 :No recipient given (PRIVMSG)\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	size_t spacePos = arg.find(' ');
	if (spacePos == std::string::npos)
	{
		std::string err = ":irc_server 412 :No text to send\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	std::string target = arg.substr(0, spacePos);
	std::string message = arg.substr(spacePos + 1);
	if (!message.empty() && message[0] == ':')
		message = message.substr(1);
	if (target.empty() || message.empty())
	{
		std::string err = ":irc_server 412 :No text to send or invalid target\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	if (target[0] == '#')
	{
		if (_channels.find(target) != _channels.end())
		{
			Channel *channel = _channels[target];
			if (!channel->isMember(client))
			{
				std::string err = ":irc_server 404 " + target + " :Cannot send to channel\r\n";
				send(client->getFd(), err.c_str(), err.length(), 0);
				return ;
			}
			std::string fullMsg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
			channel->broadcast(fullMsg, client);
		}
		else
		{
			std::string err = ":irc_server 401 " + target + " :No such nick/channel\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
		}
	}
	else
	{
		Client *targetClient = _getClientByNick(target);
		if (targetClient)
		{
			std::string fullMsg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
			send(targetClient->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
		}
		else
		{
			std::string err = ":irc_server 401 " + target + " :No such nick/channel\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
		}
	}
}

void	Server::_executeQUIT(Client *client, std::string arg)
{
	std::string	reason = "Quit: ";

	if (arg.empty())
		reason += "Client exited";
	else
	{
		if (arg[0] == ':')
			arg = arg.substr(1);
		reason += arg;
	}
	std::cout << "Client " << client->getFd() << " is quitting: " << reason << std::endl;
	std::string quitMsg = ":" + client->getPrefix() + " QUIT :" + reason + "\r\n";
	for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		Channel	*channel = it->second;
		if (channel->isMember(client))
			channel->broadcast(quitMsg, client);
	}
	closeClient(client->getFd());
}

void	Server::_executeJOIN(Client *client, std::string arg)
{
	if (arg.empty())
	{
		std::string err = ":irc_server 461 JOIN :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	size_t	spacePos = arg.find(' ');
	std::string name = "";
	std::string	password = "";
	if (spacePos == std::string::npos)
		name = arg;
	else
	{
		name = arg.substr(0, spacePos);
		password = arg.substr(spacePos + 1);
	}
	if (name[0] != '#')
		name = "#" + name;
	Channel	*channel;
	bool	isNew = false;
	if (_channels.find(name) == _channels.end())
	{
		channel = new Channel(name);
		_channels[name] = channel;
		channel->addOperator(client);
		isNew = true;
		std::cout << "Created channel: " << name << std::endl;
	}
	else
	{
		channel = _channels[name];
		// --- NEW MODE CHECKS ---
		// Check Invite-Only (+i)
		if (channel->isInviteOnly())
		{
			// TODO MINGDE: Uncomment this when you finish the isInvited function!
			// if (!channel->isInvited(client))
			// {
			// 	std::string err = ":irc_server 473 " + client->getNickname() + " " + name + " :Cannot join channel (+i)\r\n";
			// 	send(client->getFd(), err.c_str(), err.length(), 0);
			// 	return ;
			// }
		}
		// Check Password (+k)
		if (!channel->getPassword().empty())
		{
			if (password != channel->getPassword())
			{
				std::string err = ":irc_server 475 " + client->getNickname() + " " + name + " :Cannot join channel (+k)\r\n";
				send(client->getFd(), err.c_str(), err.length(), 0);
				return ;
			}
		}
		// Check User Limit (+l)
		if (channel->getUserLimit() > 0)
		{
			if (static_cast<int>(channel->getMemberCount()) >= channel->getUserLimit())
			{
				std::string err = ":irc_server 471 " + client->getNickname() + " " + name + " :Cannot join channel (+l)\r\n";
				send(client->getFd(), err.c_str(), err.length(), 0);
				return ;
			}
		}
	}
	if (channel->isMember(client))
		return ;
	channel->addMember(client);
	std::string joinMsg = ":" + client->getPrefix() + " JOIN :" + name + "\r\n";
	channel->broadcast(joinMsg, client);
	send(client->getFd(), joinMsg.c_str(), joinMsg.length(), 0);

	// 5. Send Topic (RPL_TOPIC 332) - Empty for now

	std::string names = channel->getUserList();
	std::string rpl353 = ":irc_server 353 " + client->getNickname() + " = " + name + " :" + names + "\r\n";
	send(client->getFd(), rpl353.c_str(), rpl353.length(), 0);
	std::string rpl366 = ":irc_server 366 " + client->getNickname() + " " + name + " :End of /NAMES list.\r\n";
	send(client->getFd(), rpl366.c_str(), rpl366.length(), 0);
	std::cout << client->getNickname() << " joined " << name << std::endl;
	if (isNew)
		std::cout << "Created new channel: " << name << std::endl;
	else
		std::cout << client->getNickname() << " joined existing channel: " << name << std::endl;
}

void Server::_executePART(Client *client, std::string arg)
{
	// TODO: Support multiple channels (comma-separated)
    // For now: single channel only
    // Future: parseChannelList(arg) returns vector<string>
	if (arg.empty())
	{
		std::string err = ":irc_server 461 PART :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}

	size_t spacePos = arg.find(' ');
	std::string channelName = "";
	std::string reason = "";

	if (spacePos == std::string::npos)
	{
    	channelName = arg;
    	reason = "";
	}
	else
	{
    	channelName = arg.substr(0, spacePos);
    	std::string rest = arg.substr(spacePos + 1);
		if (!rest.empty() && rest[0] == ':')
		{
			reason = rest.substr(1);
		}
		else
		{
			reason = rest;
		}
	}
	if (channelName[0] != '#')
		channelName = "#" + channelName;
	if (_channels.find(channelName) == _channels.end())
	{
		std::string err = ":irc_server 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	else
	{
		Channel	*channel;
		channel = _channels.at(channelName);

		if (!channel->isMember(client))
		{
    		std::string err = ":irc_server 442 " + client->getNickname() + " " + channelName + " :You're not on that channel\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
    		return;
		}

		std::string partMsg = ":" + client->getPrefix() + " PART " + channelName + " :" + reason + "\r\n";
		channel->broadcast(partMsg, NULL);

		channel->removeMember(client);
		if (channel->isEmpty())
		{
			delete channel;
			_channels.erase(channelName);
			return ;
		}
		else
		{
			if (!channel->hasOperators())
			{
				Client *newOp = channel->getFirstMember();
				channel->addOperator(newOp);
				//MODE #channel +o nickname
				std::string modeMsg = ":irc_server MODE " + channelName + " +o " + newOp->getNickname() + "\r\n";
    			channel->broadcast(modeMsg, NULL);
			}
		}
		return ;
	}
}

void	Server::_executeKICK(Client *client, std::string arg)
{
	if (arg.empty())
	{
		std::string err = ":irc_server 461 KICK :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	std::stringstream ss(arg);
	std::string channelName;
	std::string targetUser;
	std::string reason;
	ss >> channelName >> targetUser;
	std::getline(ss, reason);
	if (channelName.empty() || targetUser.empty())
	{
		std::string err = ":irc_server 461 KICK :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	if (!reason.empty() && reason[0] == ' ')
		reason.erase(0, 1);
	if (!reason.empty() && reason[0] == ':')
		reason = reason.substr(1);
	if (reason.empty())
		reason = "Kicked from channel";
	if (_channels.find(channelName) == _channels.end())
	{
		std::string err = ":irc_server 403 " + channelName + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	Channel *channel = _channels[channelName];
	if (!channel->isMember(client))
	{
		std::string err = ":irc_server 442 " + channelName + " :You're not on that channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	if (!channel->isOperator(client))
	{
		std::string err = ":irc_server 482 " + channelName + " :You're not channel operator\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	Client *target = _getClientByNick(targetUser);
	if (!target || !channel->isMember(target))
	{
		std::string err = ":irc_server 441 " + targetUser + " " + channelName + " :They aren't on that channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	std::string kickMsg = ":" + client->getPrefix() + " KICK " + channelName + " " + targetUser + " :" + reason + "\r\n";
	channel->broadcast(kickMsg, NULL);
	channel->removeMember(target);
	if (channel->isEmpty())
	{
		delete channel;
		_channels.erase(channelName);
		std::cout << "Channel " << channelName << " deleted (empty)." << std::endl;
	}
	else
	{
		std::cout << targetUser << " was kicked from " << channelName << " by " << client->getNickname() << std::endl;
	}
}
void Server::_executeTOPIC(Client *client, std::string arg)
{
    if (arg.empty())
	{
		std::string err = ":irc_server 461 TOPIC :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}

    //Parse: Is there a topic to SET, or just VIEW?
	size_t spacePos = arg.find(' ');
	std::string channelName = "";
	std::string newTopic = "";

	if (spacePos == std::string::npos)
	{
    	channelName = arg;
    	newTopic = "";
	}
	else
	{
    	channelName = arg.substr(0, spacePos);
    	std::string rest = arg.substr(spacePos + 1);
		if (!rest.empty() && rest[0] == ':')
		{
			newTopic = rest.substr(1);
		}
		else
		{
			newTopic = rest;
		}
	}
	if (channelName[0] != '#')
		channelName = "#" + channelName;
	if (_channels.find(channelName) == _channels.end())
	{
		std::string err = ":irc_server 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	else
	{
		Channel	*channel;
		channel = _channels.at(channelName);

		if (!channel->isMember(client))
		{
    		std::string err = ":irc_server 442 " + client->getNickname() + " " + channelName + " :You're not on that channel\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
    		return;
		}

		if (spacePos == std::string::npos)
		{
			//    TOPIC #general   → VIEW
			if (channel->getTopic().empty())
			{
				std::string reply = ":irc_server 331 " + client->getNickname() + " " + channel->getName() + " :No topic is set\r\n";
				send(client->getFd(), reply.c_str(), reply.length(), 0);
				return ;
			}
			else
			{
				std::string reply = ":irc_server 332 " + client->getNickname() + " " + channel->getName() + " :" + channel->getTopic() + "\r\n";
				send(client->getFd(), reply.c_str(), reply.length(), 0);
				return ;
			}

		}
		else
		{
			//    TOPIC #general :New text → SET
			if (channel->isTopicRestricted() && !channel->isOperator(client))
			{
				std::string err = ":irc_server 482 " + client->getNickname() + " " + channel->getName() + " :You're not channel operator\r\n";
				send(client->getFd(), err.c_str(), err.length(), 0);
				return ;
			}
			else
			{
				channel->setTopic(newTopic);
				std::string topicMsg = ":" + client->getPrefix() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
				channel->broadcast(topicMsg, NULL);
			}
		}
	}
}

void Server::_executeMODE(Client *client, std::string arg)
{
	if (arg.empty())
	{
		std::string err = ":irc_server 461 MODE :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	// Parse: "arg" could be "#general +t" or "#general -t"
	size_t spacePos = arg.find(' ');
	std::string channelName = "";
	std::string modeStr= "";

	if (spacePos == std::string::npos)
	{
    	channelName = arg;
    	modeStr = "";
	}
	else
	{
    	channelName = arg.substr(0, spacePos);
    	std::string rest = arg.substr(spacePos + 1);
		if (!rest.empty())
		{
			modeStr = rest;
		}
	}

	spacePos = modeStr.find(' ');
	std::string parameter = "";
	std::string	modeFlag = "";
	if (spacePos == std::string::npos)
	{
    	modeFlag = modeStr;
    	modeStr = "";
	}
	else
	{
    	modeFlag = modeStr.substr(0, spacePos);
    	std::string rest = modeStr.substr(spacePos + 1);
		if (!rest.empty())
		{
			parameter = rest;
		}

	}

	if (channelName[0] != '#')
		channelName = "#" + channelName;
	if (_channels.find(channelName) == _channels.end())
	{
		std::string err = ":irc_server 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	else
	{
		Channel	*channel;
		channel = _channels.at(channelName);

		if (!channel->isMember(client))
		{
    		std::string err = ":irc_server 442 " + client->getNickname() + " " + channelName + " :You're not on that channel\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
    		return ;
		}

		if (!channel->isOperator(client))
		{
			std::string err = ":irc_server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return ;
		}
		char action = '\0';
		char flag = '\0';
		if (modeFlag.size() >= 2)
		{
			action = modeFlag.at(0);
			if (modeFlag[0] == '+' || modeFlag[0] == '-')
			{
				flag = modeFlag.at(1);
			}
		}
		if (action == '-')// Removing a mode
		{
			if (flag == 't')
			{
				channel->setTopicRestricted(false);
				std::string msg = ":" + client->getPrefix() + " MODE " + channelName + " -t\r\n";
				channel->broadcast(msg, NULL);

			}
			else if (flag == 'i')
			{
				channel->setInviteOnly(false);
				std::string msg = ":" + client->getPrefix() + " MODE " + channelName + " -i\r\n";
				channel->broadcast(msg, NULL);
			}
			else if (flag == 'k')
			{
				channel->setPassword("");
				std::string msg = ":" + client->getPrefix() + " MODE " + channelName + " -k\r\n";
				channel->broadcast(msg, NULL);
			}
			else if (flag == 'o')
			{
				if (parameter.empty())
				{
					std::string err = ":irc_server 461 " + client->getNickname() + " MODE -o :Not enough parameters\r\n";
					send(client->getFd(), err.c_str(), err.length(), 0);
					return;
				}

				Client *target = _getClientByNick(parameter);
				if (!target)
				{
					std::string err = ":irc_server 401 " + parameter + " :No such nick/channel\r\n";
					send(client->getFd(), err.c_str(), err.length(), 0);
					return;
				}

    			if (!channel->isMember(target))
    			{
					std::string err = ":irc_server 441 " + parameter + " " + channelName + " :They aren't on that channel\r\n";
					send(client->getFd(), err.c_str(), err.length(), 0);
					return ;
    			}

				channel->removeOperator(target);
				std::string modeMsg = ":" + client->getPrefix() + " MODE " + channelName + " -o " + target->getNickname() + "\r\n";
    			channel->broadcast(modeMsg, NULL);
			}
			else if (flag == 'l')
			{
				channel->setUserLimit(-1);
				std::string modeMsg = ":" + client->getPrefix() + " MODE " + channelName + " -l\r\n";
    			channel->broadcast(modeMsg, NULL);
			}
			else
			{
				std::string err = ":irc_server 501 " + client->getNickname() + " " + channel->getName() + " :Unknown MODE flag\r\n";
				send(client->getFd(), err.c_str(), err.length(), 0);
				return ;
			}

		}
		else if (action == '+')// Add a mode
		{
			if (flag == 't')
			{
				channel->setTopicRestricted(true);
				std::string msg = ":" + client->getPrefix() + " MODE " + channelName + " +t\r\n";
				channel->broadcast(msg, NULL);

			}
			else if (flag == 'i')
			{
				channel->setInviteOnly(true);
				std::string msg = ":" + client->getPrefix() + " MODE " + channelName + " +i\r\n";
				channel->broadcast(msg, NULL);
			}
			else if (flag == 'k')
			{
				if (parameter.empty())
				{
					std::string err = ":irc_server 461 " + client->getNickname() + " MODE +k :Not enough parameters\r\n";
					send(client->getFd(), err.c_str(), err.length(), 0);
					return;
				}
				channel->setPassword(parameter);
				std::string msg = ":" + client->getPrefix() + " MODE " + channelName + " +k "+ channel->getPassword() + "\r\n";
				channel->broadcast(msg, NULL);
			}
			else if (flag == 'o')
			{
				if (parameter.empty())
				{
					std::string err = ":irc_server 461 " + client->getNickname() + " MODE +o :Not enough parameters\r\n";
					send(client->getFd(), err.c_str(), err.length(), 0);
					return;
				}

				Client *target = _getClientByNick(parameter);
				if (!target)
				{
					std::string err = ":irc_server 401 " + parameter + " :No such nick/channel\r\n";
					send(client->getFd(), err.c_str(), err.length(), 0);
					return;
				}

    			if (!channel->isMember(target))
    			{
					std::string err = ":irc_server 441 " + parameter + " " + channelName + " :They aren't on that channel\r\n";
					send(client->getFd(), err.c_str(), err.length(), 0);
					return ;
    			}

				channel->addOperator(target);
				std::string modeMsg = ":" + client->getPrefix() + " MODE " + channelName + " +o " + target->getNickname() + "\r\n";
    			channel->broadcast(modeMsg, NULL);
			}
			else if (flag == 'l')
			{
				if (parameter.empty())
				{
					std::string err = ":irc_server 461 " + client->getNickname() + " MODE +l :Not enough parameters\r\n";
					send(client->getFd(), err.c_str(), err.length(), 0);
            		return;
        		}

        		if (!channel->setUserLimit(parameter))
        		{
            		std::string err = ":irc_server 461 " + client->getNickname() + " MODE +l :Not enough parameters\r\n";
					send(client->getFd(), err.c_str(), err.length(), 0);
            		return;
        		}
				std::string modeMsg = ":" + client->getPrefix() + " MODE " + channelName + " +l " + parameter + "\r\n";
    			channel->broadcast(modeMsg, NULL);
        	}
			else
			{
				std::string err = ":irc_server 501 " + client->getNickname() + " " + channel->getName() + " :Unknown MODE flag\r\n";
				send(client->getFd(), err.c_str(), err.length(), 0);
				return ;
			}
		}
		else
		{
			std::string err = ":irc_server 501 " + client->getNickname() + " :Unknown MODE flag\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return ;
		}
	}
}
