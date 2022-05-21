#pragma once

namespace CMPlantuml
{
	class Plugin
	{
	public:
		Plugin(_Inout_ ColumnMode::OpenPluginArgs* args);
		void Init();

	public: //APIENTRY
		static HRESULT APIENTRY OnSave(HANDLE, LPCWSTR);
		static HRESULT APIENTRY OnSaveAs(HANDLE, LPCWSTR);
		static HRESULT APIENTRY OnLoadCompleted(HANDLE);

	private:
		std::unique_ptr<PreviewWindow> m_pPreviewWindow;
		ColumnMode::ColumnModeCallbacks m_callbacks;
	};
}