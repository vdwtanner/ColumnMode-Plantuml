#pragma once

namespace CMPlantuml
{
	class DragHelper
	{
	public:
		void StartDragging(DWORD lparam, D2D1_VECTOR_2F initialPos);
		bool StopDragging(DWORD lparam, D2D1_VECTOR_2F& out_newPos);
		bool OnMouseMove(DWORD lparam, D2D1_VECTOR_2F& out_newPos);
		bool IsDragging() { return m_dragging; }

	private:
		bool m_dragging = false;
		int m_startX;
		int m_startY;
		D2D1_VECTOR_2F m_dragTargetInitialPos;
	};
}