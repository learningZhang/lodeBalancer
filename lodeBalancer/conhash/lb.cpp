#include <iostream>
#include <vector>
using namespace std;

const int IP_LEN = 20;

class CServer
{
public:
private:
    char _ip[IP_LEN];
    unsigned short _port;
};

//算法一，采用轮循方式
class CServerMap
{
public:
    CServerMap():mIndex(0){};
    void addServer(CServer &server);
    void delServer(CServer &server);
    CServer& getNextServer()
    {
        int length = mServerMap.size();
        if(mIndex == length-1)
            mIndex = 0;
        return mServerMap[mIndex++];
    }
private:
    vector<CServer> mServerMap;
    int mIndex;
};
//服务器信息列表
CServerMap serverMap;


//算法二：一致性哈希 => 负载均衡器
CConHash serverMap;

int main()
{
    // 127.0.0.1 10000
    CServer server1("192.168.0.100", 8000);
    CServer server2("192.168.0.101", 8000);
    CServer server3("192.168.0.102", 8000);
    
    serverMap.addServer(server1);
    serverMap.addServer(server2);
    serverMap.addServer(server3);
    
    CNode_s * node1 = new CNode_s("machineA",80,"192.168.0.100");
	CNode_s * node2 = new CNode_s("machineB",80,"192.168.0.101");
	CNode_s * node3 = new CNode_s("machineC",80,"192.168.0.102");
    serverMap.addNode_s(node1);   // set<CNode_s> 
	serverMap.addNode_s(node2);
	serverMap.addNode_s(node3);
    
    CNode_s *pnode = serverMap.lookupNode_s(clientip);
    char *serverip = (char*)pnode->getData();
    //创建libevent主监听线程，启动多线程+I/O复用的工作线程
    
    
    //接收client的消息
    CServer &server = serverMap.getNextServer();
    
    
    return 0;
}