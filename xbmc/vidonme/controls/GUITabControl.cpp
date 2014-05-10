
#if defined (__VIDONME_MEDIACENTER__)

#include "GUITabControl.h"
#include "guilib/GUIControlGroupList.h"
#include "guilib/GUIScrollBarControl.h"
#include "guilib/GUIButtonControl.h"
#include "guilib/GUIToggleButtonControl.h"
#include "guilib/GUIBaseContainer.h"
#include "guilib/Key.h"
#include "utils/XBMCTinyXML.h"
#include "guilib/GUIControlFactory.h"
#include "addons/Skin.h"
#include "guilib/GUIImage.h"
#include "utils/StringUtils.h"
#include "utils/CharsetConverter.h"

class CVDMTabButtonControl : public CGUIButtonControl
{
public:
  enum {BUTTON_STATE_NOFOCUS, BUTTON_STATE_FOCUS, BUTTON_STATE_ON,};
  CVDMTabButtonControl(int parentID, int controlID,
    float posX, float posY, float width, float height,
    const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus, const CTextureInfo& textureOn,
    const CLabelInfo &label) 
    : CGUIButtonControl(parentID, controlID, posX, posY, width, height, textureFocus, textureNoFocus, label)
    , m_nButtonState(BUTTON_STATE_NOFOCUS)
    , m_imgOn(posX, posY, width, height, textureOn)
  {
    ControlType = VDMCONTROL_TABBUTTON;
  }

public:
  void SetButtonState(int nButtonState)
  {
    m_nButtonState = nButtonState;
  }

  int GetButtonState() const
  {
    return m_nButtonState;
  }

protected:

  CGUILabel::COLOR GetTextColor() const
  {
    if (IsDisabled())
      return CGUILabel::COLOR_DISABLED;

    if (BUTTON_STATE_FOCUS == m_nButtonState)
      return CGUILabel::COLOR_FOCUSED;
    
    if (BUTTON_STATE_ON == m_nButtonState)
      return CGUILabel::COLOR_SELECTED;

    return CGUILabel::COLOR_TEXT;
  }

  virtual void OnFocus()
  {
    CGUIButtonControl::OnFocus();
    m_nButtonState = BUTTON_STATE_FOCUS;
  }

  virtual void OnUnFocus()
  {
    CGUIButtonControl::OnUnFocus();
    m_nButtonState = BUTTON_STATE_NOFOCUS;
  }

  virtual void Process(unsigned int currentTime, CDirtyRegionList &dirtyregions)
  {
    if (m_bInvalidated)
    {
      m_imgFocus.SetWidth(m_width);
      m_imgFocus.SetHeight(m_height);

      m_imgNoFocus.SetWidth(m_width);
      m_imgNoFocus.SetHeight(m_height);

      m_imgOn.SetWidth(m_width);
      m_imgOn.SetHeight(m_height);
    }

    if (HasFocus())
    {
      unsigned int alphaChannel = m_alpha;
      if (m_pulseOnSelect)
      {
        unsigned int alphaCounter = m_focusCounter + 2;
        if ((alphaCounter % 128) >= 64)
          alphaChannel = alphaCounter % 64;
        else
          alphaChannel = 63 - (alphaCounter % 64);

        alphaChannel += 192;
        alphaChannel = (unsigned int)((float)m_alpha * (float)alphaChannel / 255.0f);
      }
      if (m_imgFocus.SetAlpha((unsigned char)alphaChannel))
        MarkDirtyRegion();

      m_focusCounter++;
    }
    
    m_imgFocus.SetVisible(BUTTON_STATE_FOCUS == m_nButtonState);
    m_imgNoFocus.SetVisible(BUTTON_STATE_NOFOCUS == m_nButtonState);
    m_imgOn.SetVisible(BUTTON_STATE_ON == m_nButtonState);

    m_imgFocus.Process(currentTime);
    m_imgNoFocus.Process(currentTime);
    m_imgOn.Process(currentTime);

    //ProcessText(currentTime);
    CRect labelRenderRect = m_label.GetRenderRect();
    CRect label2RenderRect = m_label2.GetRenderRect();

    bool changed = m_label.SetMaxRect(m_posX, m_posY, m_width, m_height);
    changed |= m_label.SetText(m_info.GetLabel(m_parentID));
    changed |= m_label.SetScrolling(HasFocus());

    // render the second label if it exists
    CStdString label2(m_info2.GetLabel(m_parentID));
    changed |= m_label2.SetMaxRect(m_posX, m_posY, m_width, m_height);
    changed |= m_label2.SetText(label2);
    if (!label2.IsEmpty())
    {
      changed |= m_label2.SetAlign(XBFONT_RIGHT | (m_label.GetLabelInfo().align & XBFONT_CENTER_Y) | XBFONT_TRUNCATED);
      changed |= m_label2.SetScrolling(HasFocus());

      // If overlapping was corrected - compare render rects to determine
      // if they changed since last frame.
      if (CGUILabel::CheckAndCorrectOverlap(m_label, m_label2))
        changed |= (m_label.GetRenderRect()  != labelRenderRect ||
        m_label2.GetRenderRect() != label2RenderRect);

      changed |= m_label2.SetColor(GetTextColor());
    }
    changed |= m_label.SetColor(GetTextColor());
    if (changed)
      MarkDirtyRegion();

    CGUIControl::Process(currentTime, dirtyregions);
  }

  virtual void Render()
  {
    m_imgFocus.Render();
    m_imgNoFocus.Render();
    m_imgOn.Render();

    RenderText();
    CGUIControl::Render();
  }

private:
  int m_nButtonState;
  CGUITexture m_imgOn;
};

#define TAB_BASE_ID  -1000000

