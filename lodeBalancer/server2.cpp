//from server file--
#include "head.h"

<<<<<<< HEAD
int searchFd(CMysql &db, const char *name)
=======
int searchFd(MYSQL &db, const char *name)
>>>>>>> dev
{
	if (name == NULL || strlen(name)>12)
	{
		return -1;
	}
<<<<<<< HEAD
	return db.getStates(name);//对离线信息进行存储，之后按照姓名进行查找
}

bool findMesgByName(CMysql &db, const char*name, char *temp, int length)
{
	
	return db.findMessageByName(name, temp, length);
}

bool delAllStateInfo(CMysql &db)//删除state表中的所有数据
=======
	return db.findFdByName(char name);//对离线信息进行存储，之后按照姓名进行查找
}

bool delAllStateInfo(MYSQL &db)//删除state表中的所有数据
>>>>>>> dev
{
	return db.deleteElemBySocket(0, 2);//需要通过对象调用
}

<<<<<<< HEAD
bool delStateByfd(CMysql &db, int fd)
{
	return db.deleteElemBySocket(1, fd);
}

bool insertIntoMessage(const char *fromuser, const char *touser, const char* msg, CMysql &db)
{
	return db.insertIntoMessage(fromuser, touser, msg);
}

bool delSendedMsg(CMysql &db, const char *name)
{
	return db.delInMessage(name);
}
=======
bool delStateByfd(MYSQL &db, int fd)
{
	return db.deleteElemBySocket(1, fd);
}
>>>>>>> dev
//未完成函数
//服务器停止之后只保留注册者user的信息，并且将所有的states中的信息都给删除
//当从lb中受到客户端断开连接之后，将其的states信息进行修改删除


//负载均衡器的功能
//从lb接受消息，由于一台服务器只连接一个负载均衡器，所以对数据的收和发都是对同一个fd进行的
//1.当客户端请求的是注册时，mysql中增加user,回复的信息中将发来的fd添加
//2.当客户端是登陆的时候，向state表中插入数据，回复信息中增加其发来的fd
//3.当客户端是聊天的时候，查询对方是否在线，如果在线就将此包中的fd更换，将消息进行转发，
<<<<<<< HEAD
//  --如果不在线的话就将消息存储message<message，from, to>，向发送来的fd回复提醒消息
=======
//  --如果不在线的话就将消息存储message<message，from, to>，向发送来的fd回复提醒消息
>>>>>>> dev
