#ifndef _CConHash               //��Ӧ�ý�ͷ�ļ����������ͷ�ļ���ȥ��Ӧ�÷��뵽Cpp�ļ���
#define _CConHash
#include "CHashFun.h"
#include "CNode_s.h"
#include "CRBTree.h"
#include "CVirtualNode_s.h"
#include <set>
using namespace std;

class CConHash//һ���Թ�ϣ
{
public:
	/*���캯��*/
	CConHash(CHashFun * pFunc);
	
	/*����hash����*/
	void setFunc(CHashFun * pFunc);
	
	/*����ʵ���� , 0����ɹ� , -1����ʧ��*/
	int addNode_s(CNode_s * pNode);
	
	/*ɾ��ʵ���� , 0����ɹ� , -1����ʧ��*/
	int delNode_s(CNode_s * pNode);
	
	/*����ʵ����*/
	CNode_s * lookupNode_s(const char * object);
	
	/*��ȡһ����hash�ṹ����������������*/
	int getVNodes();
private:
	/*Hash����*/
	CHashFun * func;
	/*�������ܸ���*/
	int vNodes;
	/*�洢������ĺ����*/
	util_rbtree_t * vnode_tree;//�ײ���һ���Լ�ʵ�ֵĺ����
	set<CVirtualNode_s> mVirtualNodeSet;
};
/*����������������ת��Ϊ��������*/
util_rbtree_node_t * vNode2RBNode(CVirtualNode_s * vnode);
#endif