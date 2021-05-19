#include "Server.hpp"
#include "ServerManager.hpp" // NOTE 상호참조 문제를 해결하기 위해서!
#include "Response.hpp"

extern char**	g_env;

void		Server::executeAutoindex(Connection& connection, const Request& request, std::string uri_copy)
{
	connection.set_m_response(new Response(&connection, 200, ft::makeAutoindexHTML(uri_copy)));
	Response *response = connection.get_m_response();
	response->set_m_headers("Server", "webserv");
	response->set_m_headers("Content-Type", "text/html");
	response->set_m_headers("Connection", "keep-alive");
	response->set_m_headers("Date", ft::getCurrentTime().c_str());

	connection.SetStatus(Connection::SEND_READY);
}

void		Server::executeGet(Connection& connection, const Request& request, std::string target_uri)
{
	if (ft::access(target_uri) == false)
	{
		throw 404;
	}
	else
	{
		create_Response_200(connection, target_uri, ANY_FILE);
		return ;
	}
}

void		Server::executeHead(Connection& connection, const Request& request, std::string target_uri)
{
	executeGet(connection, request, target_uri);
	std::string null_str;
	connection.get_m_response()->set_m_body(null_str);
	Response *response = connection.get_m_response();

	std::map<std::string, std::string>::iterator it = response->get_m_headers().begin();
	std::map<std::string, std::string> temp;
	while (it != response->get_m_headers().end())
	{
		if (it->first == "Date")
			temp["Date"] = it->second;
		else if (it->first == "Server")
			temp["Server"] = it->second;
		else if (it->first == "Content-Length")
			temp["Content-Length"] = it->second;
		it++;
	}
	response->copy_m_headers(temp);
	
}

void		Server::executePost(Connection& connection, const Request& request, std::string target_uri)
{
	connection.set_m_response(new Response(&connection, 200, request.getBody()));
	Response *response = connection.get_m_response();
	int fd = open(target_uri.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0755);
	int ret = write(fd, response->get_m_body().c_str(), response->get_m_body().length());
	cout << "ret: " << ret << endl;

}

void		Server::executePut(Connection& connection, const Request& request, std::string target_uri, config_iterator config_it)
{
	// NOTE 우선은 파일만 Put에 들어온다고 가정하자.
	bool file_exist = 0;
	int open_fd = open(target_uri.c_str(), O_RDWR | O_TRUNC);
	if (open_fd > 0)
	{
		file_exist = true;
		if (errno == 0) // NOTE 파일를 open으로 열었음
		{
			connection.set_m_response(new Response(&connection, 200, request.getBody()));
			write(open_fd, connection.get_m_response()->get_m_body().c_str(), connection.get_m_response()->get_m_body().length());
		}
		else if (errno == EISDIR) // NOTE 폴더를 open으로 열었음
		{
			connection.set_m_response(new Response(&connection, 204));
			std::string temp;
			connection.get_m_response()->set_m_body(temp);
		}
		close(open_fd);
	}
	else // ENOENT 2
	{
		connection.set_m_response(new Response(&connection, 201, request.getBody()));
		errno = 0;
		int open_fd = open(target_uri.c_str(), O_WRONLY | O_CREAT, 0755);
		write(open_fd, connection.get_m_response()->get_m_body().c_str(), connection.get_m_response()->get_m_body().length());
		close(open_fd);
	}
	Response *response = connection.get_m_response();

	response->set_m_headers("Date", ft::getCurrentTime().c_str());
	response->set_m_headers("Server", "webserv");
	if (errno == 0)
	{
		response->set_m_headers("Content-Length", ft::itoa(response->get_m_body().length()));
		response->set_m_headers("Content-Type", "text/html");
		response->set_m_headers("Content-Language", "en-US");
		if (file_exist == false)
		{
			response->set_m_headers("Location", target_uri);
		}
	}
	errno = 0; // NOTE 초기화!
}

