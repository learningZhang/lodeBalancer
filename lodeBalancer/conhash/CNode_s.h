#ifndef _CNode_s
#define _CNode_s

/*ʵ����*/
class CNode_s
{
public:
	/*���캯��*/
	CNode_s();
	CNode_s(char * pIden , int pVNodeCount , const void * pData);

	/*��ȡ����ʾ*/
	const char * getIden();

	/*��ȡʵ���������������*/
	int getVNodeCount();

	/*����ʵ��������ֵ*/
	void setData(void * data);

	/*��ȡʵ��������ֵ*/
	void * getData();
private:
	void setCNode_s(char * pIden, int pVNodeCount , const void * pData);
	char iden[100];/*����ʾ��*/
	int vNodeCount; /*��������Ŀ*/
	void * data;/*���ݽ��*/
};
#endif
