//from server file--
#include "head.h"

int searchFd(MYSQL &db, const char *name)
{
	if (name == NULL || strlen(name)>12)
	{
		return -1;
	}
	return db.findFdByName(char name);//对离线信息进行存储，之后按照姓名进行查找
}

bool delAllStateInfo(MYSQL &db)//删除state表中的所有数据
{
	return db.deleteElemBySocket(0, 2);//需要通过对象调用
}

bool delStateByfd(MYSQL &db, int fd)
{
	return db.deleteElemBySocket(1, fd);
}
//未完成函数
//服务器停止之后只保留注册者user的信息，并且将所有的states中的信息都给删除
//当从lb中受到客户端断开连接之后，将其的states信息进行修改删除


//负载均衡器的功能
//从lb接受消息，由于一台服务器只连接一个负载均衡器，所以对数据的收和发都是对同一个fd进行的
//1.当客户端请求的是注册时，mysql中增加user,回复的信息中将发来的fd添加
//2.当客户端是登陆的时候，向state表中插入数据，回复信息中增加其发来的fd
//3.当客户端是聊天的时候，查询对方是否在线，如果在线就将此包中的fd更换，将消息进行转发，
//  --如果不在线的话就将消息存储message<message，from, to>，向发送来的fd回复提醒消息