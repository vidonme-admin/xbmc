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

#include "GUIDialogMuteBug.h"
#include "GUIUserMessages.h"
#include "settings/Settings.h"

#if defined(__VIDONME_MEDIACENTER__)
#include "Application.h"
#endif

// the MuteBug is a true modeless dialog

CGUIDialogMuteBug::CGUIDialogMuteBug(void)
    : CGUIDialog(WINDOW_DIALOG_MUTE_BUG, "DialogMuteBug.xml")
{
  m_loadType = LOAD_ON_GUI_INIT;
}

CGUIDialogMuteBug::~CGUIDialogMuteBug(void)
{}

void CGUIDialogMuteBug::UpdateVisibility()
{
#if defined(__VIDONME_MEDIACENTER__)
	if ((g_settings.m_bMute && g_application.IsDolbyAndDTSValible()) || g_settings.m_fVolumeLevel == VOLUME_MINIMUM)
#else
  if (g_settings.m_bMute || g_settings.m_fVolumeLevel == VOLUME_MINIMUM)
#endif
    Show();
  else
    Close();
}
