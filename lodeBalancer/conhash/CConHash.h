#ifndef _CConHash               //不应该将头文件的引入放入头文件中去，应该放入到Cpp文件中
#define _CConHash
#include "CHashFun.h"
#include "CNode_s.h"
#include "CRBTree.h"
#include "CVirtualNode_s.h"
#include <set>
using namespace std;

class CConHash//一致性哈希
{
public:
	/*构造函数*/
	CConHash(CHashFun * pFunc);
	
	/*设置hash函数*/
	void setFunc(CHashFun * pFunc);
	
	/*增加实体结点 , 0代表成功 , -1代表失败*/
	int addNode_s(CNode_s * pNode);
	
	/*删除实体结点 , 0代表成功 , -1代表失败*/
	int delNode_s(CNode_s * pNode);
	
	/*查找实体结点*/
	CNode_s * lookupNode_s(const char * object);
	
	/*获取一致性hash结构的所有虚拟结点数量*/
	int getVNodes();
private:
	/*Hash函数*/
	CHashFun * func;
	/*虚拟结点总个数*/
	int vNodes;
	/*存储虚拟结点的红黑树*/
	util_rbtree_t * vnode_tree;//底层是一个自己实现的红黑树
	set<CVirtualNode_s> mVirtualNodeSet;
};
/*辅助函数，虚拟结点转化为红黑树结点*/
util_rbtree_node_t * vNode2RBNode(CVirtualNode_s * vnode);
#endif