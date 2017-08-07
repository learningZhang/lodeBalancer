class ConnectServer
{
	public:
		ConnectServer(const char *iptmp, int porttmp);
		~ConnectServer();
		static int indexForServer;//进行计数
		int clientfd;	
	private:
		bool connect_server();
		const char *ip;
		int port;
};

