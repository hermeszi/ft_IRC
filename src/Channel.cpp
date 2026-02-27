/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myuen <myuen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 20:00:44 by jngew             #+#    #+#             */
/*   Updated: 2026/02/27 20:21:28 by myuen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <algorithm>
#include <sys/socket.h>

Channel::Channel(std::string name) :
_name(name),
_topicRestricted(false), // anyone can change topic
_inviteOnly(false),
_userLimit(-1){}

Channel::Channel() {}

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
bool	Channel::hasOperators() const
{
	return !_operators.empty();
}

Client* Channel::getFirstMember() const
{
    if (_members.empty())
		return NULL;
    return _members[0];
}

bool Channel::isTopicRestricted() const
{
	return _topicRestricted;
}

std::string Channel::getTopic() const
{
	return _topic;
}

void Channel::setTopic(std::string topic)
{
	_topic = topic;
}
