typedef enum _MsgType
{
    EN_MSG_LOGIN=1,
    EN_MSG_REGISTER,
    EN_MSG_CHAT,
    EN_MSG_OFFLINE,
    EN_MSG_ACK
}EnMsgType;

bool doLogin(int fd, char *name);

bool offline(int fd);//主动打招呼断开还是直接断开--》服务器的资源

bool registe(int fd);

bool offline(int fd);//主动打招呼断开还是直接断开--》服务器的资源

void* ReadThread(void *arg);
