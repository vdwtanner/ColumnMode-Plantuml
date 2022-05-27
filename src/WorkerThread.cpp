#include "pch.h"

using namespace CMPlantuml;

WorkerThread::WorkerThread()
{
	m_killThread.clear();
	m_thread = std::thread(Run, this);
}

WorkerThread::~WorkerThread()
{
	m_killThread.test_and_set();
	m_thread.join();
}

void WorkerThread::Run(WorkerThread* pThis)
{
	while (!pThis->m_killThread.test())
	{
		WorkItem workItem;
		workItem.id = 0;	//Valid ids begin at 1
		{
			std::scoped_lock lock(pThis->m_workQueueMutex);
			if (!pThis->m_work.empty())
			{
				workItem = pThis->m_work.front();
				pThis->m_work.pop();
			}
		}
		if (workItem.id > 0)
		{
			try
			{
				workItem.func();
			}
			catch (...)
			{
				MessageBox(NULL, L"Unexpected erorr occurred on worker thread", PLUGIN_NAME, MB_OK | MB_ICONERROR);
			}
			pThis->m_lastProcessedWorkId.store(workItem.id);
			continue;
		}
		Sleep(50);
	}
}

UINT WorkerThread::EnqueueWork(std::function<void()> func)
{
	UINT id = m_nextWorkId++;
	
	std::scoped_lock lock(m_workQueueMutex);
	m_work.push({ id, func });

	return id;
}