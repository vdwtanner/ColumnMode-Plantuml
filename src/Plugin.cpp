#include "pch.h"

using namespace CMPlantuml;

Plugin::Plugin(ColumnMode::OpenPluginArgs* args)
{
	memcpy(&m_callbacks, args->pCallbacks, sizeof(ColumnMode::OpenPluginArgs));
	args->hPlugin = this;
	args->pPluginFuncs->pfnOnSave = OnSave;
	args->pPluginFuncs->pfnOnSaveAs = OnSaveAs;
	args->pPluginFuncs->pfnOnLoadCompleted = OnLoadCompleted;
}

void Plugin::Init()
{
	m_pPreviewWindow->Init();
}

#pragma region APIENTRY

HRESULT APIENTRY Plugin::OnSave(HANDLE handle, LPCWSTR)
{
	Plugin* pThis = reinterpret_cast<Plugin*>(handle);
	return S_OK;
}

HRESULT APIENTRY Plugin::OnSaveAs(HANDLE handle, LPCWSTR)
{
	Plugin* pThis = reinterpret_cast<Plugin*>(handle);
	int response = MessageBox(NULL, L"Would you like to preview this file?", PLUGIN_NAME, MB_YESNO | MB_ICONQUESTION);
	WCHAR buff[16];
	wsprintf(buff, L"Response: %d", response);
	MessageBox(NULL, buff, PLUGIN_NAME, MB_OK);

	return S_OK;
}

HRESULT APIENTRY Plugin::OnLoadCompleted(HANDLE handle)
{
	Plugin* pThis = reinterpret_cast<Plugin*>(handle);
	pThis->Init();

	return S_OK;
}

#pragma endregion