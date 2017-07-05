#include "mysql.h"

int main()
{
	CMysql db;
/*	char *name="zhang";
	char *passwd = "1234";
	char *email = "awk";
	if (!db.insertIntoUser(name, passwd, email))
		cout<<"insert error"<<endl;
	
	if (!db.queryPasswd(name, passwd))
		cout<<"passwd query error"<<endl;

	if (!db.deleteUser(name))
		cout<<"delete error"<<endl;

	int fd =100;
	if (!db.insertIntoStates(name, fd))
		cout<<"insert into states error"<<endl;

	int x = 0;
	if((x=db.getStates(name)) == -1)
		cout<<"getStates error"<<endl;
	cout<<"fd is "<<x<<endl;
///
	if(!db.deleteElemBySocket(fd, 1))
	{
		cout<<"delete error"<<endl;
	}

	char *msg = "nizhoa";
	if (!db.insertIntoMessage("zhang", "wang", msg))//from to msg
	{
		cout<<"error in insert into Mesg"<<endl;
	}
*/
	char *temp=NULL;
	if ((temp = db.findMesgByName("wanug")) == NULL)
	{
		cout<<"find error"<<endl;
	}
	else
	{
		cout<<temp<<endl;free(temp);
	}

//	if(!db.delInMessage("wang"))
//	{
//		cout<<"error in delMessage"<<endl;
//	}
	
	cout<<"ending"<<endl;
}
