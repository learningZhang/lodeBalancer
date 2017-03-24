#include <iostream>
#include <string>
#include <map>
using namespace std;
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <event.h>
#include <json/json.h>
#include <errno.h>
#include <mysql/mysql.h>
#include <list>

class CMysql
{
	public:
		CMysql();
		bool insertIntoUser(const char *name, const char *passwd, const char *call);
		bool queryPasswd(const char *name, const char *passwd);
		bool deleteUser(const char *name);
		int getStates(const char *name);
		bool insertIntoStates(const char *name, int fd);
		bool insertInto_serverfd(int fd, int id);
		int  get_fd(int id);
		int  get_id(int fd);
	private:
		MYSQL *pcon;
	 	MYSQL_RES *pres;
		MYSQL_ROW row;
		
	 	const char *ip;
		const char *user;
		const char *passwd;
		unsigned short port;
};

int get_first(list<int> &x);

void Listenfd(evutil_socket_t fd, short int, void *arg);

void pthread_pool();

bool judge(int fd);

void server_start();

bool connect_server(int port, char *ip, int index);

void* thread_func(void *);

int select_server(int fd);

int deleteEvent(int epfd,int fd, struct epoll_event *event);

int addEvent(int epfd, int fd);

void ProcListenfd(evutil_socket_t fd, short , void *arg);

void* ReadThread(void *arg);

bool registe(int fd);

void* ReadThread(void *arg);

bool doLogin(int fd);

bool offline(int fd);//主动打招呼断开还是直接断开--》服务器的资源

//test the git
