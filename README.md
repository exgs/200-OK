## How to run
1. `make` or `make re`
2. `./webserv` or `./webserv test.config` or `./webserv multiServer.config`
	- argv[1] != NULL 이면, default.config로 프로그램이 실행됨.
	- 단, multiServer.config로 실행시키기위해서는 클러스터의 아이맥마다 달라지는 private IP 을 확인해야함.
## How to change setting
1. srcs/Define.hpp
	- Change **#define** value
		- BUFFER_SIZE 1000000
		- define KEEP_ALIVE_LIMIT 10
		- define SELECT_TIMEOUT_SEC 3
		- define SELECT_TIMEOUT_USEC 0
2. `.config` 파일 수정
	- [Rule](https://www.notion.so/nginx-4aa332c284a74f7481c7458472a42a9e)
	- if you don't know, contact [yunslee](https://profile.intra.42.fr/users/yunslee)