CGUITabControl:: CGUITabControl(int parentID, int controlID, float posX, float posY, float width, float height,
  TABORIENTATION tabOrientation, float tabBarSize, float tabBarLength, uint32_t tabBarAlignment, const CTextureInfo& tabBarTextureBackground, float tabBarPadding1, float tabBarPadding2,
  float tabItemSize, float tabItemLength, float tabItemGap, uint32_t tabItemAlignment, const CScroller& scroller, 
  const CTextureInfo& tabItemTextureFocus, const CTextureInfo& tabItemTextureNoFocus, const CTextureInfo& tabItemTextureOn, const CLabelInfo& tabItemLabelInfo, unsigned char alphaCurrentTab,
  const CTextureInfo& viewTextureBackground,
  float barWidth, float barHeight, const CTextureInfo& barBackgroundTexture, const CTextureInfo& barTexture, const CTextureInfo& barTextureFocus, const CTextureInfo& nibTexture, const CTextureInfo& nibTextureFocus, bool showOnePage)
  : CGUIControlGroup(parentID, controlID, posX, posY, width, height)
  , m_tabOrientation(tabOrientation)
  , m_tabBarAlignment(tabBarAlignment)
  , m_pImageTabBackground(new CGUIImage(parentID, 0, 0, 0, 0, 0, tabBarTextureBackground))
  , m_tabItemTextureFocus(tabItemTextureFocus)
  , m_tabItemTextureNoFocus(tabItemTextureNoFocus)
  , m_tabItemTextureOn(tabItemTextureOn)
  , m_tabItemLabelInfo(tabItemLabelInfo)
  , m_tabItemSize(tabItemSize)
  , m_tabItemLength(tabItemLength)
  , m_alphaCurrentTab(alphaCurrentTab)
  , m_pImageViewBackground(new CGUIImage(parentID, 0, 0, 0, 0, 0, viewTextureBackground))
  , m_pListTabs(new CGUIControlGroupList(parentID, controlID + TAB_BASE_ID - 1, 0, 0, width, height, tabItemGap, controlID, (tabOrientation == LEFT || tabOrientation == RIGHT) ? VERTICAL : HORIZONTAL, true, tabItemAlignment, scroller))
  , m_pGroupViews(new CGUIControlGroup(parentID, controlID + TAB_BASE_ID - 2, 0, 0, width, height))
  , m_pScrollbar(new CGUIScrollBar(parentID, controlID + TAB_BASE_ID - 3, 0, 0, barWidth, barHeight, barBackgroundTexture, barTexture, barTextureFocus, nibTexture, nibTextureFocus, VERTICAL, showOnePage))
  , m_currentView(0)
  , m_bDefaultViewAdded(false)
{
  ControlType = GUICONTROL_TAB;

  m_pListTabs->SetNavigation(GetControlIdUp(), GetControlIdDown(), GetControlIdLeft(), GetControlIdRight(), GetControlIdBack());
  if (tabOrientation == LEFT)
  {
    m_pListTabs->SetPosition(tabBarPadding1, 0);
    m_pListTabs->SetWidth(tabBarSize);
    m_pListTabs->SetHeight(tabBarLength);
    m_pGroupViews->SetPosition(m_pListTabs->GetXPosition() + tabBarSize + tabBarPadding2, 0);
    m_pGroupViews->SetWidth(width - m_pGroupViews->GetXPosition());
  }
  else if (tabOrientation == TOP)
  {
    m_pListTabs->SetPosition(0, tabBarPadding1);
    m_pListTabs->SetHeight(tabBarSize);
    m_pListTabs->SetWidth(tabBarLength);
    m_pGroupViews->SetPosition(0, m_pListTabs->GetYPosition() + tabBarSize + tabBarPadding2);
    m_pGroupViews->SetHeight(height - m_pGroupViews->GetYPosition());
  }
  else if (tabOrientation == RIGHT)
  {
    m_pListTabs->SetPosition(width - tabBarSize - tabBarPadding1, 0);
    m_pListTabs->SetWidth(tabBarSize);
    m_pListTabs->SetHeight(tabBarLength);
    m_pGroupViews->SetWidth(m_pListTabs->GetXPosition() - tabBarPadding2);
  }
  else
  {
    m_pListTabs->SetPosition(0, height - tabBarSize - tabBarPadding1);
    m_pListTabs->SetHeight(tabBarSize);
    m_pListTabs->SetWidth(tabBarLength);
    m_pGroupViews->SetHeight(m_pListTabs->GetYPosition() - tabBarPadding2);
  }

  m_pImageTabBackground->SetPosition(m_pListTabs->GetXPosition(), m_pListTabs->GetYPosition());
  m_pImageTabBackground->SetWidth(m_pListTabs->CGUIControl::GetWidth());
  m_pImageTabBackground->SetHeight(m_pListTabs->CGUIControl::GetHeight());
  m_pImageViewBackground->SetPosition(m_pGroupViews->GetXPosition(), m_pGroupViews->GetYPosition());
  m_pImageViewBackground->SetWidth(m_pGroupViews->GetWidth());
  m_pImageViewBackground->SetHeight(m_pGroupViews->GetHeight());

  m_pScrollbar->SetPosition(m_pGroupViews->GetXPosition() + m_pGroupViews->GetWidth() - barWidth, m_pGroupViews->GetYPosition());
  m_pScrollbar->SetVisible(false);

  //child views may use screen coordinates as its coordinates.
  m_pGroupViews->SetPosition(0, 0);

  AddControl(m_pImageTabBackground);
  AddControl(m_pImageViewBackground);
  AddControl(m_pListTabs);
  AddControl(m_pGroupViews);
  AddControl(m_pScrollbar);

  AddDefaultView();
}

CGUITabControl::~CGUITabControl(void)
{

}

