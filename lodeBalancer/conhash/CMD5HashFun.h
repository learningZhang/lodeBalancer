#ifndef _CMD5HashFun
#define _CMD5HashFun

#include"CMD5HashFun.h"
#include"CHashFun.h"

/*��MD5�㷨�������hashֵ���̳�CHashFun����*/
class CMD5HashFun : public CHashFun         ///CMD5HashFun.cpp��ʵ��
{
public:
	virtual long getHashVal (const char * );
};


#endif