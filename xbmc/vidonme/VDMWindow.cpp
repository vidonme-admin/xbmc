
#if defined(__VIDONME_MEDIACENTER__)

#include "utils/XMLUtils.h"
#include "VDMWindow.h"


#define		VDM_WINDOW(string, id)\
{\
string.Format(\
"<window id=\"%d\">\n \
<controls>\n \
</controls>\n \
</window>\n"\
, id);\
}

#define		VDM_DEFAULT_CONTROL(string, id)\
{\
string.Format(\
"<defaultcontrol always=\"true\">%d</defaultcontrol>\n"\
, id);\
}

#define		VDM_VISIBLE_CONTROL(string, condition)\
{\
  string.Format(\
  "<visible>%s</visible>\n"\
  , condition);\
}

#define		VDM_GROUP(string, id, posx, posy, width, height)\
{\
string.Format(\
"<control type=\"group\" id=\"%d\">\n \
<posx>%d</posx>\n \
<posy>%d</posy>\n \
<width>%d</width>\n \
<height>%d</height>\n \
</control>\n"\
, id, posx, posy, width, height\
);\
}

#define		VDM_IMAGE(string, id, posx, posy, width, height, texture)\
{\
string.Format(\
"<control type=\"image\" id=\"%d\">\n \
<posx>%d</posx>\n \
<posy>%d</posy>\n \
<width>%d</width>\n \
<height>%d</height>\n \
<texture>%s</texture>\n \
</control>\n"\
, id, posx, posy, width, height, texture\
);\
}

#define		VDM_LABEL(string, id, posx, posy, width, height, label, font, textcolor, wrapmultiline, scroll, align, aligny)\
{\
string.Format(\
"<control type=\"label\" id=\"%d\">\n \
<posx>%d</posx>\n \
<posy>%d</posy>\n \
<width>%d</width>\n \
<height>%d</height>\n \
<label>%s</label>\n \
<font>%s</font>\n \
<textcolor>%s</textcolor>\n \
<wrapmultiline>%s</wrapmultiline>\n \
<scroll>%s</scroll>\n \
<align>%s</align>\n \
<aligny>%s</aligny>\n \
</control>\n"\
, id, posx, posy, width, height, label, font, textcolor, wrapmultiline, scroll, align, aligny\
);\
}

#define		VDM_TEXT(string, id, posx, posy, width, height, label, font, textcolor, shadowcolor, align, aligny)\
{\
	string.Format(\
	"<control type=\"textbox\" id=\"%d\">\n \
<posx>%d</posx>\n \
<posy>%d</posy>\n \
<width>%d</width>\n \
<height>%d</height>\n \
<label>%s</label>\n \
<font>%s</font>\n \
<textcolor>%s</textcolor>\n \
<shadowcolor>%s</shadowcolor>\n \
<align>%s</align>\n \
<aligny>%s</aligny>\n \
</control>\n"\
	, id, posx, posy, width, height, label, font, textcolor, shadowcolor, align, aligny\
	);\
}

#define		VDM_EDIT(string, id, posx, posy, width, height, label, font, textcolor, focusedcolor, shadowcolor, aligny, texturefocus, texturenofocus, onleft, onright, onup, ondown)\
{\
	string.Format(\
	"<control type=\"edit\" id=\"%d\">\n \
<posx>%d</posx>\n \
<posy>%d</posy>\n \
<width>%d</width>\n \
<height>%d</height>\n \
<label>%s</label>\n \
<font>%s</font>\n \
<textcolor>%s</textcolor>\n \
<focusedcolor>%s</focusedcolor>\n \
<shadowcolor>%s</shadowcolor>\n \
<aligny>%s</aligny>\n \
<texturefocus>%s</texturefocus>\n \
<texturenofocus>%s</texturenofocus>\n"\
	, id, posx, posy, width, height, label, font, textcolor, focusedcolor, shadowcolor, aligny, texturefocus, texturenofocus\
	);\
	\
	string.Format(\
	"%s\
<onleft>%d</onleft>\n \
<onright>%d</onright>\n \
<onup>%d</onup>\n \
<ondown>%d</ondown>\n \
</control>\n"\
	, string, onleft, onright, onup, ondown\
	);\
}

