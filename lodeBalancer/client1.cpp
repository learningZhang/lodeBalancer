#include "head.h"

char name[20];
char pwd[20];

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
    
    int choice = 0;
    bool bloginsuccess = false;
    
    while (!bloginsuccess)
    {
        cout<<"============"<<endl;
        cout<<"1.login"<<endl;
        cout<<"2.register"<<endl;
        cout<<"3.exit"<<endl;
        cout<<"============"<<endl;   //异常退出-信号
        
        cout<<"choice:";
        choice = fgetc(stdin)-'0';//一次取出一个字符
        fflush(stdin);
	
        switch(choice)
        {
            case 1:
	        {
            	if(doLogin(clientfd))    
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
    }
    return 0;
}

void* ReadThread(void *arg)
{
    int clientfd = *(int*)arg;
    char recvbuf[1024];
    Json::Reader reader;
    Json::Value root;

    while(true)
    {
        if(recv(clientfd, recvbuf, 1024, 0) <= 0)
        {
            cout<<"server connect fail!"<<endl;
	    	return 0;//can reconnnet the server??//连接失败直接退出
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
	if (size <= 0)
	{
		cout<<"register info ack fail"<<endl;
		return false;
	}

	cout<<"recv buff "<<recvbuf<<endl;

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


bool doLogin(int fd)
{
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
    
    if(recv(fd, recvbuf, 1024, 0) <= 0)
    {
        cout<<"recv server login ack fail!"<<endl;
        exit(0);
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
	send(fd, root.toStyledString().c_str(), strlen(root.toStyledString().c_str()), 0);
}

//1.密码加密   密钥+明文
//将密钥存放在客户机中，输入信息之后，和密钥进行加密，将加密后的字符发送给服务器，然后由
//服务器进行匹配

//2.长连接，短链接
//3.群聊功能
//4.突发事件的处理
//5.留言功能的实现