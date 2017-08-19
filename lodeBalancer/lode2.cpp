//for file lode1
#include "head.h"
#include "conHash.h"
#include "connectSer.h"
#include "lode.h"

extern int serverfd[FD_NUM];//均衡算法中的轮转辅助数组
extern int ppfd[2];
map<int, int> ser_to_cli;

//负载均衡器作为客户端和聊天服务器建立连接

ConnectServer** server_start(CConHash* &conhash)//试图用设计模式中的模式来将其处理的更加平滑
{
	char *ip1 = "127.0.0.2";
	char *ip2 = "127.0.0.3";
	char *ip3 = "127.0.0.4";
	ConnectServer **server = new ConnectServer*[FD_NUM];
	
	server[0] = new ConnectServer(ip1, 6002);//连接到一号服务器
	server[1] = new ConnectServer(ip2, 6003);//连接到二号服务器
	server[2] = new ConnectServer(ip3, 6004);//连接到三号服务器

	CNode_s * node1 = new CNode_s("machineA", 280, ip1);//建立一个结点
	CNode_s * node2 = new CNode_s("machineB", 280, ip2);
	CNode_s * node3 = new CNode_s("machineC", 280, ip3);

	conhash->addNode_s(node1); 
	conhash->addNode_s(node2);
	conhash->addNode_s(node3);
	
	for (int i=0; i<FD_NUM; ++i)
	{
		serverfd[i] = server[i]->clientfd;
	}	
	return server;
}

void server_free(ConnectServer**server)
{
	for (int i=0;i<FD_NUM; ++i)
	{
		delete server[i];
	}
	delete [] server;
}

//模拟的时候，在负载均衡的进程中开了三个连接到服务器的连接，然后通过负载均衡算法将通过这三个
//不同的连接socket将消息发送到同一个服务器,用此来减轻调试过程中的复杂度

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

bool judge(int fd)
{
	for(int i=0; i<FD_NUM; ++i)
	{
		if(serverfd[i] == fd)
		{
			return true;//fd是服务器
		}
	}
	return false;//fd是客户端
}

static int tag = 0;
int select_server(int fd, CConHash*&conhash)
{
	//根据fd找到ip地址
	sockaddr addr;
	socklen_t lenth = sizeof(addr);
	int x = getpeername(fd, &addr, &lenth);
	if (x == 0)
	{
		CNode_s *node;
		node = conhash->lookupNode_s(inet_ntoa((*(sockaddr_in*)&addr).sin_addr));
		if(strcmp(node->getIden(),"machineA")==0) return 0;
		if(strcmp(node->getIden(),"machineB")==0) return 1;
		if(strcmp(node->getIden(),"machineC")==0) return 2;
	}
	else
		return tag = (tag+1)%3;//返回的是序号
}

int deleteEvent(int epfd,int fd)
{
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
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
