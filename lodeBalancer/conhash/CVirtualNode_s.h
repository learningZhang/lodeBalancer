#ifndef _CVirtualNode_s
#define _CVirtualNode_s
#include "CNode_s.h"

/*虚拟结点*/
class CVirtualNode_s
{
public:
	/*构造函数*/
	CVirtualNode_s();
	CVirtualNode_s(CNode_s * pNode);

	/*设置虚拟结点所指向的实体结点*/
	void setNode_s(CNode_s * pNode);

	/*获取虚拟结点所指向的实体结点*/
	CNode_s * getNode_s();

	/*设置虚拟结点hash值*/
	void setHash(long pHash);

	/*获取虚拟结点hash值*/
	long getHash();
	bool operator>(const CVirtualNode_s &src)const
	{
		return hash > src.hash;
	}
	bool operator<(const CVirtualNode_s &src)const
	{
		return hash < src.hash;
	}
	bool operator==(const CVirtualNode_s &src)const
	{
		return hash == src.hash;
	}                           //虚拟节点如何实体化到环中
	long hash; /*hash值*/
	CNode_s * node; /*虚拟结点所指向的实体结点*/
private:
	//long hash; /*hash值*/
	//CNode_s * node; /*虚拟结点所指向的实体结点*/
	//"CVirtualNode_s::getNode_s”: 不能将“this”指针从“const CVirtualNode_s”转换为“CVirtualNode_s &”
};

#endif