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
	void DestroyThreadPool();  //销毁线程函数
	bool Push(Itask *);//投递任务函数

private:
	std::list<HANDLE> m_lstThread;  //线程链表
	std::queue<Itask *> m_qItask;//任务队列
	HANDLE m_hSemaphore;//信号量
	bool m_bFlagQuit;
	long m_CreateThread;//创建的线程数
	long m_MaxThread;//创建最大线程
	long m_RunThread;//运行的线程
	HANDLE m_hMutex;
};

