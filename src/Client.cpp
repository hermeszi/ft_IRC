/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jngew <jngew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 16:05:26 by jngew             #+#    #+#             */
/*   Updated: 2026/02/02 18:34:55 by jngew            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd, std::string ipAddr) : _fd(fd), _ipAddr(ipAddr), _isRegistered(false), _hasPassword(false) {}

Client::~Client() {}

int	Client::getFd() const { return (_fd); }

std::string	Client::getIpAddr() const { return (_ipAddr); }

std::string	Client::getBuffer() const { return (_buffer); }

void	Client::appendBuffer(std::string data)
{
	_buffer += data;
}

void	Client::clearBuffer()
{
	_buffer.clear();
}

bool	Client::isBufferReady()
{
	if (_buffer.find('\n') != std::string::npos)
		return (true);
	return (false);
}

bool	Client::hasLine()
{
	return (_buffer.find('\n') != std::string::npos);
}

std::string	Client::extractLine()
{
	size_t	pos = _buffer.find('\n');
	if (pos == std::string::npos)
		return ("");
	std::string line = _buffer.substr(0, pos + 1);
	_buffer.erase(0, pos + 1);
	return (line);
}

void	Client::setNickname(std::string nick)
{
	_nickname = nick;
}

std::string Client::getNickname() const
{
	return (_nickname);
}

void	Client::setUsername(std::string user)
{
	_username = user;
}

std::string Client::getUsername() const
{
	return (_username);
}

void	Client::setRealname(std::string real)
{
	_realname = real;
}

std::string Client::getRealname() const
{
	return (_realname);
}

void	Client::setRegistered(bool value)
{
	_isRegistered = value;
}

bool	Client::isRegistered() const
{
	return (_isRegistered);
}

void	Client::setHasPassword(bool value)
{
	_hasPassword = value;
}

bool	Client::hasPassword() const
{
	return (_hasPassword);
}

std::string	Client::getPrefix() const
{
	std::string	username = _username.empty() ? "" : "!" + _username;
	std::string	ip = _ipAddr.empty() ? "" : "@" + _ipAddr;
	return (_nickname + username + ip);
}
