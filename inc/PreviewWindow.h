#pragma once

namespace CMPlantuml
{
	class Plugin;
	class PreviewWindow
	{
	public:
		PreviewWindow(Plugin* plugin);
		HRESULT Init();
		void SetPlantumlSourcePath(std::filesystem::path path);
		bool ShowPreviewWindow();

	public:
		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	protected:
		void EnsureWindowCreated();
		void UpdateWindowTitle();

	private:
		Plugin* m_plugin;
		ATOM m_windowClassAtom;
		std::filesystem::path m_path;
		std::optional<HWND> m_hwnd;
	};
}