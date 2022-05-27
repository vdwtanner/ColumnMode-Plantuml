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