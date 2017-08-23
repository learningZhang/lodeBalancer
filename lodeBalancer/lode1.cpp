#define _LODE_ 

#include "head.h"
#include "conHash.h"
#include "connectSer.h"
#include "lode.h"
#include "mysql.h"
#include "mutex.h"
#include <signal.h>
#include <errno.h>
#include "threadPool.h"

int serverfd[FD_NUM]={0};//服务器的fd
int ppfd[2]={0};

map<int, int> ser_to_cli;

CMutex mutex; //定义信号量对象，后面信号量实现管道文件描述符的互斥操作

CConHash *conhash;

void init_ConHash()
{
	CHashFun* func = new CMD5HashFun();
	conhash = new CConHash(func);
}

void *sig_thread(void *arg)
{
	sigset_t *set = (sigset_t*)arg;	
	int s,sig;
	for (;;)
	{
		s = sigwait(set, &sig);
		if (s == 0)
		{
			if(sig == SIGINT)
			{
				cout<<"kill all thread"<<endl;
				exit(1);
			}
			cout<<"i know"<<endl;
		}
		printf("signale handing thread got signal %d\n", sig);
	}
}

//主线程可以设置一些信号处理函数，用来接受一些外部发送的信号
int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd != -1);

	sockaddr_in saddr;
	memset(&saddr, 0, sizeof(sockaddr_in));
	
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(10001);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (-1 == bind(sockfd, (sockaddr*)&saddr, sizeof(saddr)))
	{
		cout<<"error in bind"<<endl;
		return -1;
	}
	
	if (-1 == listen(sockfd, 5))
	{
		cout<<"error in listen"<<endl;
		return -1;
	}
	init_ConHash();
	
	ConnectServer **server = server_start(conhash);//连接服务器，反向代理功能

    assert(pipe(ppfd) != -1);

//设置信号掩码，在创建子线程之前进行设置
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);//向信号集中加入sigint信号
	int  s = pthread_sigmask(SIG_BLOCK, &set,NULL);
	if (s != 0)
	{
		cout<<"error"<<endl;
	}
	pthread_t thread;
	s = pthread_create(&thread, NULL, sig_thread, &set);
//专门开辟了一个线程去处理信号

	CPthreadPool::createPthreadPool(THREAD_NUM, thread_func);//创建线程池
	
 	struct event_base *base = event_init();
	struct event *listen_event = event_new(base, sockfd, EV_READ|EV_PERSIST, Listenfd, NULL);
	
	event_add( listen_event, NULL );

   	cout<<"lodebalancer is started..."<<endl;
    
   	event_base_dispatch(base);

   	server_free(server);
   	event_free(listen_event);
   	event_base_free(base);
   	return 0;
}

