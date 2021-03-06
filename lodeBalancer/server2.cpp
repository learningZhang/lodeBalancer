#include "head.h"
#include "mysql.h"
#include "server.h"

int searchFd(CMysql &db, const char *name)
{
	if (name == NULL || strlen(name)>12)
	{
		return -1;
	}
	return db.getStates(name);//对离线信息进行存储，之后按照姓名进行查找
}

bool findMesgByName(CMysql &db, const char*name, char *str, int length)
{
	return db.findMessageByName(name, str, length);
}

bool delAllStateInfo(CMysql &db)//删除state表中的所有数据
{
	return db.deleteElemBySocket(0, 2);//需要通过对象调用
}

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

//1.当客户端请求的是注册时，mysql中增加user,回复的信息中将发来的fd添加
//2.当客户端是登陆的时候，向state表中插入数据，回复信息中增加其发来的fd
//3.当客户端是聊天的时候，查询对方是否在线，如果在线就将此包中的fd更换，将消息进行转发，
//  --如果不在线的话就将消息存储message<message，from, to>，向发送来的fd回复提醒消息