void CGUITabControl::Process(unsigned int currentTime, CDirtyRegionList& dirtyregions)
{
  if (m_tabOrientation == LEFT || m_tabOrientation == RIGHT)
  {
    if (m_tabBarAlignment & XBFONT_CENTER_Y)
    {
      m_pListTabs->SetPosition(m_pListTabs->GetXPosition(), (GetHeight() - m_pListTabs->GetHeight()) * 0.5f);
      //m_pImageTabBackground->SetPosition(m_pListTabs->GetXPosition(), m_pListTabs->GetYPosition());
    }
  }
  else
  {
    if (m_tabBarAlignment & XBFONT_RIGHT)
    {
      m_pListTabs->SetPosition(GetWidth() - m_pListTabs->GetWidth(), m_pListTabs->GetYPosition());        
    }
    else if (m_tabBarAlignment & XBFONT_CENTER_X)
    {
      m_pListTabs->SetPosition((GetWidth() - m_pListTabs->GetWidth()) * 0.5f, m_pListTabs->GetYPosition());        
    }

    //m_pImageTabBackground->SetPosition(m_pListTabs->GetXPosition(), m_pListTabs->GetYPosition());
  }

  for (int i = 0; i < m_vecTabViews.size(); i++)
  {
    if (i == m_currentView)
    {
      SET_CONTROL_VISIBLE(m_vecTabViews[i].second->GetID());
      //m_vecTabViews[i].second->SetVisible(true, true);
    }
    else
    {
      SET_CONTROL_HIDDEN(m_vecTabViews[i].second->GetID());
      //m_vecTabViews[i].second->SetVisible(false, true);
    }
  }

  bool bPseudoFocused = false;
  if (!m_vecTabViews[m_currentView].first->HasFocus())
  {
    bPseudoFocused = true;
    CGUIControl* control = m_vecTabViews[m_currentView].first;
    if (control->GetControlType() == CGUIControl::GUICONTROL_BUTTON)
    {
      control->SetFocus(true);
      ((CGUIButtonControl*)control)->SetAlpha(m_alphaCurrentTab);
    }
    else if (control->GetControlType() == CGUIControl::GUICONTROL_TOGGLEBUTTON)
    {
      control->SetFocus(true);
      ((CGUIToggleButtonControl*)control)->SetSelected(true);
    }
    else if (control->GetControlType() == CGUIControl::VDMCONTROL_TABBUTTON)
    {
      ((CVDMTabButtonControl*)control)->SetButtonState(CVDMTabButtonControl::BUTTON_STATE_ON);
    }
  }

  CGUIControlGroup::Process(currentTime, dirtyregions);

  if (bPseudoFocused)
  {
    CGUIControl* control = m_vecTabViews[m_currentView].first;
    if (control->GetControlType() == CGUIControl::GUICONTROL_BUTTON)
    {
      control->SetFocus(false);
      ((CGUIButtonControl*)control)->SetAlpha(0xFF);
    }
    else if (control->GetControlType() == CGUIControl::GUICONTROL_TOGGLEBUTTON)
    {
      control->SetFocus(false);
      ((CGUIToggleButtonControl*)control)->SetSelected(false);
    }
    else if (control->GetControlType() == CGUIControl::VDMCONTROL_TABBUTTON)
    {
      ((CVDMTabButtonControl*)control)->SetButtonState(CVDMTabButtonControl::BUTTON_STATE_NOFOCUS);
    }
  }
}

void CGUITabControl::Render()
{
  CGUIControlGroup::Render();
}

EVENT_RESULT CGUITabControl::SendMouseEvent(const CPoint &point, const CMouseEvent &event)
{
  return CGUIControlGroup::SendMouseEvent(point, event);
}

bool CGUITabControl::OnMouseOver(const CPoint &point)
{
  return CGUIControlGroup::OnMouseOver(point);
}

EVENT_RESULT CGUITabControl::OnMouseEvent(const CPoint &point, const CMouseEvent &event)
{
  return CGUIControlGroup::OnMouseEvent(point, event);
}

