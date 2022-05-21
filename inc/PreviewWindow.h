#pragma once

namespace CMPlantuml
{
	class Plugin;
	class PreviewWindow
	{
	public:
		PreviewWindow(Plugin& plugin);
		void Init();
	private:
		Plugin& m_plugin;
	};
}