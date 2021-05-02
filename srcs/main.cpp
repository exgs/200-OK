#include "Webserver/Webserver.hpp"
// #include "ServerParsing/ServerConfigIdx.hpp"
// #include "Server"
// #include "Server/Server.hpp"
#include "Server/Servers.hpp"

// #include "Config/Config.hpp"
// #include "Config/Configs.hpp"
/*******************************************
내부에 try catch 문을 최대한 제거하기 위한 main 코드
********************************************/
int		main(int argc, char** argv, char** env)
{
	try {
		Webserver	s(argc, argv, env);
		Configs configss("srcs/ServerParsing/webserv.config");
		// configss.ShowConfigs();
		Servers servers;
		if (-1 == servers.SetServers(&configss))
		{
			std::cout << "error" << std::endl;
			return (-1);
		}
		servers.ShowServers();
		// STUB 대체하기
		// Server server;
		// server.SetGnl(server.config_fd);
		// SetServer(server, server.gnl);
		// server.ShowServerConfigs();
		
		// s.start_server();
	}
	catch(const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}
