#pragma once

namespace CMPlantuml
{
	class Plugin;
	class ContextMenu
	{
	public:
		ContextMenu(Plugin* pPlugin);
		void Init(int baseId);
		bool OnContextMenu(HWND hwnd, int x, int y);
		bool TryHandleWmCommand(DWORD wparam);

	protected:
		void Export();
		
	private:
		int m_baseId;
		HMENU m_hmenu;
		Plugin* m_pPlugin;

		enum ContextMenuItem
		{
			EXPORT,
			NUM_MENU_ITEMS
		};
		const std::wstring m_menuItems[NUM_MENU_ITEMS] = {
			L"Export to..."
		};
		
	};
}