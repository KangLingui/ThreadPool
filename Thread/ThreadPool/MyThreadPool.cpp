#include "MyThreadPool.h"
//项目描述：每来一个任务，释放一个信号量，然后唤醒线程池中的线程去执行任务，
//为了按照顺序的执行任务，将任务放到队列中，当多个线程去队列中取任务时会产生线程并发问题（解决办法--加锁（互斥量））
//同时队列的push和pop不能同时执行-----（解决办法--加锁（互斥量））
//避免死锁（多个线程抢一个任务时会发生僵持现象）


MyThreadPool::MyThreadPool(void)
{

	m_bFlagQuit = true;
	m_hSemaphore =NULL;
	m_CreateThread =0;//创建的人数
	m_MaxThread = 0;//创建最大线程
	m_RunThread =0;//运行的线程
	m_hMutex = CreateMutex(NULL,false,0);//创建互斥量

}


MyThreadPool::~MyThreadPool(void)
{
}


bool MyThreadPool::CreateThreadPool(long  lMinThreadNum,long lMaxThreadNum )
{
	//校验参数
	if(lMinThreadNum<=0  ||  lMaxThreadNum<lMinThreadNum)
	{
		return false;
	}
	//创建信号量
	m_hSemaphore =  CreateSemaphore(0,0,lMaxThreadNum,0);

	//创建线程
	for(long i = 0;i<lMinThreadNum;i++)
	{
		HANDLE hThread =(HANDLE)_beginthreadex(0,0,&ThreadProc,this,0,0);
		if(hThread)
		{
		m_lstThread.push_back(hThread);//如果线程创建成功，将线程加入到链表中
		m_CreateThread++;
		}
	}
	m_CreateThread = lMinThreadNum;
	m_MaxThread = lMaxThreadNum;
	
	return TRUE;

}
unsigned _stdcall MyThreadPool::ThreadProc(void *lpvoid)
{
	MyThreadPool *pthis =  (MyThreadPool*)lpvoid;
	Itask *pItask = NULL;
	while (pthis->m_bFlagQuit)
	{
		//等信号
		WaitForSingleObject(pthis->m_hSemaphore,INFINITE);
		InterlockedIncrement(&pthis->m_RunThread);//原子访问pthis->m_RunThread++;
		//从队列中取出任务
		while(!pthis->m_qItask.empty())
		{
			//多个线程会同时去取一个任务--------解决--上锁
			WaitForSingleObject(pthis->m_hMutex,INFINITE);//会产生阻塞
			if(pthis->m_qItask.empty())
			{
				ReleaseMutex(pthis->m_hMutex);
				break;
			}
			pItask = pthis->m_qItask.front(); //取出任务
			pthis->m_qItask.pop(); //队列不能同时进行push和pop,因此push和pop必须加同一把锁
			ReleaseMutex(pthis->m_hMutex);
			pItask->RunItask();//线程执行任务   不能给他加锁----加锁就是一个人一个人干活
			//执行完任务后将任务删掉
			delete  pItask;
			pItask = NULL；
		}
		InterlockedDecrement(&pthis->m_RunThread);//原子访问//pthis->m_RunThread--;
	}
	return 0;
}

void MyThreadPool::DestroyThreadPool()  //销毁线程函数
{
	m_bFlagQuit = false;
	auto ite = m_lstThread.begin();
	Itask *pItask = NULL;
	while (!m_qItask.empty())
	{
		pItask = m_qItask.front();
		m_qItask.pop();
		delete pItask;
		pItask = NULL;
	}
	while (ite!=m_lstThread.end())
	{
		if(  WAIT_TIMEOUT==WaitForSingleObject(*ite,100))
		{
			TerminateThread(*ite,-1);
		}
		CloseHandle(*ite);
		*ite = NULL;
		*ite++;
	}
	m_lstThread.clear();
	if(m_hSemaphore)
	{
		CloseHandle(m_hSemaphore);
		m_hSemaphore = NULL;
	}
	if(m_hMutex)
	{
		CloseHandle(m_hMutex);
		m_hMutex =  NULL;

	}
}

bool MyThreadPool::Push(Itask *pItask)
{
	//检验参数
	if(!pItask)
		return  false;
	//将任务加入队列
	WaitForSingleObject(m_hMutex,INFINITE);
	m_qItask.push(pItask);    //队列不能同时进行push和pop,因此push和pop必须加同一把锁
	ReleaseMutex(m_hMutex);

	//1有空服务员
	if(m_RunThread==m_CreateThread && m_CreateThread<m_MaxThread)
	{
		HANDLE hThread =(HANDLE)_beginthreadex(0,0,&ThreadProc,this,0,0);
		if(hThread)
			m_lstThread.push_back(hThread);//如果线程创建成功，将线程加入到链表中
		m_CreateThread++;
	}
	//释放信号量
	ReleaseSemaphore(m_hSemaphore,1,NULL);
	return true;

}