bool CGUITabControl::OnAction(const CAction &action)
{
#if 0
  bool bRet = false;
  switch (action.GetID())
  {
  case ACTION_SELECT_ITEM:
    {
#if 0
      if (!m_childGroup.IsVisible())
      {
        m_bSpinFocused = true;
        m_bShowDownlist = true;
      }
      else
      {
        m_bSpinFocused = false;
        m_bShowDownlist = false;
        if (m_pContainer->HasFocus())
        {
          CGUIListItemPtr pItem = m_pContainer->GetSelectedItemPtr();
          if (pItem)
          {
            SetValue(pItem->GetProperty("value"));
            //m_label.SetText(pItem->GetLabel());
            m_pSelectedItem = pItem;
            CGUIMessage msg(GUI_MSG_CLICKED, GetID(), GetParentID(), 0);
            SendWindowMessage(msg);
            //此时this对象已经析构掉，m_childGroup.SetInvalid()会失败。
            //SetInvalid();
          }
        }
      }
      return true;
#endif
    }
    break;
  case ACTION_MOVE_DOWN:
    if (m_pScrollbar->HasFocus())
    {
      return m_pScrollbar->OnAction(action);
    }

    if (m_tabOrientation == LEFT || m_tabOrientation == RIGHT)
    {
      CGUIControl* pFocusedCtrl = m_vecTabViews[m_currentView].second;
      if (!pFocusedCtrl->HasFocus())
      {
        if (m_pGroupViews->HasFocus())
        {
          CGUIMessage msgLostFocus(GUI_MSG_LOSTFOCUS, GetID(), pFocusedCtrl->GetID());
          m_pGroupViews->OnMessage(msgLostFocus);
        }

        pFocusedCtrl = m_vecTabViews[m_currentView].first;
      }

      if (!pFocusedCtrl->HasFocus())
      {
        if (m_pListTabs->HasFocus())
        {
          CGUIMessage msgLostFocus(GUI_MSG_LOSTFOCUS, GetID(), m_pListTabs->GetFocusedControlID());
          m_pListTabs->OnMessage(msgLostFocus);
        }        

        CGUIMessage msgSetFocus(GUI_MSG_SETFOCUS, GetID(), pFocusedCtrl->GetID());
        m_pListTabs->OnMessage(msgSetFocus);
      }
      else
      {
        if (pFocusedCtrl->IsGroup())
        {
          pFocusedCtrl = ((CGUIControlGroup*)pFocusedCtrl)->GetFocusedControl();
        }

        if (pFocusedCtrl)
        {
          return pFocusedCtrl->OnAction(action);
        }
        else
        {
          CGUIMessage message(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].first->GetID());
          m_pListTabs->OnMessage(message);
        }
      }    

      return true;
    }
    else if (m_tabOrientation == TOP)
    {
      if (!m_vecTabViews[m_currentView].second->HasFocus())
      {
        if (m_pGroupViews->HasFocus())
        {
          CGUIMessage msgLostFocus(GUI_MSG_LOSTFOCUS, GetID(), m_pGroupViews->GetFocusedControlID());
          m_pGroupViews->OnMessage(msgLostFocus);
        }

        CGUIMessage message(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].second->GetID());
        m_pGroupViews->OnMessage(message);
      }
      else
      {
        CGUIControl* pFocusedCtrl = m_vecTabViews[m_currentView].second;
        if (pFocusedCtrl->IsGroup())
        {
          pFocusedCtrl = ((CGUIControlGroup*)pFocusedCtrl)->GetFocusedControl();
        }

        if (pFocusedCtrl)
        {
          bRet = m_pGroupViews->GetFocusedControl()->OnAction(action);
        }

        if (!bRet)
        {
          CGUIMessage message(GUI_MSG_LOSTFOCUS, GetID(), m_vecTabViews[m_currentView].second->GetID());
          m_pGroupViews->OnMessage(message);
          CGUIMessage message2(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].first->GetID());
          m_pListTabs->OnMessage(message2);
        }
        return true;
      }
    }   

    break;
  case ACTION_MOVE_UP:
    if (m_tabOrientation == LEFT || m_tabOrientation == RIGHT)
    {
      CGUIControl* pFocusedCtrl = m_vecTabViews[m_currentView].second;
      if (!pFocusedCtrl->HasFocus())
      {
        pFocusedCtrl = m_vecTabViews[m_currentView].first;
      }

      if (!pFocusedCtrl->HasFocus())
      {
        CGUIMessage message(GUI_MSG_SETFOCUS, GetID(), pFocusedCtrl->GetID());
        m_pListTabs->OnMessage(message);
      }
      else
      {
        if (pFocusedCtrl->IsGroup())
        {
          pFocusedCtrl = ((CGUIControlGroup*)pFocusedCtrl)->GetFocusedControl();
        }

        if (pFocusedCtrl)
        {
          return pFocusedCtrl->OnAction(action);
        }
        else
        {
          CGUIMessage message(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].first->GetID());
          m_pListTabs->OnMessage(message);
        }
      }    

      return true;
    }
    else if (m_tabOrientation == BOTTOM)
    {
      if (!m_vecTabViews[m_currentView].second->HasFocus())
      {
        CGUIMessage message(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].second->GetID());
        m_pGroupViews->OnMessage(message);
      }
      else
      {
        CGUIControl* pFocusedCtrl = m_vecTabViews[m_currentView].second;
        if (pFocusedCtrl->IsGroup())
        {
          pFocusedCtrl = ((CGUIControlGroup*)pFocusedCtrl)->GetFocusedControl();
        }

        if (pFocusedCtrl)
        {
          bRet = m_pGroupViews->GetFocusedControl()->OnAction(action);
        }

        if (!bRet)
        {
          CGUIMessage message(GUI_MSG_LOSTFOCUS, GetID(), m_vecTabViews[m_currentView].second->GetID());
          m_pGroupViews->OnMessage(message);
          CGUIMessage message2(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].first->GetID());
          m_pListTabs->OnMessage(message2);
        }
        return true;
      }
    }   

    break;

  case ACTION_MOVE_LEFT:
    if (m_tabOrientation == TOP || m_tabOrientation == BOTTOM)
    {
      CGUIControl* pFocusedCtrl = m_vecTabViews[m_currentView].second;
      if (!pFocusedCtrl->HasFocus())
      {
        pFocusedCtrl = m_vecTabViews[m_currentView].first;
      }

      if (!pFocusedCtrl->HasFocus())
      {
        CGUIMessage message(GUI_MSG_SETFOCUS, GetID(), pFocusedCtrl->GetID());
        m_pListTabs->OnMessage(message);
      }
      else
      {
        if (pFocusedCtrl->IsGroup())
        {
          pFocusedCtrl = ((CGUIControlGroup*)pFocusedCtrl)->GetFocusedControl();
        }

        if (pFocusedCtrl)
        {
          return pFocusedCtrl->OnAction(action);
        }
        else
        {
          CGUIMessage message(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].first->GetID());
          m_pListTabs->OnMessage(message);
        }
      }    

      return true;
    }
    else if (m_tabOrientation == RIGHT)
    {
      if (!m_vecTabViews[m_currentView].second->HasFocus())
      {
        CGUIMessage message(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].second->GetID());
        m_pGroupViews->OnMessage(message);
      }
      else
      {
        CGUIControl* pFocusedCtrl = m_vecTabViews[m_currentView].second;
        if (pFocusedCtrl->IsGroup())
        {
          pFocusedCtrl = ((CGUIControlGroup*)pFocusedCtrl)->GetFocusedControl();
        }

        if (pFocusedCtrl)
        {
          bRet = m_pGroupViews->GetFocusedControl()->OnAction(action);
        }

        if (!bRet)
        {
          CGUIMessage message(GUI_MSG_LOSTFOCUS, GetID(), m_vecTabViews[m_currentView].second->GetID());
          m_pGroupViews->OnMessage(message);
          CGUIMessage message2(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].first->GetID());
          m_pListTabs->OnMessage(message2);
        }
        return true;
      }
    }   
    break;

  case ACTION_MOVE_RIGHT:
    if (m_tabOrientation == LEFT || m_tabOrientation == RIGHT)
    {
      CGUIControl* pFocusedCtrl = m_vecTabViews[m_currentView].second;
      if (!pFocusedCtrl->HasFocus())
      {
        pFocusedCtrl = m_vecTabViews[m_currentView].first;
      }

      if (!pFocusedCtrl->HasFocus())
      {
        CGUIMessage message(GUI_MSG_SETFOCUS, GetID(), pFocusedCtrl->GetID());
        m_pListTabs->OnMessage(message);
      }
      else
      {
        if (pFocusedCtrl->IsGroup())
        {
          pFocusedCtrl = ((CGUIControlGroup*)pFocusedCtrl)->GetFocusedControl();
        }

        if (pFocusedCtrl)
        {
          return pFocusedCtrl->OnAction(action);
        }
        else
        {
          CGUIMessage message(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].first->GetID());
          m_pListTabs->OnMessage(message);
        }
      }    

      return true;
    }
    else if (m_tabOrientation == BOTTOM)
    {
      if (!m_vecTabViews[m_currentView].second->HasFocus())
      {
        CGUIMessage message(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].second->GetID());
        m_pGroupViews->OnMessage(message);
      }
      else
      {
        CGUIControl* pFocusedCtrl = m_vecTabViews[m_currentView].second;
        if (pFocusedCtrl->IsGroup())
        {
          pFocusedCtrl = ((CGUIControlGroup*)pFocusedCtrl)->GetFocusedControl();
        }

        if (pFocusedCtrl)
        {
          bRet = m_pGroupViews->GetFocusedControl()->OnAction(action);
        }

        if (!bRet)
        {
          CGUIMessage message(GUI_MSG_LOSTFOCUS, GetID(), m_vecTabViews[m_currentView].second->GetID());
          m_pGroupViews->OnMessage(message);
          CGUIMessage message2(GUI_MSG_SETFOCUS, GetID(), m_vecTabViews[m_currentView].first->GetID());
          m_pListTabs->OnMessage(message2);
        }
        return true;
      }
    }   
    break;
  }
