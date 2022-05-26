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

	protected:
		void HandleFileChange(LPCWSTR);

	public:
		ColumnMode::ColumnModeCallbacks m_callbacks;

	private:
		PreviewWindow* m_pPreviewWindow;
		
	};
}