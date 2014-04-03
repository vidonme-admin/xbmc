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

#include "InputOperations.h"
#include "Application.h"
#include "ApplicationMessenger.h"
#include "guilib/GUIAudioManager.h"
#include "guilib/GUIWindow.h"
#include "guilib/GUIWindowManager.h"
#include "input/ButtonTranslator.h"
#include "input/XBMC_keyboard.h"
#include "input/XBMC_vkeys.h"
#include "threads/SingleLock.h"
#include "utils/CharsetConverter.h"
#if defined(__VIDONME_MEDIACENTER__)
#include "input/MouseStat.h"
#include "vidonme/VDMUtils.h"
#endif
using namespace JSONRPC;

//TODO the breakage of the screensaver should be refactored
//to one central super duper place for getting rid of
//1 million dupes
bool CInputOperations::handleScreenSaver()
{
  g_application.ResetScreenSaver();
  if (g_application.WakeUpScreenSaverAndDPMS())
    return true;

  return false;
}

JSONRPC_STATUS CInputOperations::SendAction(int actionID, bool wakeScreensaver /* = true */, bool waitResult /* = false */)
{
  if(!wakeScreensaver || !handleScreenSaver())
  {
    g_application.ResetSystemIdleTimer();
    g_audioManager.PlayActionSound(actionID);
    CApplicationMessenger::Get().SendAction(CAction(actionID), WINDOW_INVALID, waitResult);
#if defined(__VIDONME_MEDIACENTER__)
    g_Mouse.SetActive(false);
#endif
  }
  return ACK;
}

JSONRPC_STATUS CInputOperations::activateWindow(int windowID)
{
  if(!handleScreenSaver())
    CApplicationMessenger::Get().ActivateWindow(windowID, std::vector<CStdString>(), false);

  return ACK;
}

JSONRPC_STATUS CInputOperations::SendText(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CGUIWindow *window = g_windowManager.GetWindow(g_windowManager.GetFocusedWindow());
  if (!window)
    return InternalError;

  CGUIMessage msg(GUI_MSG_SET_TEXT, 0, 0);
  msg.SetLabel(parameterObject["text"].asString());
  msg.SetParam1(parameterObject["done"].asBoolean() ? 1 : 0);
  CApplicationMessenger::Get().SendGUIMessage(msg, window->GetID());
  return ACK;
}

JSONRPC_STATUS CInputOperations::ExecuteAction(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  int action;
  if (!CButtonTranslator::TranslateActionString(parameterObject["action"].asString().c_str(), action))
    return InvalidParams;

  return SendAction(action);
}

JSONRPC_STATUS CInputOperations::Left(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
#if defined(__VIDONME_MEDIACENTER__)

	CKey key(XBMCVK_LEFT | KEY_VKEY);

	int iWin = g_windowManager.GetActiveWindow() & WINDOW_ID_MASK;

	if (g_windowManager.HasModalDialog())
	{
		iWin = g_windowManager.GetTopMostModalDialogID() & WINDOW_ID_MASK;
	}

	CAction action = CButtonTranslator::GetInstance().GetAction(iWin, key);

	return SendActionEx(action);

#else

	return SendAction(ACTION_MOVE_LEFT);

#endif
}

JSONRPC_STATUS CInputOperations::Right(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
#if defined(__VIDONME_MEDIACENTER__)

	CKey key(XBMCVK_RIGHT | KEY_VKEY);

	int iWin = g_windowManager.GetActiveWindow() & WINDOW_ID_MASK;

	if (g_windowManager.HasModalDialog())
	{
		iWin = g_windowManager.GetTopMostModalDialogID() & WINDOW_ID_MASK;
	}

	CAction action = CButtonTranslator::GetInstance().GetAction(iWin, key);

	return SendActionEx(action);

#else

  return SendAction(ACTION_MOVE_RIGHT);

#endif
}

JSONRPC_STATUS CInputOperations::Down(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
#if defined(__VIDONME_MEDIACENTER__)

	CKey key(XBMCVK_DOWN | KEY_VKEY);

	int iWin = g_windowManager.GetActiveWindow() & WINDOW_ID_MASK;

	if (g_windowManager.HasModalDialog())
	{
		iWin = g_windowManager.GetTopMostModalDialogID() & WINDOW_ID_MASK;
	}

	CAction action = CButtonTranslator::GetInstance().GetAction(iWin, key);

	return SendActionEx(action);

#else

	return SendAction(ACTION_MOVE_DOWN);

#endif
}

