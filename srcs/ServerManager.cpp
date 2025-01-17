#include "ServerManager.hpp"

ServerManager::ServerManager(void)
	: mMaxFd(INIT_FD_MAX)
	, mTotalClients(0)
{
	FT_FD_ZERO(&mReadFds);
	FT_FD_ZERO(&mWriteFds);
}

void		ServerManager::ExitServer(const std::string& msg) const
{
	std::cerr << REDB "[" << ft::getCurrentTime() << "] [error] " << msg << NC << std::endl;
	exit(1);
}

void		ServerManager::CreateServers(const std::string& configurationFilePath)
{
	// ANCHOR 1단계 parsing 전처리단계
	ConfigFiles configFiles(configurationFilePath.c_str());
	if (this->isOverlapedServer(&configFiles) == true)
		throw (static_cast<const std::string>("Overlaped port and host"));
	configFiles.ShowConfigs(); // NOTE configfile의 값을 확인하고싶으면,

	// ANCHOR 2단계 parsing
	this->setServersValue(&configFiles);
	this->showServers(); // NOTE server로 구성된 값을 확인하고싶으면,

	// // ANCHOR 3단계 parsing -> port에 대해서 listen 함수까지 호출함.
	this->setServers();
}

void		ServerManager::RunServers(void)
{
	struct timeval	timeOut;
	for (std::vector<Server>::iterator it = mServers.begin(); it != mServers.end(); ++it)
	{
		FT_FD_SET(it->GetSocket(), &mReadFds);
	}
	while (true)
	{
		timeOut.tv_sec = SELECT_TIMEOUT_SEC; timeOut.tv_usec = SELECT_TIMEOUT_USEC;

		FT_FD_COPY(&mReadFds, &mReadCopyFds);
		FT_FD_COPY(&mWriteFds, &mWriteCopyFds);
		updateMaxFd();
		int	cnt = select(mMaxFd + 1, &mReadCopyFds, &mWriteCopyFds, NULL, &timeOut);
		if (cnt < 0)
		{
			for (std::vector<Server>::iterator it = mServers.begin() ; it != mServers.end() ; ++it)
			{
				serviceUnavailable(it);
			}
		}
		else if (cnt == 0)
		{
			for (std::vector<Server>::iterator it = mServers.begin() ; it != mServers.end() ; ++it)
			{
				closeOldConnection(it);
			}
		}
		else if (cnt > 0)
		{
			std::cout << "select : " << cnt << std::endl;
			std::vector<int> readSet; std::cout << "--- read ---" << std::endl;
			readSet = ft::getVectorChangedFD(&mReadCopyFds);
			std::vector<int> writeSet; std::cout << "--- write ---" << std::endl;
			writeSet = ft::getVectorChangedFD(&mWriteCopyFds);
			std::cout << "-------------------------------" << std::endl;

			for (std::vector<Server>::iterator it = mServers.begin() ; it != mServers.end() ; ++it)
			{
				it->run();
			}
		}
	}
}

void		ServerManager::updateMaxFd(void)
{
	if (gTotalClients == 0)
	{
		mMaxFd = 2 + this->mServers.size();
	}

	// REVIEW 하향식으로 전체 탐색하는 방식(단, 상향식으로는 체크할 수 없음)
	// for (int i = INIT_FD_MAX; i >= 0; --i)
	// {
	// 	// if (fdIsset(i, READ_SET) || fdIsset(i, WRITE_SET))
	// 	if (FT_FD_ISSET(i, &mReadFds) || FT_FD_ISSET(i, &mWriteFds))
	// 	{
	// 		mMaxFd = i;
	// 		break ;
	// 	}
	// }
}

