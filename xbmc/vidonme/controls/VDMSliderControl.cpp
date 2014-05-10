
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMSliderControl.h"
#include "GUIInfoManager.h"
#include "../../guilib/Key.h"
#include "utils/MathUtils.h"
#include "../../guilib/GUIWindowManager.h"

static const SliderAction actions[] = {
  {"seek",    "PlayerControl(SeekPercentage(%2f))", PLAYER_PROGRESS, false},
  {"volume",  "SetVolume(%2f)",                     PLAYER_VOLUME,   true}
 };

CVDMSliderControl::CVDMSliderControl(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& backGroundTexture, const CTextureInfo& backGroundTexture2, const CTextureInfo& nibTexture, const CTextureInfo& nibTextureFocus, int iType)
    : CGUISliderControl(parentID, controlID, posX, posY, width, height, backGroundTexture, nibTexture, nibTextureFocus, iType)
	, m_guiBackground2(posX, posY, width, height, backGroundTexture2)
{
	ControlType = VDMCONTROL_SLIDER;
}

CVDMSliderControl::~CVDMSliderControl(void)
{
}

void CVDMSliderControl::Process(unsigned int currentTime, CDirtyRegionList &dirtyregions)
{
	bool dirty = false;
	int infoCode = m_iInfoCode;
	if (m_action && (!m_dragging || m_action->fireOnDrag))
		infoCode = m_action->infoCode;
	if (infoCode)
	{
		int val;
		if (g_infoManager.GetInt(val, infoCode))
		{
			SetIntValue(val);
		}
	}

	float fScaleY = m_height == 0 ? 1.0f : m_height / m_guiBackground.GetTextureHeight();

	float fProportion = GetProportion();
	dirty |= m_guiBackground.SetPosition( m_posX, m_posY );
	dirty |= m_guiBackground.SetHeight(m_height); 
	dirty |= m_guiBackground.SetWidth(m_width); 
	dirty |= m_guiBackground.SetVisible(m_guiBackground2.IsAllocated() ? fProportion < (float)1 : true);
	dirty |= m_guiBackground.Process(currentTime);
	dirty |= m_guiBackground2.SetPosition(m_posX, m_posY);
	dirty |= m_guiBackground2.SetHeight(m_height);
	dirty |= m_guiBackground2.SetWidth(m_width * fProportion);
	dirty |= m_guiBackground2.SetVisible(fProportion > (float)0);
	dirty |= m_guiBackground2.Process(currentTime);

	CGUITexture &nibLower = (m_bHasFocus && !IsDisabled() && m_currentSelector == RangeSelectorLower) ? m_guiSelectorLowerFocus : m_guiSelectorLower;
	dirty |= ProcessSelector(nibLower, currentTime, fScaleY, RangeSelectorLower);
	if (m_rangeSelection)
	{
		CGUITexture &nibUpper = (m_bHasFocus && !IsDisabled() && m_currentSelector == RangeSelectorUpper) ? m_guiSelectorUpperFocus : m_guiSelectorUpper;
		dirty |= ProcessSelector(nibUpper, currentTime, fScaleY, RangeSelectorUpper);
	}

	if (dirty)
		MarkDirtyRegion();

	CGUIControl::Process(currentTime, dirtyregions);
}

void CVDMSliderControl::Render()
{
	m_guiBackground.Render();

	if (m_guiBackground2.IsAllocated())
	{
		m_guiBackground2.Render();
	}

	CGUITexture &nibLower = (m_bHasFocus && !IsDisabled() && m_currentSelector == RangeSelectorLower) ? m_guiSelectorLowerFocus : m_guiSelectorLower;
	nibLower.Render();
	if (m_rangeSelection)
	{
		CGUITexture &nibUpper = (m_bHasFocus && !IsDisabled() && m_currentSelector == RangeSelectorUpper) ? m_guiSelectorUpperFocus : m_guiSelectorUpper;
		nibUpper.Render();
	}

	CGUIControl::Render();
}

void CVDMSliderControl::AllocResources()
{
	CGUISliderControl::AllocResources();

	m_guiBackground2.AllocResources();
}

void CVDMSliderControl::FreeResources(bool immediately)
{
	CGUISliderControl::FreeResources(immediately);

	m_guiBackground2.FreeResources(immediately);
}

void CVDMSliderControl::DynamicResourceAlloc(bool bOnOff)
{
	CGUISliderControl::DynamicResourceAlloc(bOnOff);

	m_guiBackground2.DynamicResourceAlloc(bOnOff);
}

void CVDMSliderControl::SetInvalid()
{
	CGUISliderControl::SetInvalid();

	m_guiBackground2.SetInvalid();
}

bool CVDMSliderControl::HitTest(const CPoint &point)const
{
	if (m_guiBackground2.HitTest(point)) return true;  

	return CGUISliderControl::HitTest(point);
}

bool CVDMSliderControl::UpdateColors()
{
	bool changed = CGUISliderControl::UpdateColors();
	changed |= m_guiBackground2.SetDiffuseColor(m_diffuseColor);

	return changed;
}

void CVDMSliderControl::SetType(int nType)
{
	m_iType = nType;
}

#endif