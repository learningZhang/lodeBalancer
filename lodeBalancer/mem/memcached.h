#ifdef MEMCACHED_H

#include <libmemcached/memcached.h>
#include <iostream>
using namespace std;

//��������Ϊһ���������Գ��˶���������򣬸�memecached�ͻᱻ�ͷţ�����ע��ö���Ŀ���λ��
class CMemcached
{
public: //memcached��һ���ͻ��˷�����ģ�ͣ�memcached�Ƿ�������ͨ��Ĭ�϶˿�11211����
		//�ֲ�ʽ�ɿͻ���ʵ��
    CMemcached(char *ip, unsigned short port);
    ~CMemcached();
    
    char* queryKey(char *key);
    void saveValue(char *key, char *value);
private:
    memcached_st *memc;
    memcached_return rc;
    memcached_server_st *server;
};

#endif