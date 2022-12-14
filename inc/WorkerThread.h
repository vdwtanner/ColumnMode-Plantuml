#pragma once

namespace CMPlantuml
{
	class WorkerThread
	{
		struct WorkItem
		{
			UINT id;
			std::function<void()> func;
		};
	public:
		WorkerThread();
		~WorkerThread();
		UINT EnqueueWork(std::function<void()> func);

	protected:
		static void Run(WorkerThread* pThis);

	private:
		std::queue<WorkItem> m_work;
		std::thread m_thread;
		std::mutex m_workQueueMutex;
		UINT m_nextWorkId = 1;
		std::atomic_uint m_lastProcessedWorkId = 0;
		std::atomic_flag m_killThread;
	};
}