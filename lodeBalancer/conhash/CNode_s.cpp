#include"CNode_s.h"
#include<string.h>
#include<assert.h>

CNode_s::CNode_s()
{
	//iden = NULL;
	vNodeCount = 0;
}

CNode_s::CNode_s(char * pIden , int pVNodeCount , const void * pData)
{
	setCNode_s(pIden,pVNodeCount,(void*)pData);
}

void CNode_s::setCNode_s(char * pIden , int pVNodeCount , const void * pData)
{
	assert(pIden!=NULL&&pVNodeCount>0);
	strcpy(iden,pIden);
	vNodeCount = pVNodeCount;
	data = (void *)pData;
}

const char * CNode_s::getIden()
{
	return iden;
}

int CNode_s::getVNodeCount()
{
	return vNodeCount;
}

void CNode_s::setData(void * pData)
{
	this->data = pData;
}

void * CNode_s::getData()
{
	return this->data;
}

