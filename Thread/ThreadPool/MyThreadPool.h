#pragma once
#include<list>
#include<Windows.h>
#include<queue>
#include<process.h>
class  Itask
{

public:

	virtual void RunItask() = 0;


};

class MyThreadPool
{
public:
	MyThreadPool(void);
	~MyThreadPool(void);

public:
		bool CreateThreadPool(long  lMinThreadNum,long lMaxThreadNum );
		static unsigned _stdcall ThreadProc(void *);  
		void DestroyThreadPool();  //�����̺߳���
		bool Push(Itask *);//Ͷ��������

private:
	std::list<HANDLE> m_lstThread;  //�߳�����
	std::queue<Itask *> m_qItask;//�������
	HANDLE m_hSemaphore;//�ź���
	bool m_bFlagQuit;
	long m_CreateThread;//�������߳���
	long m_MaxThread;//��������߳�
	long m_RunThread;//���е��߳�
	HANDLE m_hMutex;
};

