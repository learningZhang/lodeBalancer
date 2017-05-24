//for file lode1
#include "head.h"
<<<<<<< HEAD
extern int serverfd[FD_NUM];
extern int ppfd[2];
map<int, int> ser_to_cli;
=======

extern map<int, int> ser_to_cli;
>>>>>>> dev

void server_start()//向第index的服务器发送数据
{
	int port = 10000;//与聊天服务器相对应的客户端
	char *ip = "127.0.0.1";
	for(int i=0; i<FD_NUM; i++)
	{
		int ret = connect_server(port, ip, i);
		if (ret == -1)
		{
			cout<<"server "<<ret<<" can't open"<<endl;
			return ;
		}
		cout<<"server["<<i<<"]   open"<<endl;
	}
}

bool connect_server(int port, char *ip, int index)
{
	int clientfd = socket(AF_INET, SOCK_STREAM, 0);
	if(clientfd == -1) {return false;}

	sockaddr_in caddr;
	memset(&caddr, 0, sizeof(caddr));
	
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	caddr.sin_addr.s_addr = inet_addr(ip);

	int ret = connect(clientfd, (sockaddr*)&caddr, sizeof(caddr));
	if (ret == -1) {return false;}
	
	serverfd[index] = clientfd; //连接上服务端
	return true;
}

void pthread_pool()
{
	pthread_t tid;
	for(int i=0; i<THREAD_NUM; i++)
	{
		int res = pthread_create(&tid, NULL, thread_func, NULL);
		assert(res != -1);
		cout<<"pthread["<<i<<"]  open"<<endl;
	}
}

<<<<<<< HEAD
bool judge(int fd)
=======
bool judge(int fd)//来自客户端就返回假
>>>>>>> dev
{
	for(int i=0; i<FD_NUM; ++i)
	{
		if(serverfd[i] == fd)
		{
<<<<<<< HEAD
			return true;//fd是服务器
		}
	}
	return false;//fd是客户端
}

static int tag = 0;

=======
			return true;
		}
	}
	return false;
}

static int tag = 0;
>>>>>>> dev
int select_server(int fd)
{
	return tag = (tag+1)%3;
}

int deleteEvent(int epfd,int fd, struct epoll_event *event)
{
<<<<<<< HEAD
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, event);
=======
	epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event);
>>>>>>> dev
}

int setnomblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

int addEvent(int epfd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1)
	{
		perror("epoll_ctl :listen_sock");
		exit(-1);
	}
}

int search_cli_to_ser_fd(int fd)
{
	map<int, int>::iterator it = ser_to_cli.find(fd);
	if (it == ser_to_cli.end())
	{
		return -1;
	}
	return it->second;
}
//根据客户端的fd的值来搜索当前数据库中是否有与之对应的服务器端，若是有的话就查找服务器的fd，否则就返回-1,让客户端进行分配

void insert_clifd_serfd(int clientfd, int serverfd)//将客户端来的作为主键，插入<clientfd,serverfd>
{
	ser_to_cli[clientfd] = serverfd;
}