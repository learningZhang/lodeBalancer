#include "head.h"

//lb的主线程
#define THREAD_NUM  3
#define MAX 1000 //单线程中一个epoll最多可以接受的文件描述符
#define FD_NUM 3 //服务器的个数

int serverfd[FD_NUM];
static int id = 0;//map<int, int> fdLog;//用于记录fd和id的对应关系

list<int> worklist;
pthread_mutex_t mutex;

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

	list<int> worklist;

	int res = pthread_mutex_init(&mutex, NULL);
	assert(res != -1);

 	struct event_base *base = event_init();

	struct event *listen_event = event_new(base, sockfd, EV_READ|EV_PERSIST, Listenfd, NULL);
	
    event_add( listen_event, NULL );

    server_start();//启动服务器
    pthread_pool();//建立线程池
    
    cout<<"lb started..."<<endl;
    
    event_base_dispatch(base);
    event_free(listen_event);
    event_base_free(base);
    
    return 0;
}

int get_first(list<int> x)
{	
	if (x.empty())
	{
		return -1;
	}
	pthread_mutex_lock(&mutex);
	list<int>::iterator it = x.begin();
	int a = *it;
	x.pop_front();
	pthread_mutex_unlock(&mutex);
	return a;
}

void Listenfd(evutil_socket_t fd, short int , void *arg)
{
	sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (sockaddr*)&client, &len);
    cout<<"new client connect server! client info:"<<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;
    //在此将fd传递给线程池中的线程  请求队列--先进先出
    pthread_mutex_lock(&mutex);      //栈--替换队列
    
    CMysql db;
    worklist.push_back(clientfd);//向对列尾部添加数据
    //fdLog[id] = clientfd;//向map中添加fd--id的记录
	db.insertInto_serverfd(id, fd);//向map中添加fd--id的记录
	id++;
    pthread_mutex_unlock(&mutex);
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

bool judge(int fd)//来自客户端就返回假
{
	for(int i=0; i<FD_NUM; ++i)
	{
		if(serverfd[i] == fd)
		{
			return true;
		}
	}
	return false;
}

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

void* thread_func(void *)
{
	//线程从工作队列中拿取任务，拿到任务可以进行下面活动
	//如何确保有任务的时候拿，没任务的时候不拿？？？？？？？
	int pfd = get_first(worklist);
	CMysql db;
	int epollfd =  epoll_create(50);
	epoll_event events[50];
	int res = 0;
	while(true)
	{
		res = epoll_wait(epollfd, events, MAX, -1);
		char buff[1024];
		int ret = 0;

		Json::Value response;
		Json::Reader reader;
		Json::Value tempstr;
		Json::Value root;

		for(int i=0; i<res; ++i)
		{			
			int fd = events[i].data.fd;
			ret = recv(fd, buff, 1024, 0);
			cout<<"recv buff"<<buff<<endl;
			if(judge(fd))//服务器来的数据：服务器来的数据上要先解封在发给客户端
			{						//考虑数据包的大小，防止分包
				if (ret <= 0)
				{
					cout<<"server down"<<endl;
					continue ;//设置 set_fd(-1) 
				}
				
				reader.parse(buff, tempstr);
				const char *temp =tempstr["ID"].asString().c_str();
				int fd = db.get_fd(atoi(temp));
				                   //将信息发送给相应的客户端
				cout<<"from server  to client"<<tempstr["mesg"].asString().c_str()<<endl;
				ret = send(fd, tempstr["mesg"].asString().c_str(), strlen(tempstr["mesg"].asString().c_str()), 0);
				if (ret <= 0){cout<<"send error"<<endl;}
			}
			else//客户端：数据加封，然后再进行发送
			{
				int index = select_server(fd);
				if (ret <= 0)
				{
					deleteEvent(epollfd, fd, events);//接收失败，断掉了连接
					continue;
				}
      
				int id_num = db.get_id(fd);
				char temp[10];
				sprintf(temp, "%d", id_num);
							
				response["ID"] = temp;
				response["mesg"] = buff;
				
				cout<<"from client to server["<<index<<"]   "<<response.asString().c_str()<<endl;
				ret = send(serverfd[index], response.asString().c_str(), strlen(response.asString().c_str()), 0);     
				
				if (ret <= 0)
				{
					cout<<"error"<<endl;
				}                 
			}	
		}
	}	
}


//<key , value>--一个根据key找value,一个根据value找key.

//如果是sockfd发来的信息呢--》记录fd, 怎么记录这个fd，采用什么格式来记录
//如何处理服务器连接lb时候的fd-->这个是我主动connect服务器吧



static int tag = 0;
int select_server(int fd)
{
	return tag = (tag+1)%3;
}

int deleteEvent(int epfd,int fd, struct epoll_event *event)
{
	epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event);
}

int setnomblocking(int fd);//将文件描述符设置为非阻塞的
int addEvent(int epfd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;//添加读事件
	epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
	setnomblocking(fd);//将文件描述符设置为非阻塞
}

int setnomblocking(int fd)//将文件描述符设置为非阻塞的
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

//如今的问题是，如何使得lb的服务端能正确的向相应的客户端发送消息