int ServerManager::setServersValue(ConfigFiles *configFiles)
{
	while (configFiles->mConfigs.size() != 0)
	{
		ConfigFile &config = configFiles->mConfigs[configFiles->mConfigs.size() - 1];
		int idxserver;
		if (-1 == (idxserver = this->getIdxServer(config.mPort, config.mHost)))
		{
			Server server(this);
			server.mPort = config.mPort;
			server.mHost = config.mHost;

			serverBlock temp;
			temp.mserverName = config.mServerName;
			temp.mTimeOut = config.mTimeOut;
			temp.mCgiExtension = config.mCgiExtension;
			server.mServerBlocks.push_back(temp);

			LocationPath temp2;
			temp2.mLocationPath = config.mLocationPath;
			temp2.mRoot = config.mRoot;
			temp2.mIndexPages = config.mIndexPages;
			temp2.mErrorPage = config.mErrorPage;
			temp2.mAutoIndex = config.mAutoIndex;
			temp2.mClientMaxBodySize = config.mClientMaxBodySize;
			temp2.mAuthBasicUserFile = config.mAuthBasicUserFile;
			temp2.mCgiProgramPath = config.mCgiProgramPath;
			temp2.mMethod = config.mMethod;
			server.mServerBlocks[0].mlocationPaths.push_back(temp2);

			this->mServers.push_back(server);
			configFiles->mConfigs.pop_back();
			continue;
		}
		Server &server = this->mServers[idxserver];
		int idxserverBlock;
		if (-1 == (idxserverBlock = this->getIdxServerBlock(server.mServerBlocks, config.mServerName)))
		{
			serverBlock temp;
			temp.mserverName = config.mServerName;
			temp.mTimeOut = config.mTimeOut;
			temp.mCgiExtension = config.mCgiExtension;
			server.mServerBlocks.push_back(temp);

			LocationPath temp2;
			temp2.mLocationPath = config.mLocationPath;
			temp2.mRoot = config.mRoot;
			temp2.mIndexPages = config.mIndexPages;
			temp2.mErrorPage = config.mErrorPage;
			temp2.mCgiProgramPath = config.mCgiProgramPath;
			temp2.mMethod = config.mMethod;
			temp2.mAutoIndex = config.mAutoIndex;
			temp2.mAuthBasicUserFile = config.mAuthBasicUserFile;
			temp2.mClientMaxBodySize = config.mClientMaxBodySize;
			server.mServerBlocks[server.mServerBlocks.size() - 1].mlocationPaths.push_back(temp2);
			configFiles->mConfigs.pop_back();
			continue;
		}
		serverBlock &temp = server.mServerBlocks[idxserverBlock];
		int idxlocationblock;
		if (-1 == (idxlocationblock = this->getIdxLocationPath(temp.mlocationPaths, config.mLocationPath)))
		{
			LocationPath temp2;
			temp2.mLocationPath = config.mLocationPath;
			temp2.mRoot = config.mRoot;
			temp2.mIndexPages = config.mIndexPages;
			temp2.mErrorPage = config.mErrorPage;
			temp2.mCgiProgramPath = config.mCgiProgramPath;
			temp2.mMethod = config.mMethod;
			temp2.mAuthBasicUserFile = config.mAuthBasicUserFile;
			temp2.mAutoIndex = config.mAutoIndex;
			temp2.mClientMaxBodySize = config.mClientMaxBodySize;
			server.mServerBlocks[server.mServerBlocks.size() - 1].mlocationPaths.push_back(temp2);
			configFiles->mConfigs.pop_back();
			continue;
		}
		// NOTE 세개가 모두 같은 경우는 에러처리한다.
		configFiles->mConfigs.pop_back();
		return (-1);
	}
	return (1);
}

int ServerManager::getIdxServer(const int& port, const std::string& host)
{
	if (this->mServers.size() == 0)
		return (-1);
	for (size_t i = 0; i < mServers.size(); i++)
	{
		if (this->mServers[i].mPort == port && this->mServers[i].mHost == host)
			return (i);
	}
	return (-1);
}

int ServerManager::getIdxServerBlock(const std::vector<serverBlock> &serverBlocks, const std::string& serverName)
{
	if (serverBlocks.size() == 0)
		return (-1);
	for (size_t i = 0; i < serverBlocks.size(); i++)
	{
		serverBlock temp = serverBlocks[i];
		if (temp.mserverName.compare(serverName) == 0)
			return (i);
	}
	return (-1);
}

int ServerManager::getIdxLocationPath(std::vector<LocationPath>& locationPaths, const Path& locationPath)
{
	if (locationPaths.size() == 0)
		return (-1);
	for (size_t i = 0; i < locationPaths.size(); i++)
	{
		LocationPath &temp = locationPaths[i];
		if (temp.mLocationPath == locationPath)
			return (i);
	}
	return (-1);
}

