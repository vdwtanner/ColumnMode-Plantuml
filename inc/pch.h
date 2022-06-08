#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define UNICODE
// Windows Header Files
#include <windows.h>
#include <windowsx.h>
#include <wrl/client.h>
using namespace ::Microsoft::WRL;

//c runtime headers
#include <filesystem>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>

//DirectX
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_3.h>
#include <wincodec.h>
#include <comdef.h>

#include "ColumnModePluginAPI.h"

#include "DragHelper.h"
#include "ContextMenu.h"
#include "D2DPreviewRenderer.h"
#include "PreviewWindow.h"
#include "WorkerThread.h"
#include "Plugin.h"

constexpr LPCWSTR PLUGIN_NAME = L"CMPlantuml";

inline void VerifyHR(HRESULT hr)
{
	if (FAILED(hr))
		__debugbreak();
}

inline void VerifyBool(bool b)
{
	if (!b)
		__debugbreak();
}

inline D2D1_VECTOR_2F Add(D2D1_VECTOR_2F a, D2D1_VECTOR_2F b)
{
	return { a.x + b.x, a.y + b.y };
}