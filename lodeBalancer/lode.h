#ifdef _LODE_
void server_free(ConnectServer **server);

ConnectServer **server_start();

void server_free(ConnectServer **server);

void Listenfd(evutil_socket_t fd, short int, void *arg);
#endif 


bool connect_server(int port, char *ip, int index);


void pthread_pool();

bool judge(int fd);

int select_server(int fd);

int addEvent(int epfd, int fd);

bool judge(int fd);

int search_cli_to_ser_fd(int fd);

int select_server(int fd);

void insert_clifd_serfd(int clientfd, int serverfd);//将客户端来的作为主键，插入<clientfd,serverfd>

int deleteEvent(int epfd,int fd, struct epoll_event *event);

int setnomblocking(int fd);

int addEvent(int epfd, int fd);//将文件描述符加入到epoll

void* thread_func(void *);

#define FD_NUM 3
#define THREAD_NUM 3






