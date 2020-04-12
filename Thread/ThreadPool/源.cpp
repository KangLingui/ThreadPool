#include<iostream>
#include"MyThreadPool.h"

using namespace std;

class AddItask : public Itask
{
public:
	AddItask(int a,int b)
	{
	m_a = a;
	m_b = b;
	}
	~AddItask()
	{
	
	}
public:
		void RunItask()
		{
			cout<<m_a<<"*"<<m_b<<"="<<m_a*m_b<<endl;
		}
private:
	int m_a;
	int m_b;

};


int main()
{

	MyThreadPool tp;
	tp.CreateThreadPool(1,100);
	for(int i = 0;i<1000;i++)
	{
		Itask *p = new AddItask(i,i+1);
		tp.Push(p);
	}




	system("pause");
return 0;

}