
#include "VDMDialog.h"
#include "GUIInfoManager.h"
#include "threads/SingleLock.h"
#include "utils/TimeUtils.h"
#include "Application.h"
#include "ApplicationMessenger.h"
#include "guilib/GUILabelControl.h"
#include "guilib/GUIWindowManager.h"

CVDMDialog::CVDMDialog(int id)
    : CVDMWindow(id)
{
  m_bModal = true;
  m_wasRunning = false;
  m_renderOrder = 1;
  m_autoClosing = false;
  m_showStartTime = 0;
  m_showDuration = 0;
  m_enableSound = true;
  m_bAutoClosed = false;
}

CVDMDialog::~CVDMDialog(void)
{}

void CVDMDialog::OnWindowLoaded()
{
  CVDMWindow::OnWindowLoaded();

  if (m_children.size())
  {
    CGUIControl* pBase = m_children[0];

    for (iControls p = m_children.begin() + 1; p != m_children.end(); ++p)
    {
      if ((*p)->GetControlType() == CGUIControl::GUICONTROL_LABEL)
      {
        CGUILabelControl* pLabel = (CGUILabelControl*)(*p);

        if (!pLabel->GetWidth())
        {
          float spacing = (pLabel->GetXPosition() - pBase->GetXPosition()) * 2;
          pLabel->SetWidth(pBase->GetWidth() - spacing);
        }
      }
    }
  }
}

bool CVDMDialog::OnAction(const CAction &action)
{
  if (!action.IsMouse() && m_autoClosing)
    SetAutoClose(m_showDuration);

  return CVDMWindow::OnAction(action);
}

bool CVDMDialog::OnBack(int actionID)
{
  Close();
  return true;
}

bool CVDMDialog::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_WINDOW_DEINIT:
    {
      CVDMWindow *pWindow = (CVDMWindow *)g_windowManager.GetWindow(g_windowManager.GetActiveWindow());
      if (pWindow)
        g_windowManager.ShowOverlay(pWindow->GetOverlayState());

      CVDMWindow::OnMessage(message);
      return true;
    }
  case GUI_MSG_WINDOW_INIT:
    {
      CVDMWindow::OnMessage(message);
      m_showStartTime = 0;
      return true;
    }
  }

  return CVDMWindow::OnMessage(message);
}

void CVDMDialog::OnDeinitWindow(int nextWindowID)
{
  if (m_active)
  {
    g_windowManager.RemoveDialog(GetID());
    m_autoClosing = false;
  }
  CVDMWindow::OnDeinitWindow(nextWindowID);
}

void CVDMDialog::DoProcess(unsigned int currentTime, CDirtyRegionList &dirtyregions)
{
  UpdateVisibility();

  if (!m_active && m_wasRunning)
    dirtyregions.push_back(m_renderRegion);

  if (m_active)
    CVDMWindow::DoProcess(currentTime, dirtyregions);

  m_wasRunning = m_active;
}

void CVDMDialog::UpdateVisibility()
{
  if (m_visibleCondition)
  {
    if (g_infoManager.GetBoolValue(m_visibleCondition))
      Show();
    else
      Close();
  }
}

void CVDMDialog::DoModal_Internal(int iWindowID, const CStdString &param)
{
  CSingleLock lock(g_graphicsContext);

  if (!g_windowManager.Initialized())
    return;

  m_closing = false;
  m_bModal = true;
  m_active = true;
  g_windowManager.RouteToWindow(this);

  CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0, 0, WINDOW_INVALID, iWindowID);
  msg.SetStringParam(param);
  OnMessage(msg);

  if (!m_windowLoaded)
    Close(true);

  lock.Leave();

  while (m_active && !g_application.m_bStop)
  {
    g_windowManager.ProcessRenderLoop();
  }
}

void CVDMDialog::Show_Internal()
{
  CSingleLock lock(g_graphicsContext);

  if (m_active && !m_closing && !IsAnimating(ANIM_TYPE_WINDOW_CLOSE)) return;

  if (!g_windowManager.Initialized())
    return;

  m_bModal = false;

  m_active = true;
  m_closing = false;
  g_windowManager.AddModeless(this);

  CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0, 0);
  OnMessage(msg);
}

void CVDMDialog::DoModal(int iWindowID, const CStdString &param)
{
  if (!g_application.IsCurrentThread())
  {
    CSingleExit leaveIt(g_graphicsContext);
    CApplicationMessenger::Get().DoModal(this, iWindowID, param);
  }
  else
    DoModal_Internal(iWindowID, param);
}

void CVDMDialog::Show()
{
  if (!g_application.IsCurrentThread())
  {
    CSingleExit leaveIt(g_graphicsContext);
    CApplicationMessenger::Get().Show(this);
  }
  else
    Show_Internal();
}

void CVDMDialog::FrameMove()
{
  if (m_autoClosing)
  {
    if (!m_showStartTime)
    {
      if (HasRendered())
        m_showStartTime = CTimeUtils::GetFrameTime();
    }
    else
    {
      if (m_showStartTime + m_showDuration < CTimeUtils::GetFrameTime() && !m_closing)
      {
        m_bAutoClosed = true;
        Close();
      }
    }
  }
  CVDMWindow::FrameMove();
}

void CVDMDialog::Render()
{
  if (!m_active)
    return;

  CVDMWindow::Render();
}

void CVDMDialog::SetDefaults()
{
  CVDMWindow::SetDefaults();
  m_renderOrder = 1;
}

void CVDMDialog::SetAutoClose(unsigned int timeoutMs)
{
   m_autoClosing = true;
   m_showDuration = timeoutMs;
   ResetAutoClose();
}

void CVDMDialog::ResetAutoClose(void)
{
  if (m_autoClosing && m_active)
    m_showStartTime = CTimeUtils::GetFrameTime();
}
