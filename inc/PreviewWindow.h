#pragma once

namespace CMPlantuml
{
	constexpr int CONTEXT_MENU_BASEID = 100;
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
		void ExportDiagram(LPCWSTR exportPath, LPCWSTR format);
		std::filesystem::path GetActiveFilePath() { return m_path; }
		bool TryGetHwnd(_Out_ HWND& pHwnd);

	public:
		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	protected:
		void EnsureWindowCreated();
		void UpdateWindowTitle();
		void ImageGeneratedCB();

	private:
		Plugin* m_plugin;
		ATOM m_windowClassAtom;
		DragHelper m_dragHelper;
		ContextMenu m_contextMenu;
		std::filesystem::path m_path;
		std::filesystem::path m_tempPath;
		std::optional<HWND> m_hwnd;
		D2DPreviewRenderer m_previewRenderer;

		bool isNewFile = false;
	};
}