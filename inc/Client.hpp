/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jngew <jngew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 16:02:12 by jngew             #+#    #+#             */
/*   Updated: 2026/01/30 16:40:37 by jngew            ###   ########.fr       */
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

				void		appendBuffer(std::string data);
				void		clearBuffer();
				bool		isBufferReady();

				bool		hasLine();
				std::string	extractLine();
	private:
				int			_fd;
				std::string	_ipAddr;
				std::string	_buffer;
};

#endif