#define		VDM_BUTTON(string, id, posx, posy, width, height, label, font, textcolor, focusedcolor, textoffsetx, align, aligny, texturefocus, texturenofocus, onclick, onleft, onright, onup, ondown, wrapmultiline)\
{\
string.Format(\
"<control type=\"button\" id=\"%d\">\n \
<posx>%d</posx>\n \
<posy>%d</posy>\n \
<width>%d</width>\n \
<height>%d</height>\n \
<label>%s</label>\n \
<font>%s</font>\n \
<textcolor>%s</textcolor>\n \
<focusedcolor>%s</focusedcolor>\n \
<textoffsetx>%d</textoffsetx>\n \
<align>%s</align>\n \
<aligny>%s</aligny>\n \
<texturefocus>%s</texturefocus>\n \
<texturenofocus>%s</texturenofocus>\n \
<onclick>%s</onclick>\n"\
, id, posx, posy, width, height, label, font, textcolor, focusedcolor, textoffsetx, align, aligny, texturefocus, texturenofocus, onclick\
);\
\
string.Format(\
"%s\
<onleft>%d</onleft>\n \
<onright>%d</onright>\n \
<onup>%d</onup>\n \
<ondown>%d</ondown>\n \
<wrapmultiline>%s</wrapmultiline>\n \
</control>\n"\
, string, onleft, onright, onup, ondown, wrapmultiline\
);\
}


#define		VDM_SLIDER(string, id, posx, posy, width, height, texturesliderbar, texturesliderbarfocus, textureslidernib, textureslidernibfocus)\
{\
  string.Format(\
"<control type=\"vdmslider\" id=\"%d\">\n \
<posx>%d</posx>\n \
<posy>%d</posy>\n \
<width>%d</width>\n \
<height>%d</height>\n \
<texturesliderbar>%s</texturesliderbar>\n \
<texturesliderbarfocus>%s</texturesliderbarfocus>\n \
<textureslidernib>%s</textureslidernib>\n \
<textureslidernibfocus>%d</textureslidernibfocus>\n \
</control>\n"\
  , id,posx, posy, width, height, texturesliderbar, texturesliderbarfocus, textureslidernib, textureslidernibfocus\
  );\
}

CVDMWindow::CVDMWindow(int id)
	:CGUIWindow(id, "NULL")
{
	m_strContext.clear();
	VDM_WINDOW(m_strContext, id);
}

CVDMWindow::~CVDMWindow(void)
{
}

void CVDMWindow::AddDefaultControl(unsigned int id)
{
	CStdString strDefaultControl;
	VDM_DEFAULT_CONTROL(strDefaultControl, id);

	m_strContext.Insert(m_strContext.Find("\">") + 3, strDefaultControl);
}

void CVDMWindow::AddGroup(unsigned int posx, 
													unsigned int posy, 
													unsigned int width, 
													unsigned int height,
													unsigned int id
													)
{
	CStdString strGroup;
	VDM_GROUP(strGroup, id, posx, posy, width, height);

	m_strContext.Insert(m_strContext.ReverseFind("</controls>"), strGroup);
}

void CVDMWindow::AddImage(unsigned int posx, 
													unsigned int posy, 
													unsigned int width, 
													unsigned int height, 
													CStdString texture,
													unsigned int id
													)
{
	CStdString strImage;
	VDM_IMAGE(strImage, id, posx, posy, width, height, texture);

	m_strContext.Insert(m_strContext.ReverseFind("</control>"), strImage);
}

void CVDMWindow::AddLabel(unsigned int posx, 
													unsigned int posy, 
													unsigned int width, 
													unsigned int height, 
													CStdString label, 
													unsigned int id, 
													CStdString font, 
													CStdString textcolor, 
													CStdString wrapmultiline, 
													CStdString align, 
													CStdString aligny,
													CStdString scroll)
{
	CStdString strLabel;
	VDM_LABEL(strLabel, id, posx, posy, width, height, label, font, textcolor, wrapmultiline, scroll, align, aligny);

	m_strContext.Insert(m_strContext.ReverseFind("</control>"), strLabel);
}

