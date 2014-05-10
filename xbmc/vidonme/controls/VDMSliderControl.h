

#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "../../guilib/GUISliderControl.h"

class CVDMSliderControl :
      public CGUISliderControl
{
public:
  CVDMSliderControl(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& backGroundTexture, const CTextureInfo& backGroundTexture2, const CTextureInfo& nibTexture, const CTextureInfo& nibTextureFocus, int iType);
  virtual ~CVDMSliderControl(void);
  virtual CVDMSliderControl *Clone() const { return new CVDMSliderControl(*this); };

  virtual void Process(unsigned int currentTime, CDirtyRegionList &dirtyregions);
  virtual void Render();
  virtual void AllocResources();
  virtual void FreeResources(bool immediately = false);
  virtual void DynamicResourceAlloc(bool bOnOff);
  virtual void SetInvalid();
	virtual void SetType(int nType);

protected:
  virtual bool HitTest(const CPoint &point) const;
  virtual bool UpdateColors();

  CGUITexture m_guiBackground2;
};

#endif
