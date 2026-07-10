//*****************************************************************************
// File: LoginWin.h
//*****************************************************************************
#pragma once

#include "UI/Widgets/Win.h"

#include "UI/Widgets/Button.h"
#include "Data/GameConfig/GameConfigConstants.h"  // kMaxSavedAccounts

class CUITextInputBox;

class CLoginWin : public CWin
{
protected:
    CSprite		m_asprInputBox[2];
    CButton		m_aBtn[2];
    // One quick-login button per launcher-saved account (captioned with the
    // username). Only m_accountBtnCount of them are created/shown.
    CButton     m_accountBtns[kMaxSavedAccounts];
    int         m_accountBtnCount = 0;
    CUITextInputBox* m_pUsernameInputBox, * m_pPasswordInputBox;

public:
    CLoginWin();
    virtual ~CLoginWin();
    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);
    bool CursorInWin(int nArea);

    void ConnectConnectionServer();

    CUITextInputBox* GetUsernameInputBox() const { return m_pUsernameInputBox; }
    CUITextInputBox* GetPasswordInputBox() const { return m_pPasswordInputBox; }

private:
    int FirstLoad = 0;

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);
    void UpdateWhileShow(double dDeltaTick);
    void RenderControls();
    void RequestLogin();
    void QuickLogin(int index);   // log in with a saved account (button press)
    void DoLogin();               // shared: validate m_Username/m_Password and send
    void CancelLogin();
};
