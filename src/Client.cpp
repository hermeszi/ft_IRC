/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jngew <jngew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 16:05:26 by jngew             #+#    #+#             */
/*   Updated: 2026/01/30 16:19:05 by jngew            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd, std::string ipAddr) : _fd(fd), _ipAddr(ipAddr) {}

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
