/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jngew <jngew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 16:02:12 by jngew             #+#    #+#             */
/*   Updated: 2026/02/02 18:35:09 by jngew            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP
# include <string>

class	Client
{
	public:
				Client(int fd, std::string ipAddr);
				~Client();

				int			getFd() const;
				std::string	getIpAddr() const;
				std::string	getBuffer() const;
				std::string	getPrefix() const;

				void		appendBuffer(std::string data);
				void		clearBuffer();
				bool		isBufferReady();
				bool		hasLine();
				std::string	extractLine();

				void		setNickname(std::string nick);
				std::string	getNickname() const;
				void		setUsername(std::string user);
				std::string	getUsername() const;
				void		setRealname(std::string real);
				std::string	getRealname() const;
				void		setRegistered(bool value);
				bool		isRegistered() const;
				void		setHasPassword(bool value);
				bool		hasPassword() const;
	private:
				int			_fd;
				std::string	_ipAddr;
				std::string	_buffer;

				std::string	_nickname;
				std::string	_username;
				std::string	_realname;
				bool		_isRegistered;
				bool		_hasPassword;
};

#endif
