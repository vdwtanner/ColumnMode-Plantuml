#include "pch.h"
#include <windowsx.h>

using namespace CMPlantuml;

void DragHelper::StartDragging(DWORD lparam, D2D1_VECTOR_2F initialPos)
{
	m_dragTargetInitialPos = initialPos;
	m_startX = GET_X_LPARAM(lparam);
	m_startY = GET_Y_LPARAM(lparam);
	m_dragging = true;
}

bool CMPlantuml::DragHelper::StopDragging(DWORD lparam, D2D1_VECTOR_2F& out_newPos)
{
	if (m_dragging)
	{
		int x = GET_X_LPARAM(lparam);
		int y = GET_Y_LPARAM(lparam);
		D2D1_VECTOR_2F delta = { (float)(x - m_startX), (float)(y - m_startY) };
		out_newPos = Add(m_dragTargetInitialPos, delta);

		m_dragging = false;
		return true;
	}
	return false;
}

bool CMPlantuml::DragHelper::OnMouseMove(DWORD lparam, D2D1_VECTOR_2F& out_newPos)
{
	if (m_dragging)
	{
		int x = GET_X_LPARAM(lparam);
		int y = GET_Y_LPARAM(lparam);
		D2D1_VECTOR_2F delta = { (float)(x - m_startX), (float)(y - m_startY) };
		out_newPos = Add(m_dragTargetInitialPos, delta);
		return true;
	}
	return false;
}
