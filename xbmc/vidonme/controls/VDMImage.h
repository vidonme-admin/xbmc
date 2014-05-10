

#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "../guilib/GUIImage.h"

class CVDMImage : public CGUIImage
{
public:
  CVDMImage(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& texture, const CTextureInfo& textureBK, const CTextureInfo& textureBorder);
  virtual ~CVDMImage(void);
  virtual CVDMImage *Clone() const { return new CVDMImage(*this); };

  virtual void Process(unsigned int currentTime, CDirtyRegionList &dirtyregions);
  virtual void Render();
  virtual void AllocResources();
	virtual void DynamicResourceAlloc(bool bOnOff);
  virtual bool IsDynamicallyAllocated() { return m_bDynamicResourceAlloc; };
  virtual void SetInvalid();

  virtual void SetInfo(const CGUIInfoLabel &info);
  virtual void SetAspectRatio(const CAspectRatio &aspect);

protected:
  virtual void FreeTextures(bool immediately = false);

	CGUITexture m_textureBK;
	CGUITexture m_textureBorder;
};

#endif
