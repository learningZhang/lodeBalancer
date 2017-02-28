#include <iostream>
#include <string>
#include <map>
using namespace std;
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <event.h>
#include <json/json.h>
#include <errno.h>

class User
{
public:
    User(){}
    User(string name, string pwd, string call,int fd)
        :_name(name), _pwd(pwd), _call(call),_fd(fd){}
    string getName(){return _name;}
    string getPwd(){return _pwd;}
    string getcall(){return _call;}
    void setfd(int fd){_fd = fd;}
    int getfd(){return _fd;}
private:
    string _name;
    string _pwd;
    string _call;
    int _fd;
};

//模拟数据库里面的人员信息
map<string, User> gUserDBMap;


typedef enum _MsgType
{
    EN_MSG_LOGIN=1,
    EN_MSG_REGISTER,
    EN_MSG_CHAT,
    EN_MSG_OFFLINE,
    EN_MSG_ACK
}EnMsgType;


//异步接收读线程
void* ReadThread(void *arg)
{
    int clientfd = (int)arg;
    while(true)
    {
        //接收client发送的消息，处理，响应
        char recvbuf[1024]={0};
        int size = 0;
        Json::Reader reader;
        Json::Value root;
        Json::Value response;
        
        size = recv(clientfd, recvbuf, 1024, 0);
        if(size < 0)
        {
            cout<<"errno:"<<errno<<endl;
            cout<<"client connect fail!"<<endl;
            close(clientfd);
            return NULL;
        }
    
        cout<<"recvbuf:"<<recvbuf<<endl;
        
        if(reader.parse(recvbuf, root))
        {
            int msgtype = root["msgtype"].asInt();
            switch(msgtype)
            {
	            //登陆函数
                case EN_MSG_LOGIN:
                {
                    response["msgtype"] = EN_MSG_ACK;
                    string name = root["name"].asString();
                    string pwd = root["pwd"].asString();
                    
                    map<string, User>::iterator it = gUserDBMap.find(name);
                    //不存在此用户名
                    if(it == gUserDBMap.end())
                    {
                        response["ackcode"] = "error";
                    }
                    else
                    {
	                    //用户名与密码均匹配成功时
                        if(pwd == it->second.getPwd())
                        {
	                        //记录
	                        it->second.setfd(clientfd);
                            response["ackcode"] = "ok";
                            cout<<clientfd<<endl;
                        }
                        else
                        {
	                        //密码匹配不成功
                            response["ackcode"] = "error";
                        }
                    }
                
                    //发送响应
                    cout<<"response:"<<response.toStyledString()<<endl;
                    send(clientfd, response.toStyledString().c_str(),
                        strlen(response.toStyledString().c_str())+1, 0);
                }
                break;
                //聊天
                case EN_MSG_CHAT:
                {
	                //打印该消息的发送方与接收方
                    cout<<root["from"]<<" => "<<root["to"]<<":"<<root["msg"]<<endl;
					//获取信息内容
                    string from = root["from"].asString();
                    string to = root["to"].asString();
                    string msg = root["msg"].asString();

                    map<string,User>::iterator it = gUserDBMap.find(to);
                    //如果接收方没有注册过，即该人不存在
                    if(it == gUserDBMap.end())
                    {
	                    //服务器打印的消息
	                    string str = "can not find this people,please check your input!";
	                    cout<<str<<endl;
	                    //服务器给客户端返回消息
	                    response["msgtype"] = EN_MSG_CHAT;
	                    response["ackcode"] = str;
	                    if(-1 == send(clientfd,response.toStyledString().c_str(), strlen(response.toStyledString().c_str())+1,0))
                        {
	                    	cout<<"server send message error"<<endl;
                    	}
                    }
                    //接收方未登陆
                    if(it->second.getfd() == -1)
                    {
	                    string str = to + "is not logining now,please wait.";
	                    cout<<str<<endl;
	                    response["msgtype"] = EN_MSG_CHAT;                	
                    	response["ackcode"] = str;
                    	if(-1 == send(clientfd,response.toStyledString().c_str(),strlen(response.toStyledString().c_str())+1,0))
                        {
	                    	cout<<"server send message error"<<endl;
                    	}
                    }
                    else
                    {
	                    response["msgtype"] = EN_MSG_CHAT;            
                    	response["from"] = from;
                    	response["ackcode"] = msg;
                    	if(-1 == send(gUserDBMap[to].getfd(),response.toStyledString().c_str(),strlen(response.toStyledString().c_str())+1,0))
                        {
	                    	cout<<"server send message error"<<endl;
                    	}
                    }
                }
                break;
                case EN_MSG_REGISTER:    
		        {
			        cout<<"one client is register"<<endl;
		            string name = root["name"].asString();
                    string pwd = root["pwd"].asString();
                    string call = root["call"].asString();
                    
		        	response["msgtype"] = EN_MSG_ACK;
					//判断用户是否已注册
					
					map<string, User>::iterator it = gUserDBMap.find(name);
                    if( (it != gUserDBMap.end()) && (it->second.getcall() == call))
                    {
		                cout<<"the user is already register"<<endl;
		                response["ackcode"] = "Error";    
		            }
		            else
	                {
		                //注册的用户信息插入到数据库中并保存		        	
                    	gUserDBMap[name] = User(name,pwd,call,-1);                    						    response["ackcode"] = "OK";
	                }	                	                  		        		        	
		        	if(-1 == send(clientfd,response.toStyledString().c_str(),strlen(response.toStyledString().c_str())+1,0))
                    {
	                    cout<<"server send register message error"<<endl;
                    }    
	            }
	            break;
	           // case EN_MSG_OFFLINE:
		        //{
			        
		        //}
		        //break;
            }
        }
        //客户端异常结束处理
        else
        {
	        cout<<"one client over"<<endl;
	        close(clientfd);
        }
    }
}

//libevent回调函数             //当有事件产生时候就会调用此，后调用线程
void ProcListenfd(evutil_socket_t fd, short , void *arg)
{
    sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (sockaddr*)&client, &len);
    cout<<"new client connect server! client info:"
        <<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;
        
    //启动异步接收读线程   一个客户端  《=》   线程
    pthread_t tid;
    pthread_create(&tid, NULL, ReadThread, (void*)clientfd);
}

int main()
{
    int listenfd;
    
    //模拟给map添加人员信息
    gUserDBMap["zhang san"] = User("zhang san", "111111", "9870986796",-1);
    gUserDBMap["xiao ming"] = User("xiao ming", "222222", "7777777",-1);
    gUserDBMap["gao yang"] = User("gao yang", "333333", "66666666",-1);
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
    {
        cout<<"listenfd create fail!"<<endl;
        return -1;
    }
    
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(10000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(-1 == bind(listenfd, (sockaddr*)&server, sizeof(server)))
    {
        cout<<"listenfd bind fail!"<<endl;
        return -1;
    }
    
    if(-1 == listen(listenfd, 20))
    {
	    
        cout<<"listenfd listen fail!"<<endl;
        return -1;
    }
    
    //创建reactor   统一事件源  socket I/O，信号，定时器
    struct event_base* base = event_init();
    
    //创建event事件
    struct event *listen_event = event_new(base, listenfd,  EV_READ|EV_PERSIST, ProcListenfd, NULL);
    //把event事件添加到reactor中
    event_add( listen_event, NULL );
    
    cout<<"server started..."<<endl;
    //启动反应堆
    event_base_dispatch(base);
    //释放事件资源
    event_free(listen_event);
    //关闭reactor
    event_base_free(base);
    
    return 0;
}
