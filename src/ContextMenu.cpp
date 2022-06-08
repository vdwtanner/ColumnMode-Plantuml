#include "pch.h"
#include "commdlg.h"

using namespace CMPlantuml;

ContextMenu::ContextMenu(Plugin* pPlugin) : m_pPlugin(pPlugin)
{
	m_baseId = 0;
	m_hmenu = NULL;
}

void ContextMenu::Init(int baseId)
{
	assert(m_hmenu == NULL); //Only init once
	m_hmenu = CreatePopupMenu();
	m_baseId = baseId;
	for (int i = 0; i < NUM_MENU_ITEMS; i++)
	{
		AppendMenu(m_hmenu, MF_STRING, m_baseId + i, m_menuItems[i].c_str());
	}
	
}

bool ContextMenu::OnContextMenu(HWND hwnd, int x, int y)
{
	SetForegroundWindow(hwnd);
	TrackPopupMenuEx(m_hmenu, TPM_TOPALIGN | TPM_LEFTALIGN, x, y, hwnd, NULL);
	return true;
}

bool ContextMenu::TryHandleWmCommand(DWORD wparam)
{
	int wmId = LOWORD(wparam) - m_baseId;
	switch (wmId)
	{
	case EXPORT:
		Export();
		return true;
	}
	return false;
}

void ContextMenu::Export()
{
	PreviewWindow* previewWindow = m_pPlugin->GetPreviewWindow();
	using namespace std::filesystem;
	path path = previewWindow->GetActiveFilePath();
	HWND hwnd;
	if (!previewWindow->TryGetHwnd(hwnd))
	{
		return;
	}
	
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	wchar_t szFile[MAX_PATH];
	wcscpy_s(szFile, L"UntitledDiagram.png");

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"PNG\0*.png\0SVG\0*.svg\0ASCII ART\0*.txt\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = path.parent_path().c_str();
	ofn.lpstrDefExt = L"png";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	if (!!GetSaveFileName(&ofn))
	{
		std::wstring format;
		switch (ofn.nFilterIndex)
		{
		case 1: format = L"png"; break;
		case 2: format = L"svg"; break;
		case 3: format = L"txt"; break;
		default: MessageBox(NULL, L"Invalid file format.", L"Export Failed", MB_OK | MB_ICONERROR); return;
		}

		previewWindow->ExportDiagram(ofn.lpstrFile, format.c_str());
	}
}
