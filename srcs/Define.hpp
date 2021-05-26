#ifndef DEFINE_HPP
# define DEFINE_HPP

#define DEFAULT_CONFIG_FILE_PATH "default.config"
#define BUFFER_SIZE 100000
#define INIT_FD_MAX 512
#define KEEP_ALIVE_LIMIT 10 // TODO 갱신처리 안됬음!
#define SELECT_TIMEOUT_SEC 3
#define SELECT_TIMEOUT_USEC 0

#define BUFIZE_HTMLFILE 300

#define NC "\e[0m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define REDB "\e[41m"
#define GRNB "\e[46m"

enum TYPE_HTML
{
	ERROR_HTML,
	INDEX_HTML,
	ANY_FILE
};

#endif
