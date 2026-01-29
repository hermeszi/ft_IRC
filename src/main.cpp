/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jngew <jngew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 20:27:43 by jngew             #+#    #+#             */
/*   Updated: 2026/01/29 20:29:56 by jngew            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	int	port = std::atoi(argv[1]);
	if (port < 1024 || port > 65535)
	{
		std::cerr << "Error: invalid port (use 1024-65535)" << std::endl;
		return (1);
	}
	std::string password = argv[2];
	try
	{
		Server server(port, password);
		server.init();
		server.run();
	} catch (const std::exception &e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}
