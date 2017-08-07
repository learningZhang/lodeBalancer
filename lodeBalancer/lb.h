#include <iostream>
using namespace std;

//和服务器建立连接的时候应该传入服务器的信息，服务器的端口，服务器的ip地址，服务器的编号
class ConnectServer
{
	public:
		ConnectServer(const char *iptmp, int *porttmp)
		{
			ip = iptmp;
			port = porttmp;
			if (connect_server(ip, port)==false)
			{
				cout<<"create error"<<endl;
			}
			++index;
		}
		~ConnectServer()
		{
			close(fd);//关闭端口
		}
		static int indexForServer;//进行计数
		int clientfd;	
	private:
		bool connect_server();
		const char *ip;
		int *port;
};
int ConnectServer::indexForServer=0;

//负载均衡器作为客户端和聊天服务器建立连接
bool ConnectServer::connect_server()
{	
	clientfd = socket(AF_INET, SOCK_STREAM, 0);
	if(clientfd == -1) {return false;}

	sockaddr_in caddr;
	memset(&caddr, 0, sizeof(caddr));
	
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	caddr.sin_addr.s_addr = inet_addr(ip);

	int ret = connect(clientfd, (sockaddr*)&caddr, sizeof(caddr));
	if (ret == -1) {return false;}
	
	/*serverfd[index] = clientfd; //连接上服务端*/
	return true;
}