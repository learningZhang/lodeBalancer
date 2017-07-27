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
	/*����hash����*/
	assert(pFunc!=NULL);
	this->func = pFunc;
	this->vNodes = 0;
	/*��ʼ�������*/
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
	/*���������㲢���뵽�������*/
	for(int i=0;i<vCount;i++)
	{
		virtualNode = new CVirtualNode_s(pNode);
		/*����str+��i���ķ���������ͬ��iden�������ں����hashֵ����*/
		//itoa(i,num,10);
		sprintf(num, "%d", i);
		strcat(str,num);
		hash = func->getHashVal(str);//��������������Ϊ�ؼ�ֵ����hashֵ�ļ��㣬Ȼ����в���
		virtualNode->setHash(hash); //˽�г�Ա���и�ֵ
//CVirtualNode_s::getNode_s��: ���ܽ���this��ָ��ӡ�const CVirtualNode_s��ת��Ϊ��CVirtualNode_s &��		
		set<CVirtualNode_s>::iterator it=mVirtualNodeSet.begin();
			//mVirtualNodeSet��CVirtualNode_s�ĳ�Ա����������set�������ײ��Ǻ����
		for(; it!=mVirtualNodeSet.end(); ++it)//�Ժ�������б����� 
		{
			if(hash == (*it).hash)//�������ôһ��hash������ȥ
			{
				break;
			}
		}
		if(it == mVirtualNodeSet.end())//��ʾû�����hashֵ�Ĵ��ڣ�������㣬���в���
		{
			CVirtualNode_s vnode;
			vnode.setHash(hash);
			vnode.setNode_s(pNode);
			mVirtualNodeSet.insert(vnode);//���������㣬Ȼ����뵽set��
			this->vNodes++;
		}
		
// 		if(!util_rbtree_search(vnode_tree,hash))
// 		{
// 			/*���ɺ�������*/
// 			rbNode = vNode2RBNode(virtualNode); 
// 			if(rbNode!=NULL)
// 			{
// 				/*���ý����뵽�������*/
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
	/*����ʵ����������������������ɾ��*/
	for(int i=0;i<vCount;i++)
	{
		//itoa(i,num,10);
		sprintf(num, "%d", i);
		strcat(str,num);/*���ø÷���������ͬ��iden��*/
		hash = func->getHashVal(str);
		rbNode = util_rbtree_search(vnode_tree,hash);
		if(rbNode!=NULL)
		{
			node = (CVirtualNode_s *) rbNode->data;
			if(node->getNode_s()==pNode && node->getHash()==hash)
			{
				this->vNodes--;
				/*���ý��Ӻ������ɾ��*/
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
	for(; it!=mVirtualNodeSet.end(); ++it)//�����˺������find�������õ��Ǳ���
	{                                      //�����ұ���ֵС�Ľ��                
		if(hash < (*it).hash)              
		{
			return (*it).node;
		}
	}
	//"CVirtualNode_s::getNode_s��: ���ܽ���this��ָ��ӡ�const CVirtualNode_s��ת��Ϊ��CVirtualNode_s &��


	/*�ں�����в���keyֵ��key�����С�Ľ��*/
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


