#include "pch.h"
#include <shellapi.h>

using namespace CMPlantuml;

//Forwards WndProc messages to PreviewWindow::WndProc for handling
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (ptr)
    {
        PreviewWindow* pThis = reinterpret_cast<PreviewWindow*>(ptr);
        return pThis->WndProc(hWnd, message, wParam, lParam);
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}

PreviewWindow::PreviewWindow(Plugin* plugin) : m_plugin(plugin)
{
    m_path.clear();
}

PreviewWindow::~PreviewWindow()
{
    DeleteFile(m_tempPath.c_str());
}

HRESULT PreviewWindow::Init()
{
    WNDCLASS windowClass{};
    windowClass.lpfnWndProc = ::WndProc;
    windowClass.lpszClassName = L"CMPlantumlClass";
    m_windowClassAtom = (*m_plugin->m_callbacks.pfnRegisterWindowClass)(windowClass);

    if (m_windowClassAtom == 0)
    {
        return E_FAIL;
    }
    return S_OK;
}

void PreviewWindow::SetPlantumlSourcePath(std::filesystem::path path)
{
    m_path.assign(path);
    std::wstring tempBuff;
    tempBuff.resize(128);
    DWORD size = GetTempPath2(128, tempBuff.data());
    m_tempPath.assign(tempBuff.substr(0, size));
    m_tempPath.append(L"CMPlantumlTemp.svg");
    //SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)&sampleWindow);
}

bool PreviewWindow::ShowPreviewWindow()
{
    EnsureWindowCreated();
    UpdateWindowTitle();
    ShowWindowAsync(m_hwnd.value(), 1);
    return true;
}

void PreviewWindow::EnsureWindowCreated()
{
    if (!m_hwnd.has_value())
    {
        ColumnMode::CreateWindowArgs args{};
        args.exWindowStyle = 0;
        args.windowClass = m_windowClassAtom;
        args.windowName = L"Plantuml Preview Window";
        args.height = 500;
        args.width = 500;

        HWND hwnd;
        if (SUCCEEDED((*m_plugin->m_callbacks.pfnOpenWindow)(args, &hwnd)))
        {
            m_hwnd.emplace(std::move(hwnd));
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
        }
    }
}

bool PreviewWindow::GeneratePreview()
{
    std::wstring params = L"-NoProfile -NonInteractive -command Get-Content ";
    params.append(L"'")
        .append(m_path.c_str())
        .append(L"' | java.exe -jar 'C:\\ProgramData\\chocolatey\\lib\\plantuml\\tools\\plantuml.jar' -tsvg -pipe > '")
        .append(m_tempPath.c_str())
        .append(L"'");

    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = L"pwsh.exe";
    ShExecInfo.lpParameters = params.c_str();
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_HIDE;
    ShExecInfo.hInstApp = NULL;
    if (!ShellExecuteEx(&ShExecInfo))
    {
        DebugBreak();
        return false;
    }
    WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
    CloseHandle(ShExecInfo.hProcess);

    ShellExecute(NULL, L"open", m_tempPath.c_str(), NULL, NULL, 1);
    return true;
}

void PreviewWindow::UpdateWindowTitle()
{
    if (m_hwnd.has_value() && !m_path.empty())
    {
        std::wstring windowTitle = PLUGIN_NAME;
        windowTitle.append(L" - ")
            .append(m_path.filename());
        SetWindowText(m_hwnd.value(), windowTitle.c_str());
    }
}

LRESULT CALLBACK PreviewWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        m_hwnd.reset();
        break;
    case WM_LBUTTONUP: MessageBox(NULL, L"Clicked in CMPlantuml's Window!", L"CMPlantuml", MB_OK); break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}