#ifndef MEMCACHED_H
#define MEMCACHED_H

#include <libmemcached/memcached.h>
#include <iostream>
using namespace std;


//将其设置为一个对象，所以出了对象的作用域，该memecached就会被释放，所以注意该对象的开辟位置
class CMemcached
{
public:
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