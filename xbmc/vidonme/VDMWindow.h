

#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "guilib/GUIWindow.h"
#include "utils/StdString.h"

class CVDMWindow : public CGUIWindow
{
public:
	CVDMWindow(int id);
	virtual ~CVDMWindow(void);

	void AddDefaultControl(unsigned int id);

	void AddGroup(unsigned int posx, 
								unsigned int posy, 
								unsigned int width, 
								unsigned int height,
								unsigned int id = 0 
								);
	
	void AddImage(unsigned int posx, 
								unsigned int posy, 
								unsigned int width, 
								unsigned int height,
								CStdString texture = "",
								unsigned int id = 0
								);
	
	void AddLabel(unsigned int posx, 
								unsigned int posy, 
								unsigned int width, 
								unsigned int height,
								CStdString label = "", 
								unsigned int id = 0, 
								CStdString font = "font13", 
								CStdString textcolor = "white", 
								CStdString wrapmultiline = "false", 
								CStdString align = "left", 
								CStdString aligny = "up",
								CStdString scroll = "false"
								);

	void AddTextBox(unsigned int posx, 
									unsigned int posy, 
									unsigned int width, 
									unsigned int height,
									CStdString label = "", 
									unsigned int id = 0, 
									CStdString font = "font13", 
									CStdString textcolor = "white", 
									CStdString shadowcolor = "black", 
									CStdString align = "left", 
									CStdString aligny = "up"
									);

	void AddButton(unsigned int posx, 
								unsigned int posy, 
								unsigned int width, 
								unsigned int height, 
								CStdString label = "", 
								unsigned int id = 0, 
								CStdString texturefocus = "", 
								CStdString texturenofocus = "", 
								CStdString font = "font13", 
								CStdString textcolor = "grey2", 
								CStdString focusedcolor = "white",
								unsigned int textoffsetx = 20,
								CStdString onclick = "", 
								CStdString align = "center", 
								CStdString aligny = "center", 
								unsigned int onleft = 0, 
								unsigned int onright = 0, 
								unsigned int onup = 0, 
								unsigned int ondown = 0, 
								CStdString wrapmultiline = "false" 
								);

	void AddEdit(unsigned int posx, 
							unsigned int posy, 
							unsigned int width, 
							unsigned int height, 
							CStdString label = "", 
							unsigned int id = 0, 
							CStdString texturefocus = "", 
							CStdString texturenofocus = "", 
							CStdString font = "font13", 
							CStdString textcolor = "grey2", 
							CStdString focusedcolor = "white",
							CStdString shadowcolor = "black",
							CStdString aligny = "center", 
							unsigned int onleft = 0, 
							unsigned int onright = 0, 
							unsigned int onup = 0, 
							unsigned int ondown = 0
							);
  void AddSlider(unsigned int posx, 
              unsigned int posy, 
              unsigned int width, 
              unsigned int height, 
              unsigned int id = 0, 
              CStdString texturesliderbar = "", 
              CStdString texturesliderbarfocus = "", 
              CStdString textureslidernib = "", 
              CStdString textureslidernibfocus = ""
              );
  void VisibleControl(CStdString condition);

bool Load(const CStdString& strFileName, bool bContainsPath = false);

private:
	CStdString m_strContext;
};

#endif