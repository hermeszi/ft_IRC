/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jngew <jngew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 20:00:44 by jngew             #+#    #+#             */
/*   Updated: 2026/02/09 16:19:53 by jngew            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <algorithm>
#include <sys/socket.h>

Channel::Channel(std::string name) : _name(name) {}

Channel::~Channel() {}

std::string Channel::getName() const
{
	return (_name);
}

void	Channel::addMember(Client *client)
{
	if (!isMember(client))
		_members.push_back(client);
}

void	Channel::removeMember(Client *client)
{
	std::vector<Client *>::iterator it = std::find(_members.begin(), _members.end(), client);
	if (it != _members.end())
		_members.erase(it);
	if (isOperator(client))
		removeOperator(client);
}

bool	Channel::isMember(Client *client)
{
	std::vector<Client *>::iterator it = std::find(_members.begin(), _members.end(), client);
	return (it != _members.end());
}

void	Channel::addOperator(Client *client)
{
	if (!isOperator(client))
		_operators.push_back(client);
}

void	Channel::removeOperator(Client *client)
{
	std::vector<Client *>::iterator it = std::find(_operators.begin(), _operators.end(), client);
	if (it != _operators.end())
		_operators.erase(it);
}

bool	Channel::isOperator(Client *client)
{
	std::vector<Client *>::iterator it = std::find(_operators.begin(), _operators.end(), client);
	return (it != _operators.end());
}

void	Channel::broadcast(std::string message, Client *exclude)
{
	for (size_t x = 0; x < _members.size(); x++)
	{
		if (_members[x] == exclude)
			continue ;
		send(_members[x]->getFd(), message.c_str(), message.length(), 0);
	}
}

bool	Channel::isEmpty() const
{
	return (_members.empty());
}

std::string	Channel::getUserList()
{
	std::string	list;
	for (size_t x = 0; x < _members.size(); x++)
	{
		if (x > 0)
			list += " ";
		if (isOperator(_members[x]))
			list += "@";
		list += _members[x]->getNickname();
	}
	return (list);
}
