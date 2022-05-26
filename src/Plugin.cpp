#include "pch.h"

using namespace CMPlantuml;

Plugin::Plugin(_Inout_ ColumnMode::OpenPluginArgs* args)
{

	memcpy(&m_callbacks, args->pCallbacks, sizeof(ColumnMode::OpenPluginArgs));
	args->hPlugin = this;
	args->pPluginFuncs->pfnOnOpen = OnOpen;
	args->pPluginFuncs->pfnOnSave = OnSave;
	args->pPluginFuncs->pfnOnSaveAs = OnSaveAs;
	args->pPluginFuncs->pfnOnLoadCompleted = OnLoadCompleted;

	m_pPreviewWindow = new PreviewWindow(this);
}

Plugin::~Plugin()
{
	delete(m_pPreviewWindow);
}

HRESULT Plugin::Init()
{
	return m_pPreviewWindow->Init();
}

void Plugin::HandleFileChange(LPCWSTR filepath)
{
	std::filesystem::path path;
	path.assign(filepath);

	auto ext = path.extension();
	LPCWSTR validExtensions[] = { L".puml", L".plantuml", L".pu" };
	bool valid = false;
	for (auto extension : validExtensions)
	{
		if (ext.compare(extension) == 0)
		{
			valid = true;
			break;
		}
	}
	if (!valid)
	{
		return;
	}

	int response = MessageBox(NULL, L"Would you like to preview this file?", PLUGIN_NAME, MB_YESNO | MB_ICONQUESTION);

	if (response == IDYES)
	{
		m_pPreviewWindow->SetPlantumlSourcePath(path);
		m_pPreviewWindow->ShowPreviewWindow();
	}
}


#pragma region APIENTRY

HRESULT APIENTRY Plugin::OnOpen(HANDLE handle, LPCWSTR filepath)
{
	Plugin* pThis = reinterpret_cast<Plugin*>(handle);
	pThis->HandleFileChange(filepath);
	return S_OK;
}

HRESULT APIENTRY Plugin::OnSave(HANDLE handle, LPCWSTR)
{
	Plugin* pThis = reinterpret_cast<Plugin*>(handle);
	return S_OK;
}

HRESULT APIENTRY Plugin::OnSaveAs(HANDLE handle, LPCWSTR filepath)
{
	Plugin* pThis = reinterpret_cast<Plugin*>(handle);
	pThis->HandleFileChange(filepath);

	return S_OK;
}

HRESULT APIENTRY Plugin::OnLoadCompleted(HANDLE handle)
{
	Plugin* pThis = reinterpret_cast<Plugin*>(handle);
	return pThis->Init();
}

#pragma endregion