int ServerManager::showServers(void)
{
	for (size_t i = 0; i < this->mServers.size(); i++)
	{
		Server &temp = this->mServers[i];
		std::cout << "port: " << temp.mPort << std::endl;
		std::cout << "host: " << temp.mHost << std::endl;
		for (size_t j = 0; j < temp.mServerBlocks.size(); j++)
		{
			serverBlock temp2 = temp.mServerBlocks[j];
			std::cout << "server_name: " << temp2.mserverName << std::endl;
			std::cout << "timeout: " << temp2.mTimeOut << std::endl;
			std::cout << "cgi_program_path: ";
			for (size_t j = 0; j < temp2.mCgiExtension.size(); j++)
			{
				std::cout << temp2.mCgiExtension[j] << " ";
			}
			std::cout << std::endl;

			std::cout << temp2.mserverName << ": " << "locationPathsize: " << temp2.mlocationPaths.size() << std::endl;
			for (size_t k = 0; k < temp2.mlocationPaths.size(); k++)
			{
				LocationPath temp3 = temp2.mlocationPaths[k];
				std::cout << "mLocationPath: " << temp3.mLocationPath.getPath() << std::endl;
				std::cout << "error_page: " << temp3.mErrorPage.getPath() << std::endl;
				std::cout << "auth_basic_user_file: " << temp3.mAuthBasicUserFile.getPath() << std::endl;
				std::cout << "root: " << temp3.mRoot.getPath() << std::endl;
				std::cout << "auto_index: " << temp3.mAutoIndex << std::endl;
				std::cout << "client_max_body_size: " << temp3.mClientMaxBodySize << std::endl;

				for (size_t l = 0; l < temp3.mIndexPages.size(); l++)
				{
					std::cout << "index_pages: " << temp3.mIndexPages[l].getPath() << std::endl;	/* code */
				}
				std::cout << "method: ";
				for (size_t j = 0; j < temp3.mMethod.size(); j++)
				{
					std::cout << temp3.mMethod[j] << " ";
				}
				std::cout << std::endl;
				std::cout << "cgi_program_path: ";
				for (size_t j = 0; j < temp3.mCgiProgramPath.size(); j++)
				{
					std::cout << temp3.mCgiProgramPath[j] << " ";
				}
				std::cout << std::endl;
				std::cout << "------------------------------ locationPath" << std::endl;
			}
			std::cout << "============================== server name" << std::endl;
		}
		std::cout << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- port" << std::endl;
	}
	return (1);
}

int ServerManager::setServers(void)
{
	for (size_t i = 0; i < this->mServers.size(); i++)
	{
		Server &server = this->mServers[i];
		server.SetSocket();
		server.mManager = this;
		server.mConnections[server.mSocket] = Connection(server.mSocket, server.mHost, server.mPort);
	}
	return (1);
}

std::vector<Server>&	ServerManager::GetServers(void) {return (this->mServers);}

void	ServerManager::closeOldConnection(const std::vector<Server>::iterator& serverIterator)
{
	std::map<int, Connection>::iterator it = serverIterator->mConnections.begin();
	while (it != serverIterator->mConnections.end())
	{
		std::map<int, Connection>::iterator it2 = it++;
		int fd = it2->first;
		if (it2->second.GetSocket() == serverIterator->mSocket)
		{
			continue ;
		}
		if (it2->second.IsKeepConnection() == false && (FT_FD_ISSET(fd, &this->mReadCopyFds) == 0))
		{
			if (it2->second.GetRequest() == NULL)
			{
				serverIterator->createResponseStatusCode(it2->second, 408);
				it2->second.GetResponse()->setHttpMessage(it2->second.GetResponse()->makeHttpMessage());
				write(it2->first, it2->second.GetResponse()->GetHttpMessage().c_str(), it2->second.GetResponse()->GetHttpMessage().length());
			}
			serverIterator->closeConnection(it2->second.GetSocket());
		}
	}
}

