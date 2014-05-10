
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMBuyNoteDlg.h"
#include "guilib/Key.h"
#include "guilib/GUIWindowManager.h"


CVDMBuyNoteDlg::CVDMBuyNoteDlg(void) 
	: CVDMDialog(VDM_WINDOW_BUY_NOTE)
{
	m_loadType = LOAD_ON_GUI_INIT;

	AddDefaultControl(100);
	AddGroup(0,0,1280, 720);
	AddImage(396, 220, 488, 282, "special://xbmc/media/BuyNote_BK1.png");
	AddImage(409, 232, 462, 186, "special://xbmc/media/BuyNote_BK2.png");
	AddLabel(432, 260, 415, 30, "60326", 0, "font12", "FFFFFFFF");
	AddLabel(432, 292, 415, 30, "60327", 0, "font12", "FFFFFFFF");
	AddLabel(432, 324, 415, 30, "60328", 0, "font12", "FFFFFFFF");

	AddButton(558, 440, 164, 42, "12321", 100, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font12", 
		"FF999999", "FFFFFFFF", 20, "PreviousMenu", "center", "center", 100, 100, 100, 100);
}

CVDMBuyNoteDlg::~CVDMBuyNoteDlg(void)
{
}

void CVDMBuyNoteDlg::OnInitWindow()
{
	CVDMDialog::OnInitWindow();
}


#endif