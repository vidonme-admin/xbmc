
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMListContainer.h"

CVDMListContainer::CVDMListContainer(int parentID, int controlID, float posX, float posY, float width, float height, ORIENTATION orientation, const CScroller& scroller, int preloadItems)
    : CGUIListContainer(parentID, controlID, posX, posY, width, height, orientation, scroller, preloadItems)
{
  ControlType = VDMCONTAINER_LIST;
}

CVDMListContainer::~CVDMListContainer(void)
{
}

CVDMListContainer::CVDMListContainer(int parentID, int controlID, float posX, float posY, float width, float height,
                                 const CLabelInfo& labelInfo, const CLabelInfo& labelInfo2,
                                 const CTextureInfo& textureButton, const CTextureInfo& textureButtonFocus,
                                 float textureHeight, float itemWidth, float itemHeight, float spaceBetweenItems)
	: CGUIListContainer(parentID, controlID, posX, posY, width, height, labelInfo, labelInfo2, textureButton, textureButtonFocus, textureHeight, itemWidth, itemHeight, spaceBetweenItems)
{
	ControlType = VDMCONTAINER_LIST;
}

void CVDMListContainer::Render()
{
	if (!m_layout || !m_focusedLayout) return;

	int offset = (int)floorf(m_scroller.GetValue() / m_layout->Size(m_orientation));

	int cacheBefore, cacheAfter;
	GetCacheOffsets(cacheBefore, cacheAfter);

	if (g_graphicsContext.SetClipRegion(m_posX, m_posY, m_width, m_height))
	{
		CPoint origin = CPoint(m_posX, m_posY) + m_renderOffset;
		float pos = (m_orientation == VERTICAL) ? origin.y : origin.x;
		float end = (m_orientation == VERTICAL) ? m_posY + m_height : m_posX + m_width;

		float drawOffset = (offset - cacheBefore) * m_layout->Size(m_orientation) - m_scroller.GetValue();
		if (GetOffset() + GetCursor() < offset)
			drawOffset += m_focusedLayout->Size(m_orientation) - m_layout->Size(m_orientation);
		pos += drawOffset;
		end += cacheAfter * m_layout->Size(m_orientation);

		float focusedPos = 0;
		CGUIListItemPtr focusedItem;
		int current = offset - cacheBefore;
		while (pos < end && m_items.size())
		{
			int itemNo = CorrectOffset(current, 0);
			if (itemNo >= (int)m_items.size())
				break;

			bool focused = (current == GetOffset() + GetCursor()) && HasFocus();

			if (itemNo >= 0)
			{
				CGUIListItemPtr item = m_items[itemNo];
				// render our item
				if (focused)
				{
					focusedPos = pos;
					focusedItem = item;
				}
				else
				{
					if (m_orientation == VERTICAL)
						RenderItem(origin.x, pos, item.get(), false);
					else
						RenderItem(pos, origin.y, item.get(), false);
				}
			}
			// increment our position
			pos += focused ? m_focusedLayout->Size(m_orientation) : m_layout->Size(m_orientation);
			current++;
		}
		// render focused item last so it can overlap other items
		if (focusedItem)
		{
			if (m_orientation == VERTICAL)
				RenderItem(origin.x, focusedPos, focusedItem.get(), true);
			else
				RenderItem(focusedPos, origin.y, focusedItem.get(), true);
		}

		g_graphicsContext.RestoreClipRegion();
	}

	UpdatePageControl(offset);

	CGUIControl::Render();
}

void CVDMListContainer::Process(unsigned int currentTime, CDirtyRegionList &dirtyregions)
{
	ValidateOffset();

	if (m_bInvalidated)
		UpdateLayout();

	if (!m_layout || !m_focusedLayout) return;

	UpdateScrollOffset(currentTime);

	int offset = (int)floorf(m_scroller.GetValue() / m_layout->Size(m_orientation));

	int cacheBefore, cacheAfter;
	GetCacheOffsets(cacheBefore, cacheAfter);

	// Free memory not used on screen
	if ((int)m_items.size() > m_itemsPerPage + cacheBefore + cacheAfter)
		FreeMemory(CorrectOffset(offset - cacheBefore, 0), CorrectOffset(offset + m_itemsPerPage + 1 + cacheAfter, 0));

	CPoint origin = CPoint(m_posX, m_posY) + m_renderOffset;
	float pos = (m_orientation == VERTICAL) ? origin.y : origin.x;
	float end = (m_orientation == VERTICAL) ? m_posY + m_height : m_posX + m_width;

	float drawOffset = (offset - cacheBefore) * m_layout->Size(m_orientation) - m_scroller.GetValue();
	if (GetOffset() + GetCursor() < offset)
		drawOffset += m_focusedLayout->Size(m_orientation) - m_layout->Size(m_orientation);
	pos += drawOffset;
	end += cacheAfter * m_layout->Size(m_orientation);

	int current = offset - cacheBefore;
	while (pos < end && m_items.size())
	{
		int itemNo = CorrectOffset(current, 0);
		if (itemNo >= (int)m_items.size())
			break;

		bool focused = (current == GetOffset() + GetCursor()) && HasFocus();

		if (itemNo >= 0)
		{
			CGUIListItemPtr item = m_items[itemNo];
			// render our item
			if (m_orientation == VERTICAL)
				ProcessItem(origin.x, pos, item, focused, currentTime, dirtyregions);
			else
				ProcessItem(pos, origin.y, item, focused, currentTime, dirtyregions);
		}
		// increment our position
		pos += focused ? m_focusedLayout->Size(m_orientation) : m_layout->Size(m_orientation);
		current++;
	}

	UpdatePageControl(offset);

	CGUIControl::Process(currentTime, dirtyregions);
}

#endif