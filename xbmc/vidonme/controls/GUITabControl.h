#ifndef GUILIB_TABCONTROL_H
#define GUILIB_TABCONTROL_H

#pragma once

#include "guilib/GUIControlGroup.h"
#include "guilib/GUITexture.h"
#include "guilib/GUILabel.h"

class CGUIControlGroupList;
class CGUIScrollBar;
class CGUIImage;

class CGUITabControl : public CGUIControlGroup
{
public:
  enum TABORIENTATION
  {
    LEFT,
    TOP,
    RIGHT,
    BOTTOM
  };

  CGUITabControl(int parentID, int controlID, float posX, float posY, float width, float height,
    TABORIENTATION tabOrientation, float tabBarSize, float tabBarLength, uint32_t tabBarAlignment, const CTextureInfo& tabBarTextureBackground, float tabBarPadding1, float tabBarPadding2, 
    float tabItemSize, float tabItemLength, float tabItemGap, uint32_t tabItemAlignment, const CScroller& scroller, 
    const CTextureInfo& tabItemTextureFocus, const CTextureInfo& tabItemTextureNoFocus, const CTextureInfo& tabItemTextureOn, const CLabelInfo& tabItemLabelInfo, unsigned char alphaCurrentTab,
    const CTextureInfo& viewTextureBackground,
    float barWidth, float barHeight, const CTextureInfo& barBackgroundTexture, const CTextureInfo& barTexture, const CTextureInfo& barTextureFocus, const CTextureInfo& nibTexture, const CTextureInfo& nibTextureFocus, bool showOnePage);
  virtual ~CGUITabControl(void);
  virtual CGUITabControl* Clone() const { return new CGUITabControl(*this); }

  void AddView(CGUIControl* pView, const CStdString& strTabLabel, int indexView = -1, bool bActive = false);
  CGUIControl* GetView(int indexView) const;
  CGUIControl* GetView(const CStdString& strTabLabel) const;
  CGUIControl* RemoveView(int indexView);
  CGUIControl* RemoveView(const CStdString& strTabLabel);
  CGUIControl* RemoveView(CGUIControl* pView);
  bool DeleteView(int indexView);
  bool DeleteView(const CStdString& strTabLabel);
  bool DeleteView(CGUIControl* pView);
  bool SetActiveView(int indexView);
  bool SetActiveView(const CStdString& strTabLabel);
  bool SetActiveView(CGUIControl* pView);
  CGUIControl* GetActiveView() const;
  int GetActiveViewIndex() const;
  void ClearAll();

	void LoadContent(TiXmlElement *content);
	bool HasFocus() const;

protected:
  virtual void Process(unsigned int currentTime, CDirtyRegionList& dirtyregions);
  virtual void Render();

  virtual EVENT_RESULT SendMouseEvent(const CPoint &point, const CMouseEvent &event);
  virtual bool OnMouseOver(const CPoint &point);
  virtual EVENT_RESULT OnMouseEvent(const CPoint &point, const CMouseEvent &event);

  virtual bool OnAction(const CAction &action);
  virtual bool OnMessage(CGUIMessage& message);

  virtual void UnfocusFromPoint(const CPoint &point);
  virtual bool HitTest(const CPoint &point) const;
  virtual void SetInvalid();
  
  virtual void OnFocus();
  virtual void OnUnFocus();
  virtual CGUIControl* GetFirstFocusableControl(int id);

  virtual void AllocResources();
  virtual void FreeResources(bool immediately = false);
  virtual void DynamicResourceAlloc(bool bOnOff);


  bool HasTabID(int id) const;
  bool HasViewID(int id) const;

  void AddDefaultView();
  void SetChildViewNavigation(CGUIControl* pView, int tabID);
  void LoadControl(TiXmlElement* pControl, CGUIControlGroup *pGroup);
protected:
  typedef std::pair<CGUIControl*, CGUIControl*> TabViewBundle;
  std::vector<TabViewBundle> m_vecTabViews;
  
  TABORIENTATION m_tabOrientation;
  uint32_t m_tabBarAlignment;
  CGUIImage* m_pImageTabBackground;

  CTextureInfo m_tabItemTextureFocus;
  CTextureInfo m_tabItemTextureNoFocus;
  CTextureInfo m_tabItemTextureOn;
  CLabelInfo m_tabItemLabelInfo;
  float m_tabItemSize;
  float m_tabItemLength;
  unsigned char m_alphaCurrentTab;

  CGUIImage* m_pImageViewBackground;

  CGUIControlGroupList* m_pListTabs;  
  CGUIControlGroup* m_pGroupViews;
  CGUIScrollBar* m_pScrollbar;

  int m_currentView;
  bool m_bDefaultViewAdded;
};

#endif