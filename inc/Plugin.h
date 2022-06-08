#pragma once

namespace CMPlantuml
{
	class Plugin
	{
	public:
		Plugin(_Inout_ ColumnMode::OpenPluginArgs* args);
		~Plugin();
		HRESULT Init();

	public: //APIENTRY
		static HRESULT APIENTRY OnOpen(HANDLE, LPCWSTR);
		static HRESULT APIENTRY OnSave(HANDLE, LPCWSTR);
		static HRESULT APIENTRY OnSaveAs(HANDLE, LPCWSTR);
		static HRESULT APIENTRY OnLoadCompleted(HANDLE);

	public:
		PreviewWindow* GetPreviewWindow() { return m_pPreviewWindow; }

	protected:
		void HandleFileChange(LPCWSTR);

	public:
		ColumnMode::ColumnModeCallbacks m_callbacks;
		WorkerThread m_workerThread;

	private:
		PreviewWindow* m_pPreviewWindow;
		
	};
}