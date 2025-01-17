#pragma once
# include "../../gnl/get_next_line_bonus.hpp"
# include "../Path/Path.hpp"
# include "../Utils/utils.hpp"
# include "../Server.hpp"
# include "../ConfigFile/ConfigFile.hpp"
# include <iostream>
# include <sys/types.h>
# include <fcntl.h>

# define START 0
# define END 1

class LocationBracket
{
public:
	int mStart;
	int mEnd;
	Path mLocationPath;
public:
	LocationBracket();
	virtual ~LocationBracket();
};

class ServerBracket
{
public:
	ServerBracket();
	virtual ~ServerBracket();
public:
	std::vector<LocationBracket> mLocationBracket;
	int mLocationBlockNum;
	// ANCHOR start = 0 & end = 0 이면, server Bracket 설정이 제대로 안된 상황
	int mStart;									// start = 0
	int mEnd;									// end = 0
};

class ServerConfigIdx
{
public:
	ServerConfigIdx();
	virtual ~ServerConfigIdx();

	// NOTE Bracket{}과 개행을 기준으로 index로 구역을 나눌 때 사용하는 함수들
	int Step1(ServerConfigIdx &configs, std::vector<std::string> &gnl);
	int Step2(ServerConfigIdx &configs, std::vector<std::string> &gnl);

	bool checkBracketSyntax(std::vector<std::string> &gnl, int start, int end);
	void SetServerBracket(std::vector<std::string> &gnl, ServerConfigIdx &configs, int start);
	void SetLocationBracket(std::vector<std::string> &gnl, ServerConfigIdx &configs, int server_idx, int start, int end);

	// NOTE string 맨 앞에 있는 '\t' 을 삭제시켜줌
	void outdentTab(std::string &str);
public :
	std::vector<ServerBracket> mServerBracket;
	int mServerNum;
	int mTotalLocationNum;
};

int parsingserverBlock(std::vector<std::string> &gnl, ConfigFile &default_location, int start, int end, ServerBracket &ServerBracket);
int parsingLocationBlock(std::vector<std::string> &gnl, ConfigFile &default_location, int start, int end);
int isCorrespondedServerDirective(int index, ServerBracket &ServerBracket);