void CVDMWindow::AddTextBox(unsigned int posx, 
	unsigned int posy, 
	unsigned int width, 
	unsigned int height, 
	CStdString label, 
	unsigned int id, 
	CStdString font, 
	CStdString textcolor, 
	CStdString shadowcolor, 
	CStdString align, 
	CStdString aligny)
{
	CStdString strLabel;
	VDM_TEXT(strLabel, id, posx, posy, width, height, label, font, textcolor, shadowcolor, align, aligny);

	m_strContext.Insert(m_strContext.ReverseFind("</control>"), strLabel);
}

void CVDMWindow::AddButton(unsigned int posx, 
													unsigned int posy, 
													unsigned int width, 
													unsigned int height, 
													CStdString label, 
													unsigned int id, 
													CStdString texturefocus, 
													CStdString texturenofocus, 
													CStdString font, 
													CStdString textcolor, 
													CStdString focusedcolor,
													unsigned int textoffsetx,
													CStdString onclick, 
													CStdString align, 
													CStdString aligny, 
													unsigned int onleft, 
													unsigned int onright, 
													unsigned int onup, 
													unsigned int ondown, 
													CStdString wrapmultiline)
{
	CStdString strLabel;
	VDM_BUTTON(strLabel, id, posx, posy, width, height, label, font, textcolor, focusedcolor, 
		textoffsetx, align, aligny, texturefocus, texturenofocus, onclick, onleft, onright, onup, ondown, wrapmultiline);

	m_strContext.Insert(m_strContext.ReverseFind("</control>"), strLabel);
}

void CVDMWindow::AddEdit(unsigned int posx, 
												unsigned int posy, 
												unsigned int width, 
												unsigned int height, 
												CStdString label, 
												unsigned int id, 
												CStdString texturefocus, 
												CStdString texturenofocus, 
												CStdString font, 
												CStdString textcolor, 
												CStdString focusedcolor,
												CStdString shadowcolor,
												CStdString aligny, 
												unsigned int onleft, 
												unsigned int onright, 
												unsigned int onup, 
												unsigned int ondown)
{
	CStdString strLabel;
	VDM_EDIT(strLabel, id, posx, posy, width, height, label, font, textcolor, focusedcolor, shadowcolor, 
		aligny, texturefocus, texturenofocus, onleft, onright, onup, ondown);

	m_strContext.Insert(m_strContext.ReverseFind("</control>"), strLabel);
}

void CVDMWindow::AddSlider(unsigned int posx, 
                          unsigned int posy, 
                          unsigned int width, 
                          unsigned int height, 
                          unsigned int id , 
                          CStdString texturesliderbar, 
                          CStdString texturesliderbarfocus, 
                          CStdString textureslidernib, 
                          CStdString textureslidernibfocus
                          )
{
  	CStdString strLabel;
    VDM_SLIDER(strLabel, id, posx, posy, width, height, texturesliderbar, texturesliderbarfocus, textureslidernib, textureslidernibfocus);

    m_strContext.Insert(m_strContext.ReverseFind("</control>"), strLabel);
}

void CVDMWindow::VisibleControl(CStdString condition)
{
  CStdString strVisibleControl;
  VDM_VISIBLE_CONTROL(strVisibleControl, condition);

  m_strContext.Insert(m_strContext.Find("\">") + 3, strVisibleControl);
}


bool CVDMWindow::Load(const CStdString& strFileName, bool bContainsPath)
{
	CXBMCTinyXML xmlDoc;
	xmlDoc.Parse(m_strContext.c_str(),0,TIXML_ENCODING_UTF8);
	m_windowXMLRootElement = (TiXmlElement*)xmlDoc.RootElement()->Clone();
	
	return CGUIWindow::Load(m_windowXMLRootElement);
}

#endif