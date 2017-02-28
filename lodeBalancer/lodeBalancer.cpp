//lb的主线程
#define PORT 10000
#define IP  "172.0.0.1"
#define THREAD_NUM  10 
#define MAX 1000 //单线程中一个epoll最多可以接受的文件描述符
#define FD_NUM 3 //服务器的个数
int serverfd[FD_NUM];

int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd != -1);

	sockaddr_in saddr;
	memset(saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORT);
	saddr.sin_addr.s_addr = inet_addr(IP);

	int res = bind(sockfd, (sockaddr*)&saddr, sizeof(saddr));
	assert(res != -1);

	listen(sockfd, 5);

	list<int> worklist;
	pthread_mutex_t mutex;
	assert(res != -1);
	
	res = pthread_mutex_init(&mutex, NULL);
	
 	struct event_base *base = event_init();

	struct event *listen_event = event_new(base, listenfd, EV_READ|EV_PERSIST, Listenfd, NULL);
	
    event_add( listen_event, NULL );
    
    cout<<"server started..."<<endl;
    event_base_dispatch(base);
    event_free(listen_event);
    event_base_free(base);
    
    return 0;
}

int get_first(list<int> x)
{	
	pthread_mutex_lock(&mutex);
	if (x.empty())
	{
		return -1;
	}
	list<int>::iterator it = x.begin();
	int a = *it;
	x.pop_front();
	pthread_mutex_unlock(&mutex);
	return a;
}

void Listenfd(evutil_socket_t fd, short , void *arg)
{
	sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (sockaddr*)&client, &len);
    cout<<"new client connect server! client info:"<<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;
    //在此将fd传递给线程池中的线程  请求队列--先进先出
    pthread_mutex_lock(&mutex);
    worklist.push_back(clientfd);
    pthread_mutex_unlock(&mutex);
}

void pthread_pool()
{
	pthread_t tid;
	for(int i=0; i<THREAD_NUM; i++)
	{
		int res = pthread_create(&tid, NULL, thread_func, NULL);
		assert(res != -1);
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

void connect_server(int port, char *ip, int index)
{
	int port;
	char *ip;
	int clientfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(clientfd != -1);

	sockaddr_in caddr;
	memset(caddr, 0, sizeof(caddr));
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	caddr.sin_addr.s_addr = inet_addr(ip);

	int ret = connect(clientfd, (sockaddr*)&caddr, sizeof(caddr));
	assert(ret != -1);
	
	serverfd[index] = clientfd; //连接上服务端
}

void thread_func()
{
	int pfd = get_first();
	int epollfd =  epoll_create(6500);
	epoll_event events;
	int res = 0;
	while(true)
	{
		res = epoll_wait(epollfd, events, MAX, -1);
		char buff[1024];
		int ret = 0;
		for(int i=0; i<res; ++i)//客户端和服务端
		{
			int fd = eventns[i].data.fd;
			if(judge(fd))//来自服务器的数据
			{
				ret = recv(fd, buff, 1024, 0);
				if (ret <= 0)
				{
					//与服务器断开了
					cout<<"server down"<<endl;
					return ;//设置 set_fd(-1) 
				}
				send();
			}
			else
			{
				int index = select_server();
				
				ret = recv(fd, buff, 1024, 0);
				if (ret <= 0)
				{
					deleteEvent(epollfd, fd);//接收失败，断掉了连接
					continue;
				}				                                 

				ret = send(serverfd[index], buff, sizeof(buff), 0);     
				if (ret <= 0)
				{
					printf("error\n");
				}                     
			}
		}
	}	
}

//如果是sockfd发来的信息呢--》记录fd, 怎么记录这个fd，采用什么格式来记录
//如何处理服务器连接lb时候的fd-->这个是我主动connect服务器吧

static int tag = 0;
int select_server(int fd)
{
	return tag = (tag+1)%3;
}

int deleteEvent(int epfd, int op, int fd, struct epoll_event *event)
{
	epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event);
}

int addEvent(int epfd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;//添加读事件
	epoll_ctl(epfd, EPOLL_CTL_ADD, fd, event);
	setnonblocking(fd);//将文件描述符设置为非阻塞
}

int setnomblocking(int fd)//将文件描述符设置为非阻塞的
{
	int ols_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}