#if 0
  bool bRet = false;
  if (m_vecTabViews[m_currentView].second->HasFocus())
  {
    bRet = m_vecTabViews[m_currentView].second->OnAction(action);
  }  

  if (!bRet)
  {
    bRet = m_vecTabViews[m_currentView].first->OnAction(action);
  }

  if (!bRet)
  {
    bRet = CGUIControl::OnAction(action);
  }
#endif
  return bRet;
#endif

  return CGUIControlGroup::OnAction(action);
}

bool CGUITabControl::OnMessage(CGUIMessage& message)
{
#if 0
  if (message.GetControlId() == GetID() )
  {
    switch (message.GetMessage())
    {
    case GUI_MSG_ITEM_SELECT:
    case GUI_MSG_LABEL_RESET:
      {
        CGUIMessage newMsg(message.GetMessage(), GetID(), m_pScrollbar->GetID(), message.GetParam1(), message.GetParam2());
        m_pScrollbar->OnMessage(newMsg);
        return true;
      }
      break;
    case GUI_MSG_PAGE_CHANGE:
      if (message.GetSenderId() == m_pScrollbar->GetID())
      {
        if (m_currentView >= 0 && m_currentView < m_vecTabViews.size())
        {
          CGUIMessage newMsg(message.GetMessage(), GetID(), m_vecTabViews[m_currentView].second->GetID(), message.GetParam1(), message.GetParam2());
          m_vecTabViews[m_currentView].second->OnMessage(newMsg);
          return true;
        }
      }
      break;
    default:
      break;
    }
  }
#endif
  switch (message.GetMessage())
  {
  case GUI_MSG_FOCUSED:
    { 
      for (int i = 0; i < m_vecTabViews.size(); i++)
      {
        if (m_vecTabViews[i].first->GetID() == message.GetControlId() || m_vecTabViews[i].second->HasID(message.GetControlId()))
        {
          m_currentView = i;
#if 0
          CStdString str;
          str.Format("+++++++++++++++++tab control: %d, currentview: %d\n", GetID(), m_currentView);
          OutputDebugString(str.c_str());
#endif
          break;
        }
      }

      // a control has been focused
      m_focusedControl = message.GetControlId();
      SetFocus(true);
      // tell our parent that we have focus
      if (m_parentControl)
        m_parentControl->OnMessage(message);

      return true;
    } 
    break;
  case GUI_MSG_SETFOCUS:
    {
      // first try our last focused control...
      if (!m_defaultAlways && m_focusedControl)
      {
        CGUIControl *control = GetFirstFocusableControl(m_focusedControl);
        if (control)
        {
          CGUIMessage msg(GUI_MSG_SETFOCUS, GetParentID(), control->GetID());
          return control->OnMessage(msg);
        }
      }
      // ok, no previously focused control, try the default control first
      if (m_defaultControl)
      {
        CGUIControl *control = GetFirstFocusableControl(m_defaultControl);
        if (control)
        {
          CGUIMessage msg(GUI_MSG_SETFOCUS, GetParentID(), control->GetID());
          return control->OnMessage(msg);
        }
      }

      //add tab control handler
      if (m_currentView >= 0 && m_currentView < m_vecTabViews.size())
      {
        CGUIControl *control = m_vecTabViews[m_currentView].first;
        if (control)
        {
          CGUIMessage msg(GUI_MSG_SETFOCUS, GetParentID(), control->GetID());
          return control->OnMessage(msg);
        }
      }

      // no success with the default control, so just find one to focus
      CGUIControl *control = GetFirstFocusableControl(0);
      if (control)
      {
        CGUIMessage msg(GUI_MSG_SETFOCUS, GetParentID(), control->GetID());
        return control->OnMessage(msg);
      }
      // unsuccessful
      return false;
      break;
    }
  case GUI_MSG_CLICKED:
    {
      for (int i = 0; i < m_vecTabViews.size(); i++)
      {
        if (m_vecTabViews[i].first->GetID() == message.GetControlId())
        {
          m_currentView = i;
          break;
        }
      }
    }
    break;
  case GUI_MSG_ITEM_SELECT:
  case GUI_MSG_LABEL_RESET:
    if (m_pGroupViews->HasVisibleID(message.GetSenderId()) && message.GetControlId() == m_pScrollbar->GetID())
    {
      CGUIMessage newMsg(message.GetMessage(), GetID(), m_pScrollbar->GetID(), message.GetParam1(), message.GetParam2());
      m_pScrollbar->OnMessage(newMsg);
      return true;
    }
    break;
  case GUI_MSG_PAGE_CHANGE:
    if (message.GetSenderId() == m_pScrollbar->GetID())
    {
      CGUIMessage newMsg(message.GetMessage(), GetID(), m_vecTabViews[m_currentView].second->GetID(), message.GetParam1(), message.GetParam2());
      m_pGroupViews->OnMessage(newMsg);
      return true;
    }
    break;
  default:
    break;
  }

  return CGUIControlGroup::OnMessage(message);
}

void CGUITabControl::UnfocusFromPoint(const CPoint &point)
{
  CGUIControlGroup::UnfocusFromPoint(point);
}

bool CGUITabControl::HitTest(const CPoint &point) const
{
  return CGUIControlGroup::HitTest(point);
}

void CGUITabControl::SetInvalid()
{
  CGUIControlGroup::SetInvalid();
}

