#include "mysql.h"
#include "memcached.h"

int main()
{
	CMysql db;
	CMemcached mem("127.0.0.1", 11211);
	char *name = "11";
	char *str = NULL;
	if ((str=queryKey(name)) != NULL)
	{
		cout<<str;
	}
	else
	{
		char *temp=(char*)malloc(1024);
		if (db.findMesgByName(name, temp, 1024)) == NULL)
		{
			cout<<"find error"<<endl;
		}
		saveValue(name, temp);
		free(temp);
		cout<<temp;
		return 0;
	}
	saveValue(name, str);
	cout<<str;
	return 0;
}

