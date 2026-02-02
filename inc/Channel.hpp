/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jngew <jngew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 19:56:36 by jngew             #+#    #+#             */
/*   Updated: 2026/02/02 20:12:06 by jngew            ###   ########.fr       */
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
				~Channel();

				std::string	getName() const;

				void	addMember(Client *client);
				void	removeMember(Client *client);
				bool	isMember(Client *client);
				void	addOperator(Client *client);
				void	removeOperator(Client *client);
				bool	isOperator(Client *client);

				void	broadcast(std::string message, Client *exclude);
	private:
				std::string				_name;
				std::vector<Client *>	_members;
				std::vector<Client *>	_operators;
};

#endif
