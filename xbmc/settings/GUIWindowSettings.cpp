/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "system.h"
#include "GUIWindowSettings.h"
#include "guilib/Key.h"

#if defined(__VIDONME_MEDIACENTER_ANDROID__)
#include "../guilib/GUILabelControl.h"
#include "../guilib/LocalizeStrings.h"
#include "GUISettings.h"
#include "Application.h"
#include "android/activity/XBMCApp.h"

static CStdString ConvertVersionString(CStdString& numVersion)
{
  if (numVersion.GetLength() >= 3)
  {
    int len = numVersion.GetLength();
    numVersion.Insert(len - 3, '.');
    numVersion.Insert(len - 1, '.');
    numVersion.Insert(len + 1, '.');
  }

  return numVersion;
}
#endif

CGUIWindowSettings::CGUIWindowSettings(void)
    : CGUIWindow(WINDOW_SETTINGS_MENU, "Settings.xml")
{
  m_loadType = KEEP_IN_MEMORY;
}

CGUIWindowSettings::~CGUIWindowSettings(void)
{
}

#if defined(__VIDONME_MEDIACENTER_ANDROID__)
void CGUIWindowSettings::OnInitWindow()
{
#if defined(__VIDONME_MEDIACENTER_ANDROID__)
  CGUILabelControl*	pVersion = (CGUILabelControl*)GetControl(21002);
  if (pVersion)
  {	
    CStdString	strVersion = VDMUtils::Instance().GetCurrentVersion();
    strVersion.Format(g_localizeStrings.Get(70060) , ConvertVersionString(strVersion));
    pVersion->SetLabel(strVersion);
  }
#endif
  CGUIWindow::OnInitWindow();
}

bool CGUIWindowSettings::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_CLICKED:
    if (message.GetSenderId() == 21103)
    {
#if defined(TARGET_ANDROID)
      CXBMCApp::StartBrowserActivity("com.android.browser", "http://www.vidon.me/");
#endif
    }    
  default:
    break;
  }

  return CGUIWindow::OnMessage(message);
}
#endif