void* thread_func(void *)
{
	int acceptCientNum = 0;
	int disable=acceptCientNum/8 - (MAX-acceptCientNum);
	//平衡因子，所有连接数量的除以8-空闲的连接数量
	
	CMysql db;
	int epollfd =  epoll_create(MAX);
	struct epoll_event events[MAX];       
	addEvent(epollfd, ppfd[0]);//还是会引起惊群现象

	for(int i=0; i<FD_NUM; ++i)
	{
		addEvent(epollfd, serverfd[i]);
	}

	char buff[1024];
	int ret = 0;
	int res = 0;

  	Json::Value response;
	Json::Reader reader;
	Json::Value root;
	
	while (true)
	{
	//	get_pipefd_clock(disable, epollfd, ppfd[0], mutex);//获取锁，成功由该线程接收新连接，否则其它线程接受，使用trylock，获取锁失败立即返回，锁是对数据进行保护
		if ((res = epoll_wait(epollfd, events, MAX, -1)) == -1)
		{        
			if (errno != EINTR)
			{
				return NULL;
			}
		}

		int pfd = 0;		
		for (int i=0; i<res; ++i)
		{			
			int fd = events[i].data.fd;
			if (events[i].data.fd == ppfd[0])
			{
				char pipebuff[10];
				//如果是ET模式
				//while(1)循环直至读完所有的事件
				
				if((ret = read(ppfd[0], pipebuff, 10)) == -1)
				{
					cout<<"get pipe file error"<<endl;
					continue;
				}
			
				addEvent(epollfd, atoi(pipebuff));//将新的连接描述符添加到epoll中，如果再有数据来就将其加入到某个服务器中
				++acceptCientNum;
				cout<<"pipe buff is "<<pipebuff<<endl;
			//	getoff_pipefd_clock(epollfd, fd, mutex); //将管道文件描述符从该epoll中剔除出去
			}

			else if (judge(fd))//服务器---lb
			{		
				ret = recv(fd, buff, 1024, 0);
				cout<<"recv from server "<<buff<<endl;
				
				//客户端和lb断开，客户端发送消息告诉服务器断开连接.然后服务器就对该采取行动
				if (ret == 0)
				{
					cout<<"with server break out"<<endl;
					close(fd);
					deleteEvent(epollfd, fd);//不进行acceptNum的改变
					//int new_given();
					continue;	
					//应该将所连接到该服务器上的所有客户端重新分配给剩下的客户端，然后continue;
				}
				else if(ret<0)
				{
					cout<<"recv error"<<endl;
					continue;
				}

				if (reader.parse(buff, root))
				{
					fd = root["FD"].asInt();
					ret = send(fd, root.toStyledString().c_str(), strlen(root.toStyledString().c_str())+1, 0);     
					if (ret <= 0)
					{
						cout<<"error"<<endl;
					}   
				}
			}
			else//客户端
			{		
				int serfd = 0;  	
				ret = recv(fd, buff, 1024, 0);
				if (ret == 0)
				{
					cout<<"with client break out"<<endl;
					serfd = serverfd[select_server(fd, conhash)];//通过一致性哈希根据键来找到对应的服务器fd
					//serfd = search_cli_to_ser_fd(fd);//通过数据库来查找是归于哪一个服务器--RR轮转
					if (serfd  == -1)//对方还没有获得处理其的服务器
					{
						cout<<"a client over and dont have serverfd"<<endl;
					}
					else
					{
						close(fd);
						db.deleteElemBySocket(fd, 1);
					}
					deleteEvent(epollfd, fd);
					--acceptCientNum;
					continue;
				}
				else if (ret < 0)
				{
					cout<<"recv error,errno is :"<<errno<<endl;
					continue;
				}

				cout<<"recv from client"<<buff<<endl;
				if(reader.parse(buff, response))
				{
					serfd = search_cli_to_ser_fd(fd);
					if (serfd == -1)
					{
						int index = select_server(fd, conhash);//选择服务器++++++++++++++++++
						//insert_clifd_serfd(fd, serverfd[index]);//RR轮转就不用数据库
						serfd = serverfd[index];
					}
					response["FD"] = fd;
					ret = send(serfd, response.toStyledString().c_str(), strlen(response.toStyledString().c_str())+1, 0);     
					if (ret == -1)
					{
						cout<<"error"<<endl;
					}   
				}              
			}	
		}
	}	
}

void Listenfd(evutil_socket_t fd, short int , void *arg)//主线程
{
	CMysql db;
    sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (sockaddr*)&client, &len);//主线程在此处接受到连接，然后通过管道发送给从线程
    assert(clientfd != -1);
    cout<<"new client connected! client info:"<<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;
    char pipe_buff[10]={0};
    sprintf(pipe_buff,"%d", clientfd);
	
    if (write(ppfd[1], pipe_buff, strlen(pipe_buff)+1) == -1)
    {
	cout<<"write to pipe error"<<endl;
    }
}
//负载均衡器作为客户端和聊天服务器建立连接

ConnectServer** server_start(CConHash* &conhash)//试图用设计模式中的模式来将其处理的更加平滑
{
	FILE * fptr = fopen(".//config.txt", "r");//以只读方式打开
	assert(fptr !=  NULL);
	ConnectServer **server = new ConnectServer*[FD_NUM];
	
	int num;fscanf(fptr, "%d", &num);
	int i=0;
	CNode_s * node = NULL;
	//服务器的配置：服务器名称,ip地址,port端口,虚拟结点的个数
	char machinename[20];
	char ip[20];
	int port;
	int virtualNodeNum;
	                   
	while (i<num )
	{
		fscanf(fptr, "%s %s %d %d", machinename, ip, &port, &virtualNodeNum);
		server[i] = new ConnectServer(ip, port);
		node = new CNode_s(machinename, virtualNodeNum, ip);//建立一个结点
		conhash->addNode_s(node); 
		++i;
	}
	
	for (int i=0; i<FD_NUM; ++i)
	{
		serverfd[i] = server[i]->clientfd;
	}	
	
	fclose(fptr);
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
	setnomblocking(fd);
}

void reset_oneshot(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLONESHOT | EPOLLIN;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
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
