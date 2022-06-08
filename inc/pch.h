#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define UNICODE
// Windows Header Files
#include <windows.h>
#include <filesystem>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>

#include "ColumnModePluginAPI.h"

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