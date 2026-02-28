/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myuen <myuen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 19:56:36 by jngew             #+#    #+#             */
/*   Updated: 2026/02/28 21:05:42 by myuen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP
# include "Client.hpp"
# include <string>
# include <vector>
# include <iostream>

class	Channel
{
public:
	Channel(std::string name);
	Channel();
	~Channel();

	std::string	getName() const;

	void		addMember(Client *client);
	void		removeMember(Client *client);
	bool		isMember(Client *client);
	void		addOperator(Client *client);
	void		removeOperator(Client *client);
	bool		isOperator(Client *client);
	bool		isEmpty() const;
	std::string	getUserList();
	bool 		hasOperators() const;
	Client* 	getFirstMember() const;
	std::string	getTopic() const;
	void		setTopic(std::string topic);
	bool		isTopicRestricted() const;
	void		setTopicRestricted(bool value);
	bool		isInviteOnly() const;
	void		setInviteOnly(bool value);
	std::string	getPassword() const;
	void		setPassword(std::string password);
	int			getUserLimit() const;
	size_t		getMemberCount() const;
	bool		setUserLimit(int num);
	bool		setUserLimit(std::string strNum);

	void		broadcast(std::string message, Client *exclude);

private:
	std::string				_name;
	std::vector<Client *>	_members;
	std::vector<Client *>	_operators;

	std::string				_topic;             // For TOPIC command
	std::string				_password;          // For MODE +k
	bool					_inviteOnly;        // For MODE +i
	bool					_topicRestricted;   // For MODE +t
	int						_userLimit;         // For MODE +l (-1 = no limit)
	std::vector<Client *>	_inviteList;		// For INVITE command
};

#endif
