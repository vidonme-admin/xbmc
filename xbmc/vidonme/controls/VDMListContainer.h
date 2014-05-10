
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "guilib/GUIListContainer.h"

class CVDMListContainer : public CGUIListContainer
{
public:
  CVDMListContainer(int parentID, int controlID, float posX, float posY, float width, float height, ORIENTATION orientation, const CScroller& scroller, int preloadItems);
  CVDMListContainer(int parentID, int controlID, float posX, float posY, float width, float height,
                         const CLabelInfo& labelInfo, const CLabelInfo& labelInfo2,
                         const CTextureInfo& textureButton, const CTextureInfo& textureButtonFocus,
                         float textureHeight, float itemWidth, float itemHeight, float spaceBetweenItems);
  virtual ~CVDMListContainer(void);
  virtual CVDMListContainer *Clone() const { return new CVDMListContainer(*this); };

protected:
	virtual void Render();
	virtual void Process(unsigned int currentTime, CDirtyRegionList &dirtyregions);
};

#endif

