#include"CConHash.h"
#include"assert.h"
#include"CVirtualNode_s.h"
#include"string"
#include<iostream>
using namespace std;
#include <string.h>
#include <stdio.h>
CConHash::CConHash(CHashFun * pFunc)
{
	/*设置hash函数*/
	assert(pFunc!=NULL);
	this->func = pFunc;
	this->vNodes = 0;
	/*初始化红黑树*/
	vnode_tree = new util_rbtree_s();
	util_rbtree_init(vnode_tree);
}

int CConHash::addNode_s(CNode_s * pNode)
{
	if(pNode==NULL) 
		return -1;
	int vCount = pNode->getVNodeCount();
	if(vCount<=0) 
		return -1;
	CVirtualNode_s * virtualNode;
	util_rbtree_node_t * rbNode;
	char str [1000];
	char num[10];
	strcpy(str,pNode->getIden());
	long hash = 0;
	/*生成虚拟结点并插入到红黑树中*/
	for(int i=0;i<vCount;i++)
	{
		virtualNode = new CVirtualNode_s(pNode);
		/*采用str+“i”的方法产生不同的iden串，用于后面的hash值计算*/
		//itoa(i,num,10);
		sprintf(num, "%d", i);
		strcat(str,num);
		hash = func->getHashVal(str);//将机器的名字作为关键值进行hash值的计算，然后进行插入
		virtualNode->setHash(hash); //私有成员进行赋值
//CVirtualNode_s::getNode_s”: 不能将“this”指针从“const CVirtualNode_s”转换为“CVirtualNode_s &”		
		set<CVirtualNode_s>::iterator it=mVirtualNodeSet.begin();
			//mVirtualNodeSet是CVirtualNode_s的成员变量，其是set容器，底部是红黑树
		for(; it!=mVirtualNodeSet.end(); ++it)//对红黑树进行遍历？ 
		{
			if(hash == (*it).hash)//如果有这么一个hash就跳出去
			{
				break;
			}
		}
		if(it == mVirtualNodeSet.end())//表示没有这个hash值的存在，建立结点，进行插入
		{
			CVirtualNode_s vnode;
			vnode.setHash(hash);
			vnode.setNode_s(pNode);
			mVirtualNodeSet.insert(vnode);//建立虚拟结点，然后插入到set中
			this->vNodes++;
		}
		
// 		if(!util_rbtree_search(vnode_tree,hash))
// 		{
// 			/*生成红黑树结点*/
// 			rbNode = vNode2RBNode(virtualNode); 
// 			if(rbNode!=NULL)
// 			{
// 				/*将该结点插入到红黑树中*/
// 				util_rbtree_insert(vnode_tree,rbNode);
// 				this->vNodes++;
// 			}
// 		}
	}
	return 0;
}

int CConHash::delNode_s(CNode_s * pNode)
{
	if(pNode==NULL) return -1;
	util_rbtree_node_t * rbNode;
	char str [1000];
	char num [10];
	strcpy(str,pNode->getIden()); 
	int vCount = pNode->getVNodeCount();
	long hash = 0;
	CVirtualNode_s * node = NULL;
	/*将该实体结点产生的所有虚拟结点进行删除*/
	for(int i=0;i<vCount;i++)
	{
		//itoa(i,num,10);
		sprintf(num, "%d", i);
		strcat(str,num);/*采用该方法产生不同的iden串*/
		hash = func->getHashVal(str);
		rbNode = util_rbtree_search(vnode_tree,hash);
		if(rbNode!=NULL)
		{
			node = (CVirtualNode_s *) rbNode->data;
			if(node->getNode_s()==pNode && node->getHash()==hash)
			{
				this->vNodes--;
				/*将该结点从红黑树中删除*/
				util_rbtree_delete(vnode_tree,rbNode);
				delete rbNode;
				delete node;
			}
		}
	}
	return 0;
}

CNode_s * CConHash::lookupNode_s(const char * object)
{
	if(object==NULL||this->vNodes==0) return NULL;
	util_rbtree_node_t * rbNode;
	int hash = this->func->getHashVal(object);

	set<CVirtualNode_s>::iterator it=mVirtualNodeSet.begin();
	for(; it!=mVirtualNodeSet.end(); ++it)//辜负了红黑树的find，而采用的是遍历
	{                                      //这是找比其值小的结点                
		if(hash < (*it).hash)              
		{
			return (*it).node;
		}
	}
	//"CVirtualNode_s::getNode_s”: 不能将“this”指针从“const CVirtualNode_s”转换为“CVirtualNode_s &”


	/*在红黑树中查找key值比key大的最小的结点*/
// 	rbNode = util_rbtree_lookup(vnode_tree,key);
// 	if(rbNode!=NULL)
// 	{
// 		return ((CVirtualNode_s *) rbNode->data)->getNode_s();
// 	}
	return NULL;
}

int CConHash::getVNodes()
{
	return this->vNodes;
}


util_rbtree_node_t * vNode2RBNode(CVirtualNode_s * vnode)
{
	if(vnode==NULL) return NULL;
	util_rbtree_node_t *rbNode = new util_rbtree_node_t(); 
	rbNode->key = vnode->getHash();
	rbNode->data = vnode;
	return rbNode;
}	


