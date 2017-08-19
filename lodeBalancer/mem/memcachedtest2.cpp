#include “memcached.h”

int main()
{
	CMemcached mm;
	char *key = "zhongguo";
	char *value = "中国是四大文明古国之一";

	mm.savaValue(key, value);
	char *ss = mm.queryKey(key);
	cout<<ss<<endl;
	return 0;
}