#include "head.h"
#include "client.h"

int main(int argc, char **argv)
{
    int port=10001;
    int clientfd;
    
    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd == -1)
    {
        cout<<"clientfd create fail!"<<endl;
        return -1;
    }
    
    sockaddr_in caddr;
    memset(&caddr, 0, sizeof(caddr));
    caddr.sin_family = AF_INET;
    caddr.sin_port = htons(port);
    caddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(-1 == connect(clientfd, (sockaddr*)&caddr, sizeof(caddr)))
    {
        cout<<"connect server fail!"<<endl;
        return -1;
    }
    
    char choice = 0;
    bool bloginsuccess = false;

    char name[20];    
    while (!bloginsuccess)
    {
        cout<<"============"<<endl;
        cout<<"1.login"<<endl;
        cout<<"2.register"<<endl;
        cout<<"3.exit"<<endl;
        cout<<"============"<<endl;   //异常退出-信号
        
        cout<<"choice:";
        scanf("%c", &choice);
        choice-='0';
        getchar();
        switch(choice)
        {
            	case 1:
	        {
            		if(doLogin(clientfd, name))    
            		{
	            		bloginsuccess = true;
	        	}  
            		else
            		{
	            		cout<<"login fail!"<<endl;
	        	}	  
            		break;
        	}
           
          	case 2:
	        {
           		if (registe(clientfd))    
           		{
	         	  	cout<<"register successful!"<<endl;
	    		}
           		else 
           		{
	          	 	cout<<"register failed!"<<endl;
	       		}
		 		continue;
		 	}
		 	
                case 3:
	        	{
					offline(clientfd);
					close(clientfd);
	        		cout<<"bye bye..."<<endl;
	        		exit(0);
	        	}
	        
                default:
	        	{
                   	cout<<"invalid input!"<<endl;
					continue;
				}
        }
    }
    
    cout<<"welcome to chat system!"<<endl;
    pthread_t tid;
    pthread_create(&tid, NULL, ReadThread, &clientfd);
    
    int size=0;
    while(true)
    {
        char chatbuf[1024] = {0};
        cin.getline(chatbuf, 1024);
        if (strlen(chatbuf) == 0)
		{
			continue;
		}
        if(strcmp(chatbuf, "quit") == 0)
        {
	        offline(clientfd);
	        cout<<"by close to shutdown client"<<endl;
			close(clientfd);
        	exit(0);
        }
        
        string parsestr = chatbuf;
        int offset = parsestr.find(':');
        Json::Value root;
        root["msgtype"] = EN_MSG_CHAT;
        root["from"] = name;
        root["to"] = parsestr.substr(0, offset);
        root["msg"] = parsestr.substr(offset+1, parsestr.length()-offset-1);
        size = send(clientfd, root.toStyledString().c_str(), strlen(root.toStyledString().c_str())+1, 0);
		if (size <= 0)
		{
			cout<<"send error"<<endl;
			if (errno)
			{
				cout<<"errno  is: "<<errno<<endl;
			}		
		}
    }
    return 0;
}

//如果我断开连接，我应该通知我的客户失去连接，并且请求重新连接
//现在我的情况是断开连接就会结束该进程

void* ReadThread(void *arg)
{
    int clientfd = *(int*)arg;
    char recvbuf[1024];
    Json::Reader reader;
    Json::Value root;

    while(true)
    {
        int size = recv(clientfd, recvbuf, 1024, 0);
		if (size == 0)
        {
 			close(clientfd);
           	cout<<"server connect fail!"<<endl;
	    	return 0;//can reconnnet the server??//连接失败直接退出
        }
		else if (size < 0)
		{
			cout<<"read error"<<endl;
		}

        if(reader.parse(recvbuf, root))
        {
            switch(root["msgtype"].asInt())
            {
                case EN_MSG_CHAT:
                {
                    cout<<"recv is  "<<root["from"].asString()<<":"<<root["msg"]<<endl;
                }
                break;
                case EN_MSG_ACK:
	        	{
					cout<<root["ackcode"].asString()<<endl;
	        	}
	       	 	break;
            }
        }
    }
}


bool registe(int fd)
{
	char name[20];
	char email[20];
	char passwd[20];
	char recvbuf[1024];

	cin.getline(name, 20);
	cout<<"your name: ";
	cin.getline(name, 20);
	cout<<"input passwd: ";
	cin.getline(passwd, 20);
	cout<<"your emial: ";
	cin.getline(email, 20);
	
	Json::Value root;
	root["msgtype"] = EN_MSG_REGISTER;
	root["name"] = name;
	root["passwd"] = passwd;
	root["email"] = email;

	int size = send(fd, root.toStyledString().c_str(),strlen(root.toStyledString().c_str()), 0);
	if (size <= 0)
	{
		cout<<"register info send fail"<<endl;
		exit(0);
	}

	size = recv(fd, recvbuf, 1024, 0);
	if (size < 0)
	{
		cout<<"register info ack fail"<<endl;
		cout<<"errno is: "<<errno<<endl;
		return false;
	}
	else if(size == 0)
	{
		close(fd);
		cout<<"bronken in 198"<<endl;
		exit(-1);
	}

	cout<<"recv buff: "<<recvbuf<<endl;

	Json::Reader reader;
	if(reader.parse(recvbuf, root))
	{
        int msgtype = root["msgtype"].asInt();
       	if(msgtype != EN_MSG_ACK)
        {
            	cout<<"recv server login ack msg invalid!"<<endl;
            	exit(0);
        }
       	string ackcode = root["ackcode"].asString();
        if(ackcode == "yes")
        {
           		return true;
        }
    }
    return false;
}


bool doLogin(int fd, char *name)
{
    char pwd[20];
    getchar();
    cout<<"name:";
    cin.getline(name, 20); 
    cout<<"pwd:";
    cin.getline(pwd, 20);
    Json::Value root;
    root["msgtype"] = EN_MSG_LOGIN;
    root["name"] = name;
    root["pwd"] = pwd;
  
    send(fd, root.toStyledString().c_str(), strlen(root.toStyledString().c_str())+1, 0);
    char recvbuf[1024]={0};
    int size = recv(fd, recvbuf, 1024, 0);
    if(size == 0)//连接断开
    {
	close(fd);
        cout<<"recv server login ack fail!"<<endl;
        exit(0);
    }
    else if(size<0)//受到信息失败
    {
	    cout<<"erron is"<<errno<<endl;
	    return false;
    }
    cout<<"recv buff is "<<recvbuf<<endl;
   
    Json::Reader reader;
    if(reader.parse(recvbuf, root))
    {
        if(root["msgtype"].asInt() != EN_MSG_ACK)
        {
            cout<<"recv server login ack msg invalid!"<<endl;
            exit(0);
        }
        if(root["ackcode"].asString() == "ok")
        {
            return true;
        }
    }
    return false;
}

bool offline(int fd)//主动打招呼断开还是直接断开--》服务器的资源
{
	Json::Value  root;
	root["msgtype"] = EN_MSG_OFFLINE;
	if(send(fd, root.toStyledString().c_str(), strlen(root.toStyledString().c_str()), 0)==0)//向lb发送信息表示自己要退出
	{
		cout<<"errno "<<strerror(errno)<<endl;
	}
}