JSONRPC_STATUS CInputOperations::Up(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
#if defined(__VIDONME_MEDIACENTER__)

	CKey key(XBMCVK_UP | KEY_VKEY);

	int iWin = g_windowManager.GetActiveWindow() & WINDOW_ID_MASK;

	if (g_windowManager.HasModalDialog())
	{
		iWin = g_windowManager.GetTopMostModalDialogID() & WINDOW_ID_MASK;
	}

	CAction action = CButtonTranslator::GetInstance().GetAction(iWin, key);

	return SendActionEx(action);

#else

	return SendAction(ACTION_MOVE_UP);

#endif
}

JSONRPC_STATUS CInputOperations::Select(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
#if defined(__VIDONME_MEDIACENTER__)

	CKey key(XBMCVK_RETURN | KEY_VKEY);

	int iWin = g_windowManager.GetActiveWindow() & WINDOW_ID_MASK;

	if (g_windowManager.HasModalDialog())
	{
		iWin = g_windowManager.GetTopMostModalDialogID() & WINDOW_ID_MASK;
	}

	CAction action = CButtonTranslator::GetInstance().GetAction(iWin, key);

	return SendActionEx(action);

#else

		return SendAction(ACTION_SELECT_ITEM);

#endif
}

JSONRPC_STATUS CInputOperations::Back(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  return SendAction(ACTION_NAV_BACK);
}

JSONRPC_STATUS CInputOperations::ContextMenu(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  return SendAction(ACTION_CONTEXT_MENU);
}

JSONRPC_STATUS CInputOperations::Info(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  return SendAction(ACTION_SHOW_INFO);
}

JSONRPC_STATUS CInputOperations::Home(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
#if defined(__VIDONME_MEDIACENTER__)
	if (VidOnMe::RM_VIDONME == VidOnMe::VDMUtils::Instance().GetRunningMode())
	{
		int i = 0;
		while (!g_windowManager.IsWindowTopMost(VDM_WINDOW_HOME) && i < 10)
		{
			SendAction(ACTION_NAV_BACK);
			++i;
		}

		return activateWindow(VDM_WINDOW_HOME);
	}
	else
	{
		return activateWindow(WINDOW_HOME);
	}
	return activateWindow(VDM_WINDOW_HOME);
#else
	return activateWindow(WINDOW_HOME);
#endif
}

JSONRPC_STATUS CInputOperations::ShowCodec(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  return SendAction(ACTION_SHOW_CODEC);
}

JSONRPC_STATUS CInputOperations::ShowOSD(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  return SendAction(ACTION_SHOW_OSD);
}

#if defined(__VIDONME_MEDIACENTER__)

JSONRPC_STATUS CInputOperations::TopMenu(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
	return SendActionEx(CAction(ACTION_SHOW_VIDEOMENU, 1.0f));
}

JSONRPC_STATUS CInputOperations::PopMenu(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
	return SendActionEx(CAction(ACTION_SHOW_VIDEOMENU, 0.0f));
}

JSONRPC_STATUS CInputOperations::Menu(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
	CKey key(XBMCVK_M|KEY_VKEY);
	return SendActionEx(CAction(ACTION_SHOW_OSD, "", key));
}

JSONRPC_STATUS CInputOperations::VolumeUp(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
	return SendAction(ACTION_VOLUME_UP);
}

JSONRPC_STATUS CInputOperations::VolumeDown(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
	return SendAction(ACTION_VOLUME_DOWN);
}

JSONRPC_STATUS CInputOperations::Audio(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
	return SendAction(ACTION_AUDIO_NEXT_LANGUAGE);
}

JSONRPC_STATUS CInputOperations::Subtitle(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
	return SendAction(ACTION_NEXT_SUBTITLE);
}

JSONRPC_STATUS CInputOperations::Angle(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
	//return SendAction(ACTION_BIG_STEP_BACK);
	return ACK;
}

JSONRPC_STATUS CInputOperations::SetMute(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
	return SendAction(ACTION_MUTE);
}

JSONRPC_STATUS CInputOperations::SendActionEx(CAction action, bool wakeScreensaver /* = true */, bool waitResult /* = false */)
{
	if(!wakeScreensaver || !handleScreenSaver())
	{
		g_application.ResetSystemIdleTimer();
		g_audioManager.PlayActionSound(action.GetID());
		CApplicationMessenger::Get().SendAction(action, WINDOW_INVALID, waitResult);
		g_Mouse.SetActive(false);
	}
	return ACK;
}

#endif