void	ServerManager::serviceUnavailable(const std::vector<Server>::iterator& serverIterator)
{
	std::map<int, Connection>::iterator it = serverIterator->mConnections.begin();
	while (it != serverIterator->mConnections.end())
	{
		std::map<int, Connection>::iterator it2 = it++;
		if (it2->second.GetSocket() == serverIterator->mSocket)
		{
			continue ;
		}
		serverIterator->createResponseStatusCode(it->second, 503);
		it->second.GetResponse()->setHttpMessage(it->second.GetResponse()->makeHttpMessage());
		write(it->first, it->second.GetResponse()->GetHttpMessage().c_str(), it->second.GetResponse()->GetHttpMessage().length());
		serverIterator->closeConnection(it2->second.GetSocket());
	}
	std::cout << "Close all connections in Servers" << std::endl;
}

class OverlapedValue
{
	public:
		std::string mHost;
		uint16_t mPort;
		std::string mserverName;
		OverlapedValue(std::string a, uint16_t b, std::string c)
			: mHost(a)
			, mPort(b)
			, mserverName(c)
		{};

		bool operator < (const OverlapedValue& t) const
		{
			if (mHost < t.mHost || mPort < t.mPort || mserverName < t.mserverName)
				return true;
			else
				return false;
		}
		bool operator > (const OverlapedValue& t) const
		{
			if (mHost > t.mHost || mPort > t.mPort || mserverName > t.mserverName)
				return true;
			else
				return false;
		}
		bool operator == (const OverlapedValue& t) const
		{
			if (mHost == t.mHost && mPort == t.mPort && mserverName == t.mserverName)
				return true;
			else
				return false;
		}
};

bool	ServerManager::isOverlapedServer(ConfigFiles* configFiles)
{
	std::vector<OverlapedValue> serverList;
	for (size_t i = 0; i < configFiles->mConfigs.size(); i++)
	{
		ConfigFile &config = configFiles->mConfigs[i];
		OverlapedValue temp(config.mHost, config.mPort, config.mServerName);
		for (size_t j = 0; j < serverList.size(); j++)
		{
			if (serverList[j] == temp)
			{
				if (config.mLocationPath.getSize() == 0)
					return (true);
			}
		}
		if (config.mLocationPath.getSize() == 0)
			serverList.push_back(temp);
	}
	return (false);
}

const fd_set&				ServerManager::GetReadCopyFds(void) const
{
	return (mReadCopyFds);
}

void						ServerManager::SetReadCopyFds(const int& fd)
{
	FT_FD_SET(fd, &mReadCopyFds);
}

void						ServerManager::ClrReadCopyFds(const int& fd)
{
	FT_FD_CLR(fd, &mReadCopyFds);
}

const fd_set&				ServerManager::GetReadFds(void) const
{
	return (mReadFds);
}

void						ServerManager::SetReadFds(const int& fd)
{
	FT_FD_SET(fd, &mReadFds);
}

void						ServerManager::ClrReadFds(const int& fd)
{
	FT_FD_CLR(fd, &mReadFds);
}

const fd_set&				ServerManager::GetWriteCopyFds(void) const
{
	return (mWriteCopyFds);
}

void						ServerManager::SetWriteCopyFds(const int& fd)
{
	FT_FD_SET(fd, &mWriteCopyFds);
}

void						ServerManager::ClrWriteCopyFds(const int& fd)
{
	FT_FD_CLR(fd, &mWriteCopyFds);
}

const fd_set&				ServerManager::GetWriteFds(void) const
{
	return (mWriteFds);
}

void						ServerManager::SetWriteFds(const int& fd)
{
	FT_FD_SET(fd, &mWriteFds);
}

void						ServerManager::ClrWriteFds(const int& fd)
{
	FT_FD_CLR(fd, &mWriteFds);
}

const int&					ServerManager::GetMaxFd(void) const{ return (this->mMaxFd);}
void						ServerManager::SetMaxFd(const int &fd)
{
	this->mMaxFd = fd;
}

const uint16_t&					ServerManager::GetTotalClients(void) const{ return (this->mTotalClients);}
void							ServerManager::SetTotalClients(const uint16_t &totalClient)
{
	this->mTotalClients = totalClient;
}