void CGUITabControl::OnFocus()
{
  CGUIControlGroup::OnFocus();
}

void CGUITabControl::OnUnFocus()
{
  CGUIControlGroup::OnUnFocus();
}

void CGUITabControl::AllocResources()
{
  CGUIControlGroup::AllocResources();
}

void CGUITabControl::FreeResources(bool immediately)
{
  CGUIControlGroup::FreeResources(immediately);
}

void CGUITabControl::DynamicResourceAlloc(bool bOnOff)
{
  CGUIControlGroup::DynamicResourceAlloc(bOnOff);
}

void CGUITabControl::AddView(CGUIControl* pView, const CStdString& strTabLabel, int indexView, bool bActive)
{
  if (m_bDefaultViewAdded)
  {
    DeleteView(0);
    m_bDefaultViewAdded = false;
  }

  if (m_vecTabViews.size() > 0)
  {
    if (indexView < 0 || indexView >= m_vecTabViews.size())
    {
      indexView = m_vecTabViews.size();
    }
  }
  else
  {
    indexView = 0;
  }

  ASSERT(pView);

  CGUIControl* pTabItem = NULL;
  bool bAddNew = false;
  int i = 0;
  for (; i < m_vecTabViews.size(); i++)
  {
    if ((m_vecTabViews[i].first->GetControlType() == GUICONTROL_BUTTON && ((CGUIButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel) ||
      (m_vecTabViews[i].first->GetControlType() == GUICONTROL_TOGGLEBUTTON && ((CGUIToggleButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel) ||
      (m_vecTabViews[i].first->GetControlType() == VDMCONTROL_TABBUTTON && ((CVDMTabButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel))
    {
      pTabItem = m_vecTabViews[i].first;

      if (m_vecTabViews[i].second != pView)
      {
        m_pGroupViews->RemoveControl(m_vecTabViews[i].second);
        m_vecTabViews[i].second->FreeResources(true);
        delete m_vecTabViews[i].second;

        m_vecTabViews[i].second = pView;
        bAddNew = true;
      }
    }
    else if (m_vecTabViews[i].second == pView)
    {
      pTabItem = m_vecTabViews[i].first;

      if (m_vecTabViews[i].first->GetControlType() == GUICONTROL_BUTTON)
      {
        ((CGUIButtonControl*)(m_vecTabViews[i].first))->SetLabel(strTabLabel);
      }
      else if (m_vecTabViews[i].first->GetControlType() == GUICONTROL_TOGGLEBUTTON)
      {
        ((CGUIToggleButtonControl*)(m_vecTabViews[i].first))->SetLabel(strTabLabel);
      }
      else if (m_vecTabViews[i].first->GetControlType() == VDMCONTROL_TABBUTTON)
      {
        ((CVDMTabButtonControl*)(m_vecTabViews[i].first))->SetLabel(strTabLabel);
      }
    }

    if (pTabItem)
    {
      break;
    }
  }

  if (pTabItem)
  {
    if (indexView > i)
    {
      indexView--;
    }

    if (indexView != i)
    {
      TabViewBundle temp = m_vecTabViews[i];
      m_vecTabViews.erase(m_vecTabViews.begin() + i);
      m_vecTabViews.insert(m_vecTabViews.begin() + indexView, 1, temp);        
      m_pListTabs->RemoveControl(temp.first);
      m_pListTabs->AddControl(temp.first, indexView);
    }
  }
  else
  {
    float width = m_tabItemLength, height = m_tabItemSize;
    if (m_tabOrientation == LEFT || m_tabOrientation == RIGHT)
    {
      width = m_tabItemSize;
      height = m_tabItemLength;
    }

    int nTabID = m_vecTabViews.size() > 0 ? m_vecTabViews[m_vecTabViews.size() - 1].first->GetID() + 1 : GetID() + TAB_BASE_ID;
    pTabItem = new CVDMTabButtonControl(GetParentID(), nTabID, 0, 0, width, height, m_tabItemTextureFocus, m_tabItemTextureNoFocus, m_tabItemTextureOn, m_tabItemLabelInfo);
    ((CVDMTabButtonControl*)pTabItem)->SetLabel(strTabLabel);
    m_vecTabViews.insert(m_vecTabViews.begin() + indexView, 1, std::make_pair(pTabItem, pView)); 
    m_pListTabs->AddControl(pTabItem, indexView);

    bAddNew = true;
  }

  if (bAddNew)
  {
    m_pGroupViews->AddControl(pView);
    if (pView->IsContainer())
    {
      ((CGUIBaseContainer*)pView)->SetPageControl(GetID());
      m_pScrollbar->SetVisible(true);
    }
    else
    {
      m_pScrollbar->SetVisible(false);
    }

    //pView->SetPosition(m_pGroupViews->GetXPosition(), m_pGroupViews->GetYPosition());
    pView->SetWidth(m_pGroupViews->GetWidth());
    pView->SetHeight(m_pGroupViews->GetHeight());
    pView->SetVisible(false);
    SetChildViewNavigation(pView, pTabItem->GetID());
    if (m_tabOrientation == LEFT || m_tabOrientation == RIGHT)
    {
      pTabItem->SetNavigation(0, 0, pView->GetID(), pView->GetID());
    }
    else
    {
      pTabItem->SetNavigation(pView->GetID(), pView->GetID(), 0, 0);
    }
  }

  if (bActive)
  {
    m_currentView = indexView;
  }
  else if(bAddNew)
  {
    if (indexView < m_currentView)
    {
      m_currentView++;
    }
  }
}

CGUIControl* CGUITabControl::GetView(int indexView) const
{
  if (indexView >= 0 && indexView < m_vecTabViews.size())
  {
    return m_vecTabViews[indexView].second;
  }

  return NULL;
}

CGUIControl* CGUITabControl::GetView(const CStdString& strTabLabel) const
{
  for (int i = 0; i < m_vecTabViews.size(); i++)
  {
    if ((m_vecTabViews[i].first->GetControlType() == GUICONTROL_BUTTON && ((CGUIButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel) ||
      (m_vecTabViews[i].first->GetControlType() == GUICONTROL_TOGGLEBUTTON && ((CGUIToggleButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel) ||
      (m_vecTabViews[i].first->GetControlType() == VDMCONTROL_TABBUTTON && ((CVDMTabButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel))
    {
      return m_vecTabViews[i].second;
    }
  }

  return NULL;
}

CGUIControl* CGUITabControl::RemoveView(int indexView)
{
  if (indexView >= 0 && indexView < m_vecTabViews.size())
  {
    m_pListTabs->RemoveControl(m_vecTabViews[indexView].first);
    m_pGroupViews->RemoveControl(m_vecTabViews[indexView].second);
    if (m_currentView > 0 && indexView <= m_currentView)
    {
      m_currentView--;
    }

    CGUIControl* pPreviousTab = NULL;
    CGUIControl* pNextTab = NULL;

    if (indexView == 0)
    {
      pPreviousTab = m_vecTabViews[m_vecTabViews.size() - 1].first;
    }
    else
    {
      pPreviousTab = m_vecTabViews[indexView - 1].first;
    }

    if (indexView == m_vecTabViews.size() - 1)
    {
      pNextTab = m_vecTabViews[0].first;
    }
    else
    {
      pNextTab = m_vecTabViews[indexView + 1].first;
    }

    if (m_tabOrientation == LEFT || m_tabOrientation == RIGHT)
    {
      pPreviousTab->SetNavigation(0, pNextTab->GetID(), 0, 0);
      pNextTab->SetNavigation(pPreviousTab->GetID(), 0, 0, 0);
    }
    else
    {
      pPreviousTab->SetNavigation(0, 0, 0, pNextTab->GetID());
      pNextTab->SetNavigation(0, 0, pPreviousTab->GetID(), 0);
    }

    CGUIControl* pTab = m_vecTabViews[indexView].first;
    CGUIControl* pView = m_vecTabViews[indexView].second;

    pTab->FreeResources(true);
    delete pTab;
    
    m_vecTabViews.erase(m_vecTabViews.begin() + indexView);

    if (m_vecTabViews.empty() && !m_bDefaultViewAdded)
    {
      AddDefaultView();
    }

    return pView;
  }

  return NULL;
}

CGUIControl* CGUITabControl::RemoveView(const CStdString& strTabLabel)
{
  for (int i = 0; i < m_vecTabViews.size(); i++)
  {
    if ((m_vecTabViews[i].first->GetControlType() == GUICONTROL_BUTTON && ((CGUIButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel) ||
      (m_vecTabViews[i].first->GetControlType() == GUICONTROL_TOGGLEBUTTON && ((CGUIToggleButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel) ||
      (m_vecTabViews[i].first->GetControlType() == VDMCONTROL_TABBUTTON && ((CVDMTabButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel))
    {
      return RemoveView(i);
    }
  }

  return NULL;
}

CGUIControl* CGUITabControl::RemoveView(CGUIControl* pView)
{
  for (int i = 0; i < m_vecTabViews.size(); i++)
  {
    if (m_vecTabViews[i].second == pView)
    {
      return RemoveView(i);
    }
  }

  return NULL;
}

bool CGUITabControl::DeleteView(int indexView)
{
  CGUIControl* pView = RemoveView(indexView);
  if (pView)
  {
    pView->FreeResources(true);
    delete pView;

    return true;
  }

  return false;
}

bool CGUITabControl::DeleteView(const CStdString& strTabLabel)
{
  for (int i = 0; i < m_vecTabViews.size(); i++)
  {
    if ((m_vecTabViews[i].first->GetControlType() == GUICONTROL_BUTTON && ((CGUIButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel) ||
      (m_vecTabViews[i].first->GetControlType() == GUICONTROL_TOGGLEBUTTON && ((CGUIToggleButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel) ||
      (m_vecTabViews[i].first->GetControlType() == VDMCONTROL_TABBUTTON && ((CVDMTabButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel))
    {
      return DeleteView(i);
    }
  }

  return false;
}

bool CGUITabControl::DeleteView(CGUIControl* pView)
{
  for (int i = 0; i < m_vecTabViews.size(); i++)
  {
    if (m_vecTabViews[i].second == pView)
    {
      return DeleteView(i);
    }
  }

  return false;
}

bool CGUITabControl::SetActiveView(int indexView)
{
  if (indexView >= 0 && indexView < m_vecTabViews.size())
  {
    m_currentView = indexView;
    return true;
  }

  return false;
}

bool CGUITabControl::SetActiveView(const CStdString& strTabLabel)
{
  for (int i = 0; i < m_vecTabViews.size(); i++)
  {
    if ((m_vecTabViews[i].first->GetControlType() == GUICONTROL_BUTTON && ((CGUIButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel) ||
      (m_vecTabViews[i].first->GetControlType() == GUICONTROL_TOGGLEBUTTON && ((CGUIToggleButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel) ||
      (m_vecTabViews[i].first->GetControlType() == VDMCONTROL_TABBUTTON && ((CVDMTabButtonControl*)(m_vecTabViews[i].first))->GetLabel() == strTabLabel))
    {
      m_currentView = i;
      return true;
    }
  }

  return false;
}

bool CGUITabControl::SetActiveView(CGUIControl* pView)
{
  for (int i = 0; i < m_vecTabViews.size(); i++)
  {
    if (m_vecTabViews[i].second == pView)
    {
      m_currentView = i;
      return true;
    }
  }

  return false;
}

CGUIControl* CGUITabControl::GetActiveView() const
{
  ASSERT(m_vecTabViews.size() > 0 && m_currentView >= 0 && m_currentView < m_vecTabViews.size());
  return m_vecTabViews[m_currentView].second;
}

int CGUITabControl::GetActiveViewIndex() const
{
  ASSERT(m_vecTabViews.size() > 0 && m_currentView >= 0 && m_currentView < m_vecTabViews.size());
  return m_currentView;
}

void CGUITabControl::ClearAll()
{
  CGUIControlGroup::ClearAll();
  m_vecTabViews.clear();
}

bool CGUITabControl::HasTabID(int id) const
{
  for (int i = 0; i < m_vecTabViews.size(); i++)
  {
    if (m_vecTabViews[i].first->HasID(id))
    {
      return true;
    }
  }

  return false;
}

bool CGUITabControl::HasViewID(int id) const
{
  for (int i = 0; i < m_vecTabViews.size(); i++)
  {
    if (m_vecTabViews[i].second->HasID(id))
    {
      return true;
    }
  }

  return false;
}

void CGUITabControl::AddDefaultView()
{
  CGUIControl* pDefaultControl = new CVDMTabButtonControl(GetParentID(), TAB_BASE_ID, 0, 0, 0, 0, CTextureInfo(), CTextureInfo(), CTextureInfo(), CLabelInfo());
  AddView(pDefaultControl, "Default");
  m_bDefaultViewAdded = true;
}

bool CGUITabControl::HasFocus() const
{
#if 0
  if (CGUIControlGroup::HasFocus())
  {
    return true;
  }

  return CGUIControl::HasFocus();
#else
  return CGUIControlGroup::HasFocus();
#endif
}
#if 1
CGUIControl* CGUITabControl::GetFirstFocusableControl(int id)
{
  if (!CanFocus()) return NULL;
  if (!id && m_currentView >= 0 && m_currentView < m_vecTabViews.size())
  {
    return m_vecTabViews[m_currentView].first;
  }

  return CGUIControlGroup::GetFirstFocusableControl(id);
}
#else
CGUIControl* CGUITabControl::GetFirstFocusableControl(int id)
{
  if (!CanFocus()) return NULL;
  CGUIControl* pFocusableCtrl = NULL;
  if (!id || (!(pFocusableCtrl = CGUIControlGroup::GetFirstFocusableControl(id)) && HasID(id)))
  {
    if (m_currentView >= 0 && m_currentView < m_vecTabViews.size())
    {
      return m_vecTabViews[m_currentView].first;
    }
  }

  return pFocusableCtrl;
}
#endif
void CGUITabControl::SetChildViewNavigation(CGUIControl* pView, int tabID)
{
  ASSERT(pView);

  int up = 0, down = 0, left = 0, right = 0, back = tabID;

  if (m_tabOrientation == TOP || m_tabOrientation == BOTTOM)
  {
    up = down = tabID;
  }
  else
  {
    left = right = tabID;
  }

  if (pView->IsGroup())
  {
    std::vector<CGUIControl*> children;

    if (pView->GetControlType() == GUICONTROL_GROUPLIST)
    {
      ((CGUIControlGroupList*)pView)->GetChildren(children);
      if (!children.empty())
      {
        ORIENTATION orientation = ((CGUIControlGroupList*)pView)->GetOrientation();
        if (orientation == HORIZONTAL)
        {
          children[0]->SetNavigationAction(ACTION_MOVE_LEFT, CGUIAction(0));
          children[children.size() - 1]->SetNavigationAction(ACTION_MOVE_RIGHT, CGUIAction(0));
        }
        else
        {
          children[0]->SetNavigationAction(ACTION_MOVE_UP, CGUIAction(0));
          children[children.size() - 1]->SetNavigationAction(ACTION_MOVE_DOWN, CGUIAction(0));
        }
      }
    }
    else
    {
      ((CGUIControlGroup*)pView)->GetChildren(children);
    }

    for (int i = 0; i < children.size(); i++)
    {
      SetChildViewNavigation(children[i], tabID);
    }
  }
  else
  {
    if (pView->GetControlIdUp())
    {
      up = 0;
    }

    if (pView->GetControlIdDown())
    {
      down = 0;
    }

    if (pView->GetControlIdLeft())
    {
      left = 0;
    }

    if (pView->GetControlIdRight())
    {
      right = 0;
    }

    pView->SetNavigation(up, down, left, right, back);
  }
}

void CGUITabControl::LoadControl(TiXmlElement* pControl, CGUIControlGroup *pGroup)
{
  ASSERT(pGroup);

  CGUIControlFactory factory;
  CRect rect;
  rect.x1 = pGroup->GetXPosition();
  rect.y1 = pGroup->GetYPosition();
  rect.x2 = rect.x1 + pGroup->GetWidth();
  rect.y2 = rect.y1 + pGroup->GetHeight();

  CGUIControl* pGUIControl = factory.Create(GetParentID(), rect, pControl);
  if (pGUIControl)
  {
    pGroup->AddControl(pGUIControl);

    // if the new control is a group, then add it's controls
    if (pGUIControl->IsGroup())
    {
      TiXmlElement *pSubControl = pControl->FirstChildElement("control");
      while (pSubControl)
      {
        LoadControl(pSubControl, (CGUIControlGroup *)pGUIControl);
        pSubControl = pSubControl->NextSiblingElement("control");
      }
    }
  }
}

void CGUITabControl::LoadContent(TiXmlElement *content)
{
  TiXmlElement *root = content->FirstChildElement("content");
  if (!root)
    return;

  TiXmlPrinter printer;
  root->Accept(&printer);
  const char* str = printer.CStr();

  TiXmlElement *item = root->FirstChildElement("item");
  CGUIControlFactory factory;
  int itemCnt = 0;

  while (item)
  {
    itemCnt++;

    CStdString strLabel = item->Attribute("label");
    if (StringUtils::IsNaturalNumber(strLabel))
      strLabel = g_localizeStrings.Get(atoi(strLabel));
    else // we assume the skin xml's aren't encoded as UTF-8
      g_charsetConverter.unknownToUTF8(strLabel);

    if (strLabel.IsEmpty() || strLabel == "-")
    {
      strLabel.Format("label%d", itemCnt);
    }

    TiXmlElement* pControlNode = item->FirstChildElement();
    if (pControlNode && pControlNode->ValueStr() == "control")
    {
      CRect rect(0, 0, GetWidth(), GetHeight());
      CGUIControl* pView = factory.Create(GetParentID(), rect, pControlNode);
      if (pView)
      {
        if (pView->IsGroup())
        {
          TiXmlElement *pSubControl = pControlNode->FirstChildElement("control");
          while (pSubControl)
          {
            LoadControl(pSubControl, (CGUIControlGroup *)pView);
            pSubControl = pSubControl->NextSiblingElement("control");
          }
        }

        AddView(pView, strLabel);
      }
    }

    item = item->NextSiblingElement("item");
  }
}

#endif
