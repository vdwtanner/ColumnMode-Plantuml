#include "pch.h"

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