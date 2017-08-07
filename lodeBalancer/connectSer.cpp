#include "head.h"
#include "connectSer.h"

int ConnectServer::indexForServer=0;

ConnectServer::ConnectServer(const char *iptmp, int porttmp)
{
	ip = iptmp;
	port = porttmp;
	if (connect_server()==false)
	{
		cout<<"create error"<<endl;
	}
	++indexForServer;
}

ConnectServer::~ConnectServer()
{
	close(clientfd);//关闭端口
}

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
	if (ret == -1) 
	{
		cout<<"errer"<<endl;
		return false;
	}
	cout<<"connect successful !"<<endl;	
	/*serverfd[index] = clientfd; //连接上服务端*/
	return true;
}
