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

PreviewWindow::PreviewWindow(Plugin* plugin) : m_plugin(plugin), m_contextMenu(plugin)
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
    m_contextMenu.Init(CONTEXT_MENU_BASEID);
    return S_OK;
}

void PreviewWindow::SetPlantumlSourcePath(std::filesystem::path path)
{
    m_path.assign(path);
    std::wstring tempBuff;
    tempBuff.resize(128);
    DWORD size = GetTempPath2(128, tempBuff.data());
    m_tempPath.assign(tempBuff.substr(0, size));
    m_tempPath.append(L"CMPlantumlTemp.png");
    isNewFile = true;
    //SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)&sampleWindow);
}

bool PreviewWindow::ShowPreviewWindow()
{
    EnsureWindowCreated();
    UpdateWindowTitle();
    if (!IsWindowVisible(m_hwnd.value()))
    {
        ShowWindowAsync(m_hwnd.value(), 1);
    }
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

        HWND hwnd=0;
        if (SUCCEEDED((*m_plugin->m_callbacks.pfnOpenWindow)(args, &hwnd)))
        {
            m_hwnd.emplace(std::move(hwnd));
            m_previewRenderer.Init(hwnd);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
        }
    }
}

bool PreviewWindow::GeneratePreview()
{
    std::wstring params = L"/C type ";
    params.append(L"\"")
        .append(m_path.c_str())
        .append(L"\" | java.exe -jar \"C:\\ProgramData\\chocolatey\\lib\\plantuml\\tools\\plantuml.jar\" -png -pipe > \"")
        .append(m_tempPath.c_str())
        .append(L"\" & exit");

    auto GenerateImage = [=]() {
        SHELLEXECUTEINFO ShExecInfo = { 0 };
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = NULL;
        ShExecInfo.lpFile = L"cmd.exe";
        ShExecInfo.lpParameters = params.c_str();
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_HIDE;
        ShExecInfo.hInstApp = NULL;
        if (!ShellExecuteEx(&ShExecInfo))
        {
            MessageBox(NULL, L"Error Generating preview image!", PLUGIN_NAME, MB_OK | MB_ICONERROR);
            return;
        }
        else
        {
            WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
            CloseHandle(ShExecInfo.hProcess);
            ImageGeneratedCB();
        }
    };
    
    m_plugin->m_workerThread.EnqueueWork(GenerateImage);

    return true;
}

void CMPlantuml::PreviewWindow::ExportDiagram(LPCWSTR exportPath, LPCWSTR format)
{
    std::wstring params = L"/C type ";
    params.append(L"\"")
        .append(m_path.c_str())
        .append(L"\" | java.exe -jar \"C:\\ProgramData\\chocolatey\\lib\\plantuml\\tools\\plantuml.jar\" -")
        .append(format)
        .append(L" -pipe > \"")
        .append(exportPath)
        .append(L"\" & \"")
        .append(exportPath)
        .append(L"\" & exit");

    auto GenerateImage = [=]() {
        SHELLEXECUTEINFO ShExecInfo = { 0 };
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = NULL;
        ShExecInfo.lpFile = L"cmd.exe";
        ShExecInfo.lpParameters = params.c_str();
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_HIDE;
        ShExecInfo.hInstApp = NULL;
        if (!ShellExecuteEx(&ShExecInfo))
        {
            MessageBox(NULL, L"Error Generating export image!", PLUGIN_NAME, MB_OK | MB_ICONERROR);
            return;
        }
        else
        {
            WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
            CloseHandle(ShExecInfo.hProcess);
        }
    };

    m_plugin->m_workerThread.EnqueueWork(GenerateImage);
}

bool CMPlantuml::PreviewWindow::TryGetHwnd(_Out_ HWND& pHwnd)
{
    pHwnd = NULL;
    if (m_hwnd.has_value())
    {
        pHwnd = m_hwnd.value();
        return true;
    }
    return false;
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

void PreviewWindow::ImageGeneratedCB()
{
    while (m_dragHelper.IsDragging())
    {
        Sleep(10);
    }
    m_previewRenderer.LoadPreviewImage(m_tempPath.c_str(), isNewFile);
    m_previewRenderer.Draw();
    isNewFile = false;
}

LRESULT CALLBACK PreviewWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!m_hwnd.has_value())
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    switch (message)
    {
    case WM_DESTROY:
        m_hwnd.reset();
        break;
    case WM_LBUTTONDOWN: {
        m_dragHelper.StartDragging(lParam, m_previewRenderer.GetPosition()); 
        return 0;
    }
    case WM_LBUTTONUP: {
        D2D1_VECTOR_2F newPos;
        if (m_dragHelper.StopDragging(lParam, newPos))
        {
            m_previewRenderer.SetPosition(newPos);
            m_previewRenderer.Draw();
        }
        return 0;
    }
    case WM_MOUSEMOVE: {
        D2D1_VECTOR_2F newPos;
        if (m_dragHelper.OnMouseMove(lParam, newPos))
        {
            m_previewRenderer.SetPosition(newPos);
            m_previewRenderer.Draw();
        }
        return 0;
    }
    case WM_MOUSELEAVE: {
        D2D1_VECTOR_2F newPos;
        if (m_dragHelper.StopDragging(lParam, newPos))
        {
            m_previewRenderer.SetPosition(newPos);
            m_previewRenderer.Draw();
        }
        return 0;
    } break;
    case WM_MOUSEWHEEL: {
        float wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam) / 120.0f;
        m_previewRenderer.Zoom(wheelDelta * .05f);
        m_previewRenderer.Draw();
        return 0;
    }
    case WM_PAINT:
        m_previewRenderer.Draw();
        break;
    case WM_SIZE:
        m_previewRenderer.OnResize();
        return 0;
    case WM_CONTEXTMENU:
        if(m_contextMenu.OnContextMenu(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
        {
            return 0;
        }
    case WM_COMMAND:
        if (m_contextMenu.TryHandleWmCommand(wParam))
        {
            return 0;
        }
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}