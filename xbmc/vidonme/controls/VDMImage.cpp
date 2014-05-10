

#if defined(__VIDONME_MEDIACENTER__)

#include "VDMImage.h"

using namespace std;

CVDMImage::CVDMImage(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& texture, const CTextureInfo& textureBK, const CTextureInfo& textureBorder)
    : CGUIImage(parentID, controlID, posX, posY, width, height, texture)
		, m_textureBK(posX, posY, width, height, textureBK)
		, m_textureBorder(posX, posY, width, height, textureBorder)
{
	ControlType = VDMCONTROL_IMAGE;
}

CVDMImage::~CVDMImage(void)
{
}

void CVDMImage::Process(unsigned int currentTime, CDirtyRegionList &dirtyregions)
{
  if (m_textureBK.SetDiffuseColor(m_diffuseColor))
    MarkDirtyRegion();

  if (m_textureBK.Process(currentTime))
    MarkDirtyRegion();

	if (m_textureBorder.SetDiffuseColor(m_diffuseColor))
		MarkDirtyRegion();

	if (m_textureBorder.Process(currentTime))
		MarkDirtyRegion();

  CGUIImage::Process(currentTime, dirtyregions);
}

void CVDMImage::Render()
{
	CRect rect;
	if (m_texture.ReadyToRender())
	{
		rect = m_texture.GetRenderRect();
		CGUIImage::Render();
	}
	else
	{
		rect = m_textureBK.GetRenderRect();
		m_textureBK.Render();
		CGUIControl::Render();
	}

	float fWidth = rect.Width();
	float fHight = rect.Height();
	m_textureBorder.SetPosition(rect.x1 - fWidth/10, rect.y1 - fHight/30);
	m_textureBorder.SetWidth(fWidth + fWidth/5);
	m_textureBorder.SetHeight(fHight + fHight/10);
	m_textureBorder.Render();
}

void CVDMImage::AllocResources()
{
	CGUIImage::AllocResources();
	m_textureBK.AllocResources();
  m_textureBorder.AllocResources();
}

void CVDMImage::FreeTextures(bool immediately)
{
	m_textureBK.FreeResources(immediately);
  m_textureBorder.FreeResources(immediately);

	CGUIImage::FreeTextures(immediately);
}

void CVDMImage::SetInvalid()
{
	m_textureBK.SetInvalid();
  m_textureBorder.SetInvalid();
  CGUIImage::SetInvalid();
}

void CVDMImage::DynamicResourceAlloc(bool bOnOff)
{
	m_textureBK.DynamicResourceAlloc(bOnOff);
  m_textureBorder.DynamicResourceAlloc(bOnOff);
  CGUIImage::DynamicResourceAlloc(bOnOff);
}

void CVDMImage::SetAspectRatio(const CAspectRatio &aspect)
{
	CAspectRatio aspectBorder;
	aspectBorder.align = ASPECT_ALIGNY_CENTER;
	m_textureBK.SetAspectRatio(aspect);
	m_textureBorder.SetAspectRatio(aspectBorder);
	CGUIImage::SetAspectRatio(aspect);
}

void CVDMImage::SetInfo(const CGUIInfoLabel &info)
{
	CGUIImage::SetInfo(info);
}

#endif