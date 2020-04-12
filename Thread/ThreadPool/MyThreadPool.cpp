#include "MyThreadPool.h"
//��Ŀ������ÿ��һ�������ͷ�һ���ź�����Ȼ�����̳߳��е��߳�ȥִ������
//Ϊ�˰���˳���ִ�����񣬽�����ŵ������У�������߳�ȥ������ȡ����ʱ������̲߳������⣨����취--����������������
//ͬʱ���е�push��pop����ͬʱִ��-----������취--����������������
//��������������߳���һ������ʱ�ᷢ����������


MyThreadPool::MyThreadPool(void)
{

	m_bFlagQuit = true;
	m_hSemaphore =NULL;
	m_CreateThread =0;//����������
	m_MaxThread = 0;//��������߳�
	m_RunThread =0;//���е��߳�
	m_hMutex = CreateMutex(NULL,false,0);//����������

}


MyThreadPool::~MyThreadPool(void)
{
}


bool MyThreadPool::CreateThreadPool(long  lMinThreadNum,long lMaxThreadNum )
{
	//У�����
	if(lMinThreadNum<=0  ||  lMaxThreadNum<lMinThreadNum)
	{
		return false;
	}
	//�����ź���
	m_hSemaphore =  CreateSemaphore(0,0,lMaxThreadNum,0);

	//�����߳�
	for(long i = 0;i<lMinThreadNum;i++)
	{
		HANDLE hThread =(HANDLE)_beginthreadex(0,0,&ThreadProc,this,0,0);
		if(hThread)
		{
			m_lstThread.push_back(hThread);//����̴߳����ɹ������̼߳��뵽������
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
		//���ź�
		WaitForSingleObject(pthis->m_hSemaphore,INFINITE);

		InterlockedIncrement(&pthis->m_RunThread);//ԭ�ӷ���//pthis->m_RunThread++;
		//�Ӷ�����ȡ������
		while(!pthis->m_qItask.empty())
		{
			//����̻߳�ͬʱȥȡһ������--------���--����

			WaitForSingleObject(pthis->m_hMutex,INFINITE);//���������
			if(pthis->m_qItask.empty())
			{
				ReleaseMutex(pthis->m_hMutex);
				break;
			}
			pItask = pthis->m_qItask.front(); //ȡ������
			pthis->m_qItask.pop(); //���в���ͬʱ����push��pop,���push��pop�����ͬһ����
			ReleaseMutex(pthis->m_hMutex);

			pItask->RunItask();//�߳�ִ������   ���ܸ�������----��������һ����һ���˸ɻ�

			//ִ�������������ɾ��
			delete  pItask;
			pItask = NULL;


		}

		InterlockedDecrement(&pthis->m_RunThread);//ԭ�ӷ���//pthis->m_RunThread--;
	}
	return 0;

}
void MyThreadPool::DestroyThreadPool()  //�����̺߳���
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
	//�������
	if(!pItask)
		return  false;
	//������������
	WaitForSingleObject(m_hMutex,INFINITE);
	m_qItask.push(pItask);    //���в���ͬʱ����push��pop,���push��pop�����ͬһ����
	ReleaseMutex(m_hMutex);

	//1�пշ���Ա
	if(m_RunThread==m_CreateThread && m_CreateThread<m_MaxThread)
	{
		HANDLE hThread =(HANDLE)_beginthreadex(0,0,&ThreadProc,this,0,0);
		if(hThread)
			m_lstThread.push_back(hThread);//����̴߳����ɹ������̼߳��뵽������


		m_CreateThread++;
	}


	//�ͷ��ź���
	ReleaseSemaphore(m_hSemaphore,1,NULL);


	return true;

}