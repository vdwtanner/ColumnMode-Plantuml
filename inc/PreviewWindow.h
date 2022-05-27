#pragma once

namespace CMPlantuml
{
	class Plugin;
	class PreviewWindow
	{
	public:
		PreviewWindow(Plugin* plugin);
		~PreviewWindow();
		HRESULT Init();
		void SetPlantumlSourcePath(std::filesystem::path path);
		bool ShowPreviewWindow();
		bool GeneratePreview();

	public:
		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	protected:
		void EnsureWindowCreated();
		void UpdateWindowTitle();
		void ImageGeneratedCB();

	private:
		Plugin* m_plugin;
		ATOM m_windowClassAtom;
		std::filesystem::path m_path;
		std::filesystem::path m_tempPath;
		std::optional<HWND> m_hwnd;
	};
}