void		Server::executeDelete(Connection& connection, const Request& request, std::string target_uri)
{
	bool file_exist = 0;
	int open_fd = open(target_uri.c_str(), O_RDWR);
	int temp = errno;
	if (open_fd > 0)
	{
		file_exist = true;
		if (errno == 0) // NOTE 파일를 open으로 열었음
		{
			int status_code = 200;
			connection.set_m_response(new Response(&connection, status_code, request.getBody()));
			std::string errorpage_body = Response::makeStatusPage(status_code, connection.get_m_request()->GetMethod());
			connection.get_m_response()->set_m_body(errorpage_body);
			unlink(target_uri.c_str());
			temp = errno;
		}
		else if (errno == EISDIR) // NOTE 폴더를 open으로 열었음
		{
			int status_code = 204;
			connection.set_m_response(new Response(&connection, status_code));
			rmdir(target_uri.c_str());
			temp = errno;
		}
		close(open_fd);
		temp = errno;
	}
	else // ENOENT 2
	{
		int status_code = 204;
		connection.set_m_response(new Response(&connection, status_code));
		int open_fd = open(target_uri.c_str(), O_WRONLY | O_CREAT, 0755);
		write(open_fd, connection.get_m_response()->get_m_body().c_str(), connection.get_m_response()->get_m_body().length());
		close(open_fd);
	}
	Response *response = connection.get_m_response();

	response->set_m_headers("Date", ft::getCurrentTime().c_str());
	response->set_m_headers("Server", "webserv");
	if (errno == 0 && file_exist == true)
	{
		response->set_m_headers("Content-Length", ft::itoa(response->get_m_body().length()));
		response->set_m_headers("Content-Type", "text/html");
		response->set_m_headers("Content-Language", "en-US");
	}
	errno = 0; // NOTE 초기화!
}

void		Server::executeOptions(Connection& connection, const Request& request, std::string target_uri, config_iterator config_it)
{
	int fd = open(target_uri.c_str(), O_RDONLY);
	if (fd < 0)
		throw 404;
	else
	{
		connection.set_m_response(new Response(&connection, 200));
		Response *response = connection.get_m_response();
		response->set_m_headers("Date", ft::getCurrentTime().c_str());
		response->set_m_headers("Server", "webserv");
		std::string value;
		for (size_t i = 0; i < config_it.locationPath->m_method.size(); i++)
		{
			value += config_it.locationPath->m_method[i];
			if (i != config_it.locationPath->m_method.size() - 1)
				value += " ";
		}
		response->set_m_headers("Allow", value);
		close(fd);
	}
}

void		Server::executeTrace(Connection& connection, const Request& request)
{

}

void		Server::executeCGI(Connection& connection, const Request& request) // NOTE request는 전혀 사용되지 않음
{
	Response*	response = connection.get_m_response();

	char*	connectionFD = ft::itoa(connection.get_m_fd());
	if (connectionFD == NULL)
	{
		throw 500;
	}
	std::string	fromCGIfileName = "fromCGI" + std::string(connectionFD);
	free(connectionFD); connectionFD = NULL;

	if (connection.GetStatus() == Connection::CGI_READY)
	{
		std::string	toCGIfileName("toCGI");

		int	toCGI = open(toCGIfileName.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0666);
		if (toCGI == -1)
		{
			throw 500;
		}

		int	fromCGI = open(fromCGIfileName.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0666);
		if (fromCGI == -1)
		{
			close(toCGI);
			throw 500;
		}
		int	cnt = write(toCGI, response->get_m_body().c_str(), response->get_m_body().length());
		if (cnt <= 0)
		{
			close(fromCGI);
			close(toCGI);
			throw 500;
		}
		if (lseek(toCGI, 0, SEEK_SET) == -1)
		{
			close(fromCGI);
			close(toCGI);
			throw 500;
		}

		char** envp = createCGIEnv(connection);
		if (envp == NULL)
		{
			close(fromCGI);
			close(toCGI);
			throw 500;
		}
		int pid = fork();
		if (pid < 0)
		{
			free(envp); envp = NULL;
			close(fromCGI);
			close(toCGI);
			throw 500;
		}
		else if (pid == 0)
		{
			dup2(toCGI, 0); dup2(fromCGI, 1);
			execve("cgi_tester", 0, envp);
			exit(0);
		}
		else
		{
			waitpid(pid, 0, 0);
			free(envp); envp = NULL;
			close(fromCGI);
			close(toCGI); //unlink(toCGIfileName.c_str());
			connection.SetStatus(Connection::CGI_ING);
		}
	}
	else
	{
		int	fromCGI = open(fromCGIfileName.c_str(), O_RDONLY, 0666);

		struct stat	statBuf;
		if (fstat(fromCGI, &statBuf) == -1)
		{
			close(fromCGI);
			throw 500;
		}

		char	buf[statBuf.st_size + 1];	// NOTE STACK영역보다 heap에 할당하는ㄱㅔ 더 많이 할당할수 있었떤 것같다 이부분에서 터질 우려도 있다.
		int		cnt = read(fromCGI, buf, sizeof(buf));
		buf[cnt] = 0;
		if (cnt <= 0)
		{
			close(fromCGI);
			throw 500;
		}

		// STUB 파싱과정 필요
		std::string fromCGI_str(buf);
	
		std::size_t seek = fromCGI_str.find("\r\n\r\n", 0);
		if (seek == std::string::npos)
		{
			throw 500;
		}
		std::size_t seek_cur = 0;
		while (true)
		{
			std::size_t	seek  = fromCGI_str.find("\r\n", seek_cur);
			if (seek == std::string::npos)
			{
				throw 500;
			}
			std::string	header = fromCGI_str.substr(seek_cur, seek - seek_cur);
			if (header.empty())
			{
				seek_cur += 2;
				break ;
			}
			std::cout << "\t|" << header << "|" << std::endl;

			// STUB header 넣는 부분. 값을 받아서 넣어줘야하는데, 그냥 수기로 넣어주고 있음.
			// Status: 200 OK
			// Content-Type: text/html; charset=utf-8
			std::size_t	found = header.find(": ");
			if (found == std::string::npos)
			{
				throw 500;
			}

			std::string	key = header.substr(0, found);
			std::locale loc;
			for(std::string::size_type i = 0; i < key.length(); i++)
			{
				key[i] = std::tolower(key[i], loc);
			}

			std::string	value = header.substr(found + 2);
			// REVIEW CGI program 에서 Chunked로 보내주기도 하나?
			// REVIEW 정보는 딱 두개 만 보내주는건가.
			// Status: 200 OK
			// Content-Type: text/html; charset=utf-8

			if (key == "status")
			{
				std::size_t found = value.find(" ");
				std::string status_code = value.substr(0, found);
				int statuscode = std::atoi(status_code.c_str());
				if (statuscode != 200)
				{
					delete (connection.get_m_response());
					//STUB CGI의 반환 header를 하나만 넣어준다면, 아래로 설정하기
					connection.set_m_response(new Response(&connection, statuscode, Response::makeStatusPage(statuscode, request.GetMethod())));

					// this->create_Response_statuscode(connection, statuscode);
					return ; // REVIEW throw 를 statuscode로 던져서 윗단에서 생성시키도록 해야하나.
				}
			}
			else
			{
				std::cout << "\t\t|" << key << "| |" << value << "|" << std::endl;
				connection.get_m_response()->set_m_headers(key, value);
			}
			seek_cur = seek + 2;
		}
		std::size_t seek_body = seek_cur;

		response->set_m_body(fromCGI_str.substr(seek_body));
		close(fromCGI); //unlink(fromCGIfileName.c_str());
		connection.SetStatus(Connection::SEND_READY);
	}
}