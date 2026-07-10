// NewUIEventTimer.h: the H-key event timer board (Blood Castle / Devil Square / Chaos Castle).
//////////////////////////////////////////////////////////////////////

#ifndef _NEWUIEVENTTIMER_H_
#define _NEWUIEVENTTIMER_H_

#pragma once

#include "UI/NewUI/NewUIBase.h"
#include "UI/NewUI/NewUIManager.h"

namespace SEASON3B
{
    class CNewUIEventTimer : public CNewUIObj
    {
    public:
        enum { MAX_EVENTS = 16 };

        // One event row, filled from the server's 0x97 event-board packet.
        struct EVENT_ROW
        {
            BYTE  type;     // 0 = Blood Castle, 1 = Devil Square, 2 = Chaos Castle
            bool  active;   // true = currently open/running
            WORD  seconds;  // seconds until next start (when not active)
            WORD  extra;    // player count (when active)
        };

    private:
        enum EVENTTIMER_SIZE
        {
            EVENTTIMER_WIDTH = 190,   // match the guild window width so the frame textures align
            EVENTTIMER_TOP_H = 64,    // guild-frame top border height
            EVENTTIMER_BOTTOM_H = 45, // guild-frame bottom border height
            EVENTTIMER_SIDE_W = 21,   // guild-frame side border width
            EVENTTIMER_ROW_HEIGHT = 18,
            EVENTTIMER_PAD = 6,
        };

        CNewUIManager* m_pNewUIMng;
        POINT          m_Pos;

        EVENT_ROW      m_Rows[MAX_EVENTS];
        int            m_iRowCount;
        DWORD          m_dwLastUpdateTick;   // GetTickCount64 low bits when the board was received

    public:
        CNewUIEventTimer();
        virtual ~CNewUIEventTimer();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();

        void OpenningProcess();
        void ClosingProcess();

        // Called by the packet handler (ReceiveEventBoard) with the latest board.
        void SetBoard(const EVENT_ROW* rows, int count);

    private:
        const wchar_t* EventName(BYTE type);
        int PanelHeight() const;
    };
}

extern SEASON3B::CNewUIEventTimer* g_pEventTimer;

#endif // _NEWUIEVENTTIMER_H_
