// NewUIEventTimer.cpp: the H-key event timer board.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/NewUI/Events/NewUIEventTimer.h"
#include "UI/NewUI/NewUISystem.h"
#include "Guild/NewUIGuildInfoWindow.h"
#include "Audio/DSPlaySound.h"

using namespace SEASON3B;

SEASON3B::CNewUIEventTimer* g_pEventTimer = NULL;

CNewUIEventTimer::CNewUIEventTimer()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iRowCount = 0;
    m_dwLastUpdateTick = 0;
    ZeroMemory(m_Rows, sizeof(m_Rows));
}

CNewUIEventTimer::~CNewUIEventTimer()
{
    Release();
}

bool CNewUIEventTimer::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_EVENTTIMER, this);

    SetPos(x, y);
    Show(false);

    g_pEventTimer = this;
    return true;
}

void CNewUIEventTimer::Release()
{
    if (g_pEventTimer == this)
        g_pEventTimer = NULL;

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIEventTimer::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

const wchar_t* CNewUIEventTimer::EventName(BYTE type)
{
    switch (type)
    {
    case 0:  return L"Blood Castle";
    case 1:  return L"Devil Square";
    case 2:  return L"Chaos Castle";
    case 3:  return L"Golden Invasion";
    case 10: return L"Golden - Budge Dragon";
    case 11: return L"Golden - Goblin";
    case 12: return L"Golden - Soldier";
    case 13: return L"Golden - Titan";
    case 14: return L"Golden - Vepar";
    case 15: return L"Golden - Lizard King";
    case 16: return L"Golden - Wheel";
    case 17: return L"Golden - Tantallos";
    case 18: return L"Golden - Dragon";
    default: return L"Event";
    }
}

int CNewUIEventTimer::PanelHeight() const
{
    int rows = (m_iRowCount > 0) ? m_iRowCount : 1;
    return EVENTTIMER_TOP_H + (rows * EVENTTIMER_ROW_HEIGHT) + EVENTTIMER_PAD + EVENTTIMER_BOTTOM_H;
}

void CNewUIEventTimer::SetBoard(const EVENT_ROW* rows, int count)
{
    if (count < 0) count = 0;
    if (count > MAX_EVENTS) count = MAX_EVENTS;
    m_iRowCount = count;
    for (int i = 0; i < count; ++i)
        m_Rows[i] = rows[i];
    m_dwLastUpdateTick = GetTickCount();
}

bool CNewUIEventTimer::UpdateMouseEvent()
{
    if (BtnProcess())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, EVENTTIMER_WIDTH, PanelHeight()))
        return false;

    return true;
}

bool CNewUIEventTimer::UpdateKeyEvent()
{
    return true;
}

bool CNewUIEventTimer::Update()
{
    // Keep the panel vertically centered as its height changes with the number of events
    // (the screen is 640x480 in UI space).
    int y = (480 - PanelHeight()) / 2;
    if (y < 4) y = 4;
    m_Pos.y = y;
    return true;
}

bool CNewUIEventTimer::Render()
{
    if (!IsVisible())
        return true;

    const int W = EVENTTIMER_WIDTH;
    const int H = PanelHeight();
    const int midH = H - EVENTTIMER_TOP_H - EVENTTIMER_BOTTOM_H;

    // Guild-style window frame: dark fill + ornate top/bottom caps + left/right side borders
    // (the side borders cover the fill's straight edges, so no trimming is needed).
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);
    RenderImage(CNewUIGuildInfoWindow::IMAGE_GUILDINFO_BACK, m_Pos.x, m_Pos.y, (float)W, (float)H);
    RenderImage(CNewUIGuildInfoWindow::IMAGE_GUILDINFO_TOP, m_Pos.x, m_Pos.y, (float)W, (float)EVENTTIMER_TOP_H);
    RenderImage(CNewUIGuildInfoWindow::IMAGE_GUILDINFO_LEFT, m_Pos.x, m_Pos.y + EVENTTIMER_TOP_H, (float)EVENTTIMER_SIDE_W, (float)midH);
    RenderImage(CNewUIGuildInfoWindow::IMAGE_GUILDINFO_RIGHT, m_Pos.x + W - EVENTTIMER_SIDE_W, m_Pos.y + EVENTTIMER_TOP_H, (float)EVENTTIMER_SIDE_W, (float)midH);
    RenderImage(CNewUIGuildInfoWindow::IMAGE_GUILDINFO_BOTTOM, m_Pos.x, m_Pos.y + H - EVENTTIMER_BOTTOM_H, (float)W, (float)EVENTTIMER_BOTTOM_H);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);

    // title (inside the top bar)
    g_pRenderText->SetTextColor(255, 220, 120, 255);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 36, L"Events", W, 0, RT3_SORT_CENTER);

    const DWORD elapsedSec = (GetTickCount() - m_dwLastUpdateTick) / 1000;
    int y = m_Pos.y + EVENTTIMER_TOP_H;
    const int contentX = m_Pos.x + EVENTTIMER_SIDE_W + EVENTTIMER_PAD;
    const int contentW = W - (2 * (EVENTTIMER_SIDE_W + EVENTTIMER_PAD));

    if (m_iRowCount == 0)
    {
        g_pRenderText->SetTextColor(180, 180, 180, 255);
        g_pRenderText->RenderText(m_Pos.x, y, L"No events scheduled", W, 0, RT3_SORT_CENTER);
    }

    wchar_t szName[80] = {};
    wchar_t szVal[32] = {};
    for (int i = 0; i < m_iRowCount; ++i)
    {
        const EVENT_ROW& r = m_Rows[i];

        // name (with alive/player count in parentheses when active), e.g. "Golden - Tantallos (2)"
        if (r.active && r.extra > 0)
            mu_swprintf(szName, L"%ls (%d)", EventName(r.type), (int)r.extra);
        else
            mu_swprintf(szName, L"%ls", EventName(r.type));

        // value: "Active" when running, otherwise the live H:MM:SS countdown (ticks down locally)
        if (r.active)
        {
            mu_swprintf(szVal, L"Active");
        }
        else
        {
            int total = (int)r.seconds - (int)elapsedSec;
            if (total < 0) total = 0;
            mu_swprintf(szVal, L"%d:%02d:%02d", total / 3600, (total % 3600) / 60, total % 60);
        }

        // event name (left)
        g_pRenderText->SetTextColor(235, 235, 235, 255);
        g_pRenderText->RenderText(contentX, y, szName, contentW, 0, RT3_SORT_LEFT);

        // value (right) - green when active
        if (r.active)
            g_pRenderText->SetTextColor(120, 235, 130, 255);
        else
            g_pRenderText->SetTextColor(235, 235, 235, 255);
        g_pRenderText->RenderText(contentX, y, szVal, contentW, 0, RT3_SORT_RIGHT);

        y += EVENTTIMER_ROW_HEIGHT;
    }

    return true;
}

bool CNewUIEventTimer::BtnProcess()
{
    // The close "X" is baked into the top frame texture at the top-right; wire a click region over it.
    const int cw = 28, ch = 28;
    const int cx = m_Pos.x + EVENTTIMER_WIDTH - cw - 3;
    const int cy = m_Pos.y + 3;
    if (CheckMouseIn(cx, cy, cw, ch) && SEASON3B::IsRelease(VK_LBUTTON))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_EVENTTIMER);
        PlayBuffer(SOUND_CLICK01);
        return true;
    }
    return false;
}

float CNewUIEventTimer::GetLayerDepth()
{
    return 1.2f;
}

void CNewUIEventTimer::OpenningProcess()
{
}

void CNewUIEventTimer::ClosingProcess()
